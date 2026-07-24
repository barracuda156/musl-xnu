/* Darwin PowerPC indirect syscall veneer
 * long __syscall(long nr, ...) -- nr in r3, up to 6 args in r4..r9.
 *
 * XNU convention: plain syscall number in r0; on error the kernel
 * returns to sc+4 with positive errno in r3, on success to sc+8.
 * The neg at sc+4 therefore executes only on the error path.
 */

	.text
	.align 2
	.globl ___syscall
___syscall:
	mr r0, r3
	mr r3, r4
	mr r4, r5
	mr r5, r6
	mr r6, r7
	mr r7, r8
	mr r8, r9
	sc
	neg r3, r3
	blr
