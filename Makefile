include config.mk

MAKEFLAGS += --no-print-directory

.SUFFIXES:
.SECONDARY:
.DELETE_ON_ERROR:
.SECONDEXPANSION:

ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

ifeq ($(OS),Windows_NT)
EXE := .exe
else
EXE :=
endif

SHELL  := /bin/bash -o pipefail
SHA1   := $(shell { command -v sha1sum || command -v shasum; } 2>/dev/null) -c

### TOOLCHAIN ###

PREFIX  := arm-none-eabi-
CC      := $(PREFIX)gcc
CC1     := tools/agbcc/bin/agbcc$(EXE)
CC1_OLD := tools/agbcc/bin/old_agbcc$(EXE)
CPP     := $(PREFIX)cpp
LD      := $(PREFIX)ld
OBJCOPY := $(PREFIX)objcopy
AS      := $(PREFIX)as

### FILES ###

OBJ_DIR := build
ROM     := $(BUILD_NAME).gba
ELF     := $(BUILD_NAME).elf
MAP     := $(BUILD_NAME).map

ASM_SUBDIR   := asm
ASM_BUILDDIR := $(OBJ_DIR)/$(ASM_SUBDIR)

C_SUBDIR   := src
C_BUILDDIR := $(OBJ_DIR)/$(C_SUBDIR)

DATA_SUBDIR   := data
DATA_BUILDDIR := $(OBJ_DIR)/$(DATA_SUBDIR)

# asm/m4a0.s is .include'd into src/m4a.c (handcrafted MP2K blob);
# it must not be assembled as a standalone .o.
ASM_SRCS := $(filter-out $(ASM_SUBDIR)/m4a0.s,$(wildcard $(ASM_SUBDIR)/*.s))
ASM_OBJS := $(patsubst $(ASM_SUBDIR)/%.s,$(ASM_BUILDDIR)/%.o,$(ASM_SRCS))

C_SRCS := $(wildcard $(C_SUBDIR)/*.c)
C_OBJS := $(patsubst $(C_SUBDIR)/%.c,$(C_BUILDDIR)/%.o,$(C_SRCS))

DATA_SRCS := $(wildcard $(DATA_SUBDIR)/*.s)
DATA_OBJS := $(patsubst $(DATA_SUBDIR)/%.s,$(DATA_BUILDDIR)/%.o,$(DATA_SRCS))

# The DWARF types-sidecar: ctx.c (a generated #include list over every project header)
# compiled by modern gcc with full debug info. It is NOT linked into the game ELF:
# it is built with -mabi=apcs-gnu (agbcc follows the old APCS, which rounds every struct
# up to a word multiple — modern AAPCS does not), and ld refuses to mix APCS/AAPCS EABI
# objects. Instead `make asmlift-elf` derives $(SYMS_ELF), a copy of the built ELF with
# the sidecar's debug sections merged in via objcopy (names from .symtab, declaration
# shapes from the DWARF). Consumed via decomp.yaml tools.asmlift.elf.
CTX_OBJ  := $(OBJ_DIR)/ctx.o
SYMS_ELF := $(BUILD_NAME)-syms.elf

OBJS     := $(C_OBJS) $(ASM_OBJS) $(DATA_OBJS)
OBJS_REL := $(patsubst $(OBJ_DIR)/%,%,$(OBJS))

### FLAGS ###

ASFLAGS  := -mcpu=arm7tdmi -mthumb-interwork
CPPFLAGS := -nostdinc -I tools/agbcc/include -iquote include
CC1FLAGS := -mthumb-interwork -Wimplicit -Wparentheses -O2 -fhex-asm -fprologue-bugfix
# -g: agbcc records each compiled function's DWARF — declaration shapes for the globals it
# sees and a signature for every function it compiles. Byte-neutral (debug sections are
# non-alloc; the ROM sha1 is unchanged), and it is what lets asmlift's symbol map carry
# CALLEE signatures for the functions already decompiled here.
CC1FLAGS += -g
# Agent-oriented instrumentation: comments-only / stderr-only, never alters
# emitted bytes. Opt-out with AGENT_INSTRUMENT=0.
AGENT_INSTRUMENT ?= 1
ifeq ($(AGENT_INSTRUMENT),1)
CC1FLAGS += -finstrument-src-locs -fdump-function-size -fdump-reg-lifetimes -fdump-pool-literals
endif

DECOMP_TOML := klonoa-eod-decomp.toml
LDSCRIPT    := ldscript.txt
LDSCRIPT_IN := ldscript.in.txt

LIBS :=

### FORMATTER ###

FORMAT := clang-format
FORMAT_SRCS := $(shell find src include -name "*.c" -o -name "*.h")

### CONTEXT ###

C_HEADERS := $(shell find include -name "*.h" -not -name "include_asm.h")

### TARGETS ###

.PHONY: all rom compare clean tidy format check_format ctx asmlift-elf

$(shell mkdir -p $(ASM_BUILDDIR) $(C_BUILDDIR) $(DATA_BUILDDIR))

all: compare

# Always builds from scratch. Verifying an incremental build is not trustworthy: the ELF and the
# ROM live outside build/, so make can skip relinking and the SHA1 check then passes against the
# PREVIOUS ROM. Demonstrated by breaking a source file and running `rm -rf build && make compare` --
# it still printed "klonoa-eod.gba: OK".
#
# A clean build is ~1s here against ~0.1s incremental, and a verification whose answer can be wrong
# is not worth 0.9s. Use `make rom` when you want a fast incremental build and are not asking
# whether it matches.
#
# Recursive because prerequisites have no guaranteed order under -j, and because each sub-make
# re-runs the mkdir of the build dirs that tidy just undid. -j is inherited through MAKEFLAGS.
compare:
	@$(MAKE) tidy
	@$(MAKE) rom
	$(SHA1) $(BUILD_NAME).sha1

rom: $(ROM)

$(ELF): $(OBJS) $(LDSCRIPT)
	@echo "$(LD) -T $(LDSCRIPT) -Map $(MAP) <objects> -o $@"
	@cd $(OBJ_DIR) && $(LD) -T ../$(LDSCRIPT) -Map ../$(MAP) $(OBJS_REL) -o ../$(ELF) $(LIBS)

$(ROM): $(ELF)
	$(OBJCOPY) -O binary $< $@

# Generate ldscript.txt by prepending symbol aliases from the TOML [renames] section
$(LDSCRIPT): $(LDSCRIPT_IN) $(DECOMP_TOML)
	@python3 scripts/generate_ldscript.py $(DECOMP_TOML) $(LDSCRIPT_IN) $(LDSCRIPT)

### RECIPES ###

# Assemble standalone .s files (crt0, libgcc, rom_header)
$(ASM_BUILDDIR)/%.o: $(ASM_SUBDIR)/%.s
	@echo "$(AS) <flags> -o $@ $<"
	@$(AS) $(ASFLAGS) -o $@ $<

# Compile m4a with old_agbcc — Nintendo's MusicPlayer2000 was prebuilt
# with an older GCC as part of the GBA SDK.
$(C_BUILDDIR)/m4a.o: $(C_SUBDIR)/m4a.c
	@echo "$(CC1_OLD) <m4a flags> -o $@ $<"
	@$(CPP) $(CPPFLAGS) $< -o $(C_BUILDDIR)/m4a.i
	@$(CC1_OLD) -mthumb-interwork -O2 -o $(C_BUILDDIR)/m4a.s $(C_BUILDDIR)/m4a.i
	@printf ".text\n\t.align\t2, 0\n" >> $(C_BUILDDIR)/m4a.s
	@$(AS) $(ASFLAGS) -o $@ $(C_BUILDDIR)/m4a.s

# Compile eeprom with old_agbcc -O1
$(C_BUILDDIR)/eeprom.o: $(C_SUBDIR)/eeprom.c
	@echo "$(CC1_OLD) <eeprom flags> -o $@ $<"
	@$(CPP) $(CPPFLAGS) $< -o $(C_BUILDDIR)/eeprom.i
	@$(CC1_OLD) -mthumb-interwork -O1 -fhex-asm -o $(C_BUILDDIR)/eeprom_raw.s $(C_BUILDDIR)/eeprom.i
	@awk '/^[[:space:]]*\.section[[:space:]]+\.rodata/{drop=1; next} drop && /^\.text/{drop=0; print; next} !drop' $(C_BUILDDIR)/eeprom_raw.s > $(C_BUILDDIR)/eeprom.s
	@printf ".text\n\t.align\t2, 0\n" >> $(C_BUILDDIR)/eeprom.s
	@$(AS) $(ASFLAGS) -o $@ $(C_BUILDDIR)/eeprom.s

# Compile C files (with INCLUDE_ASM support)
$(C_BUILDDIR)/%.o: $(C_SUBDIR)/%.c
	@echo "$(CC1) <flags> -o $@ $<"
	@$(CPP) $(CPPFLAGS) $< -o $(C_BUILDDIR)/$*.i
	@$(CC1) $(CC1FLAGS) -o $(C_BUILDDIR)/$*.s $(C_BUILDDIR)/$*.i
	@printf ".text\n\t.align\t2, 0\n" >> $(C_BUILDDIR)/$*.s
	@$(AS) $(ASFLAGS) -o $@ $(C_BUILDDIR)/$*.s

# Assemble data files
$(DATA_BUILDDIR)/%.o: $(DATA_SUBDIR)/%.s
	@echo "$(AS) <flags> -o $@ $<"
	@$(AS) $(ASFLAGS) -o $@ $<

# ctx.c
ctx.c: $(C_HEADERS)
	@for header in $(C_HEADERS); do echo "#include \"$$header\""; done > $@
	@echo "Generated ctx.c ($$(wc -l < ctx.c) lines)"

# The sidecar's ONE remaining job is macro names: agbcc's own -g DWARF (above) carries the
# types and signatures, but agbcc cannot emit macro info at all, and this project names many
# fixed RAM cells with address-cast macros rather than externs.
#   -g3            record macro definitions
#   -gdwarf-2 -gstrict-dwarf  emit .debug_macinfo (inline strings, ONE self-contained section)
#                  rather than DWARF-5 .debug_macro, which splits across COMDAT groups and
#                  references .debug_str — neither of which survives a section graft.
$(CTX_OBJ): ctx.c
	@echo "$(CC) -g3 <macro-sidecar> -o $@"
	@$(CC) $(CPPFLAGS) -mabi=apcs-gnu -gdwarf-2 -g3 -gstrict-dwarf -fno-eliminate-unused-debug-types -c $< -o $@

ctx: ctx.c

# Derive the asmlift symbol-source ELF: the built ELF (which already carries agbcc's own
# -g DWARF: types AND per-function signatures) plus ONLY the sidecar's macro table.
# Grafting the sidecar's .debug_info/.debug_str here would duplicate section names the built
# ELF now owns, and a reader takes the first — silently shadowing the richer agbcc DWARF.
# Debug sections are non-alloc; objcopy -O binary dumps only alloc sections, so flag
# each one alloc first, then graft it onto the copy.
$(SYMS_ELF): $(ELF) $(CTX_OBJ)
	@cp $(ELF) $@
	@for sec in .debug_macinfo; do \
		$(OBJCOPY) -O binary --only-section=$$sec \
			--set-section-flags $$sec=alloc $(CTX_OBJ) $(OBJ_DIR)/ctx$$sec.bin; \
		$(OBJCOPY) --add-section $$sec=$(OBJ_DIR)/ctx$$sec.bin $@; \
		rm -f $(OBJ_DIR)/ctx$$sec.bin; \
	done
	@echo "built $@"

asmlift-elf: $(SYMS_ELF)

format:
	$(FORMAT) -i -style=file $(FORMAT_SRCS)

check_format:
	$(FORMAT) -style=file --dry-run --Werror $(FORMAT_SRCS)

clean: tidy

tidy:
	$(RM) -r build
	$(RM) $(BUILD_NAME).gba $(BUILD_NAME).elf $(BUILD_NAME).map $(SYMS_ELF)
