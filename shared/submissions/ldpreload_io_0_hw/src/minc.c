/* minc.c - Minimal C library implementation */
#include <minc.h>
#include <syscall_utils.h>
/* =============================================================================
 * STRING FUNCTIONS
 * =============================================================================
 */

size_t strlen(const char *s){
  size_t cnt = 0;
  while(*s++ != '\0'){cnt++;}

  return cnt;
}

int strcmp(const char *s1, const char *s2){             //took some hints from https://stackoverflow.com/questions/34873209/implementation-of-strcmp
  const unsigned char *p1 = ( const unsigned char * )s1;
  const unsigned char *p2 = ( const unsigned char * )s2;

  while(*p1 && (*p1 == *p2)){
    p1++;
    p2++;
  }

  return ( *p1 > *p2 ) - ( *p2  > *p1 );
}

char *strcpy(char *dest, const char *src){
  char *d = dest;

  while(*d++ = *src++);

  return dest;
}

char *strcat(char *dest, const char *src){
  char *d = dest;

  while(*d)
    d++;

  while(*d++ = *src++);

  return dest;
}

char *strstr(const char *haystack, const char *needle){  // stolen from https://stackoverflow.com/questions/49131175/recreate-the-strstr-function
  const char *h;
  const char *n;

  n = needle;

  if (*n == 0) {
      return (char *)haystack;
  }

  for ( ; *haystack != 0; haystack += 1) {
    if (*haystack != *n) {
      continue;
    }

    h = haystack;
    while (1) {
      if (*n == 0) {
        return (char *)haystack;
      }

      if (*h++ != *n++) {
        break;
      }
    }
    
    n = needle;
  }

  return NULL;
}

char *strchr(const char *s, int c){
  while(*s){
    if(*s == c) 
      return (char *)s;
    s++;
  }

  if(c == '\0')
    return (char *)s;

  return NULL;
}

/* =============================================================================
 * MEMORY FUNCTIONS
 * =============================================================================
 */

void *memcpy(void *dest, const void *src, size_t n){
  char *d = dest;
  const char *s = (char *)src;

  for(size_t i=0; i < n; i++){
    *d++ = *s++;
  }

  return dest;
}

void *memset(void *s, int c, size_t n){              //stolen from https://stackoverflow.com/questions/18851835/create-my-own-memset-function-in-c 
  unsigned char *dst = (unsigned char *)s;
  while (n > 0) {
        *dst = (unsigned char)c;
        dst++;
        n--;
  }
  
  return s;
}

int memcmp(const void *s1, const void *s2, size_t n){
  const unsigned char *str1 = (const unsigned char *)s1;
  const char *str2 = (const char *)s2;

  while(n-- > 0){
    if(*str1 != *str2){
      return (*str1 > *str2) - (*str2 > *str1);
    }
    str1++;
    str2++;
  }

  return 0;
}

/* =============================================================================
 * CONVERSION FUNCTIONS
 * =============================================================================
 */

int atoi(const char *str){                    //stolen and modified from https://stackoverflow.com/questions/12791077/atoi-implementation-in-c
  int k=0;
  int sign=1;

  while(*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r' || *str == '\f' || *str == '\v'){
    str++;
  }

  if(*str == '-'){
    sign = -1;
    str++;
  } 
  else if(*str == '+'){
    str++;
  }

  while(*str >= '0' && *str <= '9'){
    k = (k*10) + (*str) - '0';
    str++;
  }
  
  return sign*k;
}

long atol(const char *str){
  long k=0;
  long sign=1;

  while(*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r' || *str == '\f' || *str == '\v'){
    str++;
  }

  if(*str == '-'){
    sign = -1;
    str++;
  }
  else if(*str == '+'){
    str++;
  }

  while(*str >= '0' && *str <= '9'){
    k = (k*10) + (*str) - '0';
    str++;
  }

  return sign*k;
}

/* =============================================================================
 * OUTPUT HELPER FUNCTIONS
 * =============================================================================
 */


void write_num(int fd, long num){
    char buf[21];
    int i = 0;
    int neg = 0;
    unsigned long n;

    if (num < 0){
        neg = 1;
        n = (unsigned long)(-(num + 1)) + 1;
    } else {
        n = (unsigned long)num;
    }

    if (n == 0){
        buf[i++] = '0';
    } else {
        while (n > 0){
            buf[i++] = '0' + (n % 10);
            n /= 10;
        }
    }

    if (neg){
        buf[i++] = '-';
    }

    char out[21];
    int j = 0;
    while (i > 0){
        out[j++] = buf[--i];
    }

    sys_write(fd, out, j);
}

void write_hex(int fd, unsigned long num){
    char buf[17];
    char hex_chars[] = "0123456789abcdef";
    int i = 0;

    if (num == 0){
        sys_write(fd, "0", 1);
        return;
    }

    while (num > 0){
        buf[i++] = hex_chars[num & 0xF];
        num >>= 4;
    }

    char out[17];
    int j = 0;
    while (i > 0){
        out[j++] = buf[--i];
    }

    sys_write(fd, out, j);
}

int parse_hex(const char *hex_str, uint8_t *bytes, size_t max_bytes){
    size_t len = strlen(hex_str);
    
    if (len == 0 || (len % 2) != 0){
        return -1;
    }

    size_t num_bytes = len / 2;
    if (num_bytes > max_bytes){
        return -1;
    }

    for (size_t i = 0; i < num_bytes; i++){
        uint8_t high, low;
        char c_high = hex_str[i * 2];
        char c_low = hex_str[i * 2 + 1];

        if (c_high >= '0' && c_high <= '9'){
            high = c_high - '0';
        } else if (c_high >= 'a' && c_high <= 'f'){
            high = c_high - 'a' + 10;
        } else if (c_high >= 'A' && c_high <= 'F'){
            high = c_high - 'A' + 10;
        } else {
            return -1;
        }

        if (c_low >= '0' && c_low <= '9'){
            low = c_low - '0';
        } else if (c_low >= 'a' && c_low <= 'f'){
            low = c_low - 'a' + 10;
        } else if (c_low >= 'A' && c_low <= 'F'){
            low = c_low - 'A' + 10;
        } else {
            return -1;
        }

        bytes[i] = (high << 4) | low;
    }

    return (int)num_bytes;
}
