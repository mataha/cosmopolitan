#include "libc/nt/ntdllimport.h"
.ntimp	NtCreateNamedPipeFile,NtCreateNamedPipeFile

	.text.windows
NtCreateNamedPipeFile:
#ifdef __x86_64__
	push	%rbp
	mov	%rsp,%rbp
	.profilable
	mov	__imp_NtCreateNamedPipeFile(%rip),%rax
	jmp	__sysv2nt14
#elif defined(__aarch64__)
	mov	x0,#0
	ret
#endif
	.endfn	NtCreateNamedPipeFile,globl
	.previous