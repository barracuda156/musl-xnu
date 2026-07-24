/* Darwin/XNU PowerPC (32-bit) syscall conventions, verified against
 * xnu-1228.15.4 (bsd/dev/ppc/systemcalls.c, osfmk/mach/ppc/syscall_sw.h):
 *
 * BSD syscalls:
 *   - plain (positive) syscall number in r0 -- there is NO 0x2000000
 *     class prefix on PPC; syscall classes only exist on x86_64/arm
 *   - arguments in r3..r10; 64-bit arguments occupy two consecutive
 *     GPRs, high word first (big-endian), with no register-pair
 *     alignment (Darwin/AIX ABI, unlike the Linux ELF ABI)
 *   - kernel pre-increments the saved PC by 4 assuming success, so a
 *     successful syscall returns to sc+8; on error it returns to sc+4
 *     with a positive errno in r3.  Placing a single `neg` at sc+4
 *     yields the -errno convention musl wants, and it is skipped
 *     entirely on success (Apple's Libc branches to cerror there).
 *   - results come back in r3 (and r4 for dual-result syscalls such as
 *     pipe/fork, and the low word of 32-bit-process off_t returns --
 *     those need dedicated wrappers, see src/unistd/powerpc-xnu/)
 *
 * Mach traps: negative trap number in r0, plain `sc`, result in r3,
 * no PC adjustment and no errno transformation.
 */

#define __SYSCALL_LL_E(x) \
((union { long long ll; long l[2]; }){ .ll = x }).l[0], \
((union { long long ll; long l[2]; }){ .ll = x }).l[1]
#define __SYSCALL_LL_O(x) __SYSCALL_LL_E((x))

struct kern_return
{
	long ret;
	long err;
};

#define __SC_CLOBBERS "r11", "r12", "cr0", "ctr", "xer", "memory"

static __inline long __syscall0(long n)
{
	register long r0 __asm__("r0") = n;
	register long r3 __asm__("r3");
	register long r4 __asm__("r4");
	__asm__ __volatile__ (
		"sc\n\t"
		"neg %1, %1"	/* executed only on error (success lands at sc+8) */
		: "+r"(r0), "=r"(r3), "=r"(r4)
		:
		: "r5", "r6", "r7", "r8", "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}

static __inline long __syscall1(long n, long a1)
{
	register long r0 __asm__("r0") = n;
	register long r3 __asm__("r3") = a1;
	register long r4 __asm__("r4");
	__asm__ __volatile__ (
		"sc\n\t"
		"neg %1, %1"
		: "+r"(r0), "+r"(r3), "=r"(r4)
		:
		: "r5", "r6", "r7", "r8", "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}

static __inline long __syscall2(long n, long a1, long a2)
{
	register long r0 __asm__("r0") = n;
	register long r3 __asm__("r3") = a1;
	register long r4 __asm__("r4") = a2;
	__asm__ __volatile__ (
		"sc\n\t"
		"neg %1, %1"
		: "+r"(r0), "+r"(r3), "+r"(r4)
		:
		: "r5", "r6", "r7", "r8", "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}

static __inline long __syscall3(long n, long a1, long a2, long a3)
{
	register long r0 __asm__("r0") = n;
	register long r3 __asm__("r3") = a1;
	register long r4 __asm__("r4") = a2;
	register long r5 __asm__("r5") = a3;
	__asm__ __volatile__ (
		"sc\n\t"
		"neg %1, %1"
		: "+r"(r0), "+r"(r3), "+r"(r4), "+r"(r5)
		:
		: "r6", "r7", "r8", "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}

static __inline long __syscall4(long n, long a1, long a2, long a3, long a4)
{
	register long r0 __asm__("r0") = n;
	register long r3 __asm__("r3") = a1;
	register long r4 __asm__("r4") = a2;
	register long r5 __asm__("r5") = a3;
	register long r6 __asm__("r6") = a4;
	__asm__ __volatile__ (
		"sc\n\t"
		"neg %1, %1"
		: "+r"(r0), "+r"(r3), "+r"(r4), "+r"(r5), "+r"(r6)
		:
		: "r7", "r8", "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}

static __inline long __syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
	register long r0 __asm__("r0") = n;
	register long r3 __asm__("r3") = a1;
	register long r4 __asm__("r4") = a2;
	register long r5 __asm__("r5") = a3;
	register long r6 __asm__("r6") = a4;
	register long r7 __asm__("r7") = a5;
	__asm__ __volatile__ (
		"sc\n\t"
		"neg %1, %1"
		: "+r"(r0), "+r"(r3), "+r"(r4), "+r"(r5), "+r"(r6), "+r"(r7)
		:
		: "r8", "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}

static __inline long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
	register long r0 __asm__("r0") = n;
	register long r3 __asm__("r3") = a1;
	register long r4 __asm__("r4") = a2;
	register long r5 __asm__("r5") = a3;
	register long r6 __asm__("r6") = a4;
	register long r7 __asm__("r7") = a5;
	register long r8 __asm__("r8") = a6;
	__asm__ __volatile__ (
		"sc\n\t"
		"neg %1, %1"
		: "+r"(r0), "+r"(r3), "+r"(r4), "+r"(r5), "+r"(r6), "+r"(r7), "+r"(r8)
		:
		: "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}

/* Mach traps: negative trap number in r0, no error-path PC skipping */

static __inline long __mach_syscall0(long n)
{
	register long r0 __asm__("r0") = -n;
	register long r3 __asm__("r3");
	__asm__ __volatile__ (
		"sc"
		: "+r"(r0), "=r"(r3)
		:
		: "r4", "r5", "r6", "r7", "r8", "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}

static __inline long __mach_syscall1(long n, long a1)
{
	register long r0 __asm__("r0") = -n;
	register long r3 __asm__("r3") = a1;
	__asm__ __volatile__ (
		"sc"
		: "+r"(r0), "+r"(r3)
		:
		: "r4", "r5", "r6", "r7", "r8", "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}

static __inline long __mach_syscall2(long n, long a1, long a2)
{
	register long r0 __asm__("r0") = -n;
	register long r3 __asm__("r3") = a1;
	register long r4 __asm__("r4") = a2;
	__asm__ __volatile__ (
		"sc"
		: "+r"(r0), "+r"(r3), "+r"(r4)
		:
		: "r5", "r6", "r7", "r8", "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}

static __inline long __mach_syscall3(long n, long a1, long a2, long a3)
{
	register long r0 __asm__("r0") = -n;
	register long r3 __asm__("r3") = a1;
	register long r4 __asm__("r4") = a2;
	register long r5 __asm__("r5") = a3;
	__asm__ __volatile__ (
		"sc"
		: "+r"(r0), "+r"(r3), "+r"(r4), "+r"(r5)
		:
		: "r6", "r7", "r8", "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}

static __inline long __mach_syscall4(long n, long a1, long a2, long a3, long a4)
{
	register long r0 __asm__("r0") = -n;
	register long r3 __asm__("r3") = a1;
	register long r4 __asm__("r4") = a2;
	register long r5 __asm__("r5") = a3;
	register long r6 __asm__("r6") = a4;
	__asm__ __volatile__ (
		"sc"
		: "+r"(r0), "+r"(r3), "+r"(r4), "+r"(r5), "+r"(r6)
		:
		: "r7", "r8", "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}

static __inline long __mach_syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
	register long r0 __asm__("r0") = -n;
	register long r3 __asm__("r3") = a1;
	register long r4 __asm__("r4") = a2;
	register long r5 __asm__("r5") = a3;
	register long r6 __asm__("r6") = a4;
	register long r7 __asm__("r7") = a5;
	__asm__ __volatile__ (
		"sc"
		: "+r"(r0), "+r"(r3), "+r"(r4), "+r"(r5), "+r"(r6), "+r"(r7)
		:
		: "r8", "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}

static __inline long __mach_syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
	register long r0 __asm__("r0") = -n;
	register long r3 __asm__("r3") = a1;
	register long r4 __asm__("r4") = a2;
	register long r5 __asm__("r5") = a3;
	register long r6 __asm__("r6") = a4;
	register long r7 __asm__("r7") = a5;
	register long r8 __asm__("r8") = a6;
	__asm__ __volatile__ (
		"sc"
		: "+r"(r0), "+r"(r3), "+r"(r4), "+r"(r5), "+r"(r6), "+r"(r7), "+r"(r8)
		:
		: "r9", "r10", __SC_CLOBBERS
	);
	return r3;
}
