#ifndef COSMOPOLITAN_LIBC_NEXGEN32E_STACKFRAME_H_
#define COSMOPOLITAN_LIBC_NEXGEN32E_STACKFRAME_H_
#ifdef _COSMO_SOURCE
COSMOPOLITAN_C_START_

struct StackFrame {
  struct StackFrame *next;
  intptr_t addr;
};

COSMOPOLITAN_C_END_
#endif /* _COSMO_SOURCE */
#endif /* COSMOPOLITAN_LIBC_NEXGEN32E_STACKFRAME_H_ */
