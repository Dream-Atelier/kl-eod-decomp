// PROOF (runtime): ProcessFrameAnimation really does ADVANCE an animation frame --
// it is not just re-pushing whatever the entry already points at.
//
// The question the name has to survive. ProcessFrameAnimation (0x0804CF80) is the
// per-frame callback StreamCmd_InitFrameAnimation installs. It ticks a timer and
// calls DmaSpriteToObjVram. A handler that merely re-DMAd the same tiles every N
// frames would look almost identical from the disassembly, and "FrameAnimation"
// would then be an overstatement. So this script does not read the increment out of
// the code; it drives the ROM's own handler over entries it controls and reports the
// sequence of frames the cartridge actually DMAs.
//
// The observable is the OBJ VRAM the DMA lands in, not a register: GBA DMA source and
// control registers are not reliably readable, and a transfer that happened is
// visible in its destination. Each animation frame in the scratch tileset is stamped
// with its own index (0xF000 | frame), so the word sitting in OBJ VRAM after a call
// names the frame that was copied.
//
// Why a direct call: the gfx-stream entry dispatcher (ProcessAnimationSteps) does not
// run in any state reachable by idling in a level, so "play until the game does it"
// observes nothing. We load a real, fully-initialised savestate, stop advancing frames
// (so no interrupt or game logic can touch the state under test), and single-step the
// cartridge's own Thumb code through _callrom.mjs.
//
// Every field is written through the DWARF MemberLocation for it, so renaming a field
// in include/gfx.h makes this script throw instead of reading the wrong bytes.
//
// Run: GBA_KIT=... node docs/dynamic-analysis/scripts/prove-frame-animation.mjs
import { mkdirSync } from 'node:fs';
import { HeadlessRuntime, REPO, SAVESTATE } from './gba-kit.mjs';
import { ELF, readField, writeField } from './_harness.mjs';
import { makeCaller, mustSymbol } from './_callrom.mjs';

const OUT = '/tmp/klonoa-frame-anim';
mkdirSync(OUT, { recursive: true });

const rt = await HeadlessRuntime.create({ romPath: `${REPO}/baserom.gba`, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const eng = rt.engine,
    bus = rt.gba.bus,
    cpu = rt.gba.armCpu,
    di = eng.debugInfo;
const call = makeCaller(cpu);

const hex = (n, w = 4) => '0x' + (n >>> 0).toString(16).padStart(w, '0');
let failures = 0;
const check = (ok, label) => {
    console.log(`   [${ok ? ' OK ' : 'FAIL'}] ${label}`);
    if (!ok) failures++;
};

const PFA = mustSymbol(di, 'ProcessFrameAnimation');
const DMA_SPRITE = mustSymbol(di, 'DmaSpriteToObjVram');
const INIT = mustSymbol(di, 'StreamCmd_InitFrameAnimation');
const STREAM_PTR = mustSymbol(di, 'gStreamPtr');
// `#define`s over literal addresses, so no symtab entry exists for them.
const ENTRIES_PTR = 0x030052a4; // gBuffer_52A4
const ALLOC_PTR = 0x030007c8; // gGfxStreamBuffer / gGfxStreamAllocs
const OBJ_VRAM = 0x06010000;

const E = (n) => {
    const f = di.structMember('GfxStreamEntry', n);
    if (!f) throw new Error(`GfxStreamEntry.${n} is not in DWARF -- renamed?`);
    return f;
};
const A = (n) => {
    const f = di.structMember('GfxStreamAlloc', n);
    if (!f) throw new Error(`GfxStreamAlloc.${n} is not in DWARF -- renamed?`);
    return f;
};

console.log('ProcessFrameAnimation      =', hex(PFA, 8));
console.log('DmaSpriteToObjVram         =', hex(DMA_SPRITE, 8));
console.log('StreamCmd_InitFrameAnimation =', hex(INIT, 8));
console.log('\nDWARF layout this script writes through (nothing hand-typed):');
for (const n of ['unk_04', 'unk_08', 'unk_0C', 'timer', 'unk_1E', 'unk_1F', 'callback'])
    console.log(`   GfxStreamEntry.${n.padEnd(9)} ${JSON.stringify(E(n))}`);
for (const n of ['pTiles', 'tileIndex', 'tileCount']) console.log(`   GfxStreamAlloc.${n.padEnd(9)} ${JSON.stringify(A(n))}`);

await eng.loadState(SAVESTATE);
await eng.wait({ frames: 2 });

// ── the world under test ──────────────────────────────────────────────────────
const SCRATCH_ENT = 0x0203e000, // GfxStreamEntry[]
    SCRATCH_ALLOC = 0x0203d000, // GfxStreamAlloc[]
    SCRATCH_TILES = 0x02030000, // "decompressed" tile data, 16 stamped frames
    SCRATCH_CMD = 0x0203f000;
const IDX = 1, // which GfxStreamEntry
    SLOT = 2, // which GfxStreamAlloc slot the entry names in unk_04
    TILEINDEX = 5, // OBJ tile the slot owns
    TILECOUNT = 2; // 32-byte tiles per animation frame => 64 bytes per frame
const FRAME_BYTES = TILECOUNT * 32;
const entryAddr = SCRATCH_ENT + IDX * 36;
const stamp = (f) => (0xf000 | f) & 0xffff;

function setup({ base, count, cur, reload, timer }) {
    bus.write32(ENTRIES_PTR, SCRATCH_ENT);
    for (let k = 0; k < 0x400; k++) bus.write8(SCRATCH_ENT + k, 0);
    bus.write32(ALLOC_PTR, SCRATCH_ALLOC);
    for (let k = 0; k < 0x100; k++) bus.write8(SCRATCH_ALLOC + k, 0);
    const slot = SCRATCH_ALLOC + SLOT * 8;
    writeField(bus, slot, A('pTiles'), SCRATCH_TILES);
    writeField(bus, slot, A('tileIndex'), TILEINDEX);
    writeField(bus, slot, A('tileCount'), TILECOUNT);
    // frame f occupies SCRATCH_TILES + f * FRAME_BYTES, every halfword stamped with f
    for (let f = 0; f < 16; f++) for (let k = 0; k < FRAME_BYTES; k += 2) bus.write16(SCRATCH_TILES + f * FRAME_BYTES + k, stamp(f));
    for (let k = 0; k < 0x400; k += 2) bus.write16(OBJ_VRAM + k, 0);
    writeField(bus, entryAddr, E('unk_04'), SLOT);
    writeField(bus, entryAddr, E('unk_0C'), cur);
    writeField(bus, entryAddr, E('unk_08'), reload);
    writeField(bus, entryAddr, E('timer'), timer);
    writeField(bus, entryAddr, E('unk_1E'), base);
    writeField(bus, entryAddr, E('unk_1F'), count);
}
const curFrame = () => readField(bus, entryAddr, E('unk_0C'));
// the word the DMA landed on, decoded back into the frame index that stamped it
const dmadFrame = () => {
    const w = bus.read16(OBJ_VRAM + TILEINDEX * 32);
    return w === 0 ? null : (w & 0x0fff) | 0;
};

function run(cfg, n) {
    setup(cfg);
    const frames = [];
    for (let i = 0; i < n; i++) {
        const r = call(PFA, [IDX], 200000);
        if (r.timedOut) throw new Error('ProcessFrameAnimation did not return');
        if (r.ret !== 1) throw new Error(`ProcessFrameAnimation returned ${r.ret}, expected 1`);
        frames.push(dmadFrame());
    }
    return frames;
}
const show = (a) => a.map((v) => (v === null ? '-' : String(v))).join(',');

// ── A. the baseline cycle ─────────────────────────────────────────────────────
console.log('\n=== A. base=0 count=4, timer reloads to 0 so a frame is pushed on every call ===');
const A0 = run({ base: 0, count: 4, cur: 0, reload: 0, timer: 1 }, 12);
console.log('   frames DMAd to OBJ VRAM: ' + show(A0));
check(A0.join() === '0,1,2,3,0,1,2,3,0,1,2,3', 'the DMAd frame ADVANCES and WRAPS -- it is an animation, not a repaint');
check(new Set(A0).size === 4, '4 distinct frames appear (a repaint would show exactly one)');

// ── B. change ONLY unk_1F ─────────────────────────────────────────────────────
console.log('\n=== B. change ONLY unk_1F, 4 -> 3 (everything else held) ===');
const B0 = run({ base: 0, count: 3, cur: 0, reload: 0, timer: 1 }, 12);
console.log('   frames DMAd to OBJ VRAM: ' + show(B0));
check(B0.join() === '0,1,2,0,1,2,0,1,2,0,1,2', 'unk_1F is the frame COUNT: the cycle length follows it');

// ── C. change ONLY unk_1E ─────────────────────────────────────────────────────
console.log('\n=== C. change ONLY unk_1E, 0 -> 5 (count still 4) ===');
const C0 = run({ base: 5, count: 4, cur: 5, reload: 0, timer: 1 }, 12);
console.log('   frames DMAd to OBJ VRAM: ' + show(C0));
check(C0.join() === '5,6,7,8,5,6,7,8,5,6,7,8', 'unk_1E is the FIRST frame: the whole cycle shifts with it, and wraps back to it');

// ── D. change ONLY unk_08 ─────────────────────────────────────────────────────
console.log('\n=== D. change ONLY unk_08 (the timer reload), 0 -> 2 ===');
const D0 = run({ base: 0, count: 4, cur: 0, reload: 2, timer: 1 }, 12);
console.log('   frames DMAd to OBJ VRAM: ' + show(D0));
check(D0.join() === '0,0,1,1,2,2,3,3,0,0,1,1', 'unk_08 is the per-frame HOLD: each frame lasts unk_08 calls');

// ── E. the control ────────────────────────────────────────────────────────────
console.log('\n=== E. CONTROL: same entry, timer parked at 1000 so it never expires ===');
const E0 = run({ base: 0, count: 4, cur: 0, reload: 0, timer: 1000 }, 12);
console.log('   frames DMAd to OBJ VRAM: ' + show(E0));
check(
    E0.every((v) => v === null),
    'nothing is written to OBJ VRAM at all -- every frame above was caused by the timer expiring',
);
check(curFrame() === 0, 'and the frame cursor never moved');

// ── F. the frame index really selects a DIFFERENT SOURCE, not a different dest ─
console.log('\n=== F. what the frame index does inside DmaSpriteToObjVram ===');
console.log('   calling DmaSpriteToObjVram(slot, frameIdx) directly, one frame at a time:');
setup({ base: 0, count: 4, cur: 0, reload: 0, timer: 1000 });
const dests = new Set();
for (const f of [0, 1, 2, 3, 7]) {
    for (let k = 0; k < 0x400; k += 2) bus.write16(OBJ_VRAM + k, 0);
    call(DMA_SPRITE, [SLOT, f], 200000);
    // which OBJ VRAM bytes changed, and what landed there
    let first = null,
        last = null;
    for (let k = 0; k < 0x400; k += 2) if (bus.read16(OBJ_VRAM + k) !== 0) (first ??= k), (last = k);
    dests.add(`${first}..${last}`);
    console.log(
        `      frameIdx=${f}: OBJ VRAM +${String(first).padStart(3)}..+${String(last).padStart(3)} = ${hex(bus.read16(OBJ_VRAM + TILEINDEX * 32))}` +
            `   (frame ${f} was stamped ${hex(stamp(f))})`,
    );
}
check(dests.size === 1, 'every frame index writes the SAME OBJ VRAM bytes (the destination is fixed by tileIndex)');
check(
    [...dests][0] === `${TILEINDEX * 32}..${TILEINDEX * 32 + FRAME_BYTES - 2}`,
    `and those bytes are exactly tileIndex*32 .. +tileCount*32 (${TILEINDEX * 32}..${TILEINDEX * 32 + FRAME_BYTES - 2})`,
);

// ── G. the installer agrees about which byte is which ─────────────────────────
console.log('\n=== G. StreamCmd_InitFrameAnimation, run on a scratch command, fills the fields above ===');
bus.write32(ENTRIES_PTR, SCRATCH_ENT);
for (let k = 0; k < 0x400; k++) bus.write8(SCRATCH_ENT + k, 0);
for (let k = 0; k < 16; k++) bus.write8(SCRATCH_CMD + k, 0);
const CMD = [0xff, 0x4b, IDX, /*[3]*/ 9, /*[4]*/ 5, /*[5]*/ 4, /*[6]*/ 3];
CMD.forEach((b, i) => bus.write8(SCRATCH_CMD + i, b));
bus.write32(STREAM_PTR, SCRATCH_CMD);
const ir = call(INIT, [], 200000);
console.log(`   stream: FF 4B ${CMD.slice(2).map((b) => b.toString(16).padStart(2, '0')).join(' ')}   advance=${((bus.read32(STREAM_PTR) >>> 0) - SCRATCH_CMD) | 0}${ir.timedOut ? ' TIMED OUT' : ''}`);
const got = {
    unk_04: readField(bus, entryAddr, E('unk_04')),
    unk_0C: readField(bus, entryAddr, E('unk_0C')),
    unk_1E: readField(bus, entryAddr, E('unk_1E')),
    unk_1F: readField(bus, entryAddr, E('unk_1F')),
    unk_08: readField(bus, entryAddr, E('unk_08')),
    timer: readField(bus, entryAddr, E('timer')),
    callback: readField(bus, entryAddr, E('callback')),
};
console.log('   entry after the command:', JSON.stringify(got));
check(got.unk_04 === CMD[3], 'byte[3] -> unk_04 (the GfxStreamAlloc slot)');
check(got.unk_1E === CMD[4] && got.unk_0C === CMD[4], 'byte[4] -> BOTH unk_1E (first frame) and unk_0C (cursor): the cursor starts at the first frame');
check(got.unk_1F === CMD[5], 'byte[5] -> unk_1F (frame count)');
check(got.unk_08 === CMD[6] && got.timer === CMD[6], 'byte[6] -> BOTH unk_08 (hold) and timer: the first frame is held as long as the rest');
check((got.callback & ~1) === PFA, 'callback is ProcessFrameAnimation');

console.log('\n=== verdict ===');
console.log('   ProcessFrameAnimation advances an animation cursor over a run of frames in ONE');
console.log('   tileset and DMAs each in turn to a fixed OBJ VRAM destination. unk_1E is the');
console.log('   first frame, unk_1F the frame count, unk_0C the cursor, unk_08 the hold, unk_04');
console.log('   the allocator slot. The name is earned, not overstated.');
console.log(`\n${failures === 0 ? 'ALL CHECKS PASSED' : failures + ' CHECK(S) FAILED'}`);
process.exit(failures === 0 ? 0 : 1);
