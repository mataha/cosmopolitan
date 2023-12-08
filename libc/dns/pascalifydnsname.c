/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│ vi: set et ft=c ts=2 sts=2 sw=2 fenc=utf-8                               :vi │
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
#include "libc/dns/dns.h"
#include "libc/str/str.h"
#include "libc/sysv/errfuns.h"

/**
 * Writes dotted hostname to DNS message wire.
 *
 * The wire format is basically a sequence of Pascal strings, for each
 * label in the name. We only do enough validation to maintain protocol
 * invariants.
 *
 * @param name is a dotted NUL-terminated hostname string
 * @return bytes written (excluding NUL) or -1 w/ errno
 */
int PascalifyDnsName(uint8_t *buf, size_t size, const char *name) {
  size_t i, j, k, namelen;
  if ((namelen = strlen(name)) > DNS_NAME_MAX) return enametoolong();
  i = 0;
  if (size || namelen) {
    if (namelen + 1 > size) return enospc();
    buf[0] = '\0';
    j = 0;
    for (;;) {
      for (k = 0; name[j + k] && name[j + k] != '.'; ++k) {
        buf[i + k + 1] = name[j + k];
      }
      if (k) {
        if (k > DNS_LABEL_MAX) return enametoolong();
        buf[i] = k;
        i += k + 1;
      }
      j += k + 1;
      if (!name[j - 1]) {
        break;
      }
    }
    buf[i] = '\0';
  }
  return i;
}
