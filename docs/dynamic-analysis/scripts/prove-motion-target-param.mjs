// PROOF (runtime): the three "Init...Motion" gfx-stream commands are three TARGETS of
// one motion engine, not a base command plus extensions -- which is why 0x0804D4D8 is
// StreamCmd_InitEntityMotion and not StreamCmd_InitLinearMotionExt.
//
// The question. 0x0804D4D8 sat one line below StreamCmd_InitLinearMotion (0x0804D408)
// and was called "...LinearMotionExt", which reads as "the extended form of the
// command above". Its actual sibling, though, is StreamCmd_InitWindowCornerMotion:
// all three install the SAME callback (ProcessMotionStepExtended) with the same
// displacement/duration arithmetic, and they differ only in the `param` field of the
// GfxStreamEntry header -- which is the index ProcessMotionStepExtended uses into its
// jump table at 0x0804CCB0. So "Ext" names a relationship the ROM does not have.
//
// The experiment. For each command: run the real handler on a scratch stream, then
// tick the real ProcessMotionStepExtended ten times, and diff THREE candidate targets
// at once -- the BG layer scroll array, the entity array, and the hardware-window
// clip bounds. A command that owns a target moves that one and leaves the other two
// bit-identical. Each is paired with a dX = dY = 0 control that must move nothing at
// all, which is what separates "this command moved it" from "something else did".
//
// The tail. StreamCmd_InitEntityMotion ends with two stores into gUnk_03002920 that
// neither sibling has. This script shows what they are for: they seed the subpixel
// accumulator from the entity's current ON-SCREEN position, so the tween starts where
// the sprite already is instead of at the origin.
//
// Run: GBA_KIT=... node docs/dynamic-analysis/scripts/prove-motion-target-param.mjs
import { mkdirSync } from 'node:fs';
import { HeadlessRuntime, REPO, SAVESTATE } from './gba-kit.mjs';
import { ELF, readField } from './_harness.mjs';
import { makeCaller, mustSymbol } from './_callrom.mjs';

const OUT = '/tmp/klonoa-motion-target';
mkdirSync(OUT, { recursive: true });

const rt = await HeadlessRuntime.create({ romPath: `${REPO}/baserom.gba`, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const eng = rt.engine,
    bus = rt.gba.bus,
    cpu = rt.gba.armCpu,
    di = eng.debugInfo;
const call = makeCaller(cpu);

const hex = (n, w = 8) => '0x' + (n >>> 0).toString(16).padStart(w, '0');
let failures = 0;
const check = (ok, label) => {
    console.log(`   [${ok ? ' OK ' : 'FAIL'}] ${label}`);
    if (!ok) failures++;
};

const PLAIN = mustSymbol(di, 'StreamCmd_InitLinearMotion');
const ENTITY_CMD = mustSymbol(di, 'StreamCmd_InitEntityMotion');
const WINDOW_CMD = mustSymbol(di, 'StreamCmd_InitWindowCornerMotion');
const STEP = mustSymbol(di, 'ProcessMotionStepExtended');
const STREAM_PTR = mustSymbol(di, 'gStreamPtr');
const BG_LAYERS = mustSymbol(di, 'gUnk_03003430'); // == gBGLayerState
const ENTITIES = mustSymbol(di, 'gUnk_03002920');
const GFX_BUFFER_PTR = 0x030034a0; // gGfxBufferPtr / gLevelStatePtr, a #define
const ENTRIES_PTR = 0x030052a4; // gBuffer_52A4, a #define

const B = (n) => {
    const f = di.structMember('BGLayerState', n);
    if (!f) throw new Error(`BGLayerState.${n} is not in DWARF -- renamed?`);
    return f;
};
const U = (n) => {
    const f = di.structMember('Unk_03002920', n);
    if (!f) throw new Error(`Unk_03002920.${n} is not in DWARF -- renamed?`);
    return f;
};
const W = (n) => {
    const f = di.structMember('LevelWindowBounds', n);
    if (!f) throw new Error(`LevelWindowBounds.${n} is not in DWARF -- renamed?`);
    return f;
};
// strides come from DWARF too, so a layout change breaks the script instead of the result
const sizeOf = (n) => {
    const s = di.struct(n);
    if (!s?.size) throw new Error(`struct ${n} has no DWARF size`);
    return s.size;
};
const BG_STRIDE = sizeOf('BGLayerState');
const ENT_STRIDE = sizeOf('Unk_03002920');

console.log('StreamCmd_InitLinearMotion       =', hex(PLAIN));
console.log('StreamCmd_InitEntityMotion       =', hex(ENTITY_CMD));
console.log('StreamCmd_InitWindowCornerMotion =', hex(WINDOW_CMD));
console.log('ProcessMotionStepExtended        =', hex(STEP));
console.log('gBGLayerState =', hex(BG_LAYERS), ' gUnk_03002920 =', hex(ENTITIES), ' gGfxBufferPtr cell =', hex(GFX_BUFFER_PTR));

await eng.loadState(SAVESTATE);
await eng.wait({ frames: 2 });

// ═════════════════════════════════════════════════════════════════════════════
// 0. The jump table itself, read out of the ROM.
// ═════════════════════════════════════════════════════════════════════════════
console.log('\n=== 0. ProcessMotionStepExtended dispatches on `param` ===');
console.log('   the switch that reads it, and the table it indexes:');
for (const i of eng.disassemble(0x0804cc90, 8, 'thumb')) console.log(`      ${hex(i.address)}  ${i.instruction}`);
const JT = 0x0804ccb0;
for (let i = 0; i <= 4; i++) console.log(`      param ${i} -> ${hex(bus.read32(JT + i * 4))}`);

// ═════════════════════════════════════════════════════════════════════════════
// 1. Where each command lives in StreamCmd_RunScript's dispatch tables.
// ═════════════════════════════════════════════════════════════════════════════
const TABLES = [
    { base: 0x081179b4, n: 16, op: (i) => 0x80 | i },
    { base: 0x0811787c, n: 50, op: (i) => 0x40 | i },
    { base: 0x081178b8, n: 16, op: (i) => 0x30 | i },
    { base: 0x08117854, n: 16, op: (i) => 0x20 | i },
    { base: 0x0811790c, n: 16, op: (i) => 0x10 | i },
    { base: 0x081178d8, n: 16, op: (i) => 0x00 | i },
];
console.log('\n=== 1. dispatch opcodes ===');
const opsFor = (addr) => {
    const out = [];
    for (const t of TABLES) for (let i = 0; i < t.n; i++) if (((bus.read32(t.base + i * 4) >>> 0) & ~1) === addr) out.push(t.op(i));
    return out;
};
for (const [nm, a] of [
    ['StreamCmd_InitLinearMotion', PLAIN],
    ['StreamCmd_InitEntityMotion', ENTITY_CMD],
    ['StreamCmd_InitWindowCornerMotion', WINDOW_CMD],
])
    console.log(`   ${nm.padEnd(34)} ${opsFor(a).map((o) => 'FF ' + o.toString(16).padStart(2, '0')).join(', ')}`);

// ═════════════════════════════════════════════════════════════════════════════
// 2. The experiment.
// ═════════════════════════════════════════════════════════════════════════════
const SCRATCH_CMD = 0x0203f000,
    SCRATCH_ENT = 0x0203e000,
    SCRATCH_GFX = 0x0203d000;
const ENTRY_IDX = 1,
    OBJ_INDEX = 3,
    TARGET_INDEX = 2,
    OBJ_SLOT = OBJ_INDEX + 13;
const DX = 160,
    DY = 80,
    DUR = 20,
    TICKS = 10;

// The BG layer array is only zeroed for layers 0..3 ON PURPOSE: gBGLayerState is
// 0x03003430 with stride 28, so "layer 4" starts at 0x030034A0 -- which IS the
// gGfxBufferPtr / gLevelStatePtr cell. Wiping a fifth layer silently nulls the pointer
// the window arm follows, and the window arm then writes to address 8 and looks inert.
const BG_LAYERS_UNDER_TEST = 4;

function world() {
    bus.write32(ENTRIES_PTR, SCRATCH_ENT);
    for (let k = 0; k < 0x400; k++) bus.write8(SCRATCH_ENT + k, 0);
    bus.write32(GFX_BUFFER_PTR, SCRATCH_GFX);
    for (let k = 0; k < 0x40; k++) bus.write8(SCRATCH_GFX + k, 0);
    for (let k = 0; k < BG_LAYERS_UNDER_TEST * BG_STRIDE; k++) bus.write8(BG_LAYERS + k, 0);
    for (let k = 0; k < 32 * ENT_STRIDE; k++) bus.write8(ENTITIES + k, 0);
    // give every entity a distinct on-screen position, so a seed is recognisable
    for (let e = 0; e < 32; e++) {
        bus.write16(ENTITIES + e * ENT_STRIDE + U('xPosScreen').offset, 100 + e);
        bus.write16(ENTITIES + e * ENT_STRIDE + U('yPosScreen').offset, 200 + e);
    }
    for (let k = 0; k < 0x40; k++) bus.write8(SCRATCH_CMD + k, 0);
}
const snapBG = () => {
    const a = [];
    for (let l = 0; l < BG_LAYERS_UNDER_TEST; l++)
        a.push([bus.read16(BG_LAYERS + l * BG_STRIDE + B('scrollX').offset), bus.read16(BG_LAYERS + l * BG_STRIDE + B('scrollY').offset)]);
    return a;
};
const snapENT = () => {
    const a = [];
    for (let e = 13; e < 21; e++)
        a.push([
            bus.read16(ENTITIES + e * ENT_STRIDE + U('xPosBg2').offset),
            bus.read16(ENTITIES + e * ENT_STRIDE + U('yPosBg2').offset),
            bus.read16(ENTITIES + e * ENT_STRIDE + U('xPosScreen').offset),
            bus.read16(ENTITIES + e * ENT_STRIDE + U('yPosScreen').offset),
        ]);
    return a;
};
const snapWIN = () => {
    const a = [];
    for (const f of [W('leftTop'), W('rightBottom')]) for (let k = 0; k < 8; k += 2) a.push(bus.read16(SCRATCH_GFX + f.offset + k));
    return a;
};
const diff = (a, b) => JSON.stringify(b.map((row, i) => (Array.isArray(row) ? row.map((v, j) => v - a[i][j]) : row - a[i])));
const moved = (a, b) => JSON.stringify(a) !== JSON.stringify(b);

const s16b = (v) => [v & 0xff, (v >> 8) & 0xff];
const CMDS = {
    // [2] packs the entry index (low 6 bits) and targetIndex (high 2 bits) into ONE byte
    plain: (dx, dy) => [0xff, 0x41, ENTRY_IDX | (TARGET_INDEX << 6), ...s16b(dx), ...s16b(dy), ...s16b(DUR)],
    entity: (dx, dy) => [0xff, 0x42, ENTRY_IDX, OBJ_INDEX, ...s16b(dx), ...s16b(dy), ...s16b(DUR)],
    window: (dx, dy) => [0xff, 0x44, ENTRY_IDX, TARGET_INDEX, ...s16b(dx), ...s16b(dy), ...s16b(DUR)],
};

function trial(kind, fn, dx, dy) {
    world();
    CMDS[kind](dx, dy).forEach((b, i) => bus.write8(SCRATCH_CMD + i, b));
    bus.write32(STREAM_PTR, SCRATCH_CMD);
    const r = call(fn, [], 200000);
    if (r.timedOut) throw new Error(`${kind}: handler did not return`);
    const advance = ((bus.read32(STREAM_PTR) >>> 0) - SCRATCH_CMD) | 0;
    const entry = SCRATCH_ENT + ENTRY_IDX * 36;
    const header = bus.read32(entry) >>> 0;
    const seed = [bus.read16(ENTITIES + OBJ_SLOT * ENT_STRIDE + U('xPosBg2').offset), bus.read16(ENTITIES + OBJ_SLOT * ENT_STRIDE + U('yPosBg2').offset)];
    const b0 = snapBG(),
        e0 = snapENT(),
        w0 = snapWIN();
    for (let t = 0; t < TICKS; t++) call(STEP, [ENTRY_IDX], 400000);
    const b1 = snapBG(),
        e1 = snapENT(),
        w1 = snapWIN();
    return {
        advance,
        header,
        // The header is ONE u32 bitfield; decoding it by shifting the whole word is the
        // safe path. See the note on readField(objIndex) at the end of this script.
        param: (header >>> 3) & 0xff,
        targetIndex: (header >>> 11) & 0xf,
        objIndex: (header >>> 15) & 0x7f,
        type: header & 7,
        callback: bus.read32(entry + 0x20) >>> 0,
        seed,
        bg: [b0, b1],
        ent: [e0, e1],
        win: [w0, w1],
    };
}

console.log('\n=== 2. one command at a time: which of three targets moves? ===');
console.log(`   dX = ${DX} px, dY = ${DY} px over ${DUR} frames, then ${TICKS} ticks (so half the distance)`);
const results = {};
for (const [kind, fn, label] of [
    ['plain', PLAIN, 'StreamCmd_InitLinearMotion'],
    ['entity', ENTITY_CMD, 'StreamCmd_InitEntityMotion'],
    ['window', WINDOW_CMD, 'StreamCmd_InitWindowCornerMotion'],
]) {
    const live = trial(kind, fn, DX, DY);
    const ctrl = trial(kind, fn, 0, 0);
    results[kind] = live;
    console.log(`\n   -- ${label}  (FF ${CMDS[kind](0, 0)[1].toString(16)}, ${live.advance} bytes)`);
    console.log(`      header=${hex(live.header)}  param=${live.param}  targetIndex=${live.targetIndex}  objIndex=${live.objIndex}  type=${live.type}  callback=${hex(live.callback)}`);
    console.log(`      d BG layer scroll [l0..l3] : ${diff(live.bg[0], live.bg[1])}`);
    console.log(`      d entity[13..20] (xBg2,yBg2,xScr,yScr):`);
    for (let i = 0; i < 8; i++) {
        const d = live.ent[1][i].map((v, j) => v - live.ent[0][i][j]);
        if (d.some((v) => v !== 0)) console.log(`           slot ${13 + i}: ${JSON.stringify(d)}`);
    }
    if (!moved(live.ent[0], live.ent[1])) console.log('           (no entity moved)');
    console.log(`      d window bounds [leftTop x4, rightBottom x4]: ${diff(live.win[0], live.win[1])}`);
    console.log(`      CONTROL dX=dY=0: bg moved=${moved(ctrl.bg[0], ctrl.bg[1])} entity moved=${moved(ctrl.ent[0], ctrl.ent[1])} window moved=${moved(ctrl.win[0], ctrl.win[1])}`);
    check((live.callback & ~1) === STEP, `${label} installs ProcessMotionStepExtended`);
    check(
        !moved(ctrl.bg[0], ctrl.bg[1]) && !moved(ctrl.ent[0], ctrl.ent[1]) && !moved(ctrl.win[0], ctrl.win[1]),
        `${label}: the dX=dY=0 control moves NOTHING`,
    );
}

const p = results.plain,
    e = results.entity,
    w = results.window;
console.log('\n   -- the exclusivity claim --');
check(p.param === 0 && e.param === 2 && w.param === 4, `param is 0 / 2 / 4 for plain / entity / window (got ${p.param} / ${e.param} / ${w.param})`);
check(moved(p.bg[0], p.bg[1]) && !moved(p.ent[0], p.ent[1]) && !moved(p.win[0], p.win[1]), 'param 0 moves the BG layer scroll ONLY');
check(!moved(e.bg[0], e.bg[1]) && moved(e.ent[0], e.ent[1]) && !moved(e.win[0], e.win[1]), 'param 2 moves the ENTITY ONLY');
check(!moved(w.bg[0], w.bg[1]) && !moved(w.ent[0], w.ent[1]) && moved(w.win[0], w.win[1]), 'param 4 moves the WINDOW BOUNDS ONLY');

// the magnitudes: half the distance in half the duration => constant velocity
const bgD = p.bg[1][TARGET_INDEX].map((v, j) => v - p.bg[0][TARGET_INDEX][j]);
const entD = e.ent[1][OBJ_SLOT - 13].map((v, j) => v - e.ent[0][OBJ_SLOT - 13][j]);
console.log(`\n   BG layer ${TARGET_INDEX} scroll moved by ${JSON.stringify(bgD)} (subpixel, >>4 = ${bgD.map((v) => v >> 4)} px)`);
console.log(`   entity ${OBJ_SLOT} moved by ${JSON.stringify(entD)} (xBg2,yBg2 subpixel; xScr,yScr in px)`);
check(bgD[0] === (DX << 4) / 2 && bgD[1] === (DY << 4) / 2, `after half the duration the BG layer has travelled half the displacement`);
check(entD[2] === DX / 2 && entD[3] === DY / 2, `after half the duration the entity has travelled half the displacement in PIXELS -- constant velocity, i.e. LINEAR`);
check(e.objIndex === OBJ_INDEX, `byte[3] lands in objIndex (${e.objIndex}), and the entity that moved is slot objIndex + 13 = ${OBJ_SLOT}`);
check(p.targetIndex === TARGET_INDEX, 'the plain command packs entry index and targetIndex into ONE stream byte, unlike the other two');

// ═════════════════════════════════════════════════════════════════════════════
// 3. What the gUnk_03002920 tail is for.
// ═════════════════════════════════════════════════════════════════════════════
console.log('\n=== 3. the tail only StreamCmd_InitEntityMotion has ===');
const seeded = e.seed;
const scrX = 100 + OBJ_SLOT,
    scrY = 200 + OBJ_SLOT;
console.log(`   before the command, entity ${OBJ_SLOT} was at xPosScreen=${scrX} yPosScreen=${scrY} with xPosBg2=yPosBg2=0`);
console.log(`   after it, xPosBg2=${seeded[0]} yPosBg2=${seeded[1]}   (${scrX} << 4 = ${scrX << 4}, ${scrY} << 4 = ${scrY << 4})`);
check(seeded[0] === scrX << 4 && seeded[1] === scrY << 4, 'the tail seeds the subpixel accumulator from the on-screen position');
console.log('   and the step does the inverse each tick (`+0x04 = +0x00 >> 4`), so the tween');
console.log('   starts from where the sprite already is rather than from the origin.');
console.log('   the two sibling handlers have no such tail because their accumulators ARE the live values:');
check(results.plain.seed[0] === 0 && results.window.seed[0] === 0, 'neither sibling touches gUnk_03002920 at all');

// ═════════════════════════════════════════════════════════════════════════════
// 4. gba-kit hazard, demonstrated rather than described.
// ═════════════════════════════════════════════════════════════════════════════
console.log('\n=== 4. why this script decodes the header by hand instead of using readField ===');
const oi = di.structMember('GfxStreamEntry', 'objIndex');
console.log(`   DWARF says GfxStreamEntry.objIndex = ${JSON.stringify(oi)} -- a 2-byte container starting at byte 1.`);
const entry = SCRATCH_ENT + ENTRY_IDX * 36;
world();
CMDS.entity(DX, DY).forEach((b, i) => bus.write8(SCRATCH_CMD + i, b));
bus.write32(STREAM_PTR, SCRATCH_CMD);
call(ENTITY_CMD, [], 200000);
const viaField = readField(bus, entry, oi);
const viaWord = ((bus.read32(entry) >>> 0) >>> 15) & 0x7f;
console.log(`   readField(objIndex)          = ${viaField}`);
console.log(`   (header word >>> 15) & 0x7F  = ${viaWord}   <- the value the command was given (${OBJ_INDEX})`);
check(viaWord === OBJ_INDEX, 'the hand-decoded value is the one the ROM stored');
if (viaField !== viaWord)
    console.log('   -> readField reads the ALIGNED halfword for an odd-byte container, so it decodes the\n' + '      wrong bits. Reported as a gba-kit defect; this script does not use it for objIndex.');

console.log('\n=== verdict ===');
console.log('   One motion engine, three targets, selected by `param`:');
console.log('     param 0 -> gBGLayerState[targetIndex].scroll   StreamCmd_InitLinearMotion');
console.log('     param 2 -> gUnk_03002920[objIndex + 13]        StreamCmd_InitEntityMotion  (was ...LinearMotionExt)');
console.log('     param 4 -> the window clip bounds              StreamCmd_InitWindowCornerMotion');
console.log('   "Ext" claimed a base/extension relationship with the plain command that does not');
console.log('   exist. The motion IS linear (constant velocity, measured), but what distinguishes');
console.log('   this handler from its siblings is the target, so the target is what it is named for.');
console.log(`\n${failures === 0 ? 'ALL CHECKS PASSED' : failures + ' CHECK(S) FAILED'}`);
process.exit(failures === 0 ? 0 : 1);
