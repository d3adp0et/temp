/* sed-lite.c - Simple find/replace using direct syscalls */
#include "syscall_utils.h"
#include "minc.h"

#define OUTPUT_BUFFER_SIZE 8192

int main(int argc, char **argv) {
  if (argc != 4) {
    const char *usage = "Usage: sed-lite <file> <search> <replace>\n";
    sys_write(2, usage, strlen(usage));
    return 1;
  }

  const char *filename = argv[1];
  const char *search = argv[2];
  const char *replace = argv[3];

  //Empty search string check
  if (strlen(search) == 0){
    const char *emptysearch = "Nothing to search!\n";
    sys_write(1, emptysearch, strlen(emptysearch));
    return 1;
  }

  //open file 
  int fd = sys_openat(AT_FDCWD, filename, O_RDONLY, 0);
  if (fd < 0){
    const char *open_err = "Error: File could not be opened\n";
    sys_write(2, open_err, strlen(open_err));
    sys_exit(1);
  }

  //fstat to get file metadata
  struct stat statbuf;
  if(sys_fstat(fd, &statbuf) < 0){
    const char *fstat_err = "Error: Cannot stat file\n";
    sys_write(2, fstat_err, strlen(fstat_err));
    sys_close(fd);
    sys_exit(1);
  }

  long bytes = statbuf.st_size;
  if(bytes == 0){
    sys_close(fd);
    return 0;
  }

  //memory mapping the file
  char *mapped = sys_mmap(NULL, bytes, PROT_READ, MAP_PRIVATE, fd, 0); 
  if (mapped == (void *)-1) {
    const char *map_err = "Error: File could not be mapped to buffer\n";
    sys_write(2, map_err, strlen(map_err));
    sys_close(fd);
    sys_exit(1);
  }

  //mapping output buffer 
  char *outbuffer = sys_mmap(NULL, OUTPUT_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (outbuffer == (void *)-1){
    const char *map_err = "Error: mmap failed";
    sys_write(2, map_err, strlen(map_err));
    sys_exit(1);
  }

  //find and replace logic -- reusing idea from log_s.c 
  size_t opos = 0, fsizecpyed = 0;
  char *mfile = mapped;
  size_t rplc_size = strlen(replace);

  while (mfile){
    char *fpos = strstr(mfile, search);
    if (fpos != NULL && opos < OUTPUT_BUFFER_SIZE - 1){
      //copies everything before searched string to buffer 
      fsizecpyed = fpos - mfile;
      memcpy(outbuffer + opos, mfile, fsizecpyed);
      opos += fsizecpyed;

      //copies the replace string to buffer 
      memcpy(outbuffer + opos, replace, rplc_size);
      opos += rplc_size;

      //new fmt starts after %s from this iteration 
      mfile += fsizecpyed + strlen(search);
    } else{
      size_t remaining = (mapped + bytes) - mfile;
      memcpy(outbuffer + opos, mfile, remaining);
      opos += remaining; 
      break;
    }
  }

  sys_write(1, outbuffer, opos);
  sys_write(fd, "\n", 1);

  sys_munmap(outbuffer, OUTPUT_BUFFER_SIZE);
  sys_munmap(mapped, bytes);
  sys_close(fd);
  return 0;
}
