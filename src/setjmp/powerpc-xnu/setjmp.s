/* Darwin PowerPC setjmp
 * int setjmp(jmp_buf env), env in r3
 *
 * Layout (jmp_buf is unsigned long[70] = 280 bytes; 240..279 spare,
 * sigsetjmp uses 240/244):
 *   0 r1, 4 r2, 8..80 r13-r31, 84 LR, 88 CR, 92 CTR, 96..232 f14-f31
 */

	.text
	.align 2
	.globl _setjmp
	.globl __setjmp
	.globl ___setjmp
_setjmp:
__setjmp:
___setjmp:
	stw r1, 0(r3)
	stw r2, 4(r3)
	stw r13, 8(r3)
	stw r14, 12(r3)
	stw r15, 16(r3)
	stw r16, 20(r3)
	stw r17, 24(r3)
	stw r18, 28(r3)
	stw r19, 32(r3)
	stw r20, 36(r3)
	stw r21, 40(r3)
	stw r22, 44(r3)
	stw r23, 48(r3)
	stw r24, 52(r3)
	stw r25, 56(r3)
	stw r26, 60(r3)
	stw r27, 64(r3)
	stw r28, 68(r3)
	stw r29, 72(r3)
	stw r30, 76(r3)
	stw r31, 80(r3)
	mflr r0
	stw r0, 84(r3)
	mfcr r0
	stw r0, 88(r3)
	mfctr r0
	stw r0, 92(r3)
	stfd f14, 96(r3)
	stfd f15, 104(r3)
	stfd f16, 112(r3)
	stfd f17, 120(r3)
	stfd f18, 128(r3)
	stfd f19, 136(r3)
	stfd f20, 144(r3)
	stfd f21, 152(r3)
	stfd f22, 160(r3)
	stfd f23, 168(r3)
	stfd f24, 176(r3)
	stfd f25, 184(r3)
	stfd f26, 192(r3)
	stfd f27, 200(r3)
	stfd f28, 208(r3)
	stfd f29, 216(r3)
	stfd f30, 224(r3)
	stfd f31, 232(r3)
	li r3, 0
	blr
