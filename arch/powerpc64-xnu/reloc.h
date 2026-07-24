#define LDSO_ARCH "powerpc64"

/* PowerPC64 relocation types - these would normally come from elf.h
 * but Darwin uses Mach-O, so these are placeholders for compatibility */
#define R_PPC64_NONE          0
#define R_PPC64_ADDR32        1
#define R_PPC64_ADDR24        2
#define R_PPC64_ADDR16        3
#define R_PPC64_ADDR16_LO     4
#define R_PPC64_ADDR16_HI     5
#define R_PPC64_ADDR16_HA     6
#define R_PPC64_ADDR14        7
#define R_PPC64_REL24         10
#define R_PPC64_REL14         11
#define R_PPC64_ADDR64        38
#define R_PPC64_COPY          19
#define R_PPC64_GLOB_DAT      20
#define R_PPC64_JMP_SLOT      21
#define R_PPC64_RELATIVE      22

#define REL_SYMBOLIC    R_PPC64_ADDR64
#define REL_GOT         R_PPC64_GLOB_DAT
#define REL_PLT         R_PPC64_JMP_SLOT
#define REL_RELATIVE    R_PPC64_RELATIVE
#define REL_COPY        R_PPC64_COPY

/* Note: Darwin PowerPC64 uses Mach-O format, not ELF, so TLS relocations
 * work differently than on Linux */
#define REL_DTPMOD      0
#define REL_DTPOFF      0
#define REL_TPOFF       0
#define REL_TLSDESC     0

#define CRTJMP(pc,sp) __asm__ __volatile__( \
	"mr 1, %1 ; mtctr %0 ; bctr" : : "r"(pc), "r"(sp) : "memory" )

#define GETFUNCSYM(fp, sym, got) __asm__ ( \
	"bl 1f\n" \
	"1: mflr %0\n" \
	"addis %0, %0, ha16(" #sym " - 1b)\n" \
	"addi %0, %0, lo16(" #sym " - 1b)\n" \
	: "=r"(*fp) : : "memory" )
