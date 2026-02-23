/* djb2sum.c - DJB2 hash computation using direct syscalls */
#include "syscall_utils.h"
#include "minc.h"

    // Your solution here!

int main(int argc, char **argv) {
  if (argc != 2) {
    const char *usage = "Usage: djb2sum <file>\n";
    sys_write(2, usage, strlen(usage));
    return 1;
  }

  const char *filename = argv[1];

  //open file 
  int fd = sys_openat(AT_FDCWD, filename, O_RDONLY, 0);
  if (fd < 0){
    const char *open_err = "Error: File could not be opened\n";
    sys_write(2, open_err, strlen(open_err));
    sys_exit(1);
  }

  unsigned long hash = 5381;

  //fstat to get file metadata -> mainly size
  struct stat statbuf;
  if(sys_fstat(fd, &statbuf) < 0){
    const char *fstat_err = "Error: Cannot stat file\n";
    sys_write(2, fstat_err, strlen(fstat_err));
    sys_close(fd);
    sys_exit(1);
  }

  size_t bsize = statbuf.st_size;
  if(bsize == 0){
    write_hex(1, hash);
    sys_write(1, " ", 1);
    sys_write(1, filename, strlen(filename));
    sys_write(1, "\n", 1);
    sys_close(fd);
    return 0;
  }

  //mapping the file into memory
  char *mapped = sys_mmap(NULL, bsize, PROT_READ, MAP_PRIVATE, fd, 0); 
  if (mapped == (void *)-1) {
    const char *map_err = "Error: File could not be mapped to buffer\n";
    sys_write(2, map_err, strlen(map_err));
    sys_close(fd);
    sys_exit(1);
  }

  //DJB2 Hashing Logic
  size_t parse = bsize;
  char *m = mapped;

  while(parse--){
    //the actual logic
    hash = ((hash << 5) + hash) + (unsigned char)*m; /* hash * 33 + mapped */
    m++;
  }

  //Printing Output
  write_hex(1, hash);
  sys_write(1, " ", 1);
  sys_write(1, filename, strlen(filename));
  sys_write(1, "\n", 1);


  sys_munmap(mapped, statbuf.st_size);
  sys_close(fd);

  return 0;
}
