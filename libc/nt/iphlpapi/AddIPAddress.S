#include "libc/nt/codegen.h"
.imp	iphlpapi,__imp_AddIPAddress,AddIPAddress,0

	.text.windows
AddIPAddress:
#ifdef __x86_64__
	push	%rbp
	mov	%rsp,%rbp
	.profilable
	mov	__imp_AddIPAddress(%rip),%rax
	jmp	__sysv2nt6
#elif defined(__aarch64__)
	mov	x0,#0
	ret
#endif
	.endfn	AddIPAddress,globl
	.previous