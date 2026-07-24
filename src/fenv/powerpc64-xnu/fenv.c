#define _GNU_SOURCE
#include <fenv.h>

static inline double get_fpscr_f(void)
{
	double d;
	__asm__ __volatile__("mffs %0" : "=d"(d));
	return d;
}

static inline long get_fpscr(void)
{
	return (union {double f; long i;}) {get_fpscr_f()}.i;
}

static inline void set_fpscr_f(double fpscr)
{
	__asm__ __volatile__("mtfsf 255, %0" : : "d"(fpscr));
}

static void set_fpscr(long fpscr)
{
	set_fpscr_f((union {long i; double f;}) {fpscr}.f);
}

int feclearexcept(int mask)
{
	long fpscr;
	mask &= FE_ALL_EXCEPT;
	if (mask & FE_INVALID) mask |= FE_ALL_INVALID;
	fpscr = get_fpscr() & ~(long)mask;
	/* Apple Libm compat: clear FX when no exceptions remain */
	if (!(fpscr & FE_ALL_EXCEPT)) fpscr &= ~(1UL<<31);
	set_fpscr(fpscr);
	return 0;
}

int feraiseexcept(int mask)
{
	mask &= FE_ALL_EXCEPT;
	if (mask & FE_INVALID) mask |= FE_INVALID_SOFTWARE;
	set_fpscr(get_fpscr() | mask);
	return 0;
}

int fetestexcept(int mask)
{
	return get_fpscr() & mask & FE_ALL_EXCEPT;
}

int fegetround(void)
{
	return get_fpscr() & 3;
}

int __fesetround(int r)
{
	set_fpscr(get_fpscr() & ~3L | r);
	return 0;
}

int fegetenv(fenv_t *envp)
{
	*envp = get_fpscr();
	return 0;
}

int fesetenv(const fenv_t *envp)
{
	/* Apple ABI: fenv_t is a 32-bit FPSCR image and FE_DFL_ENV points
	 * at the exported zero constant below, so no sentinel check. */
	set_fpscr(*envp);
	return 0;
}

const fenv_t _FE_DFL_ENV = 0;
