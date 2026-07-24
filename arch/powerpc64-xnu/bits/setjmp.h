/* PowerPC64 Darwin setjmp buffer layout
 * Based on MacOSX10.6.sdk/usr/include/ppc/setjmp.h
 *
 * 64-bit version saves:
 * - GPRs r13-r31 (19 registers, 8 bytes each)
 * - FPRs f14-f31 (18 registers, 8 bytes each)
 * - CR, LR, CTR, XER (8 bytes each on 64-bit)
 * - Vector registers (if AltiVec enabled)
 *
 * Total: at least 70 doublewords
 */
typedef unsigned long __jmp_buf[70];
