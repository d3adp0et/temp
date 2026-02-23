/* timestomp-lite.c - File timestamp modification using direct syscalls */
#include "minc.h"
#include "syscall_utils.h"

/* Helper to print error message and return error code */
static int error_exit(const char *msg) {
    sys_write(2, msg, strlen(msg));
    sys_write(2, "\n", 1);
    return 1;
}

/* Helper to check if two strings are equal */
static int streq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        const char *usage =
            "Usage: timestomp-lite <file> --at <epoch> --mt <epoch>\n"
            "       timestomp-lite <file> --copy-from <otherfile>\n";
        sys_write(2, usage, strlen(usage));
        return 1;
    }

    const char *filename = argv[1];
    struct timespec times[2];
    
    /* Initialize to UTIME_OMIT (don't change) - special value */
    /* UTIME_OMIT = ((1l << 30) - 2l) */
    times[0].tv_sec = 0;
    times[0].tv_nsec = ((1L << 30) - 2L);  /* UTIME_OMIT for atime */
    times[1].tv_sec = 0;
    times[1].tv_nsec = ((1L << 30) - 2L);  /* UTIME_OMIT for mtime */

    /* Check for --copy-from mode */
    if (argc >= 4 && streq(argv[2], "--copy-from")) {
        const char *srcfile = argv[3];
        
        /* Open source file to get its timestamps */
        int srcfd = sys_openat(AT_FDCWD, srcfile, O_RDONLY, 0);
        if (srcfd < 0) {
            return error_exit("Error: cannot open source file");
        }
        
        /* Get source file stats */
        struct stat st;
        if (sys_fstat(srcfd, &st) < 0) {
            sys_close(srcfd);
            return error_exit("Error: cannot stat source file");
        }
        sys_close(srcfd);
        
        /* Copy timestamps from stat structure */
        times[0].tv_sec = st.st_atim.tv_sec;
        times[0].tv_nsec = st.st_atim.tv_nsec;
        times[1].tv_sec = st.st_mtim.tv_sec;
        times[1].tv_nsec = st.st_mtim.tv_nsec;
    } 
    else {
        /* Parse --at and --mt arguments */
        int got_at = 0, got_mt = 0;
        
        for (int i = 2; i < argc - 1; i += 2) {
            if (streq(argv[i], "--at")) {
                times[0].tv_sec = atol(argv[i + 1]);
                times[0].tv_nsec = 0;
                got_at = 1;
            } 
            else if (streq(argv[i], "--mt")) {
                times[1].tv_sec = atol(argv[i + 1]);
                times[1].tv_nsec = 0;
                got_mt = 1;
            }
            else {
                return error_exit("Error: unknown option");
            }
        }
        
        /* Need at least one timestamp specified */
        if (!got_at && !got_mt) {
            return error_exit("Error: must specify --at, --mt, or --copy-from");
        }
    }

    /* Verify target file exists by trying to open it */
    int fd = sys_openat(AT_FDCWD, filename, O_RDONLY, 0);
    if (fd < 0) {
        return error_exit("Error: cannot open target file");
    }
    sys_close(fd);

    /* Apply timestamps using utimensat */
    /* utimensat(dirfd, pathname, times, flags) */
    /* flags = 0 means follow symlinks */
    int ret = sys_utimensat(AT_FDCWD, filename, times, 0);
    if (ret < 0) {
        return error_exit("Error: failed to set timestamps");
    }

    return 0;
}
