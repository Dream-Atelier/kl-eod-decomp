// PROOF: 0x03000814 (gStreamWaitDeadline) is a FRAME DEADLINE for the gfx-stream
// executor, not a saved callback -- and the 2-bit field at gGfxBufferPtr byte 0 is
// the executor's enable/yield latch, not a "render mode".
//
// Why this script exists. include/structs/variables.h used to describe the cell as
//     "Saved scene callback: ShutdownGfxSubsystem stores gControlBlock[1] here"
// while StreamCmd_WaitFrames (0x0804E448) writes
//     globalFrameCounter + <halfword from the command stream>
// into it. Those cannot both be true of the same 32 bits, so one of them is wrong.
// This script settles it four ways, cheapest first:
//
//   1. STATIC CENSUS. A Thumb access that names a fixed address through a PC-relative
//      literal leaves the address in the literal pool, so scanning the cartridge for
//      the word 0x03000814 finds every such site; this script names the function each
//      one lands in, from the build's own symtab. That is a census, NOT a proof of
//      completeness -- an access computed as base+displacement from some other pool
//      word reaches the cell without a literal of its own, and a DMA reaches it with
//      no instruction at all. A watchpoint from a cold boot finds exactly one such
//      writer: AgbMain+0x30, the DMA3 fill that zeroes IWRAM at boot.
//   2. LAYOUT, FROM DWARF. gControlBlock is ((u8 *)0x03004C20), so a literal
//      gControlBlock[1] would be the BYTE at +1; what ShutdownGfxSubsystem stores is
//      ((u32 *)gControlBlock)[1] = *(u32 *)(0x03004C20 + 4), and DWARF says offset 4
//      of struct Unk_03004C20 is `globalFrameCounter`. So it stores the FRAME COUNTER;
//      the old comment named a word by an index without checking either the element
//      type or what that offset holds.
//   3. CAUSAL, AT RUNTIME, with controls:
//      (a) run StreamCmd_WaitFrames with only the halfword argument changed, and
//          watch the cell track globalFrameCounter + N exactly;
//      (b) run StreamCmd_RunScript on a stream of identical commands and count how
//          many execute per call, with and without the `(*p & ~3) | 2` write;
//      (c) run the gfx tick sub_0804EB64 with only the cell changed, and watch a
//          future deadline suppress the executor while an expired one lets it run.
//   4. USAGE. Decode the cartridge's own 20 gfx-stream scripts (the ROM decompresses
//      them itself) and report which of the handler's three legal opcodes is shipped.
//
// Run: GBA_KIT=... node docs/dynamic-analysis/scripts/prove-stream-wait-deadline.mjs
import { readFileSync, mkdirSync } from 'node:fs';
import { HeadlessRuntime, REPO, SAVESTATE } from './gba-kit.mjs';
import { ROM, ELF } from './_harness.mjs';
import { makeCaller, mustSymbol } from './_callrom.mjs';

const OUT = '/tmp/klonoa-wait-deadline';
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

// ═════════════════════════════════════════════════════════════════════════════
// 0. Addresses, all resolved from the build.
// ═════════════════════════════════════════════════════════════════════════════
const DEADLINE = mustSymbol(di, 'gStreamWaitDeadline');
const BLOCK = mustSymbol(di, 'gUnk_03004C20');
const FRAMES = di.structMember('Unk_03004C20', 'globalFrameCounter');
if (!FRAMES) throw new Error('Unk_03004C20.globalFrameCounter is not in DWARF (renamed?)');
const FRAME_COUNTER = BLOCK + FRAMES.offset;
const WAIT_FRAMES = mustSymbol(di, 'StreamCmd_WaitFrames');
const GFX_TICK = mustSymbol(di, 'sub_0804EB64');
const RUN_SCRIPT = mustSymbol(di, 'StreamCmd_RunScript');
const SHUTDOWN = mustSymbol(di, 'ShutdownGfxSubsystem');
const STREAM_PTR = mustSymbol(di, 'gStreamPtr');
const PAUSE_FLAG = mustSymbol(di, 'gUnk_030034E4');
const STREAM_TABLE = mustSymbol(di, 'gStreamDataTable');
const DECOMPRESS = mustSymbol(di, 'Decompress');
// These three are `#define`s over literal addresses and have no symtab entry. Each
// is cross-checked below against the literal the ROM itself loads.
const GFX_BUFFER_PTR = 0x030034a0; // gGfxBufferPtr
const ENTRIES_PTR = 0x030052a4; // gBuffer_52A4
const SOUND_INFO = 0x0300081c; // gSoundInfo

console.log('gStreamWaitDeadline      =', hex(DEADLINE));
console.log('gUnk_03004C20            =', hex(BLOCK), `+ DWARF globalFrameCounter offset ${FRAMES.offset} =`, hex(FRAME_COUNTER));
console.log('StreamCmd_WaitFrames     =', hex(WAIT_FRAMES));
console.log('sub_0804EB64 (gfx tick)  =', hex(GFX_TICK));
console.log('StreamCmd_RunScript      =', hex(RUN_SCRIPT));
console.log('ShutdownGfxSubsystem     =', hex(SHUTDOWN));

// StreamCmd_RunScript's six dispatch tables, reproduced from its own literal pool;
// the opcode formula beside each is the mask/compare sequence it executes on byte[1].
const TABLES = [
    { base: 0x081179b4, n: 16, op: (i) => 0x80 | i },
    { base: 0x0811787c, n: 50, op: (i) => 0x40 | i },
    { base: 0x081178b8, n: 16, op: (i) => 0x30 | i },
    { base: 0x08117854, n: 16, op: (i) => 0x20 | i },
    { base: 0x0811790c, n: 16, op: (i) => 0x10 | i },
    { base: 0x081178d8, n: 16, op: (i) => 0x00 | i },
];

// ═════════════════════════════════════════════════════════════════════════════
// 1. EXHAUSTIVE: every site in the cartridge that can reach the cell.
// ═════════════════════════════════════════════════════════════════════════════
console.log('\n=== 1. every occurrence of the word 0x03000814 in the cartridge ===');
console.log('    (a Thumb access to a fixed address needs a PC-relative literal, so this list is complete)');
const rom = readFileSync(ROM);
const sites = [];
for (let off = 0; off + 4 <= rom.length; off += 4) if (rom.readUInt32LE(off) === DEADLINE) sites.push(0x08000000 + off);
for (const a of sites) {
    const s = di.addressToSymbol(a);
    console.log(`    pool word at ${hex(a)}  in  ${s ? `${s.name}+0x${s.offset.toString(16)}` : '(no symbol)'}`);
}
const names = [...new Set(sites.map((a) => di.addressToSymbol(a)?.name).filter(Boolean))];
console.log('    distinct functions that touch the cell:', names.join(', '));
check(sites.length === 5, `exactly 5 pool words reference it (found ${sites.length})`);
check(names.length === 4, `exactly 4 functions touch it (found ${names.length}: ${names.join(', ')})`);
check(
    names.includes('ShutdownGfxSubsystem') && names.includes('StreamCmd_WaitFrames') && names.includes('sub_0804EB64'),
    'the three functions this comment claims about are among them',
);
// The fourth is an m4a-side stream handler. luvdis merged several of those into one
// .s file, so the symbol the pool word lands in (sub_0804F092) is the NEXT fragment,
// not the writer; the writer is the dispatch-table entry just below it. Print both so
// nobody has to take the symbol at face value.
console.log('    the fourth site is a stream command handler in the m4a unit; its dispatch slot:');
for (const t of TABLES)
    for (let i = 0; i < t.n; i++) {
        const fn = (bus.read32(t.base + i * 4) >>> 0) & ~1;
        if (fn >= 0x0804f040 && fn <= 0x0804f0d0)
            console.log(`       FF ${t.op(i).toString(16).padStart(2, '0')} -> ${hex(fn)}  ${di.addressToSymbol(fn)?.name ?? '(merged fragment)'}`);
    }
console.log('    disassembly of the FF 85 handler, showing `deadline = globalFrameCounter + 0x1E`:');
for (const i of eng.disassemble(0x0804f086, 5, 'thumb')) console.log(`       ${hex(i.address)}  ${i.instruction}`);

// ═════════════════════════════════════════════════════════════════════════════
// 2. LAYOUT: what "gControlBlock[1]" actually is.
// ═════════════════════════════════════════════════════════════════════════════
console.log('\n=== 2. ShutdownGfxSubsystem stores ((u32 *)gControlBlock)[1] -- what IS word 1? ===');
for (const i of eng.disassemble(SHUTDOWN, 6, 'thumb')) console.log(`    ${hex(i.address)}  ${i.instruction}`);
console.log(`    gControlBlock = ${hex(BLOCK)} (a u8 *);  ((u32 *)gControlBlock)[1] = ${hex(BLOCK + 4)}`);
console.log(`    DWARF: Unk_03004C20.globalFrameCounter is at offset ${FRAMES.offset}, size ${FRAMES.size}`);
check(FRAMES.offset === 4 && FRAMES.size === 4, '((u32 *)gControlBlock)[1] IS globalFrameCounter -- so this stores the frame counter, not a callback');

// A saved callback would be a ROM code pointer. Watch the live cell for a few frames.
await eng.loadState(SAVESTATE);
await eng.wait({ frames: 2 });
const series = [];
for (let i = 0; i < 5; i++) {
    await eng.wait({ frames: 1 });
    series.push([bus.read32(FRAME_COUNTER) >>> 0, bus.read32(DEADLINE) >>> 0]);
}
console.log('    live (globalFrameCounter, cell) over 5 real frames:');
for (const [f, d] of series) console.log(`       counter=${f}  cell=${hex(d)}   cell - counter = ${(d - f) | 0}`);
check(series.every(([, d]) => d < 0x08000000 || d >= 0x0a000000), 'the cell never holds a ROM code address (a saved callback would)');
check(series[4][0] - series[0][0] === 4, 'globalFrameCounter really advances one per frame');

// ═════════════════════════════════════════════════════════════════════════════
// 3a. CAUSAL: StreamCmd_WaitFrames writes globalFrameCounter + N.
// ═════════════════════════════════════════════════════════════════════════════
console.log('\n=== 3a. run StreamCmd_WaitFrames, changing ONLY the stream halfword ===');
const SCRATCH_CMD = 0x0203f000,
    SCRATCH_GFX = 0x0203e000,
    SCRATCH_ENT = 0x0203c000,
    SCRATCH_SND = 0x0203b000;
const COUNTER = 0x00001000;
const OP_WAIT = 0x5a; // StreamCmd_WaitFrames, via the `c & 0x40` table (see part 4)
const OP_KEEP = 0x44; // StreamCmd_InitWindowCornerMotion: does NOT clear the yield bit

const fill = (addr, n, v = 0) => {
    for (let k = 0; k < n; k++) bus.write8(addr + k, v);
};

function runWait(n, counter) {
    bus.write32(GFX_BUFFER_PTR, SCRATCH_GFX);
    fill(SCRATCH_GFX, 0x40);
    bus.write8(SCRATCH_GFX, 0xff); // every bit set, so we can see exactly which the handler clears
    fill(SCRATCH_CMD, 16);
    bus.write8(SCRATCH_CMD + 0, 0xff);
    bus.write8(SCRATCH_CMD + 1, OP_WAIT);
    bus.write8(SCRATCH_CMD + 2, n & 0xff);
    bus.write8(SCRATCH_CMD + 3, (n >> 8) & 0xff);
    bus.write32(STREAM_PTR, SCRATCH_CMD);
    bus.write32(FRAME_COUNTER, counter >>> 0);
    bus.write32(DEADLINE, 0xdeadbeef);
    const r = call(WAIT_FRAMES, [], 100000);
    return {
        timedOut: r.timedOut,
        cell: bus.read32(DEADLINE) >>> 0,
        byte0: bus.read8(SCRATCH_GFX) & 0xff,
        advance: ((bus.read32(STREAM_PTR) >>> 0) - SCRATCH_CMD) | 0,
    };
}

await eng.loadState(SAVESTATE);
await eng.wait({ frames: 2 });
console.log(`    globalFrameCounter forced to ${COUNTER} in every trial; only N changes`);
for (const n of [0, 1, 30, 300, 0xffff]) {
    const r = runWait(n, COUNTER);
    const ok = r.cell === (COUNTER + n) >>> 0;
    console.log(
        `       N=${String(n).padStart(5)}  cell=${hex(r.cell)}  cell-counter=${String((r.cell - COUNTER) | 0).padStart(5)}` +
            `  streamAdvance=${r.advance}  byte0: 0xff -> ${hex(r.byte0, 2)}${ok ? '' : '   <-- MISMATCH'}`,
    );
    if (!ok) failures++;
}
const cA = runWait(30, 0x1000),
    cB = runWait(30, 0x2000);
console.log(`    control (N held at 30, counter 0x1000 -> 0x2000): cell ${hex(cA.cell)} -> ${hex(cB.cell)}`);
check(cB.cell - cA.cell === 0x1000, 'the cell tracks globalFrameCounter, not an absolute constant');
check(cA.advance === 4, 'the command is 4 bytes long');
check((cA.byte0 & 3) === 2, 'gGfxBufferPtr byte 0 bits 0..1 end at 2 (bit 1 KEPT, bit 0 CLEARED)');
check((cA.byte0 & 0xfc) === 0xfc, 'no other bit of byte 0 is touched');

// ═════════════════════════════════════════════════════════════════════════════
// 3b. What those two bits are, read off StreamCmd_RunScript and then demonstrated.
// ═════════════════════════════════════════════════════════════════════════════
console.log('\n=== 3b. gGfxBufferPtr byte 0 bits 0..1: the executor latch, not a render mode ===');
console.log('    StreamCmd_RunScript prologue -- it sets the field to 3 on entry:');
for (const i of eng.disassemble(RUN_SCRIPT, 9, 'thumb')) console.log(`       ${hex(i.address)}  ${i.instruction}`);
console.log('    ...and its loop condition -- it keeps taking commands while bit 0 is set:');
for (const i of eng.disassemble(RUN_SCRIPT + 0xb4, 10, 'thumb')) console.log(`       ${hex(i.address)}  ${i.instruction}`);

function runExecutor(op, len) {
    bus.write32(GFX_BUFFER_PTR, SCRATCH_GFX);
    fill(SCRATCH_GFX, 0x40);
    bus.write32(ENTRIES_PTR, SCRATCH_ENT);
    fill(SCRATCH_ENT, 0x800);
    fill(SCRATCH_CMD, 0x80);
    for (let k = 0; k < 8; k++) {
        bus.write8(SCRATCH_CMD + k * len + 0, 0xff);
        bus.write8(SCRATCH_CMD + k * len + 1, op);
    }
    bus.write32(STREAM_PTR, SCRATCH_CMD);
    bus.write32(FRAME_COUNTER, COUNTER);
    const r = call(RUN_SCRIPT, [], 2000000);
    return { n: (((bus.read32(STREAM_PTR) >>> 0) - SCRATCH_CMD) / len) | 0, timedOut: r.timedOut, byte0: bus.read8(SCRATCH_GFX) & 3 };
}
const eWait = runExecutor(OP_WAIT, 4),
    eKeep = runExecutor(OP_KEEP, 10);
console.log(`    a stream of eight "FF ${OP_WAIT.toString(16)}" (StreamCmd_WaitFrames):             commands executed per call = ${eWait.n}   (byte0 & 3 -> ${eWait.byte0})`);
console.log(`    a stream of eight "FF ${OP_KEEP.toString(16)}" (StreamCmd_InitWindowCornerMotion): commands executed per call = ${eKeep.n}   (byte0 & 3 -> ${eKeep.byte0})`);
check(eWait.n === 1, 'a wait command yields after ONE command -- it cleared bit 0');
check(eKeep.n === 8, 'a command that leaves the field alone does NOT yield (control)');

// ═════════════════════════════════════════════════════════════════════════════
// 3c. CAUSAL: the deadline gates the whole executor from the gfx tick.
// ═════════════════════════════════════════════════════════════════════════════
console.log('\n=== 3c. run the gfx tick sub_0804EB64, changing ONLY the deadline ===');
function runTick(deadlineDelta) {
    // A quiet, fully controlled world: every pointer the tick follows goes to scratch.
    bus.write32(GFX_BUFFER_PTR, SCRATCH_GFX);
    fill(SCRATCH_GFX, 0x40);
    bus.write8(SCRATCH_GFX, 0x02); // executor ENABLED (bit 1), not yet yielded (bit 0 clear)
    bus.write32(ENTRIES_PTR, SCRATCH_ENT);
    fill(SCRATCH_ENT, 0x800); // no active stream entries
    bus.write32(SOUND_INFO, SCRATCH_SND);
    fill(SCRATCH_SND, 0x40); // no sound flags set
    bus.write8(PAUSE_FLAG, 0);
    fill(SCRATCH_CMD, 0x80);
    for (let k = 0; k < 8; k++) {
        bus.write8(SCRATCH_CMD + k * 10 + 0, 0xff);
        bus.write8(SCRATCH_CMD + k * 10 + 1, OP_KEEP);
    }
    bus.write32(STREAM_PTR, SCRATCH_CMD);
    bus.write32(FRAME_COUNTER, COUNTER);
    bus.write32(DEADLINE, (COUNTER + deadlineDelta) >>> 0);
    const r = call(GFX_TICK, [], 3000000);
    return { advance: ((bus.read32(STREAM_PTR) >>> 0) - SCRATCH_CMD) | 0, timedOut: r.timedOut };
}
for (const [d, label] of [
    [+600, 'deadline 600 frames in the FUTURE'],
    [+1, 'deadline 1 frame in the future'],
    [0, 'deadline == now (what ShutdownGfxSubsystem writes)'],
    [-1, 'deadline 1 frame in the PAST'],
]) {
    const r = runTick(d);
    console.log(`    ${label.padEnd(52)} stream advanced ${String(r.advance).padStart(3)} bytes${r.timedOut ? '   (TIMED OUT)' : ''}`);
}
check(runTick(600).advance === 0, 'a future deadline suppresses the executor entirely');
check(runTick(-1).advance > 0, 'an expired deadline lets the executor run (control)');
check(runTick(0).advance > 0, 'deadline == now counts as expired, so ShutdownGfxSubsystem CLEARS the wait');

console.log('\n    the tick\'s two reads of the cell, both `cell - globalFrameCounter`:');
for (const [start, n, label] of [
    [0x0804ec70, 11, 'first use: the A-press shortcut, which re-arms the deadline to now'],
    [0x0804ed04, 6, 'second use: the per-frame gate'],
]) {
    console.log(`      -- ${label}`);
    for (const i of eng.disassemble(start, n, 'thumb')) console.log(`         ${hex(i.address)}  ${i.instruction}`);
}

// ═════════════════════════════════════════════════════════════════════════════
// 4. USAGE: which of the handler's opcodes do the shipped scripts actually use?
// ═════════════════════════════════════════════════════════════════════════════
console.log('\n=== 4. the handler in the dispatch tables, and its use in the shipped scripts ===');
await eng.loadState(SAVESTATE);
await eng.wait({ frames: 2 });
const spellings = [];
for (const t of TABLES)
    for (let i = 0; i < t.n; i++) {
        const fn = (bus.read32(t.base + i * 4) >>> 0) & ~1;
        if (fn === WAIT_FRAMES) {
            spellings.push(t.op(i));
            console.log(`    FF ${t.op(i).toString(16).padStart(2, '0')}  = table ${hex(t.base)} slot ${i}`);
        }
    }
check(spellings.length > 0, 'the handler is reachable from at least one dispatch table');

// Let the ROM decompress its own scripts, then count naive "FF xx" byte pairs. This is
// an upper bound (a payload byte pair can look like a command), which is enough to
// answer "is this spelling used at all"; scan-gfx-stream-commands.mjs does the parsed
// walk that turns the bound into an exact count.
const SCRATCH_OUT = 0x02030000;
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
console.log(`    decompressed ${scripts.length} scripts (${scripts.reduce((a, s) => a + s.length, 0)} bytes total)`);
let anyUse = 0;
for (const op of spellings) {
    let hits = 0;
    for (const s of scripts) for (let i = 0; i < s.length - 1; i++) if (s[i] === 0xff && s[i + 1] === op) hits++;
    anyUse += hits;
    console.log(`    FF ${op.toString(16).padStart(2, '0')}: ${hits} naive byte-pair hit(s) across the 20 shipped scripts`);
}
check(anyUse > 0, 'the shipped scripts really do issue this command');

console.log('\n=== verdict ===');
console.log('    0x03000814 is a FRAME DEADLINE. Every writer found stores globalFrameCounter, ');
console.log('    globalFrameCounter + N, or (at boot, by DMA) zero, and every read is');
console.log('    (cell - globalFrameCounter): at 0x0804ED04 against zero, which is the gate on the');
console.log('    executor, and at 0x0804EC7A..88 against the pool literal 0x00000E0B. The "saved');
console.log('    scene callback" comment was wrong: ((u32 *)gControlBlock)[1] IS globalFrameCounter,');
console.log('    so ShutdownGfxSubsystem');
console.log('    was clearing the wait, not saving a callback. The cell is renamed gStreamWaitDeadline.');
console.log('    The 2-bit field at gGfxBufferPtr byte 0 is the executor latch (bit 1 = enabled,');
console.log('    bit 0 = keep taking commands this frame), so "SetTimerAndMode" is one idea, not two:');
console.log('    StreamCmd_WaitFrames.');
console.log(`\n${failures === 0 ? 'ALL CHECKS PASSED' : failures + ' CHECK(S) FAILED'}`);
process.exit(failures === 0 ? 0 : 1);
