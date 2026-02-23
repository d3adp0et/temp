/* runrwx.c - Execute raw machine code using direct syscalls */
#include "syscall_utils.h"
#include "minc.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        const char *usage = "Usage: runrwx <codefile> [args...]\n";
        sys_write(2, usage, strlen(usage));
        return 1;
    }

    const char *codefile = argv[1];

    /* Display status message before execution */
    const char *msg1 = "Loading code from: ";
    sys_write(1, msg1, strlen(msg1));
    sys_write(1, codefile, strlen(codefile));
    sys_write(1, "\n", 1);

    /* Open the code file */
    int fd = sys_openat(AT_FDCWD, codefile, O_RDONLY, 0);
    if (fd < 0) {
        const char *err = "Error: Failed to open code file\n";
        sys_write(2, err, strlen(err));
        return 1;
    }

    /* Get file size using lseek */
    off_t file_size = sys_lseek(fd, 0, SEEK_END);
    if (file_size < 0) {
        const char *err = "Error: Failed to get file size\n";
        sys_write(2, err, strlen(err));
        sys_close(fd);
        return 1;
    }

    /* Seek back to beginning */
    if (sys_lseek(fd, 0, SEEK_SET) < 0) {
        const char *err = "Error: Failed to seek to beginning\n";
        sys_write(2, err, strlen(err));
        sys_close(fd);
        return 1;
    }

    /* Display file size */
    const char *msg2 = "Code size: ";
    sys_write(1, msg2, strlen(msg2));
    write_num(1, file_size);
    sys_write(1, " bytes\n", 7);

    /* Map memory with RWX permissions */
    void *code_mem = sys_mmap(
        NULL,
        (size_t)file_size,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    if (code_mem == (void *)-1 || code_mem == NULL) {
        const char *err = "Error: Failed to map memory\n";
        sys_write(2, err, strlen(err));
        sys_close(fd);
        return 1;
    }

    /* Read code into mapped memory */
    ssize_t bytes_read = sys_read(fd, code_mem, (size_t)file_size);
    if (bytes_read != file_size) {
        const char *err = "Error: Failed to read code file\n";
        sys_write(2, err, strlen(err));
        sys_munmap(code_mem, (size_t)file_size);
        sys_close(fd);
        return 1;
    }

    /* Close the file */
    sys_close(fd);

    /* Display execution message */
    const char *msg3 = "Executing code...\n";
    sys_write(1, msg3, strlen(msg3));

    /* Cast memory to function pointer and execute */
    /* Function signature: int func(int argc, char **argv) */
    /* Pass remaining arguments (argv+1 points to codefile, argv+2 onwards are args for the code) */
    typedef int (*code_func_t)(int, char **);
    code_func_t code_func = (code_func_t)code_mem;

    /* Execute with argc-1 and argv+1 (skip program name, pass codefile and remaining args) */
    int result = code_func(argc - 1, argv + 1);

    /* Display status message after execution */
    const char *msg4 = "Execution complete. Return value: ";
    sys_write(1, msg4, strlen(msg4));
    write_num(1, result);
    sys_write(1, "\n", 1);

    /* Cleanup: unmap the memory */
    sys_munmap(code_mem, (size_t)file_size);

    return result;
}
