/* alogappend.c - Append entries to structured log using direct syscalls */
#include "minc.h"
#include "syscall_utils.h"

#pragma pack(1)
struct log_entry {
  uint64_t id;
  uint64_t timestamp;
  uint16_t msglen;
  uint8_t padding[6];
  char message[240];
};

//  Simulare timestamp
static uint64_t get_current_timestamp(void) {
  static uint64_t counter = 1000000000; // Start from a large value
  return counter++;
}

int main(int argc, char **argv) {
  if (argc != 4) {
    const char *usage = "Usage: alogappend <logfile> <id> <message>\n";
    sys_write(2, usage, strlen(usage));
    return 1;
  }

  const char *logfile = argv[1];
  const char *id_str = argv[2];
  const char *message = argv[3];

  //id_str to integer
  const long id = atol(id_str);



  //Intialise log_entry struct
  struct log_entry input_entry;
  size_t log_size = sizeof(struct log_entry);
  memset(&input_entry, 0, log_size);

  //Set log_entry struct 
  input_entry.id = id;
  input_entry.timestamp = get_current_timestamp();
  size_t argv3_len = strlen(message); 
  input_entry.msglen = argv3_len;

  if (input_entry.msglen > 240 - 1){    //check msg length and handle overflow
    const char *overflow = "Error: Message is too big!\n";
    sys_write(2, overflow, strlen(overflow));
  }
  
  memcpy(input_entry.message, message, argv3_len);



  //open and write/append to file
  int fd = sys_openat(AT_FDCWD, logfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if (fd < 0){
    char *openat_error = "Error: Open failed!\n";
    sys_write(2, openat_error, strlen(openat_error));
    return 1;
  }

  ssize_t written = sys_write(fd, &input_entry, log_size);
  if (written != log_size){
    char *write_error = "Error: Write failed\n"; 
    sys_write(2, write_error, strlen(write_error));
    sys_close(fd);
    return 1;
  }



  sys_close(fd);
  return 0;
}
