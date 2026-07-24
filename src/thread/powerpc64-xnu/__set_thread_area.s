/* Darwin PowerPC64 __set_thread_area
 * int __set_thread_area(void *p)
 *
 * The Darwin ppc64 ABI reserves r13 as the thread-local-data pointer
 * (compilers treat it as fixed), so setting it here makes it visible
 * to __pthread_self().  The kernel context-switches all GPRs, which
 * makes r13 inherently per-thread.  We also issue fast trap 0x7FF1
 * (CthreadSetSelfNumber) so the kernel's cthread_self copy stays in
 * sync (it preserves all registers).
 */

	.text
	.align 2
	.globl ___set_thread_area
___set_thread_area:
	mr r13, r3
	li r0, 0x7FF1
	sc
	li r3, 0
	blr
