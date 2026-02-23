/* wc-mmap.c - Word count using memory mapping and direct syscalls */
#include "syscall_utils.h"
#include "minc.h"

    // Your solution here!

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        const char *usage = "Usage: wc-mmap [-l|-c] <file>\n"
                           "  -l  Count lines only\n"
                           "  -c  Count bytes only\n"
                           "  (no flag = count lines, words, and bytes)\n";
        sys_write(2, usage, strlen(usage));
        return 1;
    }

    // Parse arguments
    int lines_only = 0;
    int bytes_only = 0;
    const char *filename;

    if (argc == 3) {
        // Flag provided
        if (strcmp(argv[1], "-l") == 0) {
            lines_only = 1;
        } else if (strcmp(argv[1], "-c") == 0) {
            bytes_only = 1;
        } else {
            const char *err = "Error: Invalid flag. Use -l or -c\n";
            sys_write(2, err, strlen(err));
            return 1;
        }
        filename = argv[2];
    } else {
        // No flag, default behavior
        filename = argv[1];
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
    if(bytes_only){
        sys_write(1, "0", 1);
      } else if (lines_only) {
        sys_write(1, "0", 1);
      } else {
        sys_write(1, "0 0 0", 5);
      }

    sys_write(1, " ", 1);
    sys_write(1, filename, strlen(filename));
    sys_write(1, "\n", 1);
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


  //Counting Logic
  //bytes is already implemented in fstat
  char *m = mapped;
  long parse = bytes;
  long lines = 0, words = 0;
  int whitespace = 1;

  while(parse--){ 
  //count lines 
    if(*m == '\n')            //counting new-line character to count lines
        lines++;
    
  //count words 
    if(*m == ' ' || *m == '\n' || *m == '\t' || *m == '\v' || *m == '\f' || *m == '\r'){
      whitespace = 1;
    } else {
      if(whitespace == 1){      //counting transition from whitespace to a word 
        words++;
        whitespace = 0;
      }
    }

    m++;
  }


  //Printing Logic
  if(bytes_only){
    write_num(1, bytes);
  } else if (lines_only) {
    write_num(1, lines);
  } else {
    write_num(1, lines);
    sys_write(1, " ", 1);
    write_num(1, words);
    sys_write(1, " ", 1);
    write_num(1, bytes);
  }
  sys_write(1, " ", 1);
  sys_write(1, filename, strlen(filename));
  sys_write(1, "\n", 1);


  sys_munmap(mapped, statbuf.st_size);
  sys_close(fd);


  return 0;
}
