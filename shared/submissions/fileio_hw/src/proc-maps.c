/* proc-maps.c - Parse /proc/self/maps with line-by-line buffered reading */
#include "syscall_utils.h"
#include "minc.h"

#define PAGE_SIZE 4096
#define BUF_SIZE 4096

/* Parse a hex string and return the value, updating the pointer */
static unsigned long parse_hex_addr(const char **p) {
    unsigned long val = 0;
    const char *s = *p;
    
    while (*s) {
        char c = *s;
        if (c >= '0' && c <= '9') {
            val = (val << 4) | (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            val = (val << 4) | (c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            val = (val << 4) | (c - 'A' + 10);
        } else {
            break;
        }
        s++;
    }
    
    *p = s;
    return val;
}

/* Skip whitespace */
static void skip_spaces(const char **p) {
    while (**p == ' ' || **p == '\t') {
        (*p)++;
    }
}

/* Parse a single line from /proc/self/maps */
static void parse_maps_line(const char *line) {
    const char *p = line;
    
    /* Skip empty lines */
    if (*p == '\0' || *p == '\n') {
        return;
    }
    
    /* Parse start address */
    unsigned long start = parse_hex_addr(&p);
    
    /* Expect '-' */
    if (*p != '-') {
        return;
    }
    p++;
    
    /* Parse end address */
    unsigned long end = parse_hex_addr(&p);
    
    /* Skip space */
    skip_spaces(&p);
    
    /* Parse permissions (4 chars: rwxp) */
    char perms[5];
    int i;
    for (i = 0; i < 4 && *p && *p != ' '; i++) {
        perms[i] = *p++;
    }
    perms[i] = '\0';
    
    /* Skip space */
    skip_spaces(&p);
    
    /* Skip offset (hex) */
    while (*p && *p != ' ') p++;
    skip_spaces(&p);
    
    /* Skip dev (xx:xx) */
    while (*p && *p != ' ') p++;
    skip_spaces(&p);
    
    /* Skip inode */
    while (*p && *p != ' ') p++;
    skip_spaces(&p);
    
    /* Rest is pathname (may be empty) */
    const char *name = p;
    
    /* Find end of name (strip newline) */
    int name_len = 0;
    while (name[name_len] && name[name_len] != '\n') {
        name_len++;
    }
    
    /* Print the entry */
    sys_write(1, "Start: 0x", 9);
    write_hex(1, start);
    sys_write(1, "\n", 1);
    
    sys_write(1, "End:   0x", 9);
    write_hex(1, end);
    sys_write(1, "\n", 1);
    
    sys_write(1, "Perms: ", 7);
    sys_write(1, perms, strlen(perms));
    sys_write(1, "\n", 1);
    
    if (name_len > 0) {
        sys_write(1, "Name:  ", 7);
        sys_write(1, name, name_len);
        sys_write(1, "\n", 1);
    }
    
    sys_write(1, "---\n", 4);
}

int main(void) {
    const char *maps_path = "/proc/self/maps";
    
    /* Open /proc/self/maps */
    int fd = sys_openat(AT_FDCWD, maps_path, O_RDONLY, 0);
    if (fd < 0) {
        sys_write(2, "Error: cannot open /proc/self/maps\n", 35);
        return 1;
    }
    
    sys_write(1, "=== Process Memory Map ===\n", 27);
    
    /* Buffered reading with line handling */
    char buf[BUF_SIZE];
    char line[BUF_SIZE];
    int line_pos = 0;
    ssize_t bytes_read;
    
    while ((bytes_read = sys_read(fd, buf, BUF_SIZE)) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            char c = buf[i];
            
            if (c == '\n') {
                /* End of line - null terminate and parse */
                line[line_pos] = '\0';
                parse_maps_line(line);
                line_pos = 0;
            } else {
                /* Add character to line buffer */
                if (line_pos < BUF_SIZE - 1) {
                    line[line_pos++] = c;
                }
            }
        }
    }
    
    /* Handle last line if no trailing newline */
    if (line_pos > 0) {
        line[line_pos] = '\0';
        parse_maps_line(line);
    }
    
    sys_close(fd);
    return 0;
}
