/* Darwin PowerPC64 sigsetjmp
 * int sigsetjmp(sigjmp_buf env, int savemask)
 *
 * Uses the jmp_buf spare slots (register area ends at 336):
 *   336: caller's LR   344: caller's r31
 * env is kept in r31 across setjmp, so when siglongjmp re-enters the
 * resume point r31 still holds env (setjmp saved it).  Both paths
 * tail into __sigsetjmp_tail(env, ret): ret==0 saves the current
 * mask into env->__ss, ret!=0 (via longjmp) restores it -- which is
 * why siglongjmp is plain longjmp in this scheme.
 */

	.text
	.align 2
	.globl _sigsetjmp
	.globl __sigsetjmp
_sigsetjmp:
__sigsetjmp:
	cmpwi r4, 0
	beq 1f
	mflr r0
	std r0, 336(r3)
	std r31, 344(r3)
	mr r31, r3
	bl __setjmp
	mr r4, r3
	mr r3, r31
	ld r0, 336(r3)
	mtlr r0
	ld r31, 344(r3)
	b ___sigsetjmp_tail
1:	b __setjmp
