/* bypass_syscall.c - Direct syscalls to write protected file */
#include "minc.h"
#include "syscall_utils.h"

#define TARGET_FILE "/tmp/protected/hacked.txt"
#define PAYLOAD "1337h4x0r"

static void write_msg(int fd, const char *msg) {
    sys_write(fd, msg, strlen(msg));
}

static int write_payload(void) {
  int status = 1;
  int fd = sys_openat(AT_FDCWD, TARGET_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  
  if (fd < 0) return status;
  write_msg(fd, PAYLOAD);

  sys_close(fd);
  status = 0;
  return status;
}

int main(int argc, char **argv, char **envp) {
    (void)argc;
    (void)argv;
    (void)envp;

    return write_payload();
}
