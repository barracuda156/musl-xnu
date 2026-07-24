/* Darwin PowerPC fenv, following Apple Libm's PowerPC fenv semantics
 * (Source/PowerPC/fenv.c) with the SDK ABI: fenv_t/fexcept_t are the
 * 32-bit FPSCR image, FE_DFL_ENV = &_FE_DFL_ENV (zero constant).
 * FPSCR masks: exceptions 0x3e000000, invalid causes 0x01f80700,
 * VXSOFT 0x400, FX 0x80000000, enables 0xf8, RN 0x3.
 */

	.const
	.align 2
	.globl __FE_DFL_ENV
__FE_DFL_ENV:
	.long 0

	.text
	.align 2
	.globl _feclearexcept
_feclearexcept:
	andis. r3, r3, 0x3e00
	/* if (r3 & FE_INVALID) also clear all invalid-cause bits */
	andis. r0, r3, 0x2000
	stwu r1, -16(r1)
	beq- 1f
	oris r3, r3, 0x01f8
	ori r3, r3, 0x0700
1:	/* fpscr &= ~r3; also clear FX if no exceptions remain (Apple) */
	mffs f0
	stfd f0, 8(r1)
	lwz r9, 12(r1)
	andc r9, r9, r3
	lis r0, 0x3ff8
	ori r0, r0, 0x0700
	and. r0, r9, r0
	bne 2f
	clrlwi r9, r9, 1
2:	stw r9, 12(r1)
	lfd f0, 8(r1)
	mtfsf 255, f0
	li r3, 0
	addi r1, r1, 16
	blr

	.align 2
	.globl _feraiseexcept
_feraiseexcept:
	andis. r3, r3, 0x3e00
	/* if (r3 & FE_INVALID) raise via VXSOFT (VX is derived) */
	andis. r0, r3, 0x2000
	stwu r1, -16(r1)
	beq- 1f
	ori r3, r3, 0x0400
1:	/* fpscr |= r3 */
	mffs f0
	stfd f0, 8(r1)
	lwz r9, 12(r1)
	or r9, r9, r3
	stw r9, 12(r1)
	lfd f0, 8(r1)
	mtfsf 255, f0
	li r3, 0
	addi r1, r1, 16
	blr

	.align 2
	.globl _fetestexcept
_fetestexcept:
	andis. r3, r3, 0x3e00
	/* return r3 & fpscr */
	stwu r1, -16(r1)
	mffs f0
	stfd f0, 8(r1)
	lwz r9, 12(r1)
	addi r1, r1, 16
	and r3, r3, r9
	blr

	.align 2
	.globl _fegetround
_fegetround:
	/* return fpscr & 3 */
	stwu r1, -16(r1)
	mffs f0
	stfd f0, 8(r1)
	lwz r3, 12(r1)
	addi r1, r1, 16
	clrlwi r3, r3, 30
	blr

	.align 2
	.globl ___fesetround
___fesetround:
	/* invalid input is not checked, r3 < 4 must hold
	 * fpscr = (fpscr & -4U) | r3 */
	stwu r1, -16(r1)
	mffs f0
	stfd f0, 8(r1)
	lwz r9, 12(r1)
	clrrwi r9, r9, 2
	or r9, r9, r3
	stw r9, 12(r1)
	lfd f0, 8(r1)
	mtfsf 255, f0
	li r3, 0
	addi r1, r1, 16
	blr

	.align 2
	.globl _fegetenv
_fegetenv:
	/* *r3 = fpscr (low word only; fenv_t is unsigned int) */
	stwu r1, -16(r1)
	mffs f0
	stfd f0, 8(r1)
	lwz r0, 12(r1)
	addi r1, r1, 16
	stw r0, 0(r3)
	li r3, 0
	blr

	.align 2
	.globl _fesetenv
_fesetenv:
	/* fpscr = *r3; FE_DFL_ENV is a real pointer (zero constant),
	 * so no sentinel check.  Keep the mffs image's high word so the
	 * reloaded double is well-formed. */
	lwz r0, 0(r3)
	stwu r1, -16(r1)
	mffs f0
	stfd f0, 8(r1)
	stw r0, 12(r1)
	lfd f0, 8(r1)
	mtfsf 255, f0
	addi r1, r1, 16
	li r3, 0
	blr
