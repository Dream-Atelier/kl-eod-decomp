// Shared plumbing for the prove-*.mjs scripts: where the ROM/ELF/savestates
// live, and how to read or write a DWARF-described struct member (including
// bitfields) through the emulator bus.
//
// Nothing here decides *what* a field means — that is each script's job. This
// file only makes sure a script never hand-codes an offset or a mask: every
// address comes from `di.structMember(...)` / `di.symbolToAddress(...)` and is
// decoded here with the formula gba-kit documents for MemberLocation:
//
//     value = (read(offset, size) >>> bitOffset) & (2 ** bitWidth - 1)
//
// so renaming a field in include/ makes the script throw instead of silently
// reading the wrong bits.

import { dirname } from 'node:path';
import { REPO, SAVESTATE } from './gba-kit.mjs';

/** The built ROM (byte-identical to baserom.gba whenever `make compare` says OK). */
export const ROM = `${REPO}/klonoa-eod.gba`;

/** The build's ELF — the only source of symbol addresses and struct layouts. */
export const ELF = `${REPO}/klonoa-eod.elf`;

/** Directory holding the savestates (`savestate-*.json`), next to the default one. */
export const SAVES = dirname(SAVESTATE);

const readN = (bus, addr, size) =>
    size === 1 ? bus.read8(addr) : size === 2 ? bus.read16(addr) : bus.read32(addr) >>> 0;

const writeN = (bus, addr, size, value) =>
    size === 1
        ? bus.write8(addr, value & 0xff)
        : size === 2
          ? bus.write16(addr, value & 0xffff)
          : bus.write32(addr, value >>> 0);

function check(f) {
    if (!f) {
        throw new Error(
            'structMember/symbol lookup returned null — the field was renamed or the ELF has no DWARF for it',
        );
    }
    if (f.size == null) throw new Error(`member ${f.name ?? '?'} has no known size`);
    return f;
}

/**
 * Read a struct member out of guest memory.
 * @param bus   rt.gba.bus
 * @param base  address of the struct instance
 * @param f     a MemberLocation from di.structMember()/di.variableMember()
 */
export function readField(bus, base, f) {
    check(f);
    const raw = readN(bus, base + f.offset, f.size);
    if (f.bitWidth == null) return raw;
    return (raw >>> f.bitOffset) & ((1 << f.bitWidth) - 1);
}

/** Write a struct member in guest memory, preserving the neighbours of a bitfield. */
export function writeField(bus, base, f, value) {
    check(f);
    const addr = base + f.offset;
    if (f.bitWidth == null) {
        writeN(bus, addr, f.size, value);
        return;
    }
    const mask = ((1 << f.bitWidth) - 1) << f.bitOffset;
    const merged = ((readN(bus, addr, f.size) & ~mask) | ((value << f.bitOffset) & mask)) >>> 0;
    writeN(bus, addr, f.size, merged);
}
