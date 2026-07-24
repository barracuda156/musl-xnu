# powerpc-xnu follow-up plan (for a Sonnet session)

Context: read `POWERPC_XNU_REVIEW.md` first. The ABI-critical glue
(syscalls, TLS, atomics, crt, setjmp family, fenv, dual-register
wrappers) has already been fixed by hand against xnu-1228.15.4 — do
NOT redesign any of it. This file lists the remaining routine,
mechanical work. Ground rules:

- Reference for userspace structs/constants: MacOSX10.6.sdk ppc headers
  (https://github.com/alexey-lysiuk/macos-sdk/tree/main/MacOSX10.6.sdk)
  — 10.6 retains the ppc headers. Reference for kernel behavior:
  xnu-1228.15.4 (github apple-oss-distributions/xnu, tag tarball via
  codeload; opensource.apple.com/source is dead).
- Darwin asm syntax everywhere: registers MUST be prefixed (r3, f14),
  symbols get a leading underscore per C name (C `__syscall` = asm
  `___syscall`), cctools directives only (.globl, .align N as power of
  2, .space; no .type/.size/.hidden/@relocs/TOC).
- The x86_64-xnu port is the structural template but NOT gospel — it
  has its own latent bugs (pipe/fork dual-register returns,
  SYS_gettid=286 misuse). Don't copy bugs; flag them.
- musl-xnu convention: an empty .c file in src/<dir>/<arch>/ suppresses
  the generic implementation; an arch .s/.c with the same basename
  replaces it. Never have two arch files with the same basename.

## Task list (in order)

1. **bits/ header audit, file by file, both arches** — diff every
   header in arch/powerpc-xnu/bits/ and arch/powerpc64-xnu/bits/
   against (a) the 10.6 SDK ppc equivalents for kernel-facing layouts
   and (b) arch/powerpc{,64}/bits/ for musl-internal expectations:
   - signal.h: replace the opaque `__space[256]` mcontext_t with the
     real layout: struct with ppc_exception_state, ppc_thread_state
     (srr0, srr1, r0..r31, cr, xer, lr, ctr, mq, vrsave),
     ppc_float_state, ppc_vector_state — field names so that
     `uc_mcontext.MC_PC` (= srr0 on a pointer deref matching how
     pthread_arch.h uses it) can work later; sizes must match the SDK
     (_STRUCT_MCONTEXT). Keep ucontext_t shape as-is (it matches
     Darwin). Verify sigaltstack, sigset_t (32-bit), SIG* constants
     against the SDK; the shared include/signal.h already carries
     Darwin values under __APPLE__.
   - stat.h: verify struct stat field types/order against 10.6 SDK
     sys/stat.h for a 32-bit BE process (ino_t 64 vs 32, time fields,
     st_flags/st_gen) and against what src/stat/*-xnu C files expect.
   - alltypes.h.in: 32-bit: check wchar_t (int), time_t, off_t (64),
     suseconds_t (int32 on Darwin ppc32!), blksize_t, ino_t, dev_t,
     id_t against the SDK; 64-bit variants likewise.
   - limits.h, stdint.h, posix.h: word-size sanity (LONG_MAX etc. for
     each arch); these were copied from x86_64 so the 32-bit variant is
     the one likely wrong.
   - ioctl.h, termios.h(-in-generic), socket.h, mman.h, sysctl.h:
     Darwin constants are arch-independent — just confirm nothing
     x86-specific (e.g. struct layouts with 64-bit longs) leaked into
     the 32-bit copies.
   - endian.h: must say big-endian (already does; just confirm both).
   - mach-o.h: ppc CPU types already correct; verify the reloc/header
     structs vs the SDK mach-o/loader.h and that FIX_MACHO tooling (see
     Makefile.xnu) copes with ppc cputype.
   - setjmp.h: leave layout alone (jmp_buf[70] both arches; asm depends
     on it). Only confirm sigjmp_buf offsets: ppc32 __fl at 280, __ss
     at 284; ppc64 __fl at 560, __ss at 568.
2. **syscall.h.in trim** — diff against xnu-1228.15.4
   bsd/kern/syscalls.master: remove numbers that don't exist on
   10.4/10.5 (anything Snow-Leopard+ such as 372 thread_selfid), mark
   or drop entries where the 32-bit ABI differs. BSD numbers are
   arch-independent on Darwin, so this is the same table for both
   arches. While there: decide what to do about SYS_gettid (Darwin 286
   is gettid(uid_t*,gid_t*) — NOT a thread id; __init_tp and fork
   currently stuff its error return into td->tid; acceptable interim:
   define a helper that returns getpid() or the mach thread port, and
   fix x86_64-xnu the same way in a separate commit).
3. **powerpc64-xnu tree cross-check** — run the same byte-compare I ran
   for ppc32 (see review doc): every C file in src/*/powerpc64-xnu/
   should be byte-identical to its powerpc-xnu sibling except where
   64-bit genuinely differs (fenv.c, lseek.c). Investigate any other
   divergence.
3b. **fenv flag-function fidelity (optional)** — fenv now follows the
   Apple ABI (see review doc item 11; references: 10.6 SDK
   architecture/ppc/fenv.h, github simonbyrne/apple-libm
   Source/PowerPC/fenv.c). If exact Apple behavior is wanted for
   fegetexceptflag/fesetexceptflag (preserving FE_ALL_INVALID cause
   bits, FE_SET_FX/FE_CLR_FX management), add powerpc-xnu overrides of
   those two generic files following Apple's _fe{get,set}exceptflag;
   otherwise musl's generic roundtrip is acceptable. Do NOT change
   fenv_t/fexcept_t/FE_DFL_ENV — the ABI is settled.
4. **Assembler smoke test** — with a real cctools (`as -arch ppc`,
   `as -arch ppc64`) assemble every .s under src/*/powerpc*-xnu/ and
   both crt_arch.h blobs (compile crt1.c with the cross gcc). Fix
   syntax fallout only; do not change semantics. Watch: `bla
   0xffff8580` and `ba` absolute forms, `.space 8` inside .text,
   `mtcrf 0xff,rN`, extended mnemonics (clrrwi/clrlwi/mr.).
5. **configure** — after the Darwin ARCH suffix hunk, add for
   powerpc*-apple-darwin*: append `-mlong-double-64` to CFLAGS_AUTO (or
   fail with a clear message); the stock long-double probe at ~line 701
   otherwise rejects Darwin's default double-double. Confirm
   `./configure --target=powerpc-apple-darwin9` selects powerpc-xnu and
   completes.
6. **Build test** — `./configure ARCH=powerpc-xnu` (cross) + `make -f
   Makefile.xnu` (check which makefile the port actually uses; the
   x86_64 flow is the model). Then the example/ binaries on a real
   Tiger/Leopard box or under a PPC VM: static hello world, then
   argv/env printing, errno paths (open of nonexistent file →
   ENOENT=2), pipe/fork/lseek/gettimeofday behaviors, setjmp/longjmp
   and sigsetjmp/siglongjmp mask behavior, signal delivery.
7. **Docs + patches** — rewrite POWERPC_DARWIN_IMPLEMENTATION.md to
   describe the REAL ABI (crib from POWERPC_XNU_REVIEW.md "Ground
   truth"); delete its invented claims (carry flag, UNIX_SYSCALL
   prefix, global-pointer TLS, "Production Ready"). Regenerate
   diffs/000*.patch from the final tree.
8. **Optional/later** — AltiVec v20-v31+VRSAVE in setjmp when
   `_COMM_PAGE_ALTIVEC` says available (Apple setjmp does this);
   proper Darwin mcontext-based pthread_cancel once threading is
   unstubbed; commpage bcopy/memset as string-op fast paths.

Anything that looks like it needs an ABI decision (not listed here):
stop and ask, don't invent.
