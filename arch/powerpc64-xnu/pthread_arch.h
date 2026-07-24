/* Darwin PowerPC64 thread pointer access.
 *
 * The 64-bit Darwin PPC ABI reserves r13 as the thread-local-data
 * pointer (fixed register, never allocated by the compiler); it is
 * set in __set_thread_area.s and context-switched by the kernel like
 * any other GPR, making it per-thread.
 */

static inline struct pthread *__pthread_self()
{
	register struct pthread *self __asm__("r13");
	__asm__ ("" : "=r"(self));
	return self;
}

#define TP_ADJ(p) (p)

#define MC_PC srr0
