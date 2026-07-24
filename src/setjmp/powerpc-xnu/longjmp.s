/* Darwin PowerPC longjmp
 * void longjmp(jmp_buf env, int val), env in r3, val in r4
 * Restores the context saved by setjmp; setjmp returns val (1 if 0).
 */

	.text
	.align 2
	.globl _longjmp
	.globl __longjmp
_longjmp:
__longjmp:
	lfd f14, 96(r3)
	lfd f15, 104(r3)
	lfd f16, 112(r3)
	lfd f17, 120(r3)
	lfd f18, 128(r3)
	lfd f19, 136(r3)
	lfd f20, 144(r3)
	lfd f21, 152(r3)
	lfd f22, 160(r3)
	lfd f23, 168(r3)
	lfd f24, 176(r3)
	lfd f25, 184(r3)
	lfd f26, 192(r3)
	lfd f27, 200(r3)
	lfd f28, 208(r3)
	lfd f29, 216(r3)
	lfd f30, 224(r3)
	lfd f31, 232(r3)
	lwz r5, 84(r3)
	mtlr r5
	lwz r5, 88(r3)
	mtcrf 0xff, r5
	lwz r5, 92(r3)
	mtctr r5
	lwz r1, 0(r3)
	lwz r2, 4(r3)
	lwz r13, 8(r3)
	lwz r14, 12(r3)
	lwz r15, 16(r3)
	lwz r16, 20(r3)
	lwz r17, 24(r3)
	lwz r18, 28(r3)
	lwz r19, 32(r3)
	lwz r20, 36(r3)
	lwz r21, 40(r3)
	lwz r22, 44(r3)
	lwz r23, 48(r3)
	lwz r24, 52(r3)
	lwz r25, 56(r3)
	lwz r26, 60(r3)
	lwz r27, 64(r3)
	lwz r28, 68(r3)
	lwz r29, 72(r3)
	lwz r30, 76(r3)
	lwz r31, 80(r3)
	mr. r3, r4
	bnelr
	li r3, 1
	blr
