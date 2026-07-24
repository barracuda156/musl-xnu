# Review of Sonnet's powerpc-xnu / powerpc64-xnu port (2026-07-24)

Reviewer: Claude (Fable 5). Verified against xnu-1228.15.4 (10.5.8) and Libc-498
sources (downloaded to scratchpad), plus the existing x86_64-xnu port in this repo.
The 10.6 SDK (https://github.com/alexey-lysiuk/macos-sdk/tree/main/MacOSX10.6.sdk)
is the reference for userspace struct layouts (ppc headers are retained there).

## Verdict

The directory/file scaffolding mirrors x86_64-xnu correctly (empty .c files
suppressing generic versions, etc.), and the ~440 copied C files per arch are
mostly verbatim copies of the x86_64-xnu ones (fine, since those are
arch-independent Darwin wrappers). **However, every piece of PPC-specific ABI
glue is wrong** — the port as written cannot make a single successful syscall.
Sonnet invented a "Darwin PPC ABI" by blending the Linux PowerPC convention
(cr0.SO error flag) with the x86_64 Darwin convention (0x2000000 syscall class
in the number). Neither exists on Darwin PPC.

## Ground truth (verified in xnu-1228.15.4 sources)

### Unix syscall convention (bsd/dev/ppc/systemcalls.c)
- Syscall number: **plain BSD number in r0** — there is NO 0x2000000
  (UNIX_SYSCALL) class flag on PPC. Classes were introduced with x86_64.
- Args in r3..r10 (32-bit: 64-bit args occupy two consecutive GPRs, big-endian
  hi:lo, NO even-register alignment — Darwin/AIX ABI, unlike Linux ELF ABI).
- Return: kernel does `save_srr0 += 4` before dispatch (assume success →
  returns to sc+8); on error it does `save_srr0 -= 4` and puts **positive errno
  in r3** (returns to sc+4); ERESTART does `-= 8` (re-executes sc).
  So userspace layout is:
  ```
  sc            # sc
  <error insn>  # sc+4: executed ONLY on error (Apple: branch to cerror)
  <success>     # sc+8: success path
  ```
  For musl the minimal correct sequence is `sc; neg r3,r3` — the neg is skipped
  on success, giving -errno on error. Carry flag / SO bit is NOT involved.
- Dual results: r3 AND r4 (uu_rval[0]/[1]); for 32-bit processes an off_t
  return is split r3=hi, r4=lo (_SYSCALL_RET_OFF_T, e.g. lseek).

### Mach traps (osfmk/mach/ppc/syscall_sw.h)
```
kernel_trap: li r0, trap_number ; sc ; blr
```
Trap number is **negative** in r0 (no 0x1000000 flag). Mach traps return
normally to sc+4, no PC skipping, result in r3, no errno transform.

### Thread pointer / TLS
- Set: fast trap **0x7FF1** (CthreadSetSelfNumber, osfmk/ppc/hw_exception.s):
  `li r0,0x7FF1 ; sc` with the TLDP in r3. Preserves all regs. Works on all CPUs.
- Get (32-bit): call the commpage routine **_COMM_PAGE_PTHREAD_SELF =
  0xFFFF8580** via `bla 0xffff8580` (commpage is at 0xFFFF8000, reachable by
  absolute branch). Kernel installs `mfspr r3,259` (SPRG3) on G5 or the
  ultra-fast trap 0x7FF2 variant on G3/G4 (osfmk/ppc/commpage/pthread.s).
  Do NOT hardcode 0x7FF2 (not handled on 64-bit CPUs) or mfspr 259 (not
  readable on G3/G4). Clobbers: r0, lr (commpage code uses only r0/r3).
- 64-bit processes: the ppc64 Darwin ABI reserves **r13** as the TLDP; read it
  directly. Still use 0x7FF1 in __set_thread_area (harmless, keeps kernel's
  copy in sync) *and* set r13.

### Signal delivery (bsd/dev/ppc/unix_signal.c)
sendsig invokes the user trampoline (sa_tramp from struct __sigaction) with
r3=union __sigaction_u, r4=infostyle, r5=sig, r6=siginfo*, r7=ucontext*.
This matches the 5-arg C __sigtramp used by the (shared, identical) sigaction.c
— so the C-level trampoline mechanism works on PPC unchanged. SYS_sigreturn=184
with (uctx, infostyle).

## Bugs found (fatal unless noted)

1. **arch/powerpc-xnu/syscall_arch.h + arch/powerpc64-xnu/syscall_arch.h**
   - `UNIX_SYSCALL + n` in r0: wrong, must be plain `n`.
   - `bns 1f; neg` (cr0.SO test): wrong, must be skip-slot convention
     (`sc; neg %0,%0` with success landing at sc+8).
   - `__mach_syscallN`: `MACH_SYSCALL + n` wrong → must be `-n` in r0, plain sc.
   - `__SYSCALL_LL_E/_O(x)` identity on ppc32: wrong — must split into hi,lo
     (big-endian, no alignment padding). Identity is correct on ppc64 only.
   - r0 should be "+r" (defensive; kernel preserves it but volatile across UFTs).

2. **src/internal/powerpc-xnu/syscall.s** (`___syscall` variadic veneer)
   - Same two convention bugs, plus `addis 0,0,0x200`, which doesn't add — rA=0
     reads as literal 0, so it *overwrites* r0 with 0x2000000, destroying the
     syscall number even under its own wrong assumptions.
   - Pointless mflr/stw/lwz/mtlr of LR (leaf function, no calls). Harmless.

3. **src/thread/powerpc-xnu/syscall_cp.s**
   - Same syscall-convention and `addis`-clobber bugs.
   - `___cp_begin` is placed after the cancel-flag check → cancellation signal
     arriving between check and sc is lost (musl requires the flag check inside
     the [__cp_begin,__cp_end) window). No `___cp_cancel` symbol.
   - NOTE: x86_64-xnu has NO syscall_cp.s — the port stubs out the whole
     cancellation mechanism with empty .c files. The ppc .s is dead code that
     should simply be deleted for parity (empty syscall_cp.c already present).

4. **TLS: src/thread/powerpc-xnu/__set_thread_area.s + arch/powerpc-xnu/pthread_arch.h**
   - Stores the thread pointer in a single global (`___pthread_self_ptr`) —
     wrong for >1 thread by construction, and even for the errno path it's an
     unnecessary global load. Replace with 0x7FF1 fast trap (set) and commpage
     `bla 0xffff8580` (get); r13 on ppc64. pthread_arch.h's inline asm also
     under-declares clobbers (bl clobbers ctr/cr/volatile regs it doesn't list).

5. **src/setjmp/powerpc-xnu/sigsetjmp.s**
   - Uses env+244/248 as scratch and stores the mask at env+252 — all inside
     the __jb spare area; musl's C-side sigjmp_buf has __fl at 280 and __ss at
     284 (jmp_buf is unsigned long[70] = 280 bytes). siglongjmp reads garbage.
   - Calls _sigprocmask directly instead of the repo's __sigsetjmp_tail scheme;
     clobbers volatile regs without preserving the in-progress jmp_buf contract.
   - Rewrite per musl pattern: stash LR + savemask in __jb spare slots
     (offsets 240/244), call setjmp core, then branch to ___sigsetjmp_tail.
     (src/signal/powerpc-xnu/sigsetjmp.s is a second copy of this file — check
     which one the build uses; base musl has sigsetjmp only in src/signal/.)

6. **arch/powerpc-xnu/crt_arch.h**
   - Passes r3 = post-`stwu` sp, i.e. original_sp-16 → __start_c reads garbage
     argc. Must pass the ORIGINAL sp (which points at argc), then align.
   - `.long ___mh_execute_header` — three underscores; the linker-defined
     symbol is asm `__mh_execute_header` (C _mh_execute_header). Link error.
   - Entry symbol `_start` — Darwin static ld default entry is `start`
     (x86_64-xnu correctly uses bare `start`). Would need -e _start otherwise.
   - Slide computation uses absolute ha16/lo16 for both terms → always 0;
     harmless for non-PIE ppc but pointless. Use bcl 20,31 PC-discovery or
     just pass 0 (ppc Darwin has no PIE).
   - Stack alignment: should be `clrrwi r1,r1,5`-style (16-byte: clrrwi ...,4)
     after saving original sp, plus a zeroed back-chain word per ABI.

7. **arch/powerpc-xnu/atomic_arch.h**
   - `a_barrier` is a **compiler-only barrier** — must be `sync`. Fatal on SMP
     (dual G4/G5).
   - a_cas/a_swap/a_fetch_add/...: no leading barrier, trailing `lwsync` —
     musl semantics need full seq_cst: leading `sync`, trailing `isync`
     (this is exactly what this repo's arch/powerpc/atomic.h does — reuse it,
     atomics are OS-independent). `lwsync` is also not architecturally
     guaranteed on G3/G4 (ISA 2.x L-field); Apple used sync/isync there.
   - a_cas/a_cas_p failure path branches past the barrier → no ordering on
     failed CAS.
   - a_store: needs sync before and after store (or match repo's powerpc port).

8. **src/signal/powerpc-xnu/restore.s** — same `addis` clobber bug + wrong
   flag; on Darwin __restore/__restore_rt are unused (sa_tramp mechanism);
   x86_64-xnu keeps a vestigial stub. Reduce to `li r0,184 ; sc` or delete.

9. **Dual-register-result syscalls** (r3/r4) need custom wrappers on ppc32:
   - `pipe` (fds in r3/r4 — kernel does NOT write user memory);
   - `fork` (r4=1 in child; child must force return 0);
   - `lseek` (64-bit off_t returned split r3:r4 for 32-bit processes);
   - `gettimeofday` (Darwin returns sec/usec in retval regs when tp!=NULL —
     verify against kern_time.c / Libc stub before fixing).
   NOTE: pipe/fork appear equally broken in the existing x86_64-xnu port
   (pipe.c passes the fd array to the kernel, generic fork.c ignores rdx);
   upstream issue to report, not a copy regression. For ppc do them correctly.

10. **src/fenv/powerpc-xnu/fenv.S** — plausible overall (FPSCR via mffs/mtfsf,
    red zone use is legal on Darwin), but must be audited against
    arch/powerpc-xnu/bits/fenv.h FE_* values and musl's expected entry points
    (fegetround/`__fesetround`/feclearexcept/feraiseexcept/fetestexcept/
    fegetenv/fesetenv). Mixed public/underscore naming needs a consistency
    check with what musl C code expects to link.

11. **POWERPC_DARWIN_IMPLEMENTATION.md** — describes the invented ABI
    ("carry flag CR0[SO]", "UNIX_SYSCALL flag") as verified fact and claims
    "Status: Production Ready". Must be rewritten after fixes land.

## Still to verify (checklist for the follow-up pass)

- [ ] bits/signal.h mcontext/ucontext vs 10.6 SDK ppc/_structs.h (sigreturn
      consumes ucontext+mcontext; sizes/offsets must match kernel's sendsig).
- [ ] bits/*.h wholesale diff against 10.6 SDK ppc headers (stat, ioctl,
      termios via generic, fenv FE_* values = PPC FPSCR bits, stdint, limits,
      float.h for 106-bit long double = 128-bit IBM double-double? Darwin ppc
      long double is double-double (106-bit mantissa) with -mlong-double-128,
      but gcc default on Darwin ppc was 128-bit ld since 10.4; musl has no
      ld128-ibm support in 1.1.16 → probably must use 64-bit long double
      (-mlong-double-64) — check what CFLAGS the port sets and that float.h
      matches).
- [ ] bits/syscall.h.in numbers vs xnu-1228 bsd/kern/syscalls.master (BSD
      numbers are arch-independent on Darwin so the x86_64 copy is probably
      fine, but spot-check ~20, esp. anything >= 350 which post-dates 10.5,
      and remove syscalls that don't exist on 10.4/10.5 if targeting those).
- [ ] mach-o.h bits: CPU_TYPE_POWERPC=18, CPU_TYPE_POWERPC64=18|ABI64,
      relocations for reloc.h; check reloc.h content entirely (untouched in
      this review so far).
- [ ] crt/powerpc-xnu/{crti.s,crtn.s,crt1.c?} + Makefile.xnu crt rules
      reference obj/crt/$(ARCH)/crti.o built from .s — verify files exist and
      syntax assembles (Darwin cctools as vs GNU as directives).
- [ ] src/mach/powerpc-xnu: mach_timebase_info.c exists but x86_64 also has
      __mach_absolute_time.s — ppc lacks it; mach_absolute_time on ppc =
      commpage 0xFFFF8200 (or mftb loop). Check what callers need.
- [ ] env/powerpc-xnu/__init_tls.c: confirm it calls __set_thread_area with
      the right TP_ADJ so errno/__pthread_self work from program start.
- [ ] The 445-file C tree: verify each file is byte-identical to its
      x86_64-xnu counterpart (or justified where it differs); flag any file
      Sonnet "adapted" beyond copying, since those are where hallucinations
      hide. Same for powerpc64-xnu vs powerpc-xnu (64-bit type diffs only).
- [ ] Makefile / configure diffs: ARCH plumbing looks reasonable (ARCH=,
      DESTROOT support, mkdir -p in install rules); confirm powerpc-xnu and
      powerpc64-xnu added to configure arch detection and Makefile.xnu works
      with ARCH override; check the 4 patch files in diffs/ are regenerable.
- [ ] setjmp: consider saving VRSAVE + v20-v31 when AltiVec present (Apple
      setjmp does); acceptable to defer, but document.
- [ ] powerpc64-xnu asm files: full review pass mirroring everything above
      (std/ld vs stw/lwz offsets doubled, jmp_buf sizing, r13 TLS, commpage
      64-bit addresses: _COMM_PAGE64_BASE_ADDRESS is also 0xFFFF8000 on ppc).

## Fix status (2026-07-24 session, all fixes by hand against xnu sources)

FIXED — both arches unless noted:
1. arch/*/syscall_arch.h: real XNU convention (plain r0 number, sc+4
   error/neg, sc+8 success), Mach traps as -n, __SYSCALL_LL_E/O split
   hi/lo on ppc32, r0/r3/r4 properly in-out (kernel writes r4).
2. src/internal/*/syscall.s: rewritten (also fixes the addis-clobbers-r0
   bug); Darwin register-prefixed syntax (rN/fN) per Sergey — cctools
   requires prefixes, bare numbers do not assemble.
3. TLS: __set_thread_area.s = fast trap 0x7FF1 (+r13 on ppc64);
   pthread_arch.h = commpage bla 0xffff8580 (ppc32) / r13 (ppc64);
   global-variable pseudo-TLS removed.
4. atomic_arch.h: replaced with the repo's Linux powerpc/powerpc64
   versions verbatim (pure ISA: sync/isync + lwarx/stwcx, ldarx/stdcx).
5. crt_arch.h: entry symbol `start`, original sp passed as p, Apple-Csu
   frame setup, correct __mh_execute_header ref; ppc64 uses bcl 20,31
   picbase (loads above 4GB, no TOC on Darwin). Added missing
   crt/powerpc{,64}-xnu/crt1.c (copy of x86_64-xnu) + empty rcrt1.c.
6. setjmp.s/longjmp.s: prefixed syntax, mtcrf; layout kept
   (ppc32: regs 0..239 of jmp_buf[70]; ppc64: 0..335 of [70]*8).
7. sigsetjmp.s rewritten with the musl tail scheme (spare slots 240/244
   resp. 336/344; tail-calls ___sigsetjmp_tail; siglongjmp stays plain
   longjmp). Deleted duplicate src/setjmp/*/sigsetjmp.s (was a broken
   sigsetjmp) and the misnamed siglongjmp-in-sigsetjmp.s duplicate
   symbol problem in src/signal/.
8. Deleted src/thread/*/syscall_cp.s (dead + broken; port stubs
   cancellation, parity with x86_64-xnu).
9. restore.s: minimal vestigial stub (sa_tramp mechanism is what's used).
10. Dual-register-result wrappers added: pipe.c, lseek.c (ppc32 r3:r4
    off_t), clock_gettime.c (10.4/10.5 gettimeofday returns sec/usec in
    r3/r4, no copyout), src/process/*/fork.c (r4 child flag),
    __mach_absolute_time.s (mftbu/mftb loop; ppc64 mftb).
11. bits/fenv.h + bits/float.h were raw x86 copies. float.h: replaced
    with Linux ppc ones (LDBL=53 → build REQUIRES -mlong-double-64,
    see configure note below). fenv: rebuilt to the APPLE ABI per
    Sergey (10.6 SDK architecture/ppc/fenv.h + apple-libm
    Source/PowerPC/fenv.c): fenv_t/fexcept_t = unsigned int FPSCR
    image (not musl-Linux's double), FE_DFL_ENV = &_FE_DFL_ENV
    exported zero constant (not a -1 sentinel); FE_* values are FPSCR
    bits (these happened to match musl already). Implementations
    follow Apple semantics: feclearexcept clears FX when no flags
    remain; feraiseexcept raises INVALID via VXSOFT; fegetenv/fesetenv
    move the 32-bit word; added per-arch feholdexcept.c that clears
    exception ENABLE bits too (musl's generic only clears flags).
    ppc32 = Darwin-syntax fenv.s (+__FE_DFL_ENV in .const); ppc64 =
    adapted musl fenv.c. Deleted bogus fenv-sf.c. Remaining nuance for
    the audit: Apple's fe{get,set}exceptflag preserve the individual
    invalid-cause bits, musl's generic ones roundtrip via
    fetestexcept/feraiseexcept (VXSOFT) — functionally equivalent VX
    state, cause bits differ.
12. __arch_entropy.c contained verbatim x86 cpuid/rdrand asm (would not
    compile); replaced with mftb timebase read.

NOT yet done (delegation plan in POWERPC_XNU_TODO_SONNET.md):
- everything in "Still to verify" above, plus:
- configure: long-double check (line ~701) fails for Darwin ppc default
  double-double; either auto-append -mlong-double-64 for
  *-apple-darwin* powerpc* or document CFLAGS requirement. The
  powerpc64 ELFv2 check is skipped (ARCH=powerpc64-xnu) — fine.
- SYS_gettid=286 on Darwin is gettid(uid_t*,gid_t*) (per-thread ident
  override), NOT a thread-id; __init_tp/fork store its (error) return
  into td->tid. Same latent bug exists in x86_64-xnu; decide a fix
  port-wide (e.g. Mach thread port or 372 thread_selfid on 10.6+).
- bits/signal.h mcontext is an opaque __space[256]; OK while
  pthread_cancel is stubbed (MC_PC unused), but should become the real
  10.6-SDK ppc _STRUCT_MCONTEXT (es/ss/fs/vs) so MC_PC srr0 works.
- No assembler was available in this session: everything needs an
  assemble test with real cctools (as -arch ppc/ppc64).
