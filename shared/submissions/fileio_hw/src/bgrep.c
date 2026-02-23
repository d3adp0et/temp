/* bgrep.c - Binary grep using direct syscalls */
#include "syscall_utils.h"
#include "minc.h"

#define CHUNK_SIZE 4096
#define MAX_PATTERN_SIZE 256

/* Search for pattern in buffer, report matches adjusted by base_offset */
static int search_buffer(const uint8_t *buf, size_t buf_len,
                         const uint8_t *pattern, size_t pat_len,
                         long base_offset, int *found_any) {
    if (buf_len < pat_len) return 0;
    
    size_t limit = buf_len - pat_len + 1;
    for (size_t i = 0; i < limit; i++) {
        if (memcmp(buf + i, pattern, pat_len) == 0) {
            long off = base_offset + (long)i;
            sys_write(1, "Found at offset: ", 17);
            write_num(1, off);
            sys_write(1, " (0x", 4);
            write_hex(1, (unsigned long)off);
            sys_write(1, ")\n", 2);
            *found_any = 1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        sys_write(2, "Usage: bgrep <file> <hexpattern>\n", 33);
        return 1;
    }

    const char *filename = argv[1];
    const char *hex_pattern = argv[2];

    /* Check for empty pattern BEFORE parsing */
    if (hex_pattern[0] == '\0') {
        sys_write(2, "Error: empty hex pattern\n", 25);
        return 1;
    }

    /* Parse hex pattern */
    uint8_t pattern[MAX_PATTERN_SIZE];
    int pat_len = parse_hex(hex_pattern, pattern, MAX_PATTERN_SIZE);
    
    if (pat_len <= 0) {
        sys_write(2, "Error: invalid hex pattern\n", 27);
        return 1;
    }

    /* Open file */
    int fd = sys_openat(AT_FDCWD, filename, O_RDONLY, 0);
    if (fd < 0) {
        sys_write(2, "Error: Cannot open file\n", 24);
        return 1;
    }

    int found_any = 0;
    uint8_t buf[CHUNK_SIZE + MAX_PATTERN_SIZE];
    size_t overlap = (size_t)(pat_len - 1);
    size_t carry = 0;  /* bytes carried over from previous chunk */
    long file_offset = 0;  /* current position in file for pread */
    long search_base = 0;  /* base offset for reporting matches */

    while (1) {
        /* Read next chunk after any carried-over bytes */
        ssize_t nread = sys_pread64(fd, buf + carry, CHUNK_SIZE, file_offset);
        if (nread < 0) {
            sys_write(2, "Error: Read failed\n", 19);
            sys_close(fd);
            return 1;
        }
        
        if (nread == 0) {
            /* EOF - search any remaining carried bytes */
            if (carry > 0 && carry >= (size_t)pat_len) {
                search_buffer(buf, carry, pattern, (size_t)pat_len, search_base, &found_any);
            }
            break;
        }

        size_t total = carry + (size_t)nread;
        
        /* Search the buffer */
        search_buffer(buf, total, pattern, (size_t)pat_len, search_base, &found_any);

        /* Prepare overlap for next iteration */
        if (total >= overlap) {
            /* Copy last (pat_len-1) bytes to start of buffer */
            memcpy(buf, buf + total - overlap, overlap);
            carry = overlap;
            search_base += (long)(total - overlap);
        } else {
            /* Very small read, keep all of it */
            carry = total;
        }
        
        file_offset += nread;
    }

    sys_close(fd);
    return found_any ? 0 : 1;
}
