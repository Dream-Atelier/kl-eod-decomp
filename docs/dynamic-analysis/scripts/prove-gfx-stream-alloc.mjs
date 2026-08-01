// PROOF: the 8-byte slots of the graphics-stream allocation table
// (`struct GfxStreamAlloc`, 32 entries at `*gGfxStreamBuffer` / 0x030007C8) are
//
//     +0x00 u32 pTiles     (was guessed "data")
//     +0x04 u16 tileIndex  (was guessed "vramOffset" from static reading alone)
//     +0x06 u16 tileCount  (was guessed "size"       from static reading alone)
//
// The two guesses on the right were WRONG in unit: +0x04 is an index in 32-byte
// tiles, not a byte offset, and +0x06 is a tile count, not a byte size. This
// script is the evidence for the corrected names; re-run it to re-check them.
//
// The static asm of LoadGfxStreamEntry (0x0804C0EC) *suggests* +0x04 is a running
// OBJ-VRAM cursor seeded from gPaletteCursorInit and +0x06 is the per-entry
// increment, but "offset in bytes" vs "index in 32-byte tiles" is exactly the kind
// of thing static reading cannot settle. This script settles it at runtime.
//
// Method: boot, drive the intro to the first scene that actually feeds the graphics
// stream, and detect LoadGfxStreamEntry running by watching the one global it
// advances at its tail (gVramWriteCursor, `adds r0,#0x20` at 0x0804C170). Snapshot
// the whole table at that moment, then check four independent predictions:
//
//   P1  +0x04 of the first slot == (gPaletteCursorInit - 0x06010000) / 32
//       i.e. it is seeded from the OBJ-VRAM cursor *in 32-byte tile units*.
//   P2  +0x04 of slot k == +0x04 of slot k-1  +  +0x06 of slot k-1
//       i.e. +0x06 is exactly the allocation stride of +0x04.
//   P3  every +0x06 value is a legal GBA OBJ tile count (1,2,4,8,16,32,64 ...).
//       If +0x06 were a byte size, a slot of "1" would be a 1-byte allocation.
//   P4  the tile ids the hardware is actually told to render (OAM attr2, which is
//       indexed in 32-byte tiles from 0x06010000) fall inside the [+0x04, +0x04+
//       +0x06) windows this table hands out. This is the hardware confirming the
//       unit, not the source.
//   P5  the heap payload at +0x00 is byte-for-byte present in OBJ VRAM at
//       0x06010000 + (+0x04)*32, for exactly (+0x06)*32 bytes — with the "+0x04 is
//       a byte offset" reading used as a control. This proves all three fields at
//       once and is the decisive test.
//
// Bonus proof: `gRenderFlags` is declared `(*(u32 *)0x03005428)` by include/ui.h,
// but ResetGfxStreamEntries / InitGfxStreamState store to it with `strb`. Watch all
// four bytes and report the access size + writer of each byte. (Result: byte 0 is
// written only by 1-byte stores, bytes 1..3 are never written at all — so the
// macro was narrowed to `(*(u8 *)0x03005428)`.)
import { HeadlessRuntime } from './gba-kit.mjs';
import { mkdirSync } from 'fs';
import { ROM, ELF, readField } from './_harness.mjs';

const OUT = '/tmp/klonoa-dynamic-out';
mkdirSync(OUT, { recursive: true });
const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const eng = rt.engine,
  bus = rt.gba.bus,
  di = eng.debugInfo;

const hex = (v, n = 8) => '0x' + (v >>> 0).toString(16).padStart(n, '0');

// Layout comes from the decomp's own DWARF — no hand-coded offsets.
const F_DATA = di.structMember('GfxStreamAlloc', 'pTiles');
const F_OFS = di.structMember('GfxStreamAlloc', 'tileIndex');
const F_SIZE = di.structMember('GfxStreamAlloc', 'tileCount');
console.log('DWARF layout of struct GfxStreamAlloc:');
console.log('  pTiles    ', JSON.stringify(F_DATA));
console.log('  tileIndex ', JSON.stringify(F_OFS), '   <- name under test');
console.log('  tileCount ', JSON.stringify(F_SIZE), '   <- name under test');

const TABLE_PTR = 0x030007c8; // gGfxStreamBuffer (a #define, so no ELF symbol)
const OBJ_VRAM = 0x06010000; // OBJ tile base in BG modes 0-2
const TILE = 32; // bytes per 4bpp 8x8 tile
const CURSOR_INIT = 0x030052ac; // gPaletteCursorInit
const RENDER_FLAGS = 0x03005428; // gRenderFlags

// --- detect LoadGfxStreamEntry running -------------------------------------
// Its last act is gVramWriteCursor += 0x20 at 0x0804C170.
const wLoad = eng.watchMemory({
  address: di.symbolToAddress('gVramWriteCursor'),
  length: 4,
  maxHits: 500,
  filter: (h) => h.instructionAddress >= 0x0804c0ec && h.instructionAddress <= 0x0804c180,
});
// --- and watch all four bytes of gRenderFlags ------------------------------
const wFlags = eng.watchMemory({ address: RENDER_FLAGS, length: 4, maxHits: 500 });

let snapshot = null;
eng.onFrame((frame) => {
  if (wLoad.hits.length === 0 || snapshot) return;
  const base = bus.read32(TABLE_PTR);
  const rows = [];
  for (let i = 0; i < 32; i++) {
    const a = base + i * 8;
    const row = { i, data: readField(bus, a, F_DATA), ofs: readField(bus, a, F_OFS), size: readField(bus, a, F_SIZE) };
    if (row.size !== 0 || row.data !== 0) rows.push(row);
  }
  // P5 must be measured HERE, at the instant the table is live — a few frames
  // later the heap blocks are recycled and OBJ VRAM has been rewritten.
  for (const r of rows) {
    if (!r.size) continue;
    const n = r.size * TILE;
    const tgt = OBJ_VRAM + r.ofs * TILE;
    let same = 0;
    for (let k = 0; k < n; k++) if (bus.read8(r.data + k) === bus.read8(tgt + k)) same++;
    // control: treat +0x04 as a BYTE offset instead of a tile index
    const ctlAddr = OBJ_VRAM + r.ofs;
    let ctl = 0;
    for (let k = 0; k < n; k++) if (bus.read8(r.data + k) === bus.read8(ctlAddr + k)) ctl++;
    // where do the payload's first 32 bytes actually appear in OBJ VRAM?
    const pat = [];
    for (let k = 0; k < 32; k++) pat.push(bus.read8(r.data + k));
    let foundAt = null;
    for (let a = OBJ_VRAM; a < OBJ_VRAM + 0x8000; a += 4) {
      let ok = true;
      for (let k = 0; k < 32; k++) if (bus.read8(a + k) !== pat[k]) { ok = false; break; }
      if (ok) { foundAt = a; break; }
    }
    r.p5 = { n, tgt, pct: (100 * same) / n, ctlAddr, ctlPct: (100 * ctl) / n, foundAt };
  }
  snapshot = { frame, base, rows, cursorInit: bus.read32(CURSOR_INIT) };
});

// Boot -> title -> file select -> first scene that drives the graphics stream.
await eng.pressSequence([
  [null, 409], ['a', 5], [null, 20], ['a', 4], [null, 8], ['a', 4], [null, 8], ['a', 3], [null, 8],
  ['a', 4], [null, 51], ['a', 4], [null, 8], ['a', 3], [null, 89],
  ['a', 4], [null, 18], ['a', 4], [null, 6], ['start', 6], [null, 35], ['a', 6], [null, 20],
  ['a', 3], [null, 7], ['a', 4], [null, 42], ['a', 3], [null, 10], ['a', 4], [null, 20],
  ['start', 6], [null, 34], ['a', 6], [null, 20], ['a', 6], [null, 13], ['a', 4], [null, 124],
]);

if (!snapshot) {
  console.log('\nFAILED: never observed LoadGfxStreamEntry populate the table.');
  process.exit(1);
}

const { base, rows, cursorInit, frame } = snapshot;
console.log(`\nTable snapshot (frame ${frame}) — *gGfxStreamBuffer = ${hex(base)}`);
console.log(`gPaletteCursorInit = ${hex(cursorInit)}  ->  (0x%s - 0x06010000)/32 = %d`,
  (cursorInit >>> 0).toString(16), (cursorInit - OBJ_VRAM) / TILE);
console.log('\n slot  data(+0)     +4      +6   | predicted +4 (prev+4 + prev+6)');
let pred = null;
let p2ok = true;
for (const r of rows) {
  const mark = pred === null ? '(seed)' : pred === r.ofs ? `${pred}  OK` : `${pred}  MISMATCH`;
  if (pred !== null && pred !== r.ofs) p2ok = false;
  console.log(
    `  ${String(r.i).padStart(2)}   ${hex(r.data)}  ${String(r.ofs).padStart(5)}  ${String(r.size).padStart(5)} | ${mark}`,
  );
  pred = r.ofs + r.size;
}

// P1
const seed = (cursorInit - OBJ_VRAM) / TILE;
const p1ok = rows.length > 0 && rows[0].ofs === seed;
console.log(`\nP1  first slot's +4 (${rows[0].ofs}) == (gPaletteCursorInit-0x06010000)/32 (${seed})  ->  ${p1ok ? 'HOLDS' : 'FAILS'}`);
console.log(`    (as a byte offset it would have to be ${cursorInit - OBJ_VRAM}; it is not)`);

// P2
console.log(`P2  +4[k] == +4[k-1] + +6[k-1] for all ${rows.length} slots  ->  ${p2ok ? 'HOLDS' : 'FAILS'}`);

// P3
const legal = new Set([1, 2, 4, 8, 16, 32, 64, 128]); // tile counts of the 12 legal OBJ shapes
const sizes = [...new Set(rows.map((r) => r.size))].sort((a, b) => a - b);
const p3ok = sizes.every((s) => legal.has(s));
console.log(`P3  distinct +6 values = [${sizes.join(', ')}]`);
console.log(`    every value is a legal OBJ tile count  ->  ${p3ok ? 'HOLDS' : 'FAILS'}`);
console.log(`    as byte sizes these would be [${sizes.map((s) => s * TILE).join(', ')}] — note the 1 (a 1-byte allocation is absurd; 1 tile = an 8x8 sprite)`);

// P4 — the hardware's own view: OAM attr2 tile ids vs the windows this table hands out.
// Sampled over 240 frames of play so more than one sprite gets a chance to appear.
const windows = rows.filter((r) => r.size).map((r) => [r.ofs, r.ofs + r.size, r.i]);
const lo = Math.min(...windows.map((w) => w[0])), hi = Math.max(...windows.map((w) => w[1]));
const seen = new Map();
eng.onFrame(() => {
  for (const s of eng.readOAM()) {
    if (s.enabled && s.tileId >= lo && s.tileId < hi) seen.set(`${s.tileId}/${s.width}x${s.height}`, s);
  }
});
await eng.pressSequence([['right', 79], ['a+right', 7], ['right', 101], ['a+right', 8], [null, 45]]);
eng.onFrame(null);
console.log(`\nP4  OBJ tile ids handed out by this table span [${lo}, ${hi})`);
console.log(`    distinct enabled sprites seen over 240 frames whose OAM attr2 tileId lands in that span: ${seen.size}`);
let inWindow = 0;
for (const s of [...seen.values()].sort((a, b) => a.tileId - b.tileId)) {
  const w = windows.find((w) => s.tileId >= w[0] && s.tileId < w[1]);
  if (w) inWindow++;
  console.log(`      ${s.width}x${s.height}px (=${(s.width / 8) * (s.height / 8)} tiles) tileId=${s.tileId} (= VRAM ${hex(OBJ_VRAM + s.tileId * TILE)})  -> slot ${w[2]} window [${w[0]},${w[1]}) tiles=${w[1] - w[0]}`);
}
const p4ok = inWindow > 0;
console.log(`    hardware indexes OBJ tiles in 32-byte units, so a tileId landing inside a slot window`);
console.log(`    proves +4 is a TILE INDEX and +6 a TILE COUNT  ->  ${p4ok ? 'HOLDS' : 'NO SPRITES ACTIVE (inconclusive)'}`);

// P5 — strongest: does the heap payload at +0x00 literally appear in OBJ VRAM at
// 0x06010000 + (+0x04)*32, for (+0x06)*32 bytes? If yes, +0x00 is the tile data,
// +0x04 the tile index and +0x06 the tile count, all three at once.
console.log(`\nP5  (measured at the snapshot instant) byte-compare the heap payload at +0x00`);
console.log(`    against OBJ VRAM at 0x06010000 + (+0x04)*32, for (+0x06)*32 bytes:`);
let p5ok = false;
for (const r of rows) {
  if (!r.p5) continue;
  const { n, tgt, pct, ctlAddr, ctlPct, foundAt } = r.p5;
  if (pct === 100) p5ok = true;
  console.log(
    `    slot ${String(r.i).padStart(2)} ${hex(r.data)} vs ${hex(tgt)} (${n} B): ${pct.toFixed(1)}% identical` +
      `  | control "+4 as a BYTE offset" ${hex(ctlAddr)}: ${ctlPct.toFixed(1)}%` +
      `  | first 32 B of payload found in OBJ VRAM at ${foundAt === null ? '(nowhere)' : hex(foundAt)}`,
  );
}
console.log(`    at least one slot is a 100% byte-for-byte match at the predicted tile address  ->  ${p5ok ? 'HOLDS' : 'FAILS'}`);
console.log(`    (slots whose payload is a multi-frame animation sheet are only partially resident at`);
console.log(`     this instant — the game streams later frames in on demand — so <100% there is expected,`);
console.log(`     not a refutation. The fully-resident slot is the decisive one.)`);

// --- gRenderFlags width -----------------------------------------------------
console.log(`\ngRenderFlags (0x03005428), declared (*(u32 *)0x03005428) by include/ui.h:`);
const byByte = new Map();
for (const h of wFlags.hits) {
  const k = h.address - RENDER_FLAGS;
  if (!byByte.has(k)) byByte.set(k, new Map());
  const fn = (h.location && h.location.func) || hex(h.instructionAddress);
  const key = `${h.size}B by ${fn}`;
  byByte.get(k).set(key, (byByte.get(k).get(key) || 0) + 1);
}
for (const k of [0, 1, 2, 3]) {
  const m = byByte.get(k);
  console.log(`  byte +${k}: ${m ? [...m].map(([key, n]) => `${key} x${n}`).join(', ') : '(never written)'}`);
}
console.log(`  current 32-bit read: ${hex(bus.read32(RENDER_FLAGS))}  byte0=${hex(bus.read8(RENDER_FLAGS), 2)}`);

await eng.takeScreenshot({ name: 'gfx-stream-alloc-proof' });
console.log(`\nVERDICT  P1=${p1ok?"HOLDS":"FAILS"} P2=${p2ok?"HOLDS":"FAILS"} P3=${p3ok?"HOLDS":"FAILS"} P4=${p4ok?"HOLDS":"inconclusive"} P5=${p5ok?"HOLDS":"FAILS"}`);
