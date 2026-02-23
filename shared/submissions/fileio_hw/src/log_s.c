/* log_s.c - Lazy printf supporting only %s using direct syscalls */
#include "syscall_utils.h"
#include "minc.h"

#define BUFFER_SIZE 4096



int main(int argc, char **argv) {
  if (argc != 4) {
    const char *usage = "Usage: log_s <fd> \"<fmt>\" \"<arg>\"\n";
    sys_write(2, usage, strlen(usage));
    return 1;
  }

  const char *fd_str = argv[1];
  const char *fmt = argv[2];
  const char *arg = argv[3];

  //string file descriptor convertied to int
  int fd = atoi(fd_str);

  //mapping output buffer 
  char *outbuffer = sys_mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (outbuffer == (void *)-1){
    const char *map_err = "Error: mmap failed";
    sys_write(2, map_err, strlen(map_err));
    sys_exit(1);
  }

  //main logic
  size_t opos = 0, fmtcpyed = 0;
  size_t arg_size = strlen(arg);

  while (fmt){
    char *fpos = strstr(fmt, "%s");
    if (fpos != NULL && opos < BUFFER_SIZE - 1){
      //copies everything before %s to buffer 
      fmtcpyed = fpos - fmt;
      memcpy(outbuffer + opos, fmt, fmtcpyed);
      opos += fmtcpyed;

      //copies the arg to buffer 
      memcpy(outbuffer + opos, arg, arg_size);
      opos += arg_size;

      //new fmt starts after %s from this iteration 
      fmt += fmtcpyed + 2;
    } else{
      size_t remaining = strlen(fmt);
      memcpy(outbuffer + opos, fmt, remaining);
      opos += remaining; 
      break;
    }
  }

  sys_write(fd, outbuffer, opos);
  sys_write(fd, "\n", 1);

  sys_munmap(outbuffer, BUFFER_SIZE);
  return 0;
}
