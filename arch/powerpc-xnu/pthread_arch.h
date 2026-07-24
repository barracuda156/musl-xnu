/* Darwin PowerPC thread pointer access.
 *
 * The TLDP is set with fast trap 0x7FF1 (see __set_thread_area.s) and
 * read back through the commpage routine _COMM_PAGE_PTHREAD_SELF at
 * 0xffff8580 (osfmk/ppc/cpu_capabilities.h).  The commpage is placed
 * in the top 32KB of the address space precisely so it is reachable
 * with an absolute branch (bla sign-extends its 26-bit target).  The
 * kernel installs a CPU-appropriate implementation there: mfspr
 * r3,259 (user SPRG3) on 64-bit-capable CPUs, ultra-fast trap 0x7FF2
 * on G3/G4.  Both variants use only r0 and r3.
 */

static inline struct pthread *__pthread_self()
{
	register struct pthread *self __asm__("r3");
	__asm__ __volatile__ (
		"bla 0xffff8580"
		: "=r"(self)
		:
		: "r0", "lr");
	return self;
}

#define TP_ADJ(p) (p)

#define MC_PC srr0
