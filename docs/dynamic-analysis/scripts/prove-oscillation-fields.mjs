// PROOF: what StreamCmd_InitOscillationExt actually writes into a GfxStreamEntry.
//
// StreamCmd_InitOscillationExt (0x0804DA60) writes six things into the
// GfxStreamEntry selected by stream byte[2] and then installs ProcessMotionStep
// (0x0804CAC8) at +0x20 and marks the entry active (byte0 & 7 = 1):
//
//   word0 bits 15..21 <- stream byte[3] & 0x7F   ("F_INDEX")
//   +0x08             <- stream byte[4]          ("F_08")
//   +0x0A             <- stream byte[5]          ("F_0A")
//   +0x14             <- ReadUnalignedS16(bytes[6..7])  ("F_14")
//   +0x1E             <- stream byte[8]          ("F_1E")
//   word0 bits 3..10  <- 2                       ("F_KIND")
//
// The meaning of those fields is decided entirely by the consumer,
// ProcessMotionStep, so this script RUNS ProcessMotionStep on the real ROM under
// the real CPU and varies ONE field at a time.
//
// The gfx-stream engine (gBuffer_52A4) is not live during ordinary gameplay -- it
// is a cutscene/scripted-graphics system -- so instead of waiting for an in-game
// occurrence we build the entry ourselves in scratch EWRAM, point gBuffer_52A4 at
// it, and call the ROM's own handler directly with r0 = entry index. Every
// instruction executed is real ROM code; only the inputs are ours.
//
//   node docs/dynamic-analysis/scripts/prove-oscillation-fields.mjs
import { HeadlessRuntime, REPO } from './gba-kit.mjs';
import { mkdirSync } from 'fs';

const W = REPO;
const OUT = '/tmp/klonoa-osc-out';
mkdirSync(OUT, { recursive: true });
const rt = await HeadlessRuntime.create({ romPath: `${W}/baserom.gba`, elfPath: `${W}/klonoa-eod.elf`, outputDir: OUT, logFn: () => {} });
const eng = rt.engine, bus = rt.gba.bus, cpu = rt.gba.armCpu, di = eng.debugInfo;

// ---- everything below comes from the ELF, no magic numbers ------------------
const PROCESS_MOTION_STEP = di.symbolToAddress('ProcessMotionStep');
const OAM_TBL   = di.symbolToAddress('gUnk_03002920');   // ldscript symbol
const BGL_TBL   = di.symbolToAddress('gUnk_03003430');
const F_08 = di.structMember('GfxStreamEntry', 'unk_08');
const F_0A = di.structMember('GfxStreamEntry', 'unk_0A');
const F_14 = di.structMember('GfxStreamEntry', 'timer');
const F_1E = di.structMember('GfxStreamEntry', 'unk_1E');
const OAM_X = di.structMember('Unk_03002920', 'xPosBg2');
const OAM_Y = di.structMember('Unk_03002920', 'yPosBg2');
const OAM_STRIDE = 0x1c;   // sizeof(struct Unk_03002920)
const ENTRY_SIZE = 0x24;   // sizeof(struct GfxStreamEntry)
const GBUFFER_PTR = 0x030052a4;  // gBuffer_52A4 -- recovered from .debug_macinfo
const SCRATCH = 0x02038000;      // unused EWRAM, our fake entry array

console.log('ProcessMotionStep = 0x' + PROCESS_MOTION_STEP.toString(16),
            ' gUnk_03002920 = 0x' + OAM_TBL.toString(16),
            ' gUnk_03003430 = 0x' + BGL_TBL.toString(16));
console.log('DWARF: unk_08', JSON.stringify(F_08), 'unk_0A', JSON.stringify(F_0A),
            'timer', JSON.stringify(F_14), 'unk_1E', JSON.stringify(F_1E));

// ---- boot far enough that IWRAM/EWRAM are initialised -----------------------
await eng.pressSequence([[null, 200], ['start', 4], [null, 120]]);

const s16 = (v) => (v << 16) >> 16;
const RET = 0x08000000; // sentinel: never actually executed, just a return target

/** Call a thumb ROM function with one argument; returns r0. Real ROM code runs. */
function callThumb(addr, a0) {
  const saved = cpu.serialize();
  cpu.registers[0] = a0 >>> 0;
  cpu.registers[13] = 0x03007f00;      // private IRQ-free stack area
  cpu.registers[14] = (RET | 1) >>> 0;
  cpu.registers[15] = (addr | 1) >>> 0;
  cpu.setT(true);
  let n = 0;
  while (n++ < 200000) {
    cpu.step();
    if (((cpu.registers[15] >>> 0) & ~1) === RET) break;
  }
  const r0 = cpu.registers[0] >>> 0;
  cpu.deserialize(saved);
  return { r0, steps: n };
}

/** Build one GfxStreamEntry exactly the way StreamCmd_InitOscillationExt does. */
function installEntry({ kind, index, f08, f0a, f14, f1e }) {
  const e = SCRATCH;
  for (let i = 0; i < ENTRY_SIZE * 2; i += 4) bus.write32(SCRATCH + i, 0);
  bus.write32(GBUFFER_PTR, SCRATCH);
  // word0: bits 3..10 = kind (what the ROM writes as `| 0x10` for kind 2),
  //        bits 15..21 = index, bits 0..2 = 1 (active)
  const word0 = ((index & 0x7f) << 15) | ((kind & 0xff) << 3) | 1;
  bus.write32(e + 0x00, word0 >>> 0);
  bus.write16(e + F_08.offset, f08 & 0xffff);
  bus.write16(e + F_0A.offset, f0a & 0xffff);
  bus.write16(e + F_14.offset, f14 & 0xffff);
  bus.write8(e + F_1E.offset, f1e & 0xff);
  bus.write32(e + 0x20, (PROCESS_MOTION_STEP | 1) >>> 0);
  return e;
}
const oamX = (i) => s16(bus.read16(OAM_TBL + i * OAM_STRIDE + OAM_X.offset));
const oamY = (i) => s16(bus.read16(OAM_TBL + i * OAM_STRIDE + OAM_Y.offset));
const zeroOam = (i) => { bus.write16(OAM_TBL + i * OAM_STRIDE + OAM_X.offset, 0);
                         bus.write16(OAM_TBL + i * OAM_STRIDE + OAM_Y.offset, 0); };

/** Run `frames` calls, returning the per-call DELTA applied to the target slot. */
function runOsc(cfg, frames) {
  const slot = (cfg.index & 0x7f) + 13;
  installEntry(cfg);
  zeroOam(slot);
  const e = SCRATCH, out = [];
  for (let f = 0; f < frames; f++) {
    const px = oamX(slot), py = oamY(slot);
    const { r0 } = callThumb(PROCESS_MOTION_STEP, 0);
    out.push({ f, dx: oamX(slot) - px, dy: oamY(slot) - py,
               timer: s16(bus.read16(e + F_14.offset)), ret: r0 });
  }
  return out;
}

const fmt = (rows, k) => rows.map(r => String(r[k]).padStart(4)).join(' ');

console.log('\n================ E1: +0x1E controls the ANGULAR STEP =================');
console.log('same +0x08 amplitude (=64), same start timer (=64), only +0x1E varies.');
console.log('dx per call traces a sine; a bigger +0x1E walks the 256-entry table faster.');
for (const step of [1, 2, 4, 8]) {
  const rows = runOsc({ kind: 2, index: 3, f08: 64, f0a: 0, f14: 64, f1e: step }, 16);
  console.log(`  +0x1E=${String(step).padStart(2)}  dx:`, fmt(rows, 'dx'));
}

console.log('\n================ E2: +0x08 is the X amplitude ========================');
console.log('identical phase (+0x1E=4, +0x14=64), only +0x08 varies -> dx scales linearly.');
for (const amp of [16, 32, 64, 128]) {
  const rows = runOsc({ kind: 2, index: 3, f08: amp, f0a: 0, f14: 64, f1e: 4 }, 10);
  console.log(`  +0x08=${String(amp).padStart(3)}  dx:`, fmt(rows, 'dx'), '  dy:', fmt(rows, 'dy'));
}

console.log('\n================ E3: +0x0A is the Y amplitude (independent) ==========');
for (const amp of [16, 64]) {
  const rows = runOsc({ kind: 2, index: 3, f08: 0, f0a: amp, f14: 64, f1e: 4 }, 10);
  console.log(`  +0x0A=${String(amp).padStart(3)}  dx:`, fmt(rows, 'dx'), '  dy:', fmt(rows, 'dy'));
}
{
  const rows = runOsc({ kind: 2, index: 3, f08: 64, f0a: 32, f14: 64, f1e: 4 }, 10);
  console.log('  +0x08=64,+0x0A=32   dx:', fmt(rows, 'dx'), '  dy:', fmt(rows, 'dy'), ' <- dy = dx/2 exactly');
}

console.log('\n================ E4: +0x14 is a frame countdown, and it ENDS the effect');
console.log('start +0x14 = 5; the ROM decrements it once per call and returns 0 when spent.');
{
  const rows = runOsc({ kind: 2, index: 3, f08: 64, f0a: 0, f14: 5, f1e: 4 }, 9);
  console.log('  timer:', fmt(rows, 'timer'));
  console.log('  ret  :', fmt(rows, 'ret'), '  (ProcessAnimationSteps stores ret in byte0&7: 0 = entry deactivated)');
  console.log('  dx   :', fmt(rows, 'dx'), '  (no further motion once ret==0)');
}

console.log('\n================ E5: word0 bits 3..10 select the TARGET ==============');
for (const kind of [0, 2]) {
  const slot = 3 + 13;
  zeroOam(slot);
  const bgBefore = [];
  for (let i = 0; i < 3; i++) bgBefore.push(bus.read16(BGL_TBL + i * 28 + 8));
  installEntry({ kind, index: 3, f08: 64, f0a: 64, f14: 64, f1e: 4 });
  for (let f = 0; f < 6; f++) callThumb(PROCESS_MOTION_STEP, 0);
  const bgAfter = [];
  for (let i = 0; i < 3; i++) bgAfter.push(bus.read16(BGL_TBL + i * 28 + 8));
  const bgMoved = bgBefore.map((v, i) => bgAfter[i] - v);
  console.log(`  kind=${kind}: gUnk_03002920[${slot}].xPosBg2 moved ${oamX(slot)}` +
              `   gUnk_03003430[].scrollX moved [${bgMoved.join(',')}]`);
}

console.log('\n================ E6: word0 bits 15..21 select WHICH object ===========');
for (const index of [0, 3, 7]) {
  for (let i = 10; i < 24; i++) zeroOam(i);
  installEntry({ kind: 2, index, f08: 64, f0a: 0, f14: 64, f1e: 4 });
  for (let f = 0; f < 6; f++) callThumb(PROCESS_MOTION_STEP, 0);
  const moved = [];
  for (let i = 10; i < 24; i++) if (oamX(i) !== 0) moved.push(i);
  console.log(`  bits15..21=${index}  -> moved gUnk_03002920 slot(s) [${moved.join(',')}]   (expected ${index}+13=${index + 13})`);
}

console.log('\n================ E7: the table ProcessMotionStep indexes is gSineTable');
console.log('CalcSineVelocity (0x0804CA6C) indexes 0x080D8E14 with ((+0x14 * +0x1E) & 0xFF)');
console.log('and multiplies by the amplitude, >>8.  The values below identify the table:');
{
  const T = di.symbolToAddress('gSineTable');
  console.log('  gSineTable (ldscript symbol) = 0x' + T.toString(16), T === 0x080d8e14 ? '(== 0x080D8E14)' : '(MISMATCH)');
  const idx = [0, 32, 64, 96, 128, 160, 192, 224];
  const got = idx.map(i => s16(bus.read16(T + i * 2)));
  const want = idx.map(i => Math.round(Math.sin((i / 256) * 2 * Math.PI) * 256));
  console.log('  index      :', idx.map(v => String(v).padStart(5)).join(' '));
  console.log('  ROM value  :', got.map(v => String(v).padStart(5)).join(' '));
  console.log('  Q_8_8 sin  :', want.map(v => String(v).padStart(5)).join(' '));
}
