/* pmaps.c - Print /proc/self/maps using direct syscalls */
#include "syscall_utils.h"
#include "minc.h"

#define CHUNK_SIZE 4096

int main(void) {
  const char *maps_path = "/proc/self/maps";

  int fd = sys_openat(AT_FDCWD, maps_path, O_RDONLY, 0);
  if (fd < 0){
    char *openat_error = "Error while opening file";
    sys_write(2, openat_error, strlen(openat_error));
    sys_exit(1);
  }

  char buf[CHUNK_SIZE];
  ssize_t procread = sys_read(fd, buf, CHUNK_SIZE);



  if (procread > 0){
    sys_write(1, buf, procread);
  } else if (procread == 0) {
    char *eof_error = "Error: End of file reached";
    sys_write(2, eof_error, strlen(eof_error));
  } else {
    char *read_error = "Error while reading file"; 
    sys_write(2, read_error, strlen(read_error));
    sys_exit(1);
  } 

  return 0;
}
