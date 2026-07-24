#include <unistd.h>
#include "syscall.h"
#include "libc.h"

/* For 32-bit processes Darwin splits the 64-bit result of lseek
 * across r3 (high) and r4 (low), and takes the 64-bit offset as two
 * words with no register-pair alignment (r4:r5 here). */

off_t lseek(int fd, off_t offset, int whence)
{
	register long r0 __asm__("r0") = SYS_lseek;
	register long r3 __asm__("r3") = fd;
	register long r4 __asm__("r4") = (long)(offset>>32);
	register long r5 __asm__("r5") = (long)offset;
	register long r6 __asm__("r6") = whence;
	__asm__ __volatile__ (
		"sc\n\t"
		"neg %1, %1"
		: "+r"(r0), "+r"(r3), "+r"(r4), "+r"(r5), "+r"(r6)
		:
		: "r7", "r8", "r9", "r10", "r11", "r12",
		  "cr0", "ctr", "xer", "memory");
	if (r3 < 0) return __syscall_ret(r3);
	return ((off_t)r3 << 32) | (unsigned int)r4;
}
