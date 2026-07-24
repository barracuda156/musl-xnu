#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "syscall.h"
#include "libc.h"
#include "pthread_impl.h"

static void dummy(int x)
{
}

weak_alias(dummy, __fork_handler);

/* Darwin's fork returns twice with the child indicated in r4
 * (r4==0 parent, r4!=0 child); the child must force a 0 return.
 * (Same dual-register convention Apple's Libc handles in fork.s.) */

static pid_t __darwin_fork(void)
{
	register long r0 __asm__("r0") = SYS_fork;
	register long r3 __asm__("r3");
	register long r4 __asm__("r4");
	__asm__ __volatile__ (
		"sc\n\t"
		"neg %1, %1"
		: "+r"(r0), "=r"(r3), "=r"(r4)
		:
		: "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
		  "cr0", "ctr", "xer", "memory");
	if (r3 >= 0 && r4) return 0;
	return r3;
}

pid_t fork(void)
{
	pid_t ret;
	sigset_t set;
	__fork_handler(-1);
	__block_all_sigs(&set);
	ret = __syscall_ret(__darwin_fork());
	if (!ret) {
		pthread_t self = __pthread_self();
		self->tid = __syscall(SYS_gettid);
		self->robust_list.off = 0;
		self->robust_list.pending = 0;
		libc.threads_minus_1 = 0;
	}
	__restore_sigs(&set);
	__fork_handler(!ret);
	return ret;
}
