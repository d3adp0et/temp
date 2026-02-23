/* plt_hook.c - PLT stub patching hook for protected files */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <link.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define TARGET_OPENAT "openat"
#define TARGET_UNLINKAT "unlinkat"
#define PROTECTED_DIR "/tmp/protected"


/* --- HELPER LOGIC --- */
static int is_protected(const char *path) {
  int blocked = 0;
  size_t n = strlen(PROTECTED_DIR);
  if (strncmp(path, PROTECTED_DIR, n) == 0){
    if(path[n] == '\0' || path[n] == '/'){
      blocked = 1;
    }
  }
  return blocked;
}

/* Function pointer types for the original functions */
typedef int (*orig_openat_type)(int dirfd, const char *pathname, int flags,
                                mode_t mode);
typedef int (*orig_unlinkat_type)(int dirfd, const char *pathname, int flags);
static orig_openat_type real_openat = NULL;
static orig_unlinkat_type real_unlinkat = NULL;

int hacked_openat(int dirfd, const char *pathname, int flags, mode_t mode) {
  int result = -1;
  
  int blocked = is_protected(pathname);

  if (blocked == 1){
    errno = EACCES;
  } else {
    result = real_openat(dirfd, pathname, flags, mode);
  }
  return result;
}

int hacked_unlinkat(int dirfd, const char *pathname, int flags) {
  int result = -1;
  
  int blocked = is_protected(pathname);
  
  if (blocked == 1){
    errno = EACCES;
  } else {
    result = real_unlinkat(dirfd, pathname, flags);
  }

  return result;
}

#define PAGE_START(x) ((x) & ~(getpagesize() - 1))

static int read_exact(int fd, void *buf, size_t size, off_t offset) {
  size_t done = 0;
  while (done < size) {
    ssize_t n = pread(fd, (char *)buf + done, size - done, offset + done);
    if (n <= 0) {
      return -1;
    }
    done += (size_t)n;
  }
  return 0;
}

static int find_plt_section(const char *path, Elf64_Addr base_addr,
                            Elf64_Addr *plt_addr, size_t *plt_size,
                            int *plt_has_plt0) {
  int status = -1;
  
  int fd = open(path, O_RDONLY);
  if (fd < 0) {perror("Error opening file");}

  Elf64_Ehdr header;
  /

  return status;
}

static uint32_t encode_ldr_literal_x16(void) {
    // Your solution here!
}

static uint32_t encode_br_x16(void) {
    // Your solution here!
}

static int patch_plt_entry(void *entry, void *target) {
  int status = -1;
    // Your solution here!
  return status;
}

static int callback(struct dl_phdr_info *info, size_t size, void *data) {
  int ret = 0;
  if(strcmp(info->dlpi_name, "") == 0){
    ElfW(Addr) base_addr = info->dlpi_addr;
    const char *path = "/proc/self/exe";
    Elf64_Addr  plt_addr;
    size_t plt_size;
    int plt_has_plt0;

    find_plt_section(path, base_addr, &plt_addr, &plt_size, &plt_has_plt0);

    //TODO: complete this 
  }
  return ret;
}

__attribute__((constructor)) void install_hooks(void) {
  real_openat = (orig_openat_type)dlsym(RTLD_NEXT, "openat");
  real_unlinkat = (orig_unlinkat_type)dlsym(RTLD_NEXT, "unlinkat");

  dl_iterate_phdr(callback, NULL);
}




