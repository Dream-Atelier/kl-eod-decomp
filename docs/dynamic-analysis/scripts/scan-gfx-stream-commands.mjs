// EVIDENCE (ROM data): decode every gfx-stream script the cartridge ships and report
// which commands really appear, with which arguments.
//
// Why this is needed: the gfx-stream scripts are not plain bytes in the ROM. They are
// Huffman+LZ77 assets behind gStreamDataTable (0x08189AFC, 20 entries), so grepping
// baserom.gba for a command byte finds nothing, and grepping the *decompressed* bytes
// finds payload bytes as well as command bytes. This script does neither: it lets the
// cartridge decompress its own data and then walks each script command by command.
//
// Nothing here is hand-typed:
//   * decompression is the ROM's own Decompress() (src/code_3.c), executed on the
//     emulated CPU, so the Huffman/LZ77 handling is the game's, not a reimplementation;
//   * every command's LENGTH is MEASURED, by pointing gStreamPtr at a scratch buffer,
//     calling the real handler out of the real dispatch table, and reading how far the
//     handler moved gStreamPtr. Each opcode is measured with three different payload
//     fillings, and any opcode whose advance is not the same all three times is
//     reported instead of assumed.
//
// The walk is self-checking: with the measured lengths all 20 scripts parse from their
// first command to their last byte. A wrong length would desynchronise the walk and
// leave a tail of garbage, so "clean" is a real check, not a formality.
//
// Two of the round-5 naming questions are answered directly by the output:
//   * StreamCmd_InitSpriteWave (FF 4C): which wave-table row do real scripts select?
//   * StreamCmd_InitBg2Zoom (FF 46): what magnification does the script set before
//     the tween, and where does the tween land?
//
// Run: GBA_KIT=... node docs/dynamic-analysis/scripts/scan-gfx-stream-commands.mjs
import { HeadlessRuntime, REPO, SAVESTATE } from './gba-kit.mjs';
import { makeCaller, mustSymbol } from './_callrom.mjs';
import { mkdirSync } from 'node:fs';

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

const hex2 = (n) => (n & 0xff).toString(16).padStart(2, '0');
const s16 = (v) => (v << 16) >> 16;

// ── the six dispatch tables StreamCmd_RunScript (0x0804EA94) selects between ────
// Reproduced from that function's own literal pool; the opcode formula next to each
// is the mask/compare sequence it executes on stream byte[1].
const TABLES = [
    { base: 0x081179b4, n: 16, op: (i) => 0x80 | i, cond: 'c & 0x80' },
    { base: 0x0811787c, n: 50, op: (i) => 0x40 | i, cond: 'c & 0x40' },
    { base: 0x081178b8, n: 16, op: (i) => 0x30 | i, cond: '(c & 0x30) == 0x30' },
    { base: 0x08117854, n: 16, op: (i) => 0x20 | i, cond: '(c & 0x30) == 0x20' },
    { base: 0x0811790c, n: 16, op: (i) => 0x10 | i, cond: '(c & 0x30) == 0x10' },
    { base: 0x081178d8, n: 16, op: (i) => 0x00 | i, cond: '(c & 0x30) == 0x00' },
];

const STREAM_PTR = mustSymbol(di, 'gStreamPtr');
const STREAM_TABLE = mustSymbol(di, 'gStreamDataTable');
const DECOMPRESS = mustSymbol(di, 'Decompress');
const LEVEL_STATE = mustSymbol(di, 'gLevelStatePtr');
const ENTRIES_PTR = 0x030052a4; // gBuffer_52A4 — a #define, so no symbol to resolve
const SCRATCH_CMD = 0x0203f000,
    SCRATCH_ENT = 0x0203e000,
    SCRATCH_LVL = 0x0203d000,
    SCRATCH_OUT = 0x02030000;

console.log('gStreamPtr        =', '0x' + STREAM_PTR.toString(16));
console.log('gStreamDataTable  =', '0x' + STREAM_TABLE.toString(16));
console.log('Decompress        =', '0x' + DECOMPRESS.toString(16));

await eng.loadState(SAVESTATE);
await eng.wait({ frames: 4 });

// ═══════════════════════════════════════════════════════════════════════════════
// 1. Measure every opcode's length by running its real handler.
// ═══════════════════════════════════════════════════════════════════════════════
const romName = (addr) => {
    const n = di.addressToSymbol ? di.addressToSymbol(addr & ~1) : null;
    return n?.name ?? n ?? '';
};

const LEN = new Map(); // opcode -> length in bytes
const FN = new Map(); // opcode -> handler address
const problems = [];

for (const t of TABLES) {
    for (let i = 0; i < t.n; i++) {
        const fn = bus.read32(t.base + i * 4) >>> 0;
        const op = t.op(i);
        if (fn < 0x08000000 || fn >= 0x0a000000) {
            problems.push(`FF ${hex2(op)} -> 0x${fn.toString(16)} is not a ROM address (table slot is not a handler)`);
            continue;
        }
        const seen = [];
        for (const fill of [0x00, 0x01, 0x7f]) {
            await eng.loadState(SAVESTATE);
            await eng.wait({ frames: 2 });
            // Redirect the two pointers a handler might scribble through, so the
            // measurement cannot corrupt the state the next measurement loads.
            bus.write32(ENTRIES_PTR, SCRATCH_ENT);
            bus.write32(LEVEL_STATE, SCRATCH_LVL);
            for (let k = 0; k < 0x800; k++) bus.write8(SCRATCH_ENT + k, 0);
            for (let k = 0; k < 0x100; k++) bus.write8(SCRATCH_LVL + k, 0);
            for (let k = 0; k < 64; k++) bus.write8(SCRATCH_CMD + k, fill);
            bus.write8(SCRATCH_CMD + 0, 0xff);
            bus.write8(SCRATCH_CMD + 1, op);
            bus.write32(STREAM_PTR, SCRATCH_CMD);
            const r = call(fn, [0], 400000);
            seen.push(r.timedOut ? 'timeout' : String(((bus.read32(STREAM_PTR) >>> 0) - SCRATCH_CMD) | 0));
        }
        const uniq = [...new Set(seen)];
        if (uniq.length === 1 && uniq[0] !== 'timeout' && Number(uniq[0]) > 0) {
            LEN.set(op, Number(uniq[0]));
            FN.set(op, fn);
        } else {
            problems.push(`FF ${hex2(op)} -> 0x${fn.toString(16)} ${romName(fn)} advance=${seen.join('/')}`);
            FN.set(op, fn);
        }
    }
}
// Opcodes 0x80..0x8F are re-selected by `c & 0x0F`, so 0x90..0xFF alias them.
for (let c = 0x90; c <= 0xff; c++) {
    const a = 0x80 | (c & 0x0f);
    if (LEN.has(a)) LEN.set(c, LEN.get(a));
}

console.log(`\nmeasured a constant length for ${LEN.size} opcodes`);
console.log('opcodes that did NOT yield one constant length (each is resolved by hand below):');
for (const p of problems) console.log('   ' + p);

// Three handlers need a hand-supplied length; each reason is checkable from the
// disassembly this script prints, and none of the three is a command under test.
//   FF 24 StreamCmd_SetupSpriteGroup: `gStreamPtr += 4` in src/gfx.c. It only
//         times out because a nonsense payload makes its OAM loop run away.
//   FF 81 / FF 86: both end in `adds r0, #0x2 / str r0, [stream]`, printed below.
//         They time out inside an m4a call that spins on the zeroed sound state.
for (const [op, len] of [
    [0x24, 4],
    [0x81, 2],
    [0x86, 2],
]) {
    LEN.set(op, len);
    if (op >= 0x80) for (let c = 0x90; c <= 0xff; c++) if ((c & 0x0f) === (op & 0x0f)) LEN.set(c, len);
}
console.log('\ntail of the two m4a handlers, showing the +2 stream advance that was measured as a timeout:');
for (const a of [0x0804efdc, 0x0804f0d0]) {
    const ins = eng.disassemble(a, 30, 'thumb').filter((i) => /adds r0, #0x2|str r0, \[r1/.test(i.instruction));
    console.log(`   0x${a.toString(16)}: ` + ins.map((i) => `0x${i.address.toString(16)} ${i.instruction}`).join(' ; '));
}

// ═══════════════════════════════════════════════════════════════════════════════
// 2. Let the ROM decompress its own scripts.
// ═══════════════════════════════════════════════════════════════════════════════
const scripts = [];
for (let i = 0; i < 20; i++) {
    await eng.loadState(SAVESTATE);
    await eng.wait({ frames: 2 });
    const src = bus.read32(STREAM_TABLE + i * 4) >>> 0;
    const size = bus.read32(src) & 0x7fffffff;
    const r = call(DECOMPRESS, [SCRATCH_OUT, src], 5000000);
    if (r.timedOut) throw new Error(`Decompress(${i}) did not return`);
    const buf = new Uint8Array(size);
    for (let k = 0; k < size; k++) buf[k] = bus.read8(SCRATCH_OUT + k);
    scripts.push(buf);
}
console.log('\ndecompressed 20 scripts, sizes:', scripts.map((s) => '0x' + s.length.toString(16)).join(' '));

// ═══════════════════════════════════════════════════════════════════════════════
// 3. Walk. LoadAndDecompressStream sets gStreamPtr = buffer + 4, so the first
//    command is at offset 4; a non-0xFF byte is a music byte and advances 1.
// ═══════════════════════════════════════════════════════════════════════════════
function walk(s) {
    const cmds = [];
    let i = 4;
    while (i < s.length - 1) {
        if (s[i] !== 0xff) {
            i += 1;
            continue;
        }
        const c = s[i + 1];
        const n = LEN.get(c);
        if (n == null) return { cmds, end: i, why: `no length for FF ${hex2(c)}` };
        cmds.push({ off: i, op: c, bytes: s.slice(i, i + n) });
        i += n;
    }
    return { cmds, end: i, why: null };
}

console.log('\n=== walk (a wrong length would desynchronise and leave a garbage tail) ===');
const all = [];
for (let i = 0; i < scripts.length; i++) {
    const w = walk(scripts[i]);
    const clean = w.why == null && w.end >= scripts[i].length - 2;
    console.log(
        `  script[${String(i).padStart(2)}] size=0x${scripts[i].length.toString(16).padStart(4, '0')}` +
            ` commands=${String(w.cmds.length).padStart(5)}  ${clean ? 'parsed to the end' : 'STOPPED at 0x' + w.end.toString(16) + ' ' + (w.why ?? '')}`,
    );
    all.push(w.cmds.map((c) => ({ ...c, script: i })));
}
const flat = all.flat();

const find = (op) => flat.filter((c) => c.op === op);
const dump = (op, label) => {
    const hits = find(op);
    console.log(`\n=== FF ${hex2(op)}  ${label} — ${hits.length} occurrence(s) in the shipped scripts ===`);
    for (const h of hits)
        console.log(
            `   script[${String(h.script).padStart(2)}] @0x${h.off.toString(16).padStart(4, '0')}: ` +
                [...h.bytes].map(hex2).join(' '),
        );
    return hits;
};

// ── StreamCmd_InitSpriteWave ───────────────────────────────────────────────────
const waves = dump(0x4c, 'StreamCmd_InitSpriteWave');
console.log('   decoded (byte[4] hi nibble = wave-table row, lo nibble = frame-counter shift):');
for (const h of waves) {
    const b = h.bytes;
    console.log(
        `     entry=0x${hex2(b[2])} objIndex=${b[3]} -> gEntityArray slot ${b[3] + 13}` +
            `  row=${b[4] >> 4}  shift=${b[4] & 0xf}  amplitude=${b[5]}  duration=${s16(b[6] | (b[7] << 8))}`,
    );
}
const rows = new Set(waves.map((h) => h.bytes[4] >> 4));
console.log(`   -> rows selected by real scripts: {${[...rows].join(', ')}}   (the table has rows 0..3)`);

// ── StreamCmd_InitBg2Zoom, and the magnification write that precedes it ────
const dual = dump(0x34, 'WriteStreamValue_Dual (sets gBg2XMag = gBg2YMag)');
for (const h of dual)
    console.log(`     -> gBg2XMag = gBg2YMag = 0x${(h.bytes[2] | (h.bytes[3] << 8)).toString(16)}`);
const angle = dump(0x46, 'StreamCmd_InitBg2Zoom');
console.log('   decoded, paired with the last WriteStreamValue_Dual before each one:');
for (const h of angle) {
    const b = h.bytes;
    const total = s16(b[3] | (b[4] << 8));
    const frames = b[5];
    const prev = dual.filter((d) => d.script === h.script && d.off < h.off).pop();
    const start = prev ? prev.bytes[2] | (prev.bytes[3] << 8) : null;
    console.log(
        `     script[${h.script}] @0x${h.off.toString(16)}: entry=0x${hex2(b[2])} total=${total} frames=${frames}` +
            (start == null
                ? '   (no preceding FF 34 in this script)'
                : `   starts at 0x${start.toString(16)} (script[${prev.script}] @0x${prev.off.toString(16)}), lands on 0x${(start + total).toString(16)}`),
    );
}

// ── StreamCmd_InitWindowCornerMotion ───────────────────────────────────────────
const win = dump(0x44, 'StreamCmd_InitWindowCornerMotion');
console.log('   decoded (byte[3] = the entry targetIndex ProcessMotionStepExtended case 4 reads):');
const seenTargets = new Map();
for (const h of win) {
    const b = h.bytes;
    const t = b[3] & 0xf;
    const dx = s16(b[4] | (b[5] << 8)),
        dy = s16(b[6] | (b[7] << 8)),
        dur = s16(b[8] | (b[9] << 8));
    seenTargets.set(t, (seenTargets.get(t) ?? 0) + 1);
    if (h.script <= 1)
        console.log(
            `     script[${h.script}] @0x${h.off.toString(16)}: entry=0x${hex2(b[2])} target=${t} dX=${dx} dY=${dy} frames=${dur}`,
        );
}
console.log(`     ... (${win.length} total; first two scripts shown)`);
console.log(
    '   target byte histogram:',
    [...seenTargets.entries()].sort((a, b) => a[0] - b[0]).map(([k, v]) => `${k}:${v}`).join('  '),
);

// ── the alias question ────────────────────────────────────────────────────────
// StreamCmd_InitWindowCornerMotion sits in TWO dispatch tables, so it has two legal
// spellings. Only one is used.
console.log('\n=== the same handler, reached through two tables ===');
for (const op of [0x44, 0x2e]) {
    console.log(
        `   FF ${hex2(op)} -> 0x${(FN.get(op) ?? 0).toString(16)}   occurrences in shipped scripts: ${find(op).length}`,
    );
}

// ── a negative control for the walk itself ────────────────────────────────────
// If the length table were wrong, the walk would still emit *some* commands. The
// check that it is right is that the naive byte-pair scan and the walk agree
// exactly on every opcode under test: a desynchronised walk would report command
// starts the naive scan never sees, or miss ones it does.
console.log('\n=== control: naive "FF xx" byte-pair scan vs the parsed walk ===');
for (const op of [0x44, 0x46, 0x4c, 0x34]) {
    let naive = 0;
    for (const s of scripts) for (let i = 0; i < s.length - 1; i++) if (s[i] === 0xff && s[i + 1] === op) naive++;
    const parsed = find(op).length;
    console.log(
        `   FF ${hex2(op)}: naive byte-pair hits ${String(naive).padStart(3)}   parsed command starts ${String(parsed).padStart(3)}   ${naive === parsed ? 'AGREE' : 'DISAGREE — some hits are payload bytes'}`,
    );
}

// ── the round-6 naming round: which SPELLING of each handler do scripts use? ──
// Several handlers sit in more than one dispatch table, so a handler can have two or
// three legal opcodes. Only the parsed walk can say which are real command starts:
// FF 03 in particular is a very common byte pair inside payloads, so its naive count
// is an order of magnitude too high.
console.log('\n=== round-6 naming round: every spelling of the motion and wait handlers ===');
const NAMED = [
    [0x41, 'StreamCmd_InitLinearMotion'],
    [0x2b, 'StreamCmd_InitLinearMotion (alias)'],
    [0x42, 'StreamCmd_InitEntityMotion'],
    [0x2c, 'StreamCmd_InitEntityMotion (alias)'],
    [0x44, 'StreamCmd_InitWindowCornerMotion'],
    [0x2e, 'StreamCmd_InitWindowCornerMotion (alias)'],
    [0x5a, 'StreamCmd_WaitFrames'],
    [0x3b, 'StreamCmd_WaitFrames (alias)'],
    [0x03, 'StreamCmd_WaitFrames (alias)'],
    [0x4b, 'StreamCmd_InitFrameAnimation'],
];
for (const [op, label] of NAMED) {
    let naive = 0;
    for (const s of scripts) for (let i = 0; i < s.length - 1; i++) if (s[i] === 0xff && s[i + 1] === op) naive++;
    const parsed = find(op);
    console.log(
        `   FF ${hex2(op)}  ${label.padEnd(36)} handler 0x${(FN.get(op) ?? 0).toString(16)}  len=${LEN.get(op) ?? '?'}` +
            `   parsed uses ${String(parsed.length).padStart(4)}   (naive byte-pair hits ${String(naive).padStart(4)})`,
    );
}
const waitUses = [0x5a, 0x3b, 0x03].map((o) => find(o).length);
console.log(`   -> StreamCmd_WaitFrames is issued ${waitUses.reduce((a, b) => a + b, 0)} time(s) in total, split ${waitUses.join(' / ')} across FF 5A / FF 3B / FF 03`);
const waitCmds = [0x5a, 0x3b, 0x03].flatMap((o) => find(o));
if (waitCmds.length) {
    const ns = waitCmds.map((c) => c.bytes[2] | (c.bytes[3] << 8));
    ns.sort((a, b) => a - b);
    console.log(`   -> the frame counts it is given range ${ns[0]}..${ns[ns.length - 1]} frames (median ${ns[ns.length >> 1]})`);
}
