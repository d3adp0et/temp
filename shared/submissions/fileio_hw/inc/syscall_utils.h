#ifndef SYSCALL_UTILS_H
#define SYSCALL_UTILS_H

/* Feature test macros must come first */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/*
 * ASSIGNMENT: Implement these syscall wrappers for AArch64
 *
 * Your task is to:
 * 1. Look up the AArch64 syscall numbers for each syscall
 * 2. Implement the syscall0-syscall6 wrappers using inline assembly
 * 3. Implement each syscall wrapper function
 *
 * Use the pattern: svc #0 instruction with proper register setup
 * Syscall number goes in x8, arguments in x0-x5, return value in x0
 */

/* =============================================================================
 * PART 1: SYSCALL WRAPPERS (IMPLEMENT THESE)
 * =============================================================================
 */

/* TODO: Implement syscall wrappers for 0-6 arguments */
static inline long syscall0(long num) {
  register long x8 asm("x8") = num;
  register long x0 asm("x0");

  asm volatile(
      "svc #0"
      : "=r"(x0)
      : "r"(x8)
      : "memory"
  );

  return x0;
}

static inline long syscall1(long num, long arg1) {
  register long x0 asm("x0") = arg1;
  register long x8 asm("x8") = num;

  asm volatile(
      "svc #0"
      : "+r"(x0)
      : "r"(x8)
      : "memory"
  );

  return x0;
}

static inline long syscall2(long num, long arg1, long arg2) {
  register long x0 asm("x0") = arg1;
  register long x1 asm("x1") = arg2;
  register long x8 asm("x8") = num;

  asm volatile(
      "svc #0"
      : "+r"(x0)
      : "r"(x1), "r"(x8)
      : "memory"
  );

  return x0;
}

static inline long syscall3(long num, long arg1, long arg2, long arg3) {
  register long x0 asm("x0") = arg1;
  register long x1 asm("x1") = arg2;
  register long x2 asm("x2") = arg3;
  register long x8 asm("x8") = num;

  asm volatile(
      "svc #0"
      : "+r"(x0)
      : "r"(x1), "r"(x2), "r"(x8)
      : "memory"
  );

  return x0;
}

static inline long syscall4(long num, long arg1, long arg2, long arg3,
                            long arg4) {
  register long x0 asm("x0") = arg1;
  register long x1 asm("x1") = arg2;
  register long x2 asm("x2") = arg3;
  register long x3 asm("x3") = arg4;
  register long x8 asm("x8") = num;

  asm volatile(
      "svc #0"
      : "+r"(x0)
      : "r"(x1), "r"(x2), "r"(x3), "r"(x8)
      : "memory"
  );

  return x0;
}

static inline long syscall5(long num, long arg1, long arg2, long arg3,
                            long arg4, long arg5) {
  register long x0 asm("x0") = arg1;
  register long x1 asm("x1") = arg2;
  register long x2 asm("x2") = arg3;
  register long x3 asm("x3") = arg4;
  register long x4 asm("x4") = arg5;
  register long x8 asm("x8") = num;

  asm volatile(
      "svc #0"
      : "+r"(x0)
      : "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x8)
      : "memory"
  );

  return x0;
}

static inline long syscall6(long num, long arg1, long arg2, long arg3,
                            long arg4, long arg5, long arg6) {
  register long x0 asm("x0") = arg1;
  register long x1 asm("x1") = arg2;
  register long x2 asm("x2") = arg3;
  register long x3 asm("x3") = arg4;
  register long x4 asm("x4") = arg5;
  register long x5 asm("x5") = arg6;
  register long x8 asm("x8") = num;

  asm volatile(
      "svc #0"
      : "+r"(x0)
      : "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5), "r"(x8)
      : "memory"
  );

  return x0;
}

/* =============================================================================
 * PART 2: SYSCALL NUMBERS (LOOK THESE UP FOR AARCH64)
 * =============================================================================
 */

/* TODO: Look up AArch64 syscall numbers and define them here */
/* Hint: Check arch/arm64/include/asm/unistd32.h or use `ausyscall --dump` */

// File operations
#define __NR_openat 56
#define __NR_close 57
#define __NR_read 63
#define __NR_write 64
#define __NR_pread64 67
#define __NR_pwrite64 68
#define __NR3264_lseek 62
/* #define __NR_lseek __NR3264_lseek */
#define __NR3264_fstat 80
/* #define __NR_fstat __NR3264_fstat */
#define __NR3264_mmap 222
/* #define __NR_mmap __NR3264_mmap */
#define __NR_munmap 215
#define __NR_mprotect 226
#define __NR_exit 93
#define __NR_utimensat 88

/* =============================================================================
 * PART 3: SYSCALL WRAPPER FUNCTIONS (IMPLEMENT THESE)
 * =============================================================================
 */

/* File descriptor constants */
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif

/* Open flags */
#ifndef O_RDONLY
#define O_RDONLY 0x00
#define O_WRONLY 0x01
#define O_RDWR 0x02
#define O_CREAT 0x40
#define O_TRUNC 0x200
#define O_APPEND 0x400
#endif

/* Memory protection flags */
#ifndef PROT_READ
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#endif

/* Memory mapping flags */
#ifndef MAP_SHARED
#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_ANONYMOUS 0x20
#endif

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif

/* TODO: Implement these syscall wrapper functions */

static inline int sys_openat(int dirfd, const char *pathname, int flags,
                             mode_t mode) {
  return (int)syscall4(__NR_openat, (long)dirfd, (long)pathname, (long)flags, (long)mode);
}

static inline int sys_close(int fd) {
  return (int)syscall1(__NR_close, (long)fd);
}

static inline ssize_t sys_read(int fd, void *buf, size_t count) {
  return (ssize_t)syscall3(__NR_read, (long)fd, (long)buf, (long)count);
}

static inline ssize_t sys_write(int fd, const void *buf, size_t count) {
  return (ssize_t)syscall3(__NR_write, (long)fd, (long)buf, (long)count);
}

static inline ssize_t sys_pread64(int fd, void *buf, size_t count,
                                  off_t offset) {
  return (ssize_t)syscall4(__NR_pread64, (long)fd, (long)buf, (long)count, (long)offset);
}

static inline ssize_t sys_pwrite64(int fd, const void *buf, size_t count,
                                   off_t offset) {
  return (ssize_t)syscall4(__NR_pwrite64, (long)fd, (long)buf, (long)count, (long)offset);
}

static inline off_t sys_lseek(int fd, off_t offset, int whence) {
  return (off_t)syscall3(__NR3264_lseek, (long)fd, (long)offset, (long)whence);
}

static inline int sys_fstat(int fd, struct stat *statbuf) {
  return (int)syscall2(__NR3264_fstat, (long)fd, (long)statbuf);
}

static inline void *sys_mmap(void *addr, size_t length, int prot, int flags,
                             int fd, off_t offset) {
  return (void *)syscall6(__NR3264_mmap, (long)addr, (long)length, (long)prot, (long)flags, (long)fd, (long)offset);
}

static inline int sys_munmap(void *addr, size_t length) {
  return (int)syscall2(__NR_munmap, (long)addr, (long)length);
}

static inline int sys_mprotect(void *addr, size_t len, int prot) {
  return (int)syscall3(__NR_mprotect, (long)addr, (long)len, (long)prot);
}

static inline void sys_exit(int status) {
  syscall1(__NR_exit, (long)status);
  __builtin_unreachable();     /*stolen from google*/
}

static inline int sys_utimensat(int dirfd, const char *pathname,
                                const void *times, int flags) {
  return (int)syscall4(__NR_utimensat, (long)dirfd, (long)pathname, (long)times, (long)flags);
}

/* Note: C library functions like strlen, memcpy, etc. are implemented in minc.c
 * Students must implement these in the separate minimal C library.
 */

#endif /* SYSCALL_UTILS_H */
