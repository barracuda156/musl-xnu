/* Darwin PowerPC program entry.
 *
 * The kernel enters with r1 pointing at the word holding argc,
 * followed by argv[], NULL, envp[], NULL, apple[] (same layout the
 * x86_64 port consumes).  Protocol expected by crt1.c:
 *   _start_c(long *p, uintptr_t image_base, uintptr_t slide)
 * Darwin ppc has no PIE, so the slide is always 0.
 *
 * Frame setup follows Apple's Csu: save the original sp, step below
 * it before aligning so the argc word cannot be overwritten, null-
 * terminate the frame back chain, then allocate an initial frame.
 */

__asm__(
".data \n"
".align 2 \n"
"__image_base: \n"
".long __mh_execute_header \n"
".text \n"
".align 2 \n"
".globl start \n"
"start: \n"
"	mr r3, r1 \n"                       /* p = &argc */
"	subi r1, r1, 4 \n"
"	clrrwi r1, r1, 5 \n"                /* 32-byte align */
"	li r0, 0 \n"
"	stw r0, 0(r1) \n"                   /* terminate back chain */
"	stwu r1, -64(r1) \n"                /* initial stack frame */
"	lis r4, ha16(__image_base) \n"
"	lwz r4, lo16(__image_base)(r4) \n"  /* image base */
"	li r5, 0 \n"                        /* slide (no PIE on ppc) */
"	bl __start_c \n"
);
