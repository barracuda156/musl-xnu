/* Darwin PowerPC64 setjmp
 * int setjmp(jmp_buf env), env in r3
 *
 * Layout (jmp_buf is unsigned long[70] = 560 bytes; 336..559 spare,
 * sigsetjmp uses 336/344):
 *   0 r1, 8 r2, 16..160 r13-r31, 168 LR, 176 CR, 184 CTR,
 *   192..328 f14-f31
 */

	.text
	.align 2
	.globl _setjmp
	.globl __setjmp
	.globl ___setjmp
_setjmp:
__setjmp:
___setjmp:
	std r1, 0(r3)
	std r2, 8(r3)
	std r13, 16(r3)
	std r14, 24(r3)
	std r15, 32(r3)
	std r16, 40(r3)
	std r17, 48(r3)
	std r18, 56(r3)
	std r19, 64(r3)
	std r20, 72(r3)
	std r21, 80(r3)
	std r22, 88(r3)
	std r23, 96(r3)
	std r24, 104(r3)
	std r25, 112(r3)
	std r26, 120(r3)
	std r27, 128(r3)
	std r28, 136(r3)
	std r29, 144(r3)
	std r30, 152(r3)
	std r31, 160(r3)
	mflr r0
	std r0, 168(r3)
	mfcr r0
	std r0, 176(r3)
	mfctr r0
	std r0, 184(r3)
	stfd f14, 192(r3)
	stfd f15, 200(r3)
	stfd f16, 208(r3)
	stfd f17, 216(r3)
	stfd f18, 224(r3)
	stfd f19, 232(r3)
	stfd f20, 240(r3)
	stfd f21, 248(r3)
	stfd f22, 256(r3)
	stfd f23, 264(r3)
	stfd f24, 272(r3)
	stfd f25, 280(r3)
	stfd f26, 288(r3)
	stfd f27, 296(r3)
	stfd f28, 304(r3)
	stfd f29, 312(r3)
	stfd f30, 320(r3)
	stfd f31, 328(r3)
	li r3, 0
	blr
