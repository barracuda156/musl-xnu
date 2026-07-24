#include <unistd.h>
#include "syscall.h"

/* Darwin's pipe() returns the two descriptors in r3/r4; the kernel
 * does not write to user memory (the argument is ignored). */

int pipe(int fd[2])
{
	register long r0 __asm__("r0") = SYS_pipe;
	register long r3 __asm__("r3");
	register long r4 __asm__("r4");
	__asm__ __volatile__ (
		"sc\n\t"
		"neg %1, %1"
		: "+r"(r0), "=r"(r3), "=r"(r4)
		:
		: "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
		  "cr0", "ctr", "xer", "memory");
	if (r3 < 0) return __syscall_ret(r3);
	fd[0] = r3;
	fd[1] = r4;
	return 0;
}
