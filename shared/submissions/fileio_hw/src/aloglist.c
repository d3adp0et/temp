/* aloglist.c - List entries from structured log using direct syscalls */
#include "syscall_utils.h"
#include "minc.h"

#pragma pack(1)
struct log_entry {
    uint64_t id;
    uint64_t timestamp;
    uint16_t msglen;
    uint8_t padding[6];
    char message[240];
};

int main(int argc, char **argv) {
  if (argc != 2) {
    const char *usage = "Usage: aloglist <logfile>\n";
    sys_write(2, usage, strlen(usage));
    return 1;
  }

  const char *logfile = argv[1];

  int fd = sys_openat(AT_FDCWD, logfile, O_RDONLY, 0);
  if (fd < 0){
    char *openat_error = "Error while opening file\n";
    sys_write(2, openat_error, strlen(openat_error));
    return 1;
  }

  while(1){
    struct log_entry output;
    size_t output_len = sizeof(output);
    ssize_t read = sys_read(fd, &output, output_len);
    if (read < 0){
      const char *read_err = "Error: Read failed!\n";
      sys_write(2, read_err, strlen(read_err));
      sys_close(fd);
      return 1;
    } else if (read == 0){
      const char *eof_note = "EOF reached\n>>> Exiting\n";
      sys_write(1, eof_note, strlen(eof_note));
      sys_close(fd);
      return 0;
    } else if (read != output_len){
      const char *partial_read = "Error: Partial read occured\n";
      sys_write(2, partial_read, strlen(partial_read));
      sys_close(fd);
      return 1;
    }

    //Print entries
    write_num(1, output.id);
    sys_write(1, " : ", 3);
    write_num(1, output.timestamp);
    sys_write(1, " : ", 3);
    sys_write(1, output.message, output.msglen);
    sys_write(1, "\n", 1);
  }


  sys_close(fd);
  return 0;
}
