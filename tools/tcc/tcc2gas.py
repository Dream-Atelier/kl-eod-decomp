#!/usr/bin/env python3
"""Convert ARM tcc (Norcroft) assembly output to GNU assembler (GAS) syntax.

ARM tcc uses ARM's own assembler syntax (AREA, DCD, EXPORT, etc.)
which is incompatible with GNU as. This script converts the output
to GAS .syntax unified Thumb assembly.

Usage:
    python3 tcc2gas.py input.s output.s [--label-prefix PREFIX]
"""

import re
import sys


def convert_tcc_to_gas(input_path, output_path, label_prefix=""):
    with open(input_path) as f:
        lines = f.readlines()

    gas_lines = []
    gas_lines.append("\t.syntax unified")
    gas_lines.append("\t.code\t16")
    gas_lines.append('\t.include "asm/macros.inc"')
    gas_lines.append("")

    in_code = False
    current_func = None
    exported = set()
    pending_dcds = []

    # First pass: collect EXPORT names
    for line in lines:
        stripped = line.strip()
        m = re.match(r"EXPORT\s+(\w+)", stripped)
        if m:
            exported.add(m.group(1))

    # Second pass: convert instructions
    for line in lines:
        stripped = line.strip()

        # Skip comments, directives we don't need
        if stripped.startswith(";") or not stripped:
            continue
        if stripped in ("CODE16", "END"):
            continue
        if stripped.startswith("AREA "):
            in_code = True
            continue
        if stripped.startswith("|x$codeseg|"):
            continue
        if stripped.startswith("EXPORT") or stripped.startswith("IMPORT"):
            continue

        # Internal labels (F1L10, F2L1, etc.) — branch targets, NOT function entries
        # Must check BEFORE function names since F1L10 also matches the function regex
        m = re.match(r"^(F\d+L\d+)\s*$", stripped)
        if m:
            label = m.group(1)
            if label_prefix:
                label = f"{label_prefix}{label}"
            gas_lines.append(f".L{label}:")
            continue

        # Function labels: no leading whitespace, not an instruction, not an internal label
        if re.match(r"^[A-Za-z_]\w*$", stripped) and not stripped.startswith(("MOV", "LDR", "STR", "ADD", "SUB", "CMP",
                "BEQ", "BNE", "BCS", "BCC", "BMI", "BPL", "BVS", "BVC", "BHI", "BLS",
                "BGE", "BLT", "BGT", "BLE", "B ", "BX", "BL ", "ORR", "AND", "EOR",
                "LSL", "LSR", "ASR", "ROR", "MUL", "NEG", "MVN", "TST", "CMN",
                "PUSH", "POP", "SWI", "NOP", "CODE")):
            func_name = stripped
            if label_prefix:
                func_name = f"{label_prefix}{func_name}"
            gas_lines.append("")
            gas_lines.append(f"\t.align\t2, 0")
            if stripped in exported:
                gas_lines.append(f"\t.globl\t{func_name}")
                gas_lines.append(f"\t.type\t{func_name}, %function")
            gas_lines.append(f"\t.thumb_func")
            gas_lines.append(f"{func_name}:")
            current_func = stripped
            continue

        # DCD (data constant) - literal pool entry
        m = re.match(r"DCD\s+(0x[0-9a-fA-F]+|\d+)", stripped)
        if m:
            val = m.group(1)
            if not val.startswith("0x"):
                val = hex(int(val))
            gas_lines.append(f"\t.word\t{val}")
            continue

        # DCW (data constant word - 16-bit)
        m = re.match(r"DCW\s+(0x[0-9a-fA-F]+|\d+)", stripped)
        if m:
            val = m.group(1)
            if not val.startswith("0x"):
                val = hex(int(val))
            gas_lines.append(f"\t.short\t{val}")
            continue

        # Instructions
        if line[0] == " " or line[0] == "\t":
            instr = convert_instruction(stripped, label_prefix)
            if instr is not None:
                gas_lines.append(f"\t{instr}")
            continue

        # Anything else - pass through as comment
        gas_lines.append(f"@ {stripped}")

    gas_lines.append("")

    with open(output_path, "w") as f:
        f.write("\n".join(gas_lines) + "\n")


# Map of ARM tcc mnemonics to GAS Thumb mnemonics
THUMB_S_INSTRUCTIONS = {
    "MOV": "movs",
    "ADD": "adds",
    "SUB": "subs",
    "AND": "ands",
    "ORR": "orrs",
    "EOR": "eors",
    "LSL": "lsls",
    "LSR": "lsrs",
    "ASR": "asrs",
    "ROR": "rors",
    "MUL": "muls",
    "NEG": "negs",
    "MVN": "mvns",
    "TST": "tst",
    "CMN": "cmn",
    "CMP": "cmp",
    "BIC": "bics",
    "ADC": "adcs",
    "SBC": "sbcs",
}

# Instructions that don't get 's' suffix
THUMB_PLAIN_INSTRUCTIONS = {
    "LDR": "ldr",
    "LDRB": "ldrb",
    "LDRH": "ldrh",
    "LDRSB": "ldrsb",
    "LDRSH": "ldrsh",
    "STR": "str",
    "STRB": "strb",
    "STRH": "strh",
    "PUSH": "push",
    "POP": "pop",
    "BX": "bx",
    "BL": "bl",
    "B": "b",
    "BEQ": "beq",
    "BNE": "bne",
    "BCS": "bcs",
    "BCC": "bcc",
    "BMI": "bmi",
    "BPL": "bpl",
    "BVS": "bvs",
    "BVC": "bvc",
    "BHI": "bhi",
    "BLS": "bls",
    "BGE": "bge",
    "BLT": "blt",
    "BGT": "bgt",
    "BLE": "ble",
    "SWI": "swi",
    "NOP": "nop",
}


def convert_instruction(instr, label_prefix=""):
    """Convert a single ARM asm instruction to GAS syntax."""
    # Split mnemonic and operands
    parts = instr.split(None, 1)
    if not parts:
        return None

    mnemonic = parts[0].upper()
    operands = parts[1] if len(parts) > 1 else ""

    # Convert operands: hex literals &XX -> 0xXX
    operands = re.sub(r"#&([0-9a-fA-F]+)", r"#0x\1", operands)

    # Convert label references in branches: F1L10 -> .LF1L10
    operands = re.sub(r"\b(F\d+L\d+)\b", lambda m: f".L{label_prefix}{m.group(1)}" if label_prefix else f".L{m.group(1)}", operands)

    # Convert register lists: {r0-r3} stays the same in GAS
    # Convert sp-relative: [sp,#XX] stays the same

    # Look up mnemonic
    if mnemonic in THUMB_S_INSTRUCTIONS:
        gas_mnemonic = THUMB_S_INSTRUCTIONS[mnemonic]
        return f"{gas_mnemonic}\t{operands}"
    elif mnemonic in THUMB_PLAIN_INSTRUCTIONS:
        gas_mnemonic = THUMB_PLAIN_INSTRUCTIONS[mnemonic]
        return f"{gas_mnemonic}\t{operands}"
    else:
        # Unknown - pass through lowercase
        return f"{mnemonic.lower()}\t{operands}"


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} input.s output.s [--label-prefix PREFIX]")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]
    label_prefix = ""
    if "--label-prefix" in sys.argv:
        idx = sys.argv.index("--label-prefix")
        label_prefix = sys.argv[idx + 1]

    convert_tcc_to_gas(input_path, output_path, label_prefix)
