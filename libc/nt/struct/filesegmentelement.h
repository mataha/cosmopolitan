#ifndef COSMOPOLITAN_LIBC_NT_STRUCT_FILESEGMENTELEMENT_H_
#define COSMOPOLITAN_LIBC_NT_STRUCT_FILESEGMENTELEMENT_H_

union NtFileSegmentElement {
  void *Buffer;
  uint64_t Alignment;
};

#endif /* COSMOPOLITAN_LIBC_NT_STRUCT_FILESEGMENTELEMENT_H_ */
