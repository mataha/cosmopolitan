#include "libc/x/x.h"

static _Atomic(void *) cp950ext_decmap_ptr;
static const unsigned char cp950ext_decmap_rodata[] = {
  0x63, 0x60, 0x18, 0x05, 0xa3, 0x60, 0x60, 0x01, 0x23, 0x83, 0xeb, 0xe7, 0x0d,
  0x0c, 0x8e, 0xee, 0xdb, 0x19, 0x1e, 0x3e, 0x1c, 0x0d, 0x0d, 0xea, 0x83, 0x1d,
  0x0c, 0xd7, 0xfe, 0xe1, 0x92, 0x03, 0x00,
};

optimizesize void *cp950ext_decmap(void) {
  return xload(&cp950ext_decmap_ptr,
               cp950ext_decmap_rodata,
               33, 1024); /* 3.22266% profit */
}
