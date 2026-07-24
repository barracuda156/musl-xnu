/* C99 non-stop mode per Apple Libm: save the environment, then clear
 * both the exception flags (FX, summary+flags+invalid causes,
 * 0xfff80700) and the exception enable bits (0xf8), unlike musl's
 * generic version which only clears the flags. */

	.text
	.align 2
	.globl _feholdexcept
_feholdexcept:
	/* *r3 = fpscr; then fpscr &= ~0xfff807f8 */
	stwu r1, -16(r1)
	mffs f0
	stfd f0, 8(r1)
	lwz r9, 12(r1)
	stw r9, 0(r3)
	lis r0, 0xfff8
	ori r0, r0, 0x07f8
	andc r9, r9, r0
	stw r9, 12(r1)
	lfd f0, 8(r1)
	mtfsf 255, f0
	addi r1, r1, 16
	li r3, 0
	blr
