/* Darwin PowerPC mach_absolute_time
 * uint64_t mach_absolute_time(void) -- timebase in r3(hi):r4(lo).
 * Same mftbu/mftb/mftbu loop the commpage uses; unprivileged on all
 * OS X-capable CPUs.
 */

	.text
	.align 2
	.globl _mach_absolute_time
_mach_absolute_time:
1:	mftbu r3
	mftb r4
	mftbu r5
	cmplw r3, r5
	bne- 1b
	blr
