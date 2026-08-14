// PROOF for the function called UpdateAffineBGParams (src/engine.c, ROM 0x08000FCC).
//
// Four questions, each answered by an intervention with its control:
//
//   Q1  Is it really the H-Blank handler, i.e. does it run once per scanline?
//       INTERVENTION: none needed -- the world map (savestate-in-gameplay) has
//       H-Blank enabled.  Watch the four hardware halfwords it writes for one
//       frame and count the hits and the REG_VCOUNT at each.
//       CONTROL: the same watch on savestate-in-level-idle, where REG_IE has
//       INTR_FLAG_HBLANK clear -- the handler is installed but never entered, so
//       the only writer left is the once-per-frame V-Blank callback.
//
//   Q2  Which RAM word feeds which axis?  The two "delta" globals are macros
//       (include/gfx.h), so they have no DWARF symbol; instead of hand-typing an
//       address this script reads the function's OWN literal pool out of the ROM,
//       anchors it on the two words that DO have symbols (gBg2X / gBg2Y), and
//       then decides which of the two remaining IWRAM words drives which axis by
//       forcing one at a time and watching REG_BG2X / REG_BG2Y.
//
//   Q3  Are those two words sin(gBg2Alpha) and cos(gBg2Alpha), as include/gfx.h
//       claims in a comment?  INTERVENTION: force gBg2Alpha to a sweep of angles
//       and compare the two words against gSineTable (a real ELF symbol) at
//       [alpha] and [alpha + PI_2].  CONTROL: the unforced frame.
//
//   Q4  Where is the pivot -- the scanline whose reference point is left equal to
//       gBg2X/gBg2Y?  Measured, not assumed: reconstruct the 32-bit value written
//       on every scanline and find the line where the displacement is zero, then
//       force both deltas to 0 and diff the framebuffer row by row to see which
//       rows the ramp was actually moving.
//
// Hardware labels used below, cross-checked against include/io_reg.h:
//   0x04000004 REG_DISPSTAT (bit 4 = DISPSTAT_HBLANK_INTR)
//   0x04000006 REG_VCOUNT   (REG_VCOUNT_L is its low byte)
//   0x04000028 REG_BG2X_L   0x0400002A REG_BG2X_H
//   0x0400002C REG_BG2Y_L   0x0400002E REG_BG2Y_H
//   0x04000200 REG_IE       (bit 1 = INTR_FLAG_HBLANK, bit 0 = INTR_FLAG_VBLANK)
//   0x04000020..0x04000026 REG_BG2PA..REG_BG2PD -- NOT written by this function.
import { HeadlessRuntime } from './gba-kit.mjs';
import { mkdirSync } from 'fs';
import { ROM, ELF, SAVES, readField } from './_harness.mjs';

const OUT = '/tmp/klonoa-dyn-bg2';
mkdirSync(OUT, { recursive: true });
const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const eng = rt.engine;
const bus = rt.gba.bus;
const di = eng.debugInfo;
const hex = (n) => '0x' + (n >>> 0).toString(16);

/** Every address in this script comes from the ELF or from the ROM; never hand-typed RAM. */
const need = (name) => {
    const a = di.symbolToAddress(name);
    if (a == null) throw new Error(`symbolToAddress('${name}') returned null`);
    return a >>> 0;
};

// --- io_reg.h labels (the only hand-written constants here, and they are the
// --- hardware map, verified line by line against include/io_reg.h) -----------
const REG_DISPSTAT = 0x04000004;
const REG_VCOUNT = 0x04000006;
const REG_BG2X_L = 0x04000028;
const REG_BG2Y_L = 0x0400002c;
const REG_BG2PA = 0x04000020;
const REG_IE = 0x04000200;
const INTR_FLAG_VBLANK = 1 << 0;
const INTR_FLAG_HBLANK = 1 << 1;
const DISPSTAT_HBLANK_INTR = 0x0010;
const PI_2 = 64; // include/data/trig.h: full circle = 256

// ---------------------------------------------------------------------------
// Q2a: read the function's literal pool out of the ROM and anchor it on symbols
// ---------------------------------------------------------------------------
const FN = need('UpdateAffineBGParams') & ~1;
const A_BG2X = need('gBg2X');
const A_BG2Y = need('gBg2Y');
const A_ALPHA = need('gBg2Alpha');
const A_SINE = need('gSineTable');
const A_INTR = need('gIntrTable');
const M_HBLANK = di.structMember('IntrTable', 'hBlank');
if (!M_HBLANK) throw new Error('no DWARF member IntrTable.hBlank');

console.log('UpdateAffineBGParams @', hex(FN), ' gBg2X', hex(A_BG2X), ' gBg2Y', hex(A_BG2Y),
    ' gBg2Alpha', hex(A_ALPHA), ' gSineTable', hex(A_SINE));
console.log('\n--- literal pool of the function itself (read from ROM) ---------');
const pool = [];
for (let off = 0x44; off <= 0x58; off += 4) {
    const w = bus.read32(FN + off) >>> 0;
    pool.push(w);
    const sym = w === A_BG2X ? 'gBg2X' : w === A_BG2Y ? 'gBg2Y' : w === REG_VCOUNT ? 'REG_VCOUNT (io_reg.h)'
        : w === REG_BG2X_L ? 'REG_BG2X_L (io_reg.h)' : '(unnamed IWRAM word -> delta candidate)';
    console.log(`  ${hex(FN + off)}: ${hex(w)}  ${sym}`);
}
if (!pool.includes(A_BG2X) || !pool.includes(A_BG2Y)) throw new Error('pool does not contain gBg2X/gBg2Y -- wrong function or stale ELF');
if (!pool.includes(REG_VCOUNT)) throw new Error('pool does not contain REG_VCOUNT');
if (!pool.includes(REG_BG2X_L)) throw new Error('pool does not contain REG_BG2X_L');
if (pool.includes(REG_BG2PA)) throw new Error('pool contains REG_BG2PA -- the affine MATRIX would be written');
const deltas = pool.filter((w) => w >>> 24 === 0x03 && w !== A_BG2X && w !== A_BG2Y);
if (deltas.length !== 2) throw new Error('expected exactly two unnamed IWRAM words in the pool, got ' + deltas.length);
console.log('  delta candidates:', deltas.map(hex).join(' , '), ' (which is which is decided by experiment below)');
console.log('  REG_BG2PA (0x04000020) in pool:', pool.includes(REG_BG2PA), '<- the affine matrix is NOT touched here');

const s16 = (a) => (bus.read16(a) << 16) >> 16;
const sine = (i) => s16(A_SINE + 2 * i);

// ---------------------------------------------------------------------------
// helper: watch the four reference-point halfwords for one frame
// ---------------------------------------------------------------------------
async function traceRefPointWrites(frames = 1) {
    const hits = [];
    const w = eng.watchMemory({
        address: REG_BG2X_L,
        length: 8, // REG_BG2X_L/_H, REG_BG2Y_L/_H
        filter: (h) => {
            hits.push({ addr: h.address >>> 0, value: h.value, pc: h.instructionAddress >>> 0, vcount: eng.read16(REG_VCOUNT) & 0xff, loc: h.location });
            return false;
        },
    });
    await eng.wait({ frames });
    w.stop();
    return hits;
}
const inFn = (pc) => pc >= FN && pc < FN + 0x44;
const srcOf = (h) => (h.loc ? `${h.loc.func} ${h.loc.file.replace(/^.*\/src\//, 'src/')}:${h.loc.line}` : (di.pcToFunction(h.pc)?.name ?? '?'));

// ===========================================================================
// Q1 INTERVENTION: the world map, where H-Blank is on
// ===========================================================================
console.log('\n=== Q1 world map (savestate-in-gameplay): is it entered per scanline? ===');
await eng.loadState(`${SAVES}/savestate-in-gameplay.json`);
await eng.wait({ frames: 8 });
const ie = eng.read16(REG_IE);
const dst = eng.read16(REG_DISPSTAT);
const hb = bus.read32(A_INTR + M_HBLANK.offset) >>> 0;
console.log(`  REG_IE=${hex(ie)}  INTR_FLAG_HBLANK set: ${(ie & INTR_FLAG_HBLANK) !== 0}   INTR_FLAG_VBLANK set: ${(ie & INTR_FLAG_VBLANK) !== 0}`);
console.log(`  REG_DISPSTAT=${hex(dst)}  DISPSTAT_HBLANK_INTR set: ${(dst & DISPSTAT_HBLANK_INTR) !== 0}`);
console.log(`  gIntrTable.hBlank=${hex(hb)} -> ${di.addressToSymbol(hb & ~1)?.name}  (function+thumb bit = ${hex(FN | 1)})`);
await eng.takeScreenshot({ name: 'worldmap-baseline' });

const mapHits = await traceRefPointWrites(1);
const byPc = new Map();
for (const h of mapHits) byPc.set(h.pc, (byPc.get(h.pc) ?? 0) + 1);
console.log(`  writes to REG_BG2X/REG_BG2Y in one frame: ${mapHits.length}`);
for (const [pc, n] of [...byPc].sort((a, b) => b[1] - a[1]).slice(0, 6)) {
    console.log(`    ${hex(pc)} x${n}  ${inFn(pc) ? 'INSIDE UpdateAffineBGParams' : srcOf(mapHits.find((h) => h.pc === pc))}`);
}
const fnHits = mapHits.filter((h) => inFn(h.pc));
const lines = [...new Set(fnHits.map((h) => h.vcount))].sort((a, b) => a - b);
console.log(`  hits from inside the function: ${fnHits.length} over ${lines.length} distinct REG_VCOUNT values`);
console.log(`  REG_VCOUNT range ${lines[0]}..${lines[lines.length - 1]}; first ten: ${lines.slice(0, 10).join(',')}`);
console.log(`  every scanline exactly once (4 halfwords each): ${fnHits.length === 4 * lines.length}`);

// ===========================================================================
// Q1 CONTROL: an in-level state with INTR_FLAG_HBLANK clear
// ===========================================================================
console.log('\n=== Q1 CONTROL in-level (savestate-in-level-idle): H-Blank IRQ off ===');
await eng.loadState(`${SAVES}/savestate-in-level-idle.json`);
await eng.wait({ frames: 8 });
const ie2 = eng.read16(REG_IE);
const dst2 = eng.read16(REG_DISPSTAT);
const hb2 = bus.read32(A_INTR + M_HBLANK.offset) >>> 0;
console.log(`  REG_IE=${hex(ie2)}  INTR_FLAG_HBLANK set: ${(ie2 & INTR_FLAG_HBLANK) !== 0}`);
console.log(`  REG_DISPSTAT=${hex(dst2)}  DISPSTAT_HBLANK_INTR set: ${(dst2 & DISPSTAT_HBLANK_INTR) !== 0}`);
console.log(`  gIntrTable.hBlank still ${hex(hb2)} -> ${di.addressToSymbol(hb2 & ~1)?.name} (installed but not armed)`);
const lvlHits = await traceRefPointWrites(1);
console.log(`  writes to REG_BG2X/REG_BG2Y in one frame: ${lvlHits.length}`);
for (const h of [...new Map(lvlHits.map((h) => [h.pc, h])).values()]) {
    console.log(`    ${hex(h.pc)}  ${inFn(h.pc) ? 'INSIDE UpdateAffineBGParams' : srcOf(h)}  at REG_VCOUNT=${h.vcount}`);
}
console.log(`  hits from inside the function: ${lvlHits.filter((h) => inFn(h.pc)).length}`);

// ===========================================================================
// Q2 / Q3 / Q4 all run on the world map
// ===========================================================================
await eng.loadState(`${SAVES}/savestate-in-gameplay.json`);
await eng.wait({ frames: 8 });

// --- Q1b: does it touch ANY other hardware register? ----------------------
console.log('\n=== Q1b every I/O register written from inside the function, one frame ===');
const io = new Map();
const wIo = eng.watchMemory({
    address: 0x04000000,
    length: 0x60,
    filter: (h) => {
        if (inFn(h.instructionAddress >>> 0)) io.set(h.address >>> 0, (io.get(h.address >>> 0) ?? 0) + 1);
        return false;
    },
});
await eng.wait({ frames: 1 });
wIo.stop();
const label = { 0x04000028: 'REG_BG2X_L', 0x0400002a: 'REG_BG2X_H', 0x0400002c: 'REG_BG2Y_L', 0x0400002e: 'REG_BG2Y_H' };
for (const [a, n] of [...io].sort((x, y) => x[0] - y[0])) console.log(`    ${hex(a)} x${n}  ${label[a] ?? '*** UNEXPECTED REGISTER ***'}`);
console.log(`  registers written: ${io.size} (REG_BG2PA..REG_BG2PD 0x04000020..0x04000026 among them: ${[...io.keys()].some((a) => a >= 0x04000020 && a <= 0x04000026)})`);
const dispcnt = eng.readDisplayControl();
console.log(`  display: mode ${dispcnt.mode}, BG2 on: ${dispcnt.bg[2]}, REG_BG2PA..PD = ${[0, 2, 4, 6].map((o) => s16(REG_BG2PA + o)).join(' , ')}`);
// io_reg.h: REG_OFFSET_WIN0H 0x40, WIN1H 0x42, WIN0V 0x44, WIN1V 0x46 -- the V
// registers are the ones that could clip the affine layer to a band of scanlines.
const win = (o) => eng.read16(0x04000040 + o);
console.log(`  REG_WIN0H=${hex(win(0))} REG_WIN1H=${hex(win(2))} REG_WIN0V=${hex(win(4))} REG_WIN1V=${hex(win(6))} REG_WININ=${hex(win(8))} REG_WINOUT=${hex(win(10))}`);
console.log(`  (a WINnV of 0xTTBB clips that window to scanlines 0xTT..0xBB)`);

console.log('\n=== Q3 are the two pool words sin/cos of gBg2Alpha? ===');
const wDelta = eng.watchMemory({ address: deltas[0], length: 2, maxHits: 4 });
const wDelta2 = eng.watchMemory({ address: deltas[1], length: 2, maxHits: 4 });
await eng.wait({ frames: 2 });
wDelta.stop();
wDelta2.stop();
for (const [name, w] of [[hex(deltas[0]), wDelta], [hex(deltas[1]), wDelta2]]) {
    const h = w.hits[0];
    console.log(`  ${name} written by ${h ? hex(h.instructionAddress) + '  ' + (h.location ? h.location.func + ' ' + h.location.file.replace(/^.*\/src\//, 'src/') + ':' + h.location.line : '(no source)') : '(nobody)'}`);
}
console.log('  CONTROL (no forcing):  gBg2Alpha =', bus.read8(A_ALPHA),
    ` ${hex(deltas[0])}=${s16(deltas[0])} ${hex(deltas[1])}=${s16(deltas[1])}`,
    ` | SIN=${sine(bus.read8(A_ALPHA))} COS=${sine(bus.read8(A_ALPHA) + PI_2)}`);
console.log('  INTERVENTION: force gBg2Alpha each frame and read both words back');
let sinOk = true, cosOk = true;
for (const a of [0, 1, 16, 32, 64, 100, 128, 192, 255]) {
    eng.onFrame(() => bus.write8(A_ALPHA, a));
    await eng.wait({ frames: 3 });
    eng.onFrame(null);
    const d0 = s16(deltas[0]), d1 = s16(deltas[1]);
    const S = sine(a), C = sine(a + PI_2);
    sinOk &&= d0 === S;
    cosOk &&= d1 === C;
    console.log(`    alpha=${String(a).padStart(3)}  ${hex(deltas[0])}=${String(d0).padStart(5)} (SIN=${String(S).padStart(5)} ${d0 === S ? 'match' : 'MISMATCH'})   ${hex(deltas[1])}=${String(d1).padStart(5)} (COS=${String(C).padStart(5)} ${d1 === C ? 'match' : 'MISMATCH'})`);
}
console.log(`  verdict: ${hex(deltas[0])} == SIN(gBg2Alpha): ${sinOk};  ${hex(deltas[1])} == COS(gBg2Alpha): ${cosOk}`);

// --- Q2b: which delta drives which axis -----------------------------------
console.log('\n=== Q2 which delta drives which axis? (force one, hold the other) ===');
async function rampOf() {
    // Reconstruct the 32-bit value written per scanline from the L/H halfword pairs.
    const hits = await traceRefPointWrites(1);
    const perLine = new Map();
    for (const h of hits.filter((x) => inFn(x.pc))) {
        const e = perLine.get(h.vcount) ?? { x: [0, 0], y: [0, 0] };
        if (h.addr === REG_BG2X_L) e.x[0] = h.value & 0xffff;
        else if (h.addr === REG_BG2X_L + 2) e.x[1] = h.value & 0xffff;
        else if (h.addr === REG_BG2Y_L) e.y[0] = h.value & 0xffff;
        else if (h.addr === REG_BG2Y_L + 2) e.y[1] = h.value & 0xffff;
        perLine.set(h.vcount, e);
    }
    const join = (p) => ((p[1] << 16) | p[0]) | 0;
    return [...perLine.entries()].sort((a, b) => a[0] - b[0]).map(([line, e]) => ({ line, x: join(e.x), y: join(e.y) }));
}
for (const [which, other] of [[0, 1], [1, 0]]) {
    eng.onFrame(() => {
        bus.write16(deltas[which], 0x0100);
        bus.write16(deltas[other], 0);
    });
    await eng.wait({ frames: 2 });
    const r = await rampOf();
    eng.onFrame(null);
    const first = r[0], last = r[r.length - 1];
    const spanX = last.x - first.x, spanY = last.y - first.y;
    console.log(`  force ${hex(deltas[which])}=0x100, ${hex(deltas[other])}=0 -> BG2X spans ${spanX}, BG2Y spans ${spanY}  => drives ${Math.abs(spanX) > Math.abs(spanY) ? 'X' : 'Y'}`);
}
eng.onFrame(null);
await eng.wait({ frames: 4 });

// --- Q4: where is the pivot line, and which rows does the ramp move? -------
console.log('\n=== Q4 the pivot scanline (measured from the writes) ===');
await eng.loadState(`${SAVES}/savestate-in-gameplay.json`);
await eng.wait({ frames: 8 });
// Force a known non-zero delta pair so the ramp is unambiguous, then locate the
// line whose written value equals the un-displaced gBg2X / gBg2Y.
eng.onFrame(() => {
    bus.write16(deltas[0], 0x0040);
    bus.write16(deltas[1], 0x0040);
});
await eng.wait({ frames: 2 });
const ramp = await rampOf();
const baseX = bus.read32(A_BG2X) | 0;
const baseY = bus.read32(A_BG2Y) | 0;
eng.onFrame(null);
console.log(`  gBg2X=${baseX} gBg2Y=${baseY}; per-scanline written value (every 20th line):`);
for (const r of ramp.filter((r) => r.line % 20 === 0)) {
    console.log(`    line ${String(r.line).padStart(3)}  BG2X=${String(r.x).padStart(9)} (gBg2X${r.x - baseX >= 0 ? '+' : ''}${r.x - baseX})  BG2Y=${String(r.y).padStart(9)} (gBg2Y${r.y - baseY >= 0 ? '+' : ''}${r.y - baseY})`);
}
const zeroX = ramp.find((r) => r.x === baseX);
const zeroY = ramp.find((r) => r.y === baseY);
console.log(`  scanline whose BG2X equals gBg2X unchanged: ${zeroX ? zeroX.line : '(none)'}`);
console.log(`  scanline whose BG2Y equals gBg2Y unchanged: ${zeroY ? zeroY.line : '(none)'}`);
const step = ramp.length > 1 ? (ramp[1].x - ramp[0].x) : 0;
console.log(`  per-scanline step at delta=0x40: ${step} (= 3 * 0x40 = ${3 * 0x40} -> the "3 *" in 3*line-180)`);

// --- Q4b: which rows of the picture does the ramp actually move? ----------
console.log('\n=== Q4 CONTROL: force both deltas to 0 and diff the framebuffer ===');
await eng.loadState(`${SAVES}/savestate-in-gameplay.json`);
await eng.wait({ frames: 8 });
const rowHash = () => Array.from({ length: 160 }, (_, y) => eng.hashRegion(0, y, 240, 1));
const before = rowHash();
await eng.takeScreenshot({ name: 'worldmap-deltas-natural' });
eng.onFrame(() => {
    bus.write16(deltas[0], 0);
    bus.write16(deltas[1], 0);
});
await eng.wait({ frames: 2 });
const after = rowHash();
await eng.takeScreenshot({ name: 'worldmap-deltas-zero' });
eng.onFrame(null);
const changed = after.map((h, y) => (h === before[y] ? null : y)).filter((y) => y !== null);
console.log(`  rows whose pixels changed when both deltas were zeroed: ${changed.length}`);
console.log(`  first changed row ${changed[0]}, last ${changed[changed.length - 1]}`);
const unchangedRuns = [];
for (let y = 0, run = null; y <= 160; y++) {
    const same = y < 160 && after[y] === before[y];
    if (same && !run) run = { from: y };
    if (!same && run) { run.to = y - 1; unchangedRuns.push(run); run = null; }
}
console.log('  unchanged row runs:', unchangedRuns.map((r) => `${r.from}..${r.to}`).join(' , '));
console.log('\nscreenshots in', OUT);
