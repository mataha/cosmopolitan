#include "libc/nt/codegen.h"
.imp	iphlpapi,__imp_GetAdapterOrderMap,GetAdapterOrderMap,0

	.text.windows
GetAdapterOrderMap:
#ifdef __x86_64__
	push	%rbp
	mov	%rsp,%rbp
	.profilable
	sub	$32,%rsp
	call	*__imp_GetAdapterOrderMap(%rip)
	leave
#elif defined(__aarch64__)
	mov	x0,#0
#endif
	ret
	.endfn	GetAdapterOrderMap,globl
	.previous