/* binding_hook.c - LD_PRELOAD passive shim for protected files */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* --- PART 1: FUNCTION SIGNATURES --- */
typedef int (*orig_open_type)(const char *pathname, int flags, mode_t mode);
typedef int (*orig_open64_type)(const char *pathname, int flags, mode_t mode);
typedef int (*orig_openat_type)(int dirfd, const char *pathname, int flags,
                                mode_t mode);
typedef int (*orig_unlink_type)(const char *pathname);
typedef int (*orig_unlinkat_type)(int dirfd, const char *pathname, int flags);

/* Global pointers to real functions */
static orig_open_type real_open = NULL;
static orig_open64_type real_open64 = NULL;
static orig_openat_type real_openat = NULL;
static orig_unlink_type real_unlink = NULL;
static orig_unlinkat_type real_unlinkat = NULL;

/* --- PART 2: THE CONSTRUCTOR (INSTALLER) --- */
__attribute__((constructor)) void install_hooks(void) {
  // save the real virtual address
  real_open = (orig_open_type)dlsym(RTLD_NEXT, "open");
  real_open64 = (orig_open64_type)dlsym(RTLD_NEXT, "open64");
  real_openat = (orig_openat_type)dlsym(RTLD_NEXT, "openat");
  real_unlink = (orig_unlink_type)dlsym(RTLD_NEXT, "unlink");
  real_unlinkat = (orig_unlinkat_type)dlsym(RTLD_NEXT, "unlinkat");
}

/* --- PART 3: HELPER LOGIC --- */
#define PROTECTED_DIR "/tmp/protected"
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

/* --- PART 4: THE HOOKS (DETOURS) --- */

int open(const char *pathname, int flags, ...) {
  int result = -1;
  va_list args;
  mode_t mode = 0;

  int blocked = is_protected(pathname);

  if (blocked == 1){
    errno = EACCES;
  } else {
    if (flags & O_CREAT){
      va_start(args, flags);
      mode = va_arg(args, mode_t);
      va_end(args);
    }

    result = real_open(pathname, flags, mode);

  }
  return result;
}

int open64(const char *pathname, int flags, ...) {
  int result = -1;
  //define mode in case it is provided
  va_list args;
  mode_t mode = 0;

  int blocked = is_protected(pathname);

  if (blocked == 1){
    errno = EACCES;
  } else {
    if (flags & O_CREAT){
      va_start(args, flags);
      mode = va_arg(args, mode_t);
      va_end(args);
    } 

    result = real_open64(pathname, flags, mode);
  }
  return result;
}

int openat(int dirfd, const char *pathname, int flags, ...) {
  int result = -1;
  
  va_list args;
  mode_t mode = 0;

  int blocked = is_protected(pathname);

  if (blocked == 1){
    errno = EACCES;
  } else {
    if (flags & O_CREAT){
      va_start(args, flags);
      mode = va_arg(args, mode_t);
      va_end(args);
    }

    result = real_openat(dirfd, pathname, flags, mode);
  }
  return result;
}

int unlink(const char *pathname) {
  int result = -1;

  int blocked = is_protected(pathname);
  
  if (blocked == 1){
    errno = EACCES;
  } else{
    result = real_unlink(pathname);
  }

  return result;
}

int unlinkat(int dirfd, const char *pathname, int flags) {
  int result = -1;

  int blocked = is_protected(pathname);
  
  if (blocked == 1){
    errno = EACCES;
  } else {
    result = real_unlinkat(dirfd, pathname, flags);
  }
  return result;
}
