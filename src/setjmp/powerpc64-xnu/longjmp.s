/* Darwin PowerPC64 longjmp
 * void longjmp(jmp_buf env, int val), env in r3, val in r4
 * Restores the context saved by setjmp; setjmp returns val (1 if 0).
 */

	.text
	.align 2
	.globl _longjmp
	.globl __longjmp
_longjmp:
__longjmp:
	lfd f14, 192(r3)
	lfd f15, 200(r3)
	lfd f16, 208(r3)
	lfd f17, 216(r3)
	lfd f18, 224(r3)
	lfd f19, 232(r3)
	lfd f20, 240(r3)
	lfd f21, 248(r3)
	lfd f22, 256(r3)
	lfd f23, 264(r3)
	lfd f24, 272(r3)
	lfd f25, 280(r3)
	lfd f26, 288(r3)
	lfd f27, 296(r3)
	lfd f28, 304(r3)
	lfd f29, 312(r3)
	lfd f30, 320(r3)
	lfd f31, 328(r3)
	ld r5, 168(r3)
	mtlr r5
	ld r5, 176(r3)
	mtcrf 0xff, r5
	ld r5, 184(r3)
	mtctr r5
	ld r1, 0(r3)
	ld r2, 8(r3)
	ld r13, 16(r3)
	ld r14, 24(r3)
	ld r15, 32(r3)
	ld r16, 40(r3)
	ld r17, 48(r3)
	ld r18, 56(r3)
	ld r19, 64(r3)
	ld r20, 72(r3)
	ld r21, 80(r3)
	ld r22, 88(r3)
	ld r23, 96(r3)
	ld r24, 104(r3)
	ld r25, 112(r3)
	ld r26, 120(r3)
	ld r27, 128(r3)
	ld r28, 136(r3)
	ld r29, 144(r3)
	ld r30, 152(r3)
	ld r31, 160(r3)
	mr. r3, r4
	bnelr
	li r3, 1
	blr
