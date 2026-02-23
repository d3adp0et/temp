/* active_hook.c - GOT/PLT patching hook for protected files */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <elf.h>
#include <errno.h>
#include <link.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define TARGET_OPENAT "openat"
#define TARGET_UNLINKAT "unlinkat"
#define PROTECTED_DIR "/tmp/protected"

/* Function pointer types for the original functions */
typedef int (*orig_openat_type)(int dirfd, const char *pathname, int flags,
                                mode_t mode);
typedef int (*orig_unlinkat_type)(int dirfd, const char *pathname, int flags);
static orig_openat_type real_openat = NULL;
static orig_unlinkat_type real_unlinkat = NULL;

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

static int callback(struct dl_phdr_info *info, size_t size, void *data) {
  int ret = 0;
  Elf64_Addr v_jmprel, v_symtab, v_strtab, haddr;
  Elf64_Xword v_pltrelsz;
  Elf64_Dyn *dyn;
  Elf64_Rela *rela;
  Elf64_Sym *sym;

  if(strcmp(info->dlpi_name, "") == 0){
    
    //looping through program headers
    for(size_t i = 0; i < info->dlpi_phnum; i++){
      if (info->dlpi_phdr[i].p_type == PT_DYNAMIC){       //finding the dynamic section
        
        haddr = info->dlpi_addr + info->dlpi_phdr[i].p_vaddr;
        dyn = (Elf64_Dyn *)haddr;
        
        //parse dynamic segment
        for (size_t j=0; ; j++){
          if (dyn[j].d_tag == DT_NULL) break;

          if (dyn[j].d_tag == DT_JMPREL){
            v_jmprel = dyn[j].d_un.d_ptr;
          } else if (dyn[j].d_tag == DT_PLTRELSZ){
            v_pltrelsz = dyn[j].d_un.d_val;
          } else if (dyn[j].d_tag == DT_SYMTAB){
            v_symtab = dyn[j].d_un.d_ptr;
          } else if (dyn[j].d_tag == DT_STRTAB){
            v_strtab = dyn[j].d_un.d_ptr;
          }
        }

        Elf64_Xword rinfo;
        rela = (Elf64_Rela *)v_jmprel;

        //Parse PLT relocations
        for (size_t j=0; j < (v_pltrelsz/sizeof(Elf64_Rela)); j++){
          rinfo = rela[j].r_info;

          //Extract symbol index and relocation type 
          uint64_t sym_idx = ELF64_R_SYM(rinfo);
          uint64_t type = ELF64_R_TYPE(rinfo);

          //check architecture
          if (type != R_AARCH64_JUMP_SLOT) continue;

          sym = (Elf64_Sym*)v_symtab;
          char *str_table_base = (char*)v_strtab;
          size_t offset = sym[sym_idx].st_name;

          char *name = (char*)(str_table_base + offset);
          size_t got_slot_addr = info->dlpi_addr + rela[j].r_offset;

          if (strcmp(name, "openat") == 0){
            if (mprotect((void*)PAGE_START(got_slot_addr), getpagesize(), PROT_READ | PROT_WRITE) == -1){
              perror("mprotect to read and write failed");
              break;
            }

            *(void **)got_slot_addr = hacked_openat; //FINALLLYYYYY: patching GOT openat!
          }

          if (strcmp(name, "unlinkat") == 0){
            if (mprotect((void*)PAGE_START(got_slot_addr), getpagesize(), PROT_READ | PROT_WRITE) == -1){
              perror("mprotect to read and write failed");
              break;
            }

            *(void **)got_slot_addr = hacked_unlinkat; //FINALLLYYYYY: patching GOT for unlinkat!
          }
          
        }

      }
    }
  } else {
    return ret;
  }

  return ret;
}

__attribute__((constructor)) void install_hooks(void) {
  real_openat = (orig_openat_type)dlsym(RTLD_NEXT, "openat");
  real_unlinkat = (orig_unlinkat_type)dlsym(RTLD_NEXT, "unlinkat");

  dl_iterate_phdr(callback, NULL);
}
