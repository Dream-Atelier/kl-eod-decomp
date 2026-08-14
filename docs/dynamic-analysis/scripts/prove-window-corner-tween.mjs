// PROOF (runtime): StreamCmd_InitWindowCornerMotion (0x0804D798) has nothing to do
// with palettes. It animates one CORNER of one hardware window: a linear tween on a
// pair of gLevelStatePtr window-clip bounds, which UpdateAffineRegisters packs into
// REG_WIN0H / REG_WIN1H / REG_WIN0V / REG_WIN1V.
//
// The four claims under test:
//   C1. The command writes no palette RAM, no blend register, no mosaic register.
//   C2. Stream byte[3] selects a corner: bit 1 picks leftTop vs rightBottom, bit 0
//       picks window 0 vs window 1 (the same encoding StreamCmd_SetWindowCorner uses).
//   C3. The command's dX drives the HORIZONTAL component of that corner and its dY
//       drives the VERTICAL one — so dX reaches a WINxH register and dY a WINxV one.
//   C4. Those registers are the ones include/io_reg.h names, in the order the ROM's
//       own pointer walk implies.
//
// C4 first, because it is the cheap proof and it is the one this project has got
// wrong before: an earlier round shipped a script with WIN0V and WIN1H transposed, so
// the observations were right and the labels were not. UpdateAffineRegisters
// (0x0804B2EC) loads r5 = 0x04000040 once and walks it +4, -2, +4 while consuming
// the bounds block in order. That walk is disassembled below and each stop is looked
// up in include/io_reg.h by ADDRESS, so the label cannot be typed in by hand.
//
// C1..C3 are then tested by causal intervention. The gfx-stream dispatcher does not
// run in any reachable savestate, so this freezes a real in-level state (no frames
// advance, nothing else can touch a window register) and executes the ROM's own code
// end to end: the shipped command bytes -> StreamCmd_RunScript's dispatch table ->
// StreamCmd_InitWindowCornerMotion -> the installed ProcessMotionStepExtended
// callback, ticked -> UpdateAffineRegisters -> the hardware registers.
//
// Every trial has a control: the identical run with dX = dY = 0, which must move no
// register at all.
//
// Run: GBA_KIT=... node docs/dynamic-analysis/scripts/prove-window-corner-tween.mjs
import { HeadlessRuntime, REPO, SAVESTATE } from './gba-kit.mjs';
import { makeCaller, mustSymbol } from './_callrom.mjs';
import { mkdirSync, readFileSync } from 'node:fs';

const OUT = '/tmp/klonoa-r5';
mkdirSync(OUT, { recursive: true });

const rt = await HeadlessRuntime.create({
    romPath: `${REPO}/baserom.gba`,
    elfPath: `${REPO}/klonoa-eod.elf`,
    outputDir: OUT,
    logFn: () => {},
});
const eng = rt.engine,
    bus = rt.gba.bus,
    cpu = rt.gba.armCpu,
    di = eng.debugInfo;
const call = makeCaller(cpu);
const h4 = (n) => '0x' + (n & 0xffff).toString(16).padStart(4, '0');
const s16 = (v) => (v << 16) >> 16;

// ── every REG_ name in this script comes from include/io_reg.h, by address ─────
const ioReg = readFileSync(`${REPO}/include/io_reg.h`, 'utf8');
const OFFSETS = new Map();
for (const m of ioReg.matchAll(/^#define\s+REG_OFFSET_(\w+)\s+(0x[0-9a-fA-F]+)/gm))
    OFFSETS.set(0x04000000 + parseInt(m[2], 16), m[1]);
const regName = (addr) => OFFSETS.get(addr) ?? `<no REG_OFFSET_* in io_reg.h for 0x${addr.toString(16)}>`;
const regAddr = (name) => {
    for (const [a, n] of OFFSETS) if (n === name) return a;
    throw new Error(`include/io_reg.h has no REG_OFFSET_${name}`);
};
console.log('=== window registers as include/io_reg.h defines them ===');
for (const n of ['WIN0H', 'WIN1H', 'WIN0V', 'WIN1V'])
    console.log(`  REG_${n} = 0x${regAddr(n).toString(16)}`);

// ═══════════════════════════════════════════════════════════════════════════════
// C4. The ROM's own addressing arithmetic, disassembled.
// ═══════════════════════════════════════════════════════════════════════════════
const UAR = mustSymbol(di, 'UpdateAffineRegisters');
console.log(`\n=== C4. UpdateAffineRegisters (0x${UAR.toString(16)}) register-pointer walk ===`);
{
    let ptr = null;
    const stops = [];
    for (const i of eng.disassemble(UAR & ~1, 50, 'thumb')) {
        // The disassembler prints the literal POOL ADDRESS (`=0x0804b350`), not the
        // word stored there — reading the printed number as the value would silently
        // label every store with a ROM address instead of an I/O register.
        const lit = /ldr r5, \[pc[^=]*=(0x[0-9a-f]+)/i.exec(i.instruction);
        if (lit) ptr = bus.read32(parseInt(lit[1], 16)) >>> 0;
        const add = /^adds r5, #(0x[0-9a-f]+)/i.exec(i.instruction);
        const sub = /^subs r5, #(0x[0-9a-f]+)/i.exec(i.instruction);
        if (add) ptr += parseInt(add[1], 16);
        if (sub) ptr -= parseInt(sub[1], 16);
        if (/^strh r1, \[r5/.test(i.instruction)) stops.push(ptr);
        console.log(
            `  0x${i.address.toString(16)}  ${i.instruction.padEnd(34)}` +
                (/^(ldr r5|adds r5|subs r5|strh r1, \[r5|ldrh r[01], \[r2)/.test(i.instruction)
                    ? `   ; r5 = 0x${(ptr ?? 0).toString(16)}${OFFSETS.has(ptr) ? ' = REG_' + OFFSETS.get(ptr) : ''}`
                    : ''),
        );
        if (stops.length === 4) break;
    }
    console.log('\n  the four stores, in order, labelled from include/io_reg.h by address:');
    const srcOffsets = [0x08, 0x0a, 0x0c, 0x0e]; // the leftTop halfword each store consumes
    stops.forEach((a, i) =>
        console.log(
            `    store ${i + 1}: [gLevelStatePtr + 0x${srcOffsets[i].toString(16)}] (and +0x${(srcOffsets[i] + 8).toString(16)}) -> 0x${a.toString(16)} = REG_${regName(a)}`,
        ),
    );
    console.log('  so the block at +0x08 is [window][axis], not [axis][window]:');
    console.log('    +0x08 -> WIN0H, +0x0A -> WIN0V, +0x0C -> WIN1H, +0x0E -> WIN1V');
    console.log('    (the +4 / -2 / +4 walk is what makes WIN1H come third, at 0x04000042.)');
}

// ── the DWARF view of the same block ──────────────────────────────────────────
const LT = di.structMember('LevelWindowBounds', 'leftTop');
const RB = di.structMember('LevelWindowBounds', 'rightBottom');
console.log(
    `\n  DWARF: LevelWindowBounds.leftTop +0x${LT.offset.toString(16)} size ${LT.size},` +
        ` .rightBottom +0x${RB.offset.toString(16)} size ${RB.size}  (s16 [2][2] each)`,
);
const boundAddr = (base, group, win, axis) => base + (group === 'leftTop' ? LT.offset : RB.offset) + win * 4 + axis * 2;

// ═══════════════════════════════════════════════════════════════════════════════
// C1..C3. Run the shipped commands on a frozen in-level state.
// ═══════════════════════════════════════════════════════════════════════════════
const STREAM_PTR = mustSymbol(di, 'gStreamPtr');
const LEVEL_STATE = mustSymbol(di, 'gLevelStatePtr');
const ENTRIES_PTR = 0x030052a4; // gBuffer_52A4 (#define, no symbol)
const SCRATCH_CMD = 0x0203f000,
    SCRATCH_ENT = 0x0203e000,
    SCRATCH_LVL = 0x0203d000;
const STRIDE = 36;
const F = {
    param: di.structMember('GfxStreamEntry', 'param'),
    targetIndex: di.structMember('GfxStreamEntry', 'targetIndex'),
    unk_04: di.structMember('GfxStreamEntry', 'unk_04'),
    unk_06: di.structMember('GfxStreamEntry', 'unk_06'),
    unk_08: di.structMember('GfxStreamEntry', 'unk_08'),
    unk_0A: di.structMember('GfxStreamEntry', 'unk_0A'),
    callback: di.structMember('GfxStreamEntry', 'callback'),
};

function handler(op) {
    const pick =
        (op & 0x80) !== 0
            ? [0x081179b4, op & 0x0f]
            : (op & 0x40) !== 0
              ? [0x0811787c, op & 0x3f]
              : (op & 0x30) === 0x30
                ? [0x081178b8, op & 0x0f]
                : (op & 0x30) === 0x20
                  ? [0x08117854, op & 0x0f]
                  : (op & 0x30) === 0x10
                    ? [0x0811790c, op & 0x0f]
                    : [0x081178d8, op & 0x0f];
    const fn = bus.read32(pick[0] + pick[1] * 4) >>> 0;
    if (fn < 0x08000000 || fn >= 0x0a000000) throw new Error(`FF ${op.toString(16)} dispatches to 0x${fn.toString(16)}`);
    return fn;
}
const HANDLER_44 = handler(0x44);
console.log(
    `\n  FF 44 dispatches to 0x${HANDLER_44.toString(16)} (StreamCmd_InitWindowCornerMotion = 0x${mustSymbol(di, 'StreamCmd_InitWindowCornerMotion').toString(16)})`,
);

const WINREGS = ['WIN0H', 'WIN1H', 'WIN0V', 'WIN1V'];
const readWin = () => Object.fromEntries(WINREGS.map((n) => [n, bus.read16(regAddr(n))]));
// Palette RAM plus every register the "WithPalette" name would predict.
const PAL_BASE = 0x05000000,
    PAL_LEN = 0x400;
const OTHER = ['BLDCNT', 'BLDALPHA', 'BLDY', 'MOSAIC', 'WININ', 'WINOUT'];
const readOther = () => Object.fromEntries(OTHER.map((n) => [n, bus.read16(regAddr(n))]));
const readPal = () => {
    const a = new Uint8Array(PAL_LEN);
    for (let i = 0; i < PAL_LEN; i++) a[i] = bus.read8(PAL_BASE + i);
    return a;
};

/**
 * Execute one shipped `FF 44` command and tick it to completion, on a frozen state.
 * Returns what moved.
 */
async function trial({ entry, target, dx, dy, frames, ticks }) {
    await eng.loadState(SAVESTATE);
    await eng.wait({ frames: 2 });
    bus.write32(ENTRIES_PTR, SCRATCH_ENT);
    bus.write32(LEVEL_STATE, SCRATCH_LVL);
    for (let k = 0; k < 0x400; k++) bus.write8(SCRATCH_ENT + k, 0);
    for (let k = 0; k < 0x40; k++) bus.write8(SCRATCH_LVL + k, 0);
    // A known baseline: both windows open across the whole screen. Bounds are 12.4
    // fixed point (StreamCmd_SetWindowCorner stores `byte << 4`), so 240 px = 0xF00.
    for (const w of [0, 1]) {
        bus.write16(boundAddr(SCRATCH_LVL, 'leftTop', w, 0), 0); // left
        bus.write16(boundAddr(SCRATCH_LVL, 'leftTop', w, 1), 0); // top
        bus.write16(boundAddr(SCRATCH_LVL, 'rightBottom', w, 0), 240 << 4); // right
        bus.write16(boundAddr(SCRATCH_LVL, 'rightBottom', w, 1), 160 << 4); // bottom
    }
    call(mustSymbol(di, 'UpdateAffineRegisters'), []);
    const win0 = readWin(),
        other0 = readOther(),
        pal0 = readPal();

    const cmd = [0xff, 0x44, entry, target, dx & 0xff, (dx >> 8) & 0xff, dy & 0xff, (dy >> 8) & 0xff, frames & 0xff, (frames >> 8) & 0xff];
    cmd.forEach((b, i) => bus.write8(SCRATCH_CMD + i, b));
    bus.write32(STREAM_PTR, SCRATCH_CMD);
    call(HANDLER_44, [0]);

    const ent = SCRATCH_ENT + entry * STRIDE;
    const installed = {
        param: (bus.read16(ent + F.param.offset) >>> F.param.bitOffset) & ((1 << F.param.bitWidth) - 1),
        target: (bus.read8(ent + F.targetIndex.offset) >>> F.targetIndex.bitOffset) & ((1 << F.targetIndex.bitWidth) - 1),
        unk_04: s16(bus.read16(ent + F.unk_04.offset)),
        unk_06: s16(bus.read16(ent + F.unk_06.offset)),
        unk_08: s16(bus.read16(ent + F.unk_08.offset)),
        unk_0A: s16(bus.read16(ent + F.unk_0A.offset)),
        cb: bus.read32(ent + F.callback.offset) >>> 0,
    };
    for (let t = 0; t < ticks; t++) call(installed.cb, [entry]);
    call(mustSymbol(di, 'UpdateAffineRegisters'), []);

    const win1 = readWin(),
        other1 = readOther(),
        pal1 = readPal();
    let palDiff = 0;
    for (let i = 0; i < PAL_LEN; i++) if (pal0[i] !== pal1[i]) palDiff++;
    const bounds = {};
    for (const g of ['leftTop', 'rightBottom'])
        for (const w of [0, 1])
            for (const a of [0, 1]) bounds[`${g}[${w}][${a}]`] = s16(bus.read16(boundAddr(SCRATCH_LVL, g, w, a)));
    return { installed, win0, win1, other0, other1, palDiff, bounds };
}

console.log('\n=== C1/C2/C3. one shipped-shape command per corner, ticked to completion ===');
console.log('    baseline for every trial: both windows fully open (WIN0H=WIN1H=0x00F0, WIN0V=WIN1V=0x00A0)');

const TRIALS = [
    // taken from the shipped scripts (scan-gfx-stream-commands.mjs): every target
    // byte the 62 real occurrences use, plus its dY twin.
    { label: 'target 0, dX +40 (script[1] shape)', target: 0, dx: 40, dy: 0 },
    { label: 'target 0, dY +40', target: 0, dx: 0, dy: 40 },
    { label: 'target 1, dX +40', target: 1, dx: 40, dy: 0 },
    { label: 'target 1, dY +40', target: 1, dx: 0, dy: 40 },
    { label: 'target 2, dX +40', target: 2, dx: 40, dy: 0 },
    { label: 'target 2, dY +40 (script[1] shape)', target: 2, dx: 0, dy: 40 },
    { label: 'target 3, dX +40 (script[0] shape)', target: 3, dx: 40, dy: 0 },
    { label: 'target 3, dY +40', target: 3, dx: 0, dy: 40 },
    { label: 'CONTROL: target 0, dX 0 dY 0', target: 0, dx: 0, dy: 0 },
    { label: 'CONTROL: target 3, dX 0 dY 0', target: 3, dx: 0, dy: 0 },
];
const rows = [];
for (const t of TRIALS) {
    const r = await trial({ entry: 0x0d, target: t.target, dx: t.dx, dy: t.dy, frames: 20, ticks: 24 });
    const moved = WINREGS.filter((n) => r.win0[n] !== r.win1[n]);
    const movedBounds = Object.entries(r.bounds).filter(
        ([k, v]) => v !== (k.startsWith('leftTop') ? 0 : k.endsWith('[0]') ? 240 << 4 : 160 << 4),
    );
    rows.push({ t, r, moved });
    console.log(
        `\n  ${t.label}` +
            `\n    entry after the handler: param=${r.installed.param} targetIndex=${r.installed.target}` +
            ` unk_04=${r.installed.unk_04} unk_06=${r.installed.unk_06} unk_08(step X)=${r.installed.unk_08} unk_0A(step Y)=${r.installed.unk_0A}` +
            `\n    bounds that moved: ${movedBounds.length ? movedBounds.map(([k, v]) => `${k}=${v} (${v / 16}px)`).join(', ') : 'none'}` +
            `\n    registers: ` +
            WINREGS.map((n) => `${n} ${h4(r.win0[n])}->${h4(r.win1[n])}`).join('  ') +
            `\n    -> moved: ${moved.length ? moved.map((n) => 'REG_' + n).join(', ') : 'NOTHING'}` +
            `\n    palette RAM bytes changed: ${r.palDiff}   ` +
            OTHER.map((n) => `${n} ${h4(r.other0[n])}->${h4(r.other1[n])}`).join('  '),
    );
}

console.log('\n=== verdicts ===');
const by = (label) => rows.find((x) => x.t.label === label);
console.log('  C1  palette / blend / mosaic:');
console.log(
    `      palette RAM bytes changed across all ${rows.length} trials: ${rows.reduce((a, x) => a + x.r.palDiff, 0)}`,
);
console.log(
    `      REG_BLDCNT / REG_BLDALPHA / REG_BLDY / REG_MOSAIC / REG_WININ / REG_WINOUT changed in any trial: ` +
        `${rows.some((x) => OTHER.some((n) => x.r.other0[n] !== x.r.other1[n])) ? 'YES' : 'NO'}`,
);
console.log('  C2/C3  which register each (target, axis) reaches:');
for (const r of rows.filter((x) => !x.t.label.startsWith('CONTROL')))
    console.log(
        `      target ${r.t.target}, ${r.t.dx ? 'dX' : 'dY'} -> ${r.moved.map((n) => 'REG_' + n).join(',') || 'nothing'}`,
    );
console.log(
    `  control (dX = dY = 0) moved: ${by('CONTROL: target 0, dX 0 dY 0').moved.length + by('CONTROL: target 3, dX 0 dY 0').moved.length} registers`,
);
