/* Darwin PowerPC64 program entry.
 *
 * The kernel enters with r1 pointing at the doubleword holding argc,
 * followed by argv[], NULL, envp[], NULL, apple[].  Protocol expected
 * by crt1.c:
 *   _start_c(long *p, uintptr_t image_base, uintptr_t slide)
 * Darwin ppc64 has no PIE, so the slide is always 0.
 *
 * ppc64 executables link above 4GB, so __image_base must be loaded
 * PC-relatively (bcl 20,31 picbase idiom) -- absolute ha16/lo16
 * addressing cannot reach.  There is no TOC on Darwin.
 */

__asm__(
".data \n"
".align 3 \n"
"__image_base: \n"
".quad __mh_execute_header \n"
".text \n"
".align 3 \n"
".globl start \n"
"start: \n"
"	mr r3, r1 \n"                            /* p = &argc */
"	subi r1, r1, 8 \n"
"	clrrdi r1, r1, 5 \n"                     /* 32-byte align */
"	li r0, 0 \n"
"	std r0, 0(r1) \n"                        /* terminate back chain */
"	stdu r1, -128(r1) \n"                    /* initial stack frame */
"	bcl 20, 31, 1f \n"
"1:	mflr r10 \n"
"	addis r11, r10, ha16(__image_base - 1b) \n"
"	ld r4, lo16(__image_base - 1b)(r11) \n"  /* image base */
"	li r5, 0 \n"                             /* slide (no PIE on ppc) */
"	bl __start_c \n"
);
