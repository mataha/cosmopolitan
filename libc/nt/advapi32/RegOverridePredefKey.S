#include "libc/nt/codegen.h"
.imp	advapi32,__imp_RegOverridePredefKey,RegOverridePredefKey,1658

	.text.windows
RegOverridePredefKey:
#ifdef __x86_64__
	push	%rbp
	mov	%rsp,%rbp
	.profilable
	mov	__imp_RegOverridePredefKey(%rip),%rax
	jmp	__sysv2nt
#elif defined(__aarch64__)
	mov	x0,#0
	ret
#endif
	.endfn	RegOverridePredefKey,globl
	.previous