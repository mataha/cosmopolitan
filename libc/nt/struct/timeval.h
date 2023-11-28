#ifndef COSMOPOLITAN_LIBC_NT_STRUCT_TIMEVAL_H_
#define COSMOPOLITAN_LIBC_NT_STRUCT_TIMEVAL_H_
COSMOPOLITAN_C_START_

struct NtTimeval {
  int32_t tv_sec; /* [sic] */
  int32_t tv_usec;
};

COSMOPOLITAN_C_END_
#endif /* COSMOPOLITAN_LIBC_NT_STRUCT_TIMEVAL_H_ */
