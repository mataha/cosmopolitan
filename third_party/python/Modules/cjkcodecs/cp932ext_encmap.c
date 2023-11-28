#include "libc/x/x.h"

static _Atomic(void *) cp932ext_encmap_ptr;
static const unsigned char cp932ext_encmap_rodata[] = {
  0x63, 0x60, 0x18, 0x78, 0xc0, 0xc8, 0x20, 0x56, 0x99, 0xca, 0x20, 0xb8, 0x1f,
  0xc4, 0x16, 0x61, 0x4c, 0x28, 0x26, 0x45, 0xaf, 0x06, 0xa3, 0xac, 0x3c, 0x88,
  0xd6, 0x66, 0x34, 0x5c, 0xb1, 0x98, 0x91, 0xf9, 0x2c, 0xbd, 0xdc, 0x9c, 0xc7,
  0xa4, 0xf1, 0xc7, 0x99, 0x99, 0xe1, 0xbf, 0x33, 0x8b, 0xdc, 0x17, 0x29, 0x56,
  0xaf, 0x37, 0x7b, 0x59, 0x45, 0x6f, 0xb7, 0xb0, 0x31, 0xdc, 0x4d, 0x62, 0xef,
  0xfa, 0x7f, 0x83, 0xbd, 0xad, 0x0d, 0xa4, 0xe6, 0x26, 0x7b, 0xe4, 0x09, 0x4f,
  0x8e, 0x79, 0x9b, 0xe2, 0x38, 0xb8, 0x77, 0x81, 0xf8, 0x7c, 0x9c, 0x61, 0x6f,
  0x96, 0x72, 0xca, 0x7d, 0xad, 0xe5, 0x52, 0xbf, 0x00, 0xe2, 0xab, 0x73, 0x2b,
  0xde, 0x7b, 0xca, 0x1d, 0xfb, 0xa9, 0x9a, 0x47, 0x70, 0x06, 0x33, 0xaf, 0xf0,
  0xb2, 0xe9, 0xbc, 0x5f, 0xbf, 0xce, 0xe0, 0x4d, 0x38, 0xc7, 0xce, 0xe7, 0xe7,
  0xc7, 0xc1, 0xc7, 0xf0, 0x9b, 0x85, 0x9f, 0xef, 0xc0, 0x76, 0x7e, 0xc6, 0xf3,
  0x6d, 0x02, 0x19, 0x8f, 0x18, 0x05, 0x0d, 0x9e, 0x6c, 0x13, 0xbc, 0x76, 0x6d,
  0xbb, 0xa0, 0xfd, 0xad, 0x60, 0x21, 0x96, 0x3f, 0x3e, 0xc2, 0xea, 0xfb, 0x9f,
  0x0a, 0x77, 0x7c, 0x0d, 0x16, 0x61, 0xdd, 0xcd, 0x25, 0xca, 0xf2, 0x8f, 0x55,
  0x6c, 0xe3, 0x3e, 0x61, 0x31, 0x95, 0xaf, 0x4f, 0xc5, 0xd8, 0xe7, 0xd7, 0x89,
  0x33, 0xe6, 0xbf, 0x15, 0x6f, 0x5a, 0x26, 0x24, 0xe1, 0xe6, 0x26, 0x2c, 0xa1,
  0x58, 0x95, 0x2b, 0x61, 0x30, 0xfb, 0xa6, 0xc4, 0xc5, 0xd7, 0x5f, 0x24, 0xe6,
  0xcd, 0x03, 0xd9, 0xfb, 0x55, 0xc2, 0xe3, 0x5a, 0x8b, 0x64, 0x50, 0xd7, 0x5e,
  0x49, 0xf7, 0x85, 0xc8, 0x7e, 0x92, 0x90, 0x62, 0xfc, 0xc6, 0x27, 0xed, 0x71,
  0x67, 0xb1, 0x74, 0xf0, 0x06, 0x78, 0xdc, 0xc8, 0xb0, 0x7f, 0xfd, 0x20, 0x23,
  0x23, 0xf3, 0x51, 0x46, 0xe8, 0xdb, 0x35, 0xd9, 0xe0, 0x7a, 0x66, 0xb9, 0x0f,
  0x5f, 0x38, 0xe4, 0x84, 0xca, 0x72, 0xe5, 0xce, 0x9f, 0x07, 0x87, 0x83, 0x5c,
  0xfa, 0xbd, 0x67, 0x72, 0xa2, 0xef, 0x0e, 0xc8, 0xb3, 0xfd, 0xdf, 0xa5, 0xc0,
  0xf4, 0x63, 0xa3, 0xa2, 0xa1, 0xc7, 0x49, 0xc5, 0x49, 0x93, 0x4e, 0x29, 0xce,
  0x5d, 0x7f, 0x57, 0xd1, 0x38, 0x94, 0x41, 0x29, 0x3c, 0x95, 0x5f, 0x49, 0x7d,
  0x5e, 0xbb, 0x92, 0xdf, 0x1d, 0x31, 0xe5, 0xa2, 0xdd, 0x09, 0xca, 0x0c, 0x0c,
  0x89, 0xca, 0xd9, 0x05, 0xe9, 0xca, 0x92, 0x17, 0x19, 0x46, 0x01, 0x5d, 0x80,
  0x82, 0x8a, 0xe6, 0x9d, 0x2b, 0x2a, 0x7c, 0xba, 0xe8, 0xe2, 0x5f, 0x54, 0x98,
  0x9e, 0x00, 0x00,
};

optimizesize void *cp932ext_encmap(void) {
  return xload(&cp932ext_encmap_ptr,
               cp932ext_encmap_rodata,
               367, 1024); /* 35.8398% profit */
}
