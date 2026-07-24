#include <fenv.h>

/* C99 non-stop mode per Apple Libm: save the environment, then clear
 * both the exception flags (FX, summary+flags+invalid causes,
 * 0xfff80700) and the exception enable bits (0xf8), unlike musl's
 * generic version which only clears the flags. */

int feholdexcept(fenv_t *envp)
{
	union {double f; unsigned int i[2];} u;
	__asm__ __volatile__("mffs %0" : "=d"(u.f));
	*envp = u.i[1];
	u.i[1] &= ~0xfff807f8u;
	__asm__ __volatile__("mtfsf 255, %0" : : "d"(u.f));
	return 0;
}
