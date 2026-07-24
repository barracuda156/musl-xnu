/* PowerPC Darwin setjmp buffer layout
 * Based on MacOSX10.6.sdk/usr/include/ppc/setjmp.h
 *
 * jmp_buf saves:
 * - GPRs r13-r31 (19 registers)
 * - FPRs f14-f31 (18 registers, each is 8 bytes = 2 words)
 * - CR, LR, CTR, XER
 * - Vector registers (if AltiVec enabled)
 *
 * Total: at least 70 words for basic context
 */
typedef unsigned long __jmp_buf[70];
