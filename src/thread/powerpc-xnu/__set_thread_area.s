/* Darwin PowerPC __set_thread_area
 * int __set_thread_area(void *p)
 *
 * Fast trap 0x7FF1 (CthreadSetSelfNumber, osfmk/ppc/hw_exception.s)
 * stores r3 as the per-thread "cthread self" value (TLDP).  It is
 * read back via the commpage _COMM_PAGE_PTHREAD_SELF routine, which
 * the kernel implements per-CPU (mfspr r3,259 on G5, ultra-fast trap
 * 0x7FF2 on G3/G4).  The fast trap preserves all registers.
 */

	.text
	.align 2
	.globl ___set_thread_area
___set_thread_area:
	li r0, 0x7FF1
	sc
	li r3, 0
	blr
