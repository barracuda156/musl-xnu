#include <time.h>
#include <errno.h>
#include <stdint.h>
#include "syscall.h"
#include "libc.h"
#include "atomic.h"

/* On PPC-era Darwin (10.4/10.5) the gettimeofday syscall returns the
 * time in registers (r3=seconds, r4=microseconds) and does NOT copy
 * out through the timeval pointer; the pointer just needs to be
 * non-NULL for the kernel to compute the time at all. */

int clock_gettime(clockid_t clk, struct timespec *ts)
{
	if (clk == CLOCK_REALTIME) {
		register long r0 __asm__("r0") = SYS_gettimeofday;
		register long r3 __asm__("r3") = (long)ts;
		register long r4 __asm__("r4") = 0;
		__asm__ __volatile__ (
			"sc\n\t"
			"neg %1, %1"
			: "+r"(r0), "+r"(r3), "+r"(r4)
			:
			: "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
			  "cr0", "ctr", "xer", "memory");
		if (r3 < 0) return __syscall_ret(r3);
		ts->tv_sec = r3;
		ts->tv_nsec = r4 * 1000;
		return 0;
	}
	return __syscall_ret(-EINVAL);
}
