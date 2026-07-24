/* Darwin PowerPC floating-point environment.
 * ABI matches MacOSX10.6.sdk architecture/ppc/fenv.h (and Apple Libm):
 * fenv_t/fexcept_t are unsigned int FPSCR images and FE_DFL_ENV points
 * to the exported constant _FE_DFL_ENV; the FE_* values are the FPSCR
 * bit positions.  The FE_INVALID_* breakdown matches the FPSCR too and
 * is used internally by the implementation.
 */

#define FE_INEXACT    0x02000000
#define FE_DIVBYZERO  0x04000000
#define FE_UNDERFLOW  0x08000000
#define FE_OVERFLOW   0x10000000
#define FE_INVALID    0x20000000

#define FE_ALL_EXCEPT 0x3e000000

#define FE_TONEAREST  0
#define FE_TOWARDZERO 1
#define FE_UPWARD     2
#define FE_DOWNWARD   3

#ifdef _GNU_SOURCE
#define FE_INVALID_SNAN		0x01000000
#define FE_INVALID_ISI		0x00800000
#define FE_INVALID_IDI		0x00400000
#define FE_INVALID_ZDZ		0x00200000
#define FE_INVALID_IMZ		0x00100000
#define FE_INVALID_COMPARE	0x00080000
#define FE_INVALID_SOFTWARE	0x00000400
#define FE_INVALID_SQRT		0x00000200
#define FE_INVALID_INTEGER_CONVERSION	0x00000100

#define FE_ALL_INVALID		0x01f80700
#endif

typedef unsigned int fenv_t;
typedef unsigned int fexcept_t;

extern const fenv_t _FE_DFL_ENV;
#define FE_DFL_ENV (&_FE_DFL_ENV)
