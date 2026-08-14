// PROOF (runtime): StreamCmd_InitSpriteWave (0x0804DD48) starts a slow VERTICAL
// oscillation — a bob — of one stream-owned sprite. It is a wave, not a shake.
//
// The open questions this answers:
//   Q1. Is the effect a jitter/shake or a smooth wave? The 64-byte table at
//       0x081177F4 is a table of per-frame VELOCITIES, not positions: the handler's
//       callback ADDS amplitude * sample to the sprite's Y every frame. So the shape
//       that reaches the screen is the table's running sum, not the table. Row 0 reads
//       like a jitter (1,1,2,-2,-1,-1,0,0) and integrates to a smooth 4-pixel hump.
//   Q2. Which row do the shipped scripts actually select? (answered by
//       scan-gfx-stream-commands.mjs; reproduced here from the same command bytes)
//   Q3. What is the table at 0x03002920 that the callback indexes, and why +13?
//   Q4. Does the effect touch X at all? (No — only Y.)
//
// METHOD. ProcessAnimationSteps never runs in a reachable savestate, so this freezes
// a real in-level state and executes ROM code directly: the shipped `FF 4C ...` bytes
// -> StreamCmd_RunScript's dispatch table -> StreamCmd_InitSpriteWave -> the installed
// ProcessSpriteOscillation callback, ticked frame by frame while the sprite's position
// fields are read out of gEntityArray.
//
// Controls: the same command with the amplitude set to 0, and with the wave-row nibble
// pointing at row 2 (which the ROM data shows is all zeroes). Both must produce no
// motion at all while everything else is identical.
//
// Run: GBA_KIT=... node docs/dynamic-analysis/scripts/prove-sprite-wave.mjs
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
const s16 = (v) => (v << 16) >> 16;
const s8 = (v) => (v << 24) >> 24;

const OSC = mustSymbol(di, 'ProcessSpriteOscillation');
const INIT = mustSymbol(di, 'StreamCmd_InitSpriteWave');
const STREAM_PTR = mustSymbol(di, 'gStreamPtr');
const ENTITIES = mustSymbol(di, 'gEntityArray');
const ENTRIES_PTR = 0x030052a4; // gBuffer_52A4 (#define)

// The entity stride is not typed in: the ldscript defines gOamBuffer0/gOamBuffer1 as
// two adjacent entries of the same array, so their difference IS the stride, and
// gOamBuffer6 pins the biased slot the callback lands on.
const STRIDE = mustSymbol(di, 'gOamBuffer1') - mustSymbol(di, 'gOamBuffer0');
const OAM6 = mustSymbol(di, 'gOamBuffer6');
console.log('=== addresses ===');
console.log(`  ProcessSpriteOscillation  0x${OSC.toString(16)}`);
console.log(`  StreamCmd_InitSpriteWave  0x${INIT.toString(16)}`);
console.log(`  gEntityArray              0x${ENTITIES.toString(16)}   stride ${STRIDE} (0x${STRIDE.toString(16)})`);
console.log(
    `  gOamBuffer6               0x${OAM6.toString(16)}  =  gEntityArray + ${(OAM6 - ENTITIES) / STRIDE} * stride` +
        `   <- despite the name this is entry ${(OAM6 - ENTITIES) / STRIDE}, and it is exactly the +13 bias below`,
);

await eng.loadState(SAVESTATE);
await eng.wait({ frames: 4 });

// ═══════════════════════════════════════════════════════════════════════════════
// Q1/Q3 (cheap proof first). Read the two literals out of the callback itself, so
// neither the wave table nor the entity table is a hand-typed address.
// ═══════════════════════════════════════════════════════════════════════════════
const lits = [];
for (const i of eng.disassemble(OSC & ~1, 20, 'thumb')) {
    const m = /=(0x[0-9a-f]{8})/i.exec(i.instruction);
    // the printed number is the literal POOL address; the value lives at it
    if (m) lits.push(bus.read32(parseInt(m[1], 16)) >>> 0);
}
const WAVE = lits.find((v) => v >= 0x08000000 && v < 0x0a000000);
const OBJTAB = lits.find((v) => v === ENTITIES);
console.log(
    `\n  literals ProcessSpriteOscillation loads: ${lits.map((v) => '0x' + v.toString(16)).join(' ')}` +
        `\n  -> wave table 0x${WAVE.toString(16)},  object table 0x${(OBJTAB ?? 0).toString(16)} (${OBJTAB === ENTITIES ? 'IS gEntityArray' : 'NOT gEntityArray'})`,
);

console.log('\n=== Q1. the wave table, and what it becomes after the callback integrates it ===');
console.log('  (the callback does obj.yPosBg2 += amplitude * (s8)wave[i], so position is the running sum)');
for (let r = 0; r < 4; r++) {
    const row = [];
    for (let k = 0; k < 16; k++) row.push(s8(bus.read8(WAVE + r * 16 + k)));
    let acc = 0;
    const integral = row.map((v) => (acc += v));
    console.log(`  row ${r}  velocity: [${row.map((v) => String(v).padStart(2)).join(',')}]`);
    console.log(`         position: [${integral.map((v) => String(v).padStart(2)).join(',')}]  (x amplitude, in 1/16 px)`);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Run the shipped command.
// ═══════════════════════════════════════════════════════════════════════════════
const E = {
    xBg2: di.structMember('Unk_03002920', 'xPosBg2'),
    yBg2: di.structMember('Unk_03002920', 'yPosBg2'),
    xScr: di.structMember('Unk_03002920', 'xPosScreen'),
    yScr: di.structMember('Unk_03002920', 'yPosScreen'),
};
const F = {
    unk_0E: di.structMember('GfxStreamEntry', 'unk_0E'),
    unk_1A: di.structMember('GfxStreamEntry', 'unk_1A'),
    unk_1C: di.structMember('GfxStreamEntry', 'unk_1C'),
    unk_1E: di.structMember('GfxStreamEntry', 'unk_1E'),
    unk_1F: di.structMember('GfxStreamEntry', 'unk_1F'),
    objIndex: di.structMember('GfxStreamEntry', 'objIndex'),
    timer: di.structMember('GfxStreamEntry', 'timer'),
    callback: di.structMember('GfxStreamEntry', 'callback'),
};
const ENT_STRIDE = 36;
const SCRATCH_CMD = 0x0203f000,
    SCRATCH_ENT = 0x0203e000;

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
const HANDLER_4C = handler(0x4c);
console.log(
    `\n  FF 4C dispatches to 0x${HANDLER_4C.toString(16)} (StreamCmd_InitSpriteWave | thumb = 0x${(INIT | 1).toString(16)})`,
);

const slot = (n) => ENTITIES + n * STRIDE;
const pos = (n) => ({
    x: bus.read16(slot(n) + E.xBg2.offset),
    y: bus.read16(slot(n) + E.yBg2.offset),
    xs: bus.read16(slot(n) + E.xScr.offset),
    ys: bus.read16(slot(n) + E.yScr.offset),
});

async function run({ entry = 0x12, obj = 1, rowShift = 0x03, amplitude = 2, duration = -1, ticks = 140, watch = null }) {
    await eng.loadState(SAVESTATE);
    await eng.wait({ frames: 2 });
    bus.write32(ENTRIES_PTR, SCRATCH_ENT);
    for (let k = 0; k < 0x400; k++) bus.write8(SCRATCH_ENT + k, 0);
    const cmd = [0xff, 0x4c, entry, obj, rowShift, amplitude, duration & 0xff, (duration >> 8) & 0xff];
    cmd.forEach((b, i) => bus.write8(SCRATCH_CMD + i, b));
    bus.write32(STREAM_PTR, SCRATCH_CMD);
    call(HANDLER_4C, [0]);
    const ent = SCRATCH_ENT + entry * ENT_STRIDE;
    // NOTE: do NOT use readField() for objIndex. Its DWARF MemberLocation is
    // {offset:1, size:2, bitOffset:7, bitWidth:7} — a halfword container starting at
    // an ODD byte — and readField would issue bus.read16() on an unaligned address,
    // which this emulator answers with the aligned halfword. That silently returns a
    // different field (it read back 0 for a command byte of 1 while the ROM itself
    // used 1, which sent this script's trace to the wrong sprite until it was caught).
    // The bits are 15..21 of the 32-bit header, so read the header as a word.
    const installed = {
        objIndex: (bus.read32(ent) >>> 15) & 0x7f,
        unk_1A: bus.read16(ent + F.unk_1A.offset),
        unk_1C: bus.read16(ent + F.unk_1C.offset),
        unk_1E: bus.read8(ent + F.unk_1E.offset),
        unk_1F: bus.read8(ent + F.unk_1F.offset),
        timer: s16(bus.read16(ent + F.timer.offset)),
        cb: bus.read32(ent + F.callback.offset) >>> 0,
    };
    const watched = watch ?? [installed.objIndex + 13];
    const base = Object.fromEntries(watched.map((n) => [n, pos(n)]));
    const trace = [];
    for (let t = 1; t <= ticks; t++) {
        const r = call(installed.cb, [entry]);
        trace.push({
            t,
            ret: r.ret,
            d: Object.fromEntries(
                watched.map((n) => {
                    const p = pos(n);
                    return [
                        n,
                        {
                            dx: s16(p.x - base[n].x),
                            dy: s16(p.y - base[n].y),
                            dxs: s16(p.xs - base[n].xs),
                            dys: s16(p.ys - base[n].ys),
                        },
                    ];
                }),
            ),
        });
    }
    return { installed, trace, watched };
}

// ── the shipped command, verbatim: script[0] @0x0c1f `ff 4c 12 01 03 02 ff ff` ──
console.log('\n=== Q1/Q4. the shipped command `FF 4C 12 01 03 02 FF FF`, ticked 140 times ===');
const live = await run({});
const n = live.installed.objIndex + 13;
console.log(
    `  entry after the handler: objIndex=${live.installed.objIndex} (-> gEntityArray slot ${n})` +
        ` unk_1F(row)=${live.installed.unk_1F} unk_1A(shift)=${live.installed.unk_1A}` +
        ` unk_1C(amplitude)=${live.installed.unk_1C} timer=${live.installed.timer} unk_1E(finite?)=${live.installed.unk_1E}` +
        ` callback=0x${live.installed.cb.toString(16)}`,
);
console.log(
    `  baseline before any tick: slot ${n} yPosBg2=${pos(n).y} yPosScreen=${pos(n).ys}` +
        ` (the callback overwrites yPosScreen with yPosBg2 >> 4 on its first tick, which is the one-off jump below;` +
        ` the OSCILLATION is the change after that)`,
);
console.log('  tick :  dyPosBg2 (1/16 px)   dyPosScreen (px)   dxPosBg2   dxPosScreen   return');
for (const s of live.trace)
    if (s.t % 8 === 0 || s.t <= 2)
        console.log(
            `  ${String(s.t).padStart(4)} : ${String(s.d[n].dy).padStart(14)} ${String(s.d[n].dys).padStart(18)}` +
                ` ${String(s.d[n].dx).padStart(10)} ${String(s.d[n].dxs).padStart(13)} ${String(s.ret).padStart(8)}`,
        );
const dys = live.trace.map((s) => s.d[n].dy);
const dxs = live.trace.map((s) => s.d[n].dx || s.d[n].dxs);
console.log(
    `\n  Y displacement over 140 ticks: min ${Math.min(...dys)} max ${Math.max(...dys)} (1/16 px)` +
        `  =  ${Math.min(...dys) / 16} .. ${Math.max(...dys) / 16} pixels`,
);
console.log(`  X displacement over 140 ticks: ${dxs.every((v) => v === 0) ? 'ZERO on every tick' : 'NONZERO'}`);
const zeros = live.trace.filter((s) => s.d[n].dy === 0).map((s) => s.t);
console.log(
    `  ticks where Y is back at its starting value: ${zeros[0]}..${zeros.filter((v, i) => v === zeros[0] + i).pop()}` +
        ` and ${zeros.find((v) => v > 70)}..${zeros[zeros.length - 1]}  ->  period = 64 frames (~1.07 s at 59.7 Hz)`,
);
console.log(`  peak Y offset reached at ticks: ${live.trace.filter((s) => s.d[n].dy === Math.max(...dys)).map((s) => s.t).join(', ')}`);
console.log(`  callback ever returned 0 (retire) in 140 ticks: ${live.trace.some((s) => s.ret === 0) ? 'YES' : 'NO'}`);

// ── controls ──────────────────────────────────────────────────────────────────
console.log('\n=== controls: same command, one field changed ===');
for (const [label, opts] of [
    ['amplitude 0 (byte[5] = 0)', { amplitude: 0, ticks: 140 }],
    ['row 2 (byte[4] hi nibble = 2)', { rowShift: 0x23, ticks: 140 }],
    ['row 3 (byte[4] hi nibble = 3)', { rowShift: 0x33, ticks: 140 }],
    ['row 1 (byte[4] hi nibble = 1)', { rowShift: 0x13, ticks: 140 }],
]) {
    const r = await run(opts);
    const m = r.installed.objIndex + 13;
    const d = r.trace.map((s) => s.d[m].dy);
    console.log(
        `  ${label.padEnd(32)} row=${r.installed.unk_1F} amplitude=${r.installed.unk_1C}` +
            `  Y range over 140 ticks: ${Math.min(...d)} .. ${Math.max(...d)} (1/16 px)` +
            `   ${Math.min(...d) === 0 && Math.max(...d) === 0 ? '<- NO MOTION' : ''}`,
    );
}

// ── Q3: the +13 bias ──────────────────────────────────────────────────────────
console.log('\n=== Q3. objIndex selects gEntityArray slot objIndex + 13 ===');
const watch = [0, 1, 12, 13, 14, 15, 16, 17, 18];
for (const obj of [0, 1, 3, 5]) {
    const r = await run({ obj, ticks: 40, watch });
    const moved = watch.filter((k) => r.trace.some((s) => s.d[k].dy !== 0));
    console.log(
        `  objIndex=${obj}: slots that moved out of {${watch.join(',')}} -> {${moved.join(',')}}` +
            `   expected {${obj + 13}}   ${moved.length === 1 && moved[0] === obj + 13 ? 'MATCH' : 'MISMATCH'}`,
    );
}
console.log(
    '  slots 0..12 are the engine-owned sprites (TransformSingleEntityToScreen in src/code_0.c takes a',
);
console.log(
    '  different template table for arg0 > 0xC); every gfx-stream command that names a sprite adds 13,',
);
console.log('  e.g. StreamCmd_SetEntityTransform writes gUnk_03002920[gStreamPtr[2] + 0xD].');

// ── the -1 duration ───────────────────────────────────────────────────────────
console.log('\n=== the duration field: -1 means "run forever" ===');
for (const duration of [-1, 20]) {
    const r = await run({ duration, ticks: 40 });
    const retired = r.trace.findIndex((s) => s.ret === 0);
    console.log(
        `  duration=${String(duration).padStart(3)}  entry.timer=${r.installed.timer} entry.unk_1E=${r.installed.unk_1E}` +
            `  callback first returned 0 at tick ${retired < 0 ? 'never (40 ticks)' : retired + 1}`,
    );
}
