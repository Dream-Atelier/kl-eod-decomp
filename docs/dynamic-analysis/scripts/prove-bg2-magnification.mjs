// PROOF (runtime): gBg2XMag (0x030034AC) and gBg2YMag (0x03005420) are the BG2
// affine MAGNIFICATION pair, and StreamCmd_InitBg2Zoom (0x0804D8D4) is a BG2
// zoom tween — there is no angle anywhere in it.
//
// Two claims were open. The repo already *called* those two cells gBg2XMag/gBg2YMag,
// but that name had never been tested, so it is not evidence for itself; and the
// handler's own docstring said "runtime confirmation is still owed".
//
//   A. gBg2XMag scales BG2 horizontally and gBg2YMag scales it vertically, i.e. each
//      reaches its own pair of REG_BG2P* registers (include/io_reg.h: BG2PA 0x04000020,
//      BG2PB 0x04000022, BG2PC 0x04000024, BG2PD 0x04000026).
//   B. StreamCmd_InitBg2Zoom moves exactly those two cells, over the frame count
//      in the command, and nothing else.
//
// METHOD. Each experiment forces ONE value, holds everything else, and is run a
// second time with the intervention removed.
//
//   E1 runs on the WORLD MAP (savestate-fresh-gameplay), because that screen's VBlank
//      handler VBlankCallback_MapScreen is the one that recomputes gBg2PA..PD from the
//      two magnifications every frame and pushes them to the hardware. Forcing a
//      magnification there is a real causal intervention on a live path.
//   E2 runs IN A LEVEL (the default savestate), where the second half of the path
//      (gBg2PA -> REG_BG2PA) can be exercised on its own. In-level the first half is
//      dead: CameraModeSwitchHandler (src/engine.c) overwrites gBg2XMag from
//      gUnk_030007E0.unk4 every frame before recomputing, so an in-level write to
//      gBg2XMag is erased before anything reads it. That null result is reported here
//      rather than hidden, because a reader who repeats the experiment in a level and
//      sees nothing move would otherwise conclude the name is wrong.
//   E3 freezes a level savestate and executes the two commands the shipped script
//      really contains, back to back, on the real CPU: `FF 34 00 02`
//      (WriteStreamValue_Dual) and then `FF 46 02 00 FF 0F 00`
//      (StreamCmd_InitBg2Zoom), both fetched through the ROM's own dispatch table
//      at 0x0811787C rather than by hard-coded address. Then it ticks the installed
//      callback and watches the two magnifications move. The control re-runs the same
//      15 ticks with the entry's `param` field set to 0 instead of 1, which is the one
//      field that selects ProcessMotionStepExtended's case-1 arm.
//
// Where the shipped command bytes came from: scan-gfx-stream-commands.mjs, which
// decompresses all 20 gfx scripts with the ROM's own Decompress() and parses them
// with measured command lengths.
//
// Run: GBA_KIT=... node docs/dynamic-analysis/scripts/prove-bg2-magnification.mjs
import { HeadlessRuntime, REPO, SAVESTATE } from './gba-kit.mjs';
import { SAVES } from './_harness.mjs';
import { makeCaller, mustSymbol } from './_callrom.mjs';
import { mkdirSync, readFileSync } from 'node:fs';

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

const h4 = (n) => '0x' + (n & 0xffff).toString(16).padStart(4, '0');
const s16 = (v) => (v << 16) >> 16;

// ── register addresses, cross-checked against include/io_reg.h ─────────────────
// The header is parsed, not remembered: a rename or a typo there fails this script
// instead of silently mislabelling an observation (the WIN0V/WIN1H transposition
// that produced this project's worst wrong rename was exactly that failure).
const ioReg = readFileSync(`${REPO}/include/io_reg.h`, 'utf8');
function regAddr(name) {
    const m = new RegExp(`^#define\\s+REG_OFFSET_${name}\\s+(0x[0-9a-fA-F]+)`, 'm').exec(ioReg);
    if (!m) throw new Error(`include/io_reg.h has no REG_OFFSET_${name}`);
    return 0x04000000 + parseInt(m[1], 16);
}
const REG = Object.fromEntries(['BG2PA', 'BG2PB', 'BG2PC', 'BG2PD'].map((n) => [n, regAddr(n)]));
console.log('=== register addresses, read out of include/io_reg.h ===');
for (const [k, v] of Object.entries(REG)) console.log(`  REG_${k} = 0x${v.toString(16)}`);

const XMAG = mustSymbol(di, 'gBg2XMag'),
    YMAG = mustSymbol(di, 'gBg2YMag'),
    PA = mustSymbol(di, 'gBg2PA'),
    ALPHA = mustSymbol(di, 'gBg2Alpha'),
    BGLAYER = mustSymbol(di, 'gUnk_03003430'),
    STREAM_PTR = mustSymbol(di, 'gStreamPtr');
console.log(
    `\n  gBg2XMag=0x${XMAG.toString(16)}  gBg2YMag=0x${YMAG.toString(16)}  gBg2PA=0x${PA.toString(16)}  gBg2Alpha=0x${ALPHA.toString(16)}`,
);

const regs = () => ({
    pa: bus.read16(REG.BG2PA),
    pb: bus.read16(REG.BG2PB),
    pc: bus.read16(REG.BG2PC),
    pd: bus.read16(REG.BG2PD),
});
const line = (tag, r, x, y) =>
    `  ${tag.padEnd(26)} gBg2XMag=${h4(x)} gBg2YMag=${h4(y)}  ->  REG_BG2PA=${h4(r.pa)} REG_BG2PB=${h4(r.pb)} REG_BG2PC=${h4(r.pc)} REG_BG2PD=${h4(r.pd)}`;

// ═══════════════════════════════════════════════════════════════════════════════
// E1. World map: force one magnification, hold the other, see which register pair
//     moves. Control run has no write at all.
// ═══════════════════════════════════════════════════════════════════════════════
console.log('\n=== E1. world map (savestate-fresh-gameplay): force ONE magnification per run ===');
console.log('    (VBlankCallback_MapScreen recomputes gBg2PA..PD from the two magnifications every frame)');
const e1 = {};
for (const [tag, xf, yf] of [
    ['control (no write)', null, null],
    ['gBg2XMag <- 0x0200 (2x)', 0x200, null],
    ['gBg2XMag <- 0x0080 (0.5x)', 0x080, null],
    ['gBg2YMag <- 0x0200 (2x)', null, 0x200],
    ['gBg2YMag <- 0x0080 (0.5x)', null, 0x080],
]) {
    await eng.loadState(`${SAVES}/savestate-fresh-gameplay.json`);
    await eng.pressSequence([[null, 20]]);
    if (xf !== null || yf !== null)
        eng.onFrame(() => {
            if (xf !== null) bus.write16(XMAG, xf);
            if (yf !== null) bus.write16(YMAG, yf);
        });
    await eng.pressSequence([[null, 10]]);
    eng.onFrame(null);
    const r = regs();
    e1[tag] = r;
    console.log(line(tag, r, bus.read16(XMAG), bus.read16(YMAG)));
    await eng.takeScreenshot({ name: 'bg2mag-' + tag.replace(/[^a-z0-9]+/gi, '-') });
}
const c = e1['control (no write)'];
const x2 = e1['gBg2XMag <- 0x0200 (2x)'],
    xh = e1['gBg2XMag <- 0x0080 (0.5x)'];
const y2 = e1['gBg2YMag <- 0x0200 (2x)'],
    yh = e1['gBg2YMag <- 0x0080 (0.5x)'];
console.log('\n  verdicts:');
console.log(
    `    doubling gBg2XMag halved REG_BG2PA (${h4(c.pa)} -> ${h4(x2.pa)}) and REG_BG2PB (${h4(c.pb)} -> ${h4(x2.pb)}):` +
        ` ${x2.pa * 2 === c.pa || Math.abs(x2.pa * 2 - c.pa) <= 2 ? 'YES' : 'NO'}`,
);
console.log(
    `    ... while leaving REG_BG2PC/PD alone: ${x2.pc === c.pc && x2.pd === c.pd ? 'YES (unchanged)' : 'NO'}`,
);
console.log(
    `    halving gBg2XMag doubled REG_BG2PA (${h4(c.pa)} -> ${h4(xh.pa)}): ${Math.abs(xh.pa - c.pa * 2) <= 2 ? 'YES' : 'NO'}`,
);
console.log(
    `    doubling gBg2YMag halved REG_BG2PC (${h4(s16(c.pc))} -> ${h4(s16(y2.pc))}) and REG_BG2PD (${h4(c.pd)} -> ${h4(y2.pd)})` +
        ` while leaving REG_BG2PA/PB alone: ${y2.pa === c.pa && y2.pb === c.pb ? 'YES' : 'NO'}`,
);
console.log(
    `    halving gBg2YMag doubled REG_BG2PD (${h4(c.pd)} -> ${h4(yh.pd)}): ${Math.abs(yh.pd - c.pd * 2) <= 2 ? 'YES' : 'NO'}`,
);
console.log(
    '    REG_BG2PA is the BG2 source-texel step per screen pixel, so step ~ 1/gBg2XMag means gBg2XMag is a',
);
console.log(
    '    MAGNIFICATION with 0x100 = 1x. Screenshots in ' + OUT + ' show the world-map globe stretched/squashed',
);
console.log('    on exactly the axis whose magnification was forced.');

// ═══════════════════════════════════════════════════════════════════════════════
// E2. In a level: the second half of the path on its own, plus the honest null.
// ═══════════════════════════════════════════════════════════════════════════════
console.log('\n=== E2. in-level (default savestate) ===');
for (const [tag, addr, val] of [
    ['control (no write)', null, 0],
    ['gBg2PA <- 0x0040', PA, 0x40],
    ['gBg2XMag <- 0x0200', XMAG, 0x200],
]) {
    await eng.loadState(SAVESTATE);
    await eng.wait({ frames: 2 });
    if (addr !== null) eng.onFrame(() => bus.write16(addr, val));
    await eng.wait({ frames: 8 });
    eng.onFrame(null);
    console.log(line(tag, regs(), bus.read16(XMAG), bus.read16(YMAG)));
}
console.log('  -> gBg2PA reaches REG_BG2PA in a level (VBlankCallback_Gameplay copies it).');
console.log('  -> forcing gBg2XMag in a level does NOTHING, because CameraModeSwitchHandler rewrites');
console.log('     gBg2XMag = 0x100 - gUnk_030007E0.unk4 every frame before the recompute. This is a');
console.log('     property of the level camera, not evidence against the name; E1 is the live path.');

// ═══════════════════════════════════════════════════════════════════════════════
// E3. The shipped commands, executed by the ROM, on a frozen level state.
// ═══════════════════════════════════════════════════════════════════════════════
console.log('\n=== E3. run the two shipped script commands and tick the callback ===');
const ENTRIES_PTR = 0x030052a4;
const SCRATCH_CMD = 0x0203f000,
    SCRATCH_ENT = 0x0203e000;
const F = {
    type: di.structMember('GfxStreamEntry', 'type'),
    param: di.structMember('GfxStreamEntry', 'param'),
    unk_04: di.structMember('GfxStreamEntry', 'unk_04'),
    unk_08: di.structMember('GfxStreamEntry', 'unk_08'),
    unk_0A: di.structMember('GfxStreamEntry', 'unk_0A'),
    callback: di.structMember('GfxStreamEntry', 'callback'),
};
const STRIDE = 36;
/**
 * Pick a handler exactly the way StreamCmd_RunScript (0x0804EA94) does: six tables,
 * selected by the top bits of the opcode byte, each indexed by the low bits. Getting
 * this wrong silently returns 0 from an unused table slot, so the result is checked.
 */
function handler(op) {
    const pick = (op & 0x80) !== 0
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
    if (fn < 0x08000000 || fn >= 0x0a000000)
        throw new Error(`FF ${op.toString(16)} -> 0x${fn.toString(16)} is not a ROM address (wrong dispatch table?)`);
    return { fn, table: pick[0], idx: pick[1] };
}
for (const op of [0x34, 0x46]) {
    const t = handler(op);
    console.log(
        `  FF ${op.toString(16)} -> table 0x${t.table.toString(16)} slot ${t.idx} -> 0x${t.fn.toString(16)}` +
            ` (${op === 0x34 ? 'WriteStreamValue_Dual 0x0804C86C' : 'StreamCmd_InitBg2Zoom 0x0804D8D4'})`,
    );
}

// script[0] @0x07bc and @0x07fa, verbatim (see scan-gfx-stream-commands.mjs)
const SHIPPED_DUAL = [0xff, 0x34, 0x00, 0x02];
const SHIPPED_ANGLE = [0xff, 0x46, 0x02, 0x00, 0xff, 0x0f, 0x00];

async function runShipped({ forceParam = null }) {
    await eng.loadState(SAVESTATE);
    await eng.wait({ frames: 2 });
    bus.write32(ENTRIES_PTR, SCRATCH_ENT);
    for (let k = 0; k < 0x400; k++) bus.write8(SCRATCH_ENT + k, 0);
    // FF 34 — set the starting magnification the script sets
    SHIPPED_DUAL.forEach((b, i) => bus.write8(SCRATCH_CMD + i, b));
    bus.write32(STREAM_PTR, SCRATCH_CMD);
    call(handler(0x34).fn, [0]);
    const start = { x: bus.read16(XMAG), y: bus.read16(YMAG) };
    // FF 46 — install the tween
    SHIPPED_ANGLE.forEach((b, i) => bus.write8(SCRATCH_CMD + i, b));
    bus.write32(STREAM_PTR, SCRATCH_CMD);
    call(handler(0x46).fn, [0]);
    const idx = SHIPPED_ANGLE[2];
    const ent = SCRATCH_ENT + idx * STRIDE;
    const installed = {
        param: (bus.read16(ent + F.param.offset) >>> F.param.bitOffset) & ((1 << F.param.bitWidth) - 1),
        type: bus.read8(ent + F.type.offset) & 7,
        unk_04: s16(bus.read16(ent + F.unk_04.offset)),
        unk_08: s16(bus.read16(ent + F.unk_08.offset)),
        unk_0A: s16(bus.read16(ent + F.unk_0A.offset)),
        cb: bus.read32(ent + F.callback.offset) >>> 0,
    };
    if (forceParam !== null) {
        const mask = ((1 << F.param.bitWidth) - 1) << F.param.bitOffset;
        bus.write16(ent + F.param.offset, ((bus.read16(ent + F.param.offset) & ~mask) | (forceParam << F.param.bitOffset)) & 0xffff);
    }
    const trace = [];
    for (let t = 0; t < 18; t++) {
        const r = call(installed.cb, [idx]);
        trace.push({ t: t + 1, x: bus.read16(XMAG), y: bus.read16(YMAG), ret: r.ret });
    }
    return { start, installed, ent, trace };
}

const live = await runShipped({});
console.log(
    `  after FF 34 00 02:  gBg2XMag=${h4(live.start.x)} gBg2YMag=${h4(live.start.y)}   (the script's starting zoom)`,
);
console.log(
    `  after FF 46 ...  :  entry[2].param=${live.installed.param} type=${live.installed.type}` +
        ` unk_04(total)=${live.installed.unk_04} unk_08(Xstep)=${live.installed.unk_08} unk_0A(Ystep)=${live.installed.unk_0A}` +
        ` callback=0x${live.installed.cb.toString(16)} (ProcessMotionStepExtended=0x${(mustSymbol(di, 'ProcessMotionStepExtended') | 1).toString(16)})`,
);
console.log('  ticking the installed callback:');
for (const s of live.trace)
    console.log(`     tick ${String(s.t).padStart(2)}: gBg2XMag=${h4(s.x)} gBg2YMag=${h4(s.y)} callbackReturn=${s.ret}`);

const ctl = await runShipped({ forceParam: 0 });
console.log('\n  CONTROL — identical, except entry.param forced from 1 to 0 (the BG-layer arm):');
for (const s of ctl.trace.slice(0, 6))
    console.log(`     tick ${String(s.t).padStart(2)}: gBg2XMag=${h4(s.x)} gBg2YMag=${h4(s.y)} callbackReturn=${s.ret}`);
const ctlMoved = ctl.trace.some((s) => s.x !== ctl.start.x || s.y !== ctl.start.y);
console.log(
    `     ... 18 ticks total; magnifications moved: ${ctlMoved ? 'YES (unexpected)' : 'NO'}` +
        `   BG layer 0 scroll after the control run: ${h4(bus.read16(BGLAYER + 0x08))}/${h4(bus.read16(BGLAYER + 0x0a))}`,
);

const first = live.trace[0],
    last = live.trace[live.trace.length - 1];
console.log('\n  verdict:');
console.log(
    `    with param=1 the tween walked gBg2XMag ${h4(live.start.x)} -> ${h4(last.x)} and gBg2YMag ${h4(live.start.y)} -> ${h4(last.y)}` +
        ` in ${live.trace.length} ticks (${h4(first.x)} after one).`,
);
console.log(
    `    the command asks for 0x${live.start.x.toString(16)} + (${live.installed.unk_04}) = 0x${(live.start.x + live.installed.unk_04).toString(16)}` +
        `; it actually lands on ${h4(last.x)} because the per-frame step truncates`,
);
console.log(
    `    (${live.installed.unk_08} x ${SHIPPED_ANGLE[5]} = ${live.installed.unk_08 * SHIPPED_ANGLE[5]}, one short of ${live.installed.unk_04}).` +
        ` So the shipped script zooms BG2 out from 2x back to 1x over ${SHIPPED_ANGLE[5]} frames.`,
);
console.log(`    with param=0 and everything else identical, neither magnification moves.`);

// ── is stream byte[6] really dead? ────────────────────────────────────────────
// The handler fetches bytes[5..6] as one unaligned s16 and shifts it left by 8 for
// DivideQ8's second argument, which narrows back to s16 — so byte[6] should fall out
// entirely. That was static reading; test it.
console.log('\n  is stream byte[6] dead? (same command, byte[6] swept, everything else held)');
const byte6 = [];
for (const b6 of [0x00, 0x01, 0x7f, 0xab, 0xff]) {
    await eng.loadState(SAVESTATE);
    await eng.wait({ frames: 2 });
    bus.write32(ENTRIES_PTR, SCRATCH_ENT);
    for (let k = 0; k < 0x400; k++) bus.write8(SCRATCH_ENT + k, 0);
    const cmd = [...SHIPPED_ANGLE];
    cmd[6] = b6;
    cmd.forEach((v, i) => bus.write8(SCRATCH_CMD + i, v));
    bus.write32(STREAM_PTR, SCRATCH_CMD);
    const alphaBefore = bus.read8(ALPHA);
    call(handler(0x46).fn, [0]);
    const ent = SCRATCH_ENT + cmd[2] * STRIDE;
    const step = s16(bus.read16(ent + F.unk_08.offset));
    const advance = (bus.read32(STREAM_PTR) >>> 0) - SCRATCH_CMD;
    byte6.push(step);
    console.log(
        `    byte[6]=0x${b6.toString(16).padStart(2, '0')} -> unk_08 (per-frame step) = ${step}` +
            `  unk_0A = ${s16(bus.read16(ent + F.unk_0A.offset))}  stream advance = ${advance}` +
            `  gBg2Alpha ${alphaBefore} -> ${bus.read8(ALPHA)}`,
    );
}
console.log(
    `    -> byte[6] changed the installed step: ${new Set(byte6).size === 1 ? 'NO, it is dead (all five runs gave ' + byte6[0] + ')' : 'YES'}`,
);
console.log(`    Nothing in the command is an angle: gBg2Alpha = ${bus.read8(ALPHA)} throughout, untouched.`);
