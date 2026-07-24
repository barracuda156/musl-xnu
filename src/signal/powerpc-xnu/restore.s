/* Vestigial __restore/__restore_rt stubs (Linux sa_restorer ABI).
 * Darwin delivers signals through the sa_tramp trampoline registered
 * by sigaction() (see sigaction.c), so these are never invoked; kept
 * only so any stray references still link.  SYS_sigreturn = 184.
 */

	.text
	.align 2
	.globl ___restore_rt
	.globl ___restore
___restore_rt:
___restore:
	li r0, 184
	sc
	blr
