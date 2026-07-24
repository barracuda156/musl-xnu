#include <fenv.h>

/* C99 non-stop mode per Apple Libm: save the environment, then clear
 * both the exception flags (FX, summary+flags+invalid causes,
 * 0xfff80700) and the exception enable bits (0xf8), unlike musl's
 * generic version which only clears the flags. */

static inline double get_fpscr_f(void)
{
	double d;
	__asm__ __volatile__("mffs %0" : "=d"(d));
	return d;
}

static inline void set_fpscr_f(double fpscr)
{
	__asm__ __volatile__("mtfsf 255, %0" : : "d"(fpscr));
}

int feholdexcept(fenv_t *envp)
{
	long fpscr = (union {double f; long i;}) {get_fpscr_f()}.i;
	*envp = fpscr;
	set_fpscr_f((union {long i; double f;}) {fpscr & ~0xfff807f8L}.f);
	return 0;
}
