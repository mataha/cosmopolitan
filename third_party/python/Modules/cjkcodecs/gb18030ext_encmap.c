#include "libc/x/x.h"

static _Atomic(void *) gb18030ext_encmap_ptr;
static const unsigned char gb18030ext_encmap_rodata[] = {
  0x63, 0x60, 0x60, 0x60, 0x60, 0x64, 0xf8, 0xf9, 0x93, 0x61, 0x80, 0x00, 0x13,
  0xc3, 0x9a, 0x35, 0xe4, 0xe8, 0x63, 0x66, 0x68, 0x3c, 0xe5, 0xcb, 0xf0, 0xe1,
  0x77, 0x24, 0x83, 0x9d, 0x1d, 0xb2, 0x78, 0x14, 0x83, 0x7b, 0x71, 0x3b, 0xc3,
  0xbc, 0x79, 0x1d, 0x0c, 0x7c, 0x52, 0x30, 0xb1, 0xa9, 0x0c, 0x12, 0xf7, 0x63,
  0x19, 0x8b, 0x8b, 0xe3, 0x18, 0xfd, 0xfc, 0xe2, 0x19, 0xf3, 0x1e, 0x20, 0xab,
  0xbf, 0xc4, 0x18, 0x16, 0x76, 0x99, 0x31, 0x3e, 0x1e, 0xc4, 0xbe, 0xc2, 0x68,
  0x7e, 0xb7, 0x9a, 0xe9, 0xda, 0x35, 0x10, 0xbb, 0x86, 0xc9, 0x27, 0x71, 0x12,
  0x93, 0x72, 0x2f, 0x88, 0xfd, 0x97, 0xc9, 0x7d, 0x3b, 0x4c, 0x7d, 0x1e, 0x73,
  0xf9, 0xe2, 0xd9, 0xcc, 0xc2, 0xeb, 0x18, 0x46, 0xc1, 0x28, 0xa0, 0x02, 0x30,
  0x67, 0x61, 0xf8, 0x6f, 0xce, 0x0a, 0xc4, 0x6c, 0x40, 0xcc, 0x0e, 0xc4, 0x1c,
  0x40, 0xcc, 0x09, 0xc4, 0x5c, 0x40, 0xcc, 0x0d, 0xc4, 0x3c, 0x0c, 0x29, 0xb4,
  0xb4, 0x1f, 0x00,
};

optimizesize void *gb18030ext_encmap(void) {
  return xload(&gb18030ext_encmap_ptr,
               gb18030ext_encmap_rodata,
               146, 1024); /* 14.2578% profit */
}
