/* Darwin PowerPC64 mach_absolute_time
 * uint64_t mach_absolute_time(void) -- full timebase in r3.
 */

	.text
	.align 2
	.globl _mach_absolute_time
_mach_absolute_time:
	mftb r3
	blr
