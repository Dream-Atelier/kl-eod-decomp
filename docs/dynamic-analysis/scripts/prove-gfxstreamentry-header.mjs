// PROOF / REFUTATION: the packed header halfword of `struct GfxStreamEntry` (+0x00).
//
// The layout `type:3 / param:8 / speed:4 / dir:1` was DERIVED STATICALLY, from the
// masks that StreamCmd_InitStaticScroll (0x0804DB38) writes:
//     strb: x & -8   | 1            -> a 3-bit field at byte 0, bits 0-2
//     strh: x & 0xFFFFF807          -> an 8-bit field spanning bits 3-10 of the halfword
//     strb: x & -0x79 | (v&0xF)<<3  -> a 4-bit field at byte 1, bits 3-6
// The *widths and positions* are solid (they fall straight out of the masks, and
// gba-kit reads them back out of the DWARF, below). The *names* were guesses.
//
// This script tested the third guess — `speed` — at runtime and REFUTED it: the field
// is a BG-LAYER INDEX, not a rate. `include/gfx.h` now calls it `bgLayer`.
//
// Static counter-evidence first, from the handler this command installs,
// ProcessStaticBGScroll (0x0804CF26):
//     ldrb r1,[r3,#0x01] ; lsls r1,#0x19 ; lsrs r1,#0x1C   -> byte1 bits 3-6 (the field)
//     lsls r0,r1,#3 ; subs r0,r0,r1 ; lsls r0,r0,#2        -> field * 28
//     adds r0,r0,r2                                        -> + gUnk_03003430
//     ldrh r1,[r3,#0x08] ; ldrh r4,[r0,#0x08] ; adds r1,r1,r4 ; strh r1,[r0,#0x08]
// 28 == sizeof(struct BGLayerState). So the 4-bit field is used as an ARRAY INDEX
// into gBGLayerState, not as a rate. The per-frame step is entry->unk_08 / unk_0A.
//
// Runtime experiment (below): hijack a live stream entry, repoint its callback at
// ProcessStaticBGScroll, and sweep the 4-bit field over 0/1/2 while holding the step
// constant. A "speed" would change HOW FAST one layer moves; an index changes WHICH
// layer moves. The observed behaviour is the latter.
//
// Run:  node docs/dynamic-analysis/scripts/prove-gfxstreamentry-header.mjs
import { HeadlessRuntime } from './gba-kit.mjs';
import { mkdirSync } from 'fs';
import { ELF, ROM, readField, writeField } from './_harness.mjs';

const OUT = '/tmp/klonoa-dynamic-out';
mkdirSync(OUT, { recursive: true });
const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const eng = rt.engine,
  bus = rt.gba.bus,
  di = eng.debugInfo;

// Everything below is resolved from the ELF/DWARF — no hand-coded masks or offsets.
const F = {
  type: di.structMember('GfxStreamEntry', 'type'),
  param: di.structMember('GfxStreamEntry', 'param'),
  // The field under test. It was first guessed to be `speed`, then `bgLayer`;
  // this script refuted both and it is now `targetIndex`.
  field: di.structMember('GfxStreamEntry', 'targetIndex'),
  // Bits 15-21 of the header word: the +13-biased gUnk_03002920 object index.
  objIndex: di.structMember('GfxStreamEntry', 'objIndex'),
  stepX: di.structMember('GfxStreamEntry', 'unk_08'),
  stepY: di.structMember('GfxStreamEntry', 'unk_0A'),
  cb: di.structMember('GfxStreamEntry', 'callback'),
};
const SCROLLX = di.structMember('BGLayerState', 'scrollX');
const LAYERS = di.symbolToAddress('gUnk_03003430'); // gBGLayerState[]
const LAYER_STRIDE = 28; // sizeof(struct BGLayerState)
const HANDLER = (di.symbolToAddress('ProcessStaticBGScroll') | 1) >>> 0; // thumb bit
const ENTRY_STRIDE = 36; // sizeof(struct GfxStreamEntry)

console.log('DWARF-resolved layout of the GfxStreamEntry header word:');
for (const k of ['type', 'param', 'field', 'objIndex'])
  console.log(`  ${k.padEnd(6)} byte+${F[k].offset}  bits ${F[k].bitOffset}..${F[k].bitOffset + F[k].bitWidth - 1}  (width ${F[k].bitWidth})`);
console.log(`  ProcessStaticBGScroll = 0x${HANDLER.toString(16)}   gBGLayerState = 0x${LAYERS.toString(16)} (stride ${LAYER_STRIDE})\n`);

const scrollX = (l) => bus.read16(LAYERS + l * LAYER_STRIDE + SCROLLX.offset);
const allX = () => [0, 1, 2].map(scrollX);

// --- get into a state where the gfx stream is running and has live entries -----
await eng.pressSequence([...Array(8)].map(() => [null, 45]).flatMap((x) => [['start', 4], x]));
await eng.pressSequence([...Array(27)].map(() => [null, 40]).flatMap((x) => [['a', 4], x]));

const base = bus.read32(0x030052a4) >>> 0; // gBuffer_52A4 -> entry array
let slot = -1;
for (let i = 0; i < 32; i++) {
  const e = base + i * ENTRY_STRIDE;
  const cb = bus.read32(e + F.cb.offset) >>> 0;
  if (readField(bus, e, F.type) !== 0 && cb >>> 24 === 0x08 && cb & 1) { slot = i; break; }
}
if (slot < 0) { console.log('no live stream entry found — aborting'); process.exit(1); }
const E = base + slot * ENTRY_STRIDE;
console.log(`hijacking live stream entry #${slot} @ 0x${E.toString(16)} (was callback 0x${(bus.read32(E + F.cb.offset) >>> 0).toString(16)})\n`);

/** Install a static-scroll entry with the header field set to `v`, run `frames`, return per-layer scrollX deltas. */
async function trial(label, v, step, frames = 60, param = 0) {
  bus.write32(E + F.cb.offset, HANDLER);
  writeField(bus, E, F.type, 1); // 1 = active/dispatch (what StreamCmd_InitStaticScroll writes)
  writeField(bus, E, F.param, param);
  writeField(bus, E, F.field, v);
  bus.write16(E + F.stepX.offset, step);
  bus.write16(E + F.stepY.offset, 0);
  const before = allX();
  await eng.pressSequence([[null, frames]]);
  const after = allX();
  const d = [0, 1, 2].map((i) => ((after[i] - before[i]) << 16) >> 16);
  const held = (bus.read32(E + F.cb.offset) >>> 0) === HANDLER;
  console.log(
    `${label.padEnd(34)} dScrollX = [ layer0 ${String(d[0]).padStart(6)} | layer1 ${String(d[1]).padStart(6)} | layer2 ${String(d[2]).padStart(6)} ]${held ? '' : '   (entry was reclaimed!)'}`,
  );
  return d;
}

console.log('--- A. baseline: handler installed but step = 0 (isolates background camera motion) ---');
await trial('field=0  step=0', 0, 0);
await trial('field=1  step=0', 1, 0);
await trial('field=2  step=0', 2, 0);

console.log('\n--- B. sweep the 4-bit field, step held CONSTANT at 0x40 ---');
console.log('    "speed"       predicts: same layer moves, distance scales with the field');
console.log('    "layerIndex"  predicts: distance is identical, but a DIFFERENT layer moves');
const b0 = await trial('field=0  step=0x40', 0, 0x40);
const b1 = await trial('field=1  step=0x40', 1, 0x40);
const b2 = await trial('field=2  step=0x40', 2, 0x40);

console.log('\n--- C. hold the field, vary the step (which field is the real rate?) ---');
await trial('field=1  step=0x10', 1, 0x10);
await trial('field=1  step=0x20', 1, 0x20);
await trial('field=1  step=0x40', 1, 0x40);

console.log('\n--- D. the 8-bit `param` field as a gate (handler does: if (param != 0) skip) ---');
await trial('field=1  step=0x40  param=0', 1, 0x40, 60, 0);
await trial('field=1  step=0x40  param=1', 1, 0x40, 60, 1);
await trial('field=1  step=0x40  param=7', 1, 0x40, 60, 7);

console.log('\n=== VERDICT ===');
const moved = (d) => d.map((v, i) => (Math.abs(v) > 0 ? i : -1)).filter((i) => i >= 0);
console.log(`field=0 moved layer(s): [${moved(b0)}]   field=1: [${moved(b1)}]   field=2: [${moved(b2)}]`);
const isIndex = moved(b0).join() === '0' && moved(b1).join() === '1' && moved(b2).join() === '2';
console.log(isIndex ? 'field selects WHICH layer scrolls -> it is a BG-layer INDEX, not a speed.' : 'inconclusive');
