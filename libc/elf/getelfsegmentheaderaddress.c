/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2020 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/elf/elf.h"
#include "libc/stdckdint.h"

Elf64_Phdr *GetElfSegmentHeaderAddress(const Elf64_Ehdr *elf,  //
                                       size_t mapsize,         //
                                       Elf64_Half i) {         //
  uint64_t last, addr;
  if (i >= elf->e_phnum) return 0;
  if (ckd_add(&addr, (uintptr_t)elf, elf->e_phoff)) return 0;
  if (ckd_add(&addr, addr, (unsigned)i * elf->e_phentsize)) return 0;
  if (ckd_add(&last, addr, elf->e_phentsize)) return 0;
  if (last > (uintptr_t)elf + mapsize) return 0;
  return (Elf64_Phdr *)addr;
}
