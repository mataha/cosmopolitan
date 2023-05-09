#include "libc/nt/codegen.h"
.imp	kernel32,__imp_GetQueuedCompletionStatusEx,GetQueuedCompletionStatusEx,0

	.text.windows
GetQueuedCompletionStatusEx:
#ifdef __x86_64__
	push	%rbp
	mov	%rsp,%rbp
	.profilable
	mov	__imp_GetQueuedCompletionStatusEx(%rip),%rax
	jmp	__sysv2nt6
#elif defined(__aarch64__)
	mov	x0,#0
	ret
#endif
	.endfn	GetQueuedCompletionStatusEx,globl
	.previous