#include <stdint.h>

/* PPC has no RDRAND equivalent; use the timebase as a weak entropy
 * source (mftb yields the low word in 32-bit mode, the full register
 * in 64-bit mode), same spirit as the x86 fallback. */

uintptr_t __arch_entropy()
{
	uintptr_t entropy;
	__asm__ __volatile__("mftb %0" : "=r"(entropy));
	return entropy;
}
