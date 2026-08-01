// PROOF (runtime): what the fields StreamCmd_InitOscillation writes actually mean.
//
// StreamCmd_InitOscillation fills a 36-byte GfxStreamEntry (in the array at
// gBuffer_52A4) from 9 stream bytes and installs ProcessMotionStep as the entry's
// per-frame callback. The field names in include/gfx.h were guesses from masks and
// an old comment; this script tests them by RUNNING the ROM's own ProcessMotionStep
// over entries we control, and watching what it writes.
//
// Why a direct call rather than "play until the game makes one": the gfx-stream
// entry dispatcher (ProcessAnimationSteps, 0x0804C8F4) never runs in any state
// reachable from boot -> title -> world map -> level 1-1 (see
// diag-stream-dispatch.mjs output: dispatcherRuns=false everywhere, and in-level
// the gBuffer_52A4 allocation is reused as a plain decompression buffer). So we
// boot to a real, fully-initialised in-level state, FREEZE it (no frames run, so
// nothing else can touch the BG scroll registers), point gBuffer_52A4 at a scratch
// entry, and invoke the real ProcessMotionStep on the real CPU, one instruction at
// a time. Every number below is produced by ROM code at 0x0804CAC8.
//
// Claims under test (each was a guess):
//   A. +0x01 bits[6:3]  named `speed`        -> REFUTED: selects WHICH BG layer
//   B. +0x08 / +0x0A    "phase counters"     -> REFUTED: X / Y amplitude
//   C. +0x14            "amplitude"          -> REFUTED: countdown timer + phase source
//   D. +0x1E            "wave index"         -> angular step per tick (phase = +0x1E * +0x14)
//   E. +0x01 bit 7      named `direction`    -> UNSUPPORTED: no consumer reads it
import { HeadlessRuntime, REPO } from './gba-kit.mjs';
import { mkdirSync } from 'fs';

const ROOT = REPO;
const OUT = '/tmp/klonoa-osc-out';
mkdirSync(OUT, { recursive: true });

const rt = await HeadlessRuntime.create({
    romPath: `${ROOT}/baserom.gba`,
    elfPath: `${ROOT}/klonoa-eod.elf`,
    outputDir: OUT,
    logFn: () => {},
});
const eng = rt.engine,
    bus = rt.gba.bus,
    cpu = rt.gba.armCpu,
    di = eng.debugInfo;

const hex = (n, w = 8) => '0x' + (n >>> 0).toString(16).padStart(w, '0');
const s16 = (v) => (v << 16) >> 16;

// Addresses and field layout come from the build (symtab / DWARF), not magic numbers.
const PROCESS_MOTION_STEP = di.symbolToAddress('ProcessMotionStep');
const READ_UNALIGNED_S16 = di.symbolToAddress('ReadUnalignedS16');
const BG_LAYER_STATE = di.symbolToAddress('gUnk_03003430'); // == gBGLayerState, stride 28
const G_BUFFER_52A4 = 0x030052a4; // a `#define`: lives in .debug_macinfo, not .symtab
const TRIG = 0x080d8e14; // s16 LUT that CalcSinCosVelocity indexes with (0x1E * 0x14) & 0xFF
const SCRATCH = 0x0203f000; // scratch entry array (game is frozen; nothing else runs)
const BG_STRIDE = 28;

const F = (n) => di.structMember('GfxStreamEntry', n);
const scrollX = (l) => bus.read16(BG_LAYER_STATE + l * BG_STRIDE + 0x08);
const scrollY = (l) => bus.read16(BG_LAYER_STATE + l * BG_STRIDE + 0x0a);

// ---------------------------------------------------------------- call harness
const TRAP = 0x08000000;
/** Invoke a Thumb ROM function on the real CPU, single-stepping until it returns. */
function callThumb(addr, r0) {
    const snap = cpu.serialize();
    cpu.registers[0] = r0 >>> 0;
    cpu.registers[14] = addr === 0 ? 0 : TRAP | 1;
    cpu.setT(true);
    cpu.registers[15] = addr & ~1;
    let n = 0;
    while ((cpu.registers[15] & ~1) !== TRAP && n < 500000) {
        cpu.step();
        n++;
    }
    const ret = cpu.registers[0] | 0;
    cpu.deserialize(snap);
    return { ret, steps: n };
}

// Boot -> title -> world map -> level 1-1, then stop advancing frames.
const ENTRY = [
    [null, 409], ['a', 5], [null, 20], ['a', 4], [null, 8], ['a', 4], [null, 8], ['a', 3], [null, 8], ['a', 4],
    [null, 51], ['a', 4], [null, 8], ['a', 3], [null, 89], ['a', 4], [null, 18], ['a', 4], [null, 6],
    ['start', 6], [null, 35], ['a', 6], [null, 20], ['a', 3], [null, 7], ['a', 4], [null, 42], ['a', 3],
    [null, 10], ['a', 4], [null, 20], ['start', 6], [null, 34], ['a', 6], [null, 20], ['a', 6], [null, 13],
    ['a', 4], [null, 124],
];
await eng.pressSequence(ENTRY);
await eng.pressSequence([['right', 60]]);
await eng.takeScreenshot({ name: 'osc-proof-state' });

console.log(`ProcessMotionStep = ${hex(PROCESS_MOTION_STEP)}   gBGLayerState = ${hex(BG_LAYER_STATE)}`);
const SP = F('targetIndex'); // renamed from `speed` after this script's verdict
console.log(
    `DWARF: GfxStreamEntry.targetIndex = +0x${SP.offset.toString(16)} bits[${SP.bitOffset}..${SP.bitOffset + SP.bitWidth - 1}]  (the field under test)\n`,
);

// --- harness sanity check: call a function whose answer we already know ---
bus.write8(SCRATCH + 0x100, 0x34);
bus.write8(SCRATCH + 0x101, 0xf2);
const chk = callThumb(READ_UNALIGNED_S16, SCRATCH + 0x100);
console.log(
    `harness check: ReadUnalignedS16([34 f2]) -> ${s16(chk.ret)} (expected ${s16(0xf234)}) in ${chk.steps} instructions` +
        `  => ${s16(chk.ret) === s16(0xf234) ? 'OK' : 'BROKEN'}\n`,
);

// ------------------------------------------------------------------- the entry
bus.write32(G_BUFFER_52A4, SCRATCH); // ProcessMotionStep reads the array base from here

function install({ field, ampX, ampY, timer, step, bit7 = 0 }) {
    for (let k = 0; k < 36; k++) bus.write8(SCRATCH + k, 0);
    bus.write16(SCRATCH + 0x00, 0); // target-selector bits[10:3] = 0 -> "BG layer" mode
    bus.write8(SCRATCH + 0x01, ((field & 0xf) << 3) | (bit7 << 7));
    bus.write16(SCRATCH + 0x08, ampX & 0xffff);
    bus.write16(SCRATCH + 0x0a, ampY & 0xffff);
    bus.write16(SCRATCH + 0x14, timer & 0xffff);
    bus.write8(SCRATCH + 0x1e, step);
    bus.write32(SCRATCH + 0x20, PROCESS_MOTION_STEP | 1);
    bus.write8(SCRATCH + 0x00, 1);
}

function trial(label, cfg) {
    install(cfg);
    const bx = [0, 1, 2, 3].map(scrollX),
        by = [0, 1, 2, 3].map(scrollY);
    const { ret } = callThumb(PROCESS_MOTION_STEP, 0);
    const dx = [0, 1, 2, 3].map((l) => s16(scrollX(l) - bx[l]));
    const dy = [0, 1, 2, 3].map((l) => s16(scrollY(l) - by[l]));
    const timerAfter = s16(bus.read16(SCRATCH + 0x14));
    console.log(
        `${label.padEnd(52)} dScrollX=[${dx.map((v) => String(v).padStart(5)).join(',')}]` +
            ` dScrollY=[${dy.map((v) => String(v).padStart(5)).join(',')}] +0x14:${cfg.timer}->${timerAfter} ret=${ret}`,
    );
    return { dx, dy, timerAfter, ret };
}

console.log('=== A. is +0x01 bits[6:3] a "speed", or an index picking WHICH BG layer? ===');
const A = [0, 1, 2, 3].map((f) =>
    trial(`field=${f}  amp=(0x140,0) step=3 timer=901`, { field: f, ampX: 0x140, ampY: 0, timer: 901, step: 3 }),
);

console.log('\n=== B. are +0x08/+0x0A "phase counters", or the X/Y amplitudes? ===');
const b1 = trial('field=1  amp=(0x140,0)      step=3 timer=901', { field: 1, ampX: 0x140, ampY: 0, timer: 901, step: 3 });
const b2 = trial('field=1  amp=(0x280,0) 2xX  step=3 timer=901', { field: 1, ampX: 0x280, ampY: 0, timer: 901, step: 3 });
const b3 = trial('field=1  amp=(0,0x140) Yonly step=3 timer=901', { field: 1, ampX: 0, ampY: 0x140, timer: 901, step: 3 });

console.log('\n=== C. is +0x14 an "amplitude", or a countdown timer? ===');
const c1 = trial('field=1  amp=(0x140,0) step=3 timer=901', { field: 1, ampX: 0x140, ampY: 0, timer: 901, step: 3 });
const c2 = trial('field=1  amp=(0x140,0) step=3 timer=1  ', { field: 1, ampX: 0x140, ampY: 0, timer: 1, step: 3 });
const c3 = trial('field=1  amp=(0x140,0) step=3 timer=0  ', { field: 1, ampX: 0x140, ampY: 0, timer: 0, step: 3 });

console.log('\n=== D. +0x1E: predicted dx = (trig[(0x1E * 0x14) & 0xFF] * ampX) >> 8 ===');
for (const st of [0, 1, 3, 17, 64]) {
    const timer = 901,
        ampX = 0x140;
    const idx = (st * timer) & 0xff;
    const pred = (Math.imul(s16(bus.read16(TRIG + idx * 2)), ampX) >> 8) & 0xffff;
    const r = trial(`field=1  amp=(0x140,0) step=${String(st).padStart(2)} timer=901`, {
        field: 1, ampX, ampY: 0, timer, step: st,
    });
    console.log(
        `      trig[(${st}*901)&0xFF = ${idx}] = ${s16(bus.read16(TRIG + idx * 2))}  =>  predicted dx=${s16(pred)}` +
            `  observed dx=${r.dx[1]}  ${s16(pred) === r.dx[1] ? 'MATCH' : 'MISMATCH'}`,
    );
}

console.log('\n=== E. does +0x01 bit 7 ("direction") change anything? ===');
const e0 = trial('field=1 bit7=0  amp=(0x140,0) step=3 timer=901', { field: 1, ampX: 0x140, ampY: 0, timer: 901, step: 3, bit7: 0 });
const e1 = trial('field=1 bit7=1  amp=(0x140,0) step=3 timer=901', { field: 1, ampX: 0x140, ampY: 0, timer: 901, step: 3, bit7: 1 });

console.log('\n=== VERDICTS ===');
const only = (r, l) => r.dx[l] !== 0 && r.dx.filter((v, i) => i !== l && v !== 0).length === 0;
console.log(`A  field=f moved ONLY BG layer f, for f=0,1,2,3: ${[0, 1, 2, 3].map((f) => only(A[f], f)).join(', ')}`);
console.log(`   -> +0x01 bits[6:3] is a BG-LAYER INDEX (scaled by 28 = sizeof BGLayerState). NOT a speed.`);
console.log(`B  doubling +0x08 doubled dx: ${b1.dx[1]} -> ${b2.dx[1]} (ratio ${b2.dx[1] / b1.dx[1]}); +0x0A drives dScrollY only: dy=${b3.dy[1]}, dx=${b3.dx[1]}`);
console.log(`   -> +0x08/+0x0A are the X/Y AMPLITUDES, not phase counters.`);
console.log(`C  +0x14 after one call: 901->${c1.timerAfter}, 1->${c2.timerAfter}; at 0 the callback returns ${c3.ret} (0 = retire)`);
console.log(`   -> +0x14 is a COUNTDOWN TIMER (decremented once per tick, drives the phase, and ends the effect).`);
console.log(`E  bit7=0 dx=${e0.dx[1]}  vs  bit7=1 dx=${e1.dx[1]}  -> ${e0.dx[1] === e1.dx[1] ? 'IDENTICAL: bit 7 is not read' : 'differs'}`);
