// PROOF for the function called UpdateHUDTimePanel (src/code_1.c, ROM 0x08026128)
// and for the three fields it draws.
//
// The name claims "lives". gUnk_03005220.lives is offset 0x4C and is already
// proven (docs/dynamic-analysis/scripts/find-lives.mjs); this function never
// touches 0x4C -- it reads unk4D/unk4E/unk4F. So the questions are:
//
//   Q1  What are unk4D/unk4E/unk4F?  INTERVENTION: gUnk_03004C20.unk10 is the
//       flag InitLevelBG sets to 1 for exactly the stages that own this panel
//       (src/engine.c:364-373).  Force it to 1 in an ordinary level and the tick
//       code runs; hold everything else.  CONTROL: the identical run with unk10
//       left at 0.  Then drive the fields to their boundaries to read off the
//       carry chain, and finish with a TRANSPOSITION control -- the same three
//       numbers in the wrong order must NOT reproduce the effect.
//
//   Q2  Does the re-arm on the non-level-1 path really write the level-1 triple?
//       That is the "ROM bug" claim, so it gets a real experiment: plant markers
//       in the save record behind gUnk_03004670, run the function once through
//       the game's own frame-callback queue, and read back which triple moved.
//       CONTROLS: (a) the same planting with the function NOT installed, (b) the
//       level-1 path, which must re-arm its own triple, (c) a non-zero triple,
//       which must not be re-armed at all.
//
// How the function is invoked: gCallbackQueue (0x03003510) is the main loop's
// callback list; the dispatcher at 0x080005E6 calls current[0 .. currentCount-2]
// and treats current[currentCount-1] as a sentinel (0 = swap next[] in, anything
// else = keep going).  So appending an entry before the sentinel and bumping
// currentCount runs a function once per frame with the game's own dispatcher --
// no CPU state is forged.  In the shipped ROM the only caller is InitLevelBG,
// under `world == 6 && (level == 1 || level == 3)`; the shipped savestates are
// all world 1, so the branch input (gUnk_03004C20.level) is taken from the
// savestate rather than forced: the world map has level == 0 (the "else" path)
// and the in-level state has level == 1 (the "if" path).
import { HeadlessRuntime } from './gba-kit.mjs';
import { mkdirSync } from 'fs';
import { ROM, ELF, SAVES, readField, writeField } from './_harness.mjs';

const OUT = '/tmp/klonoa-dyn-hud';
mkdirSync(OUT, { recursive: true });
const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const eng = rt.engine;
const bus = rt.gba.bus;
const di = eng.debugInfo;
const hex = (n) => '0x' + (n >>> 0).toString(16);
const need = (n) => {
    const a = di.symbolToAddress(n);
    if (a == null) throw new Error(`symbolToAddress('${n}') returned null`);
    return a >>> 0;
};
const member = (s, f) => {
    const m = di.structMember(s, f);
    if (!m) throw new Error(`no DWARF member ${s}.${f}`);
    return m;
};

const FN = need('UpdateHUDTimePanel') & ~1;
const A_C20 = need('gUnk_03004C20');
const A_5220 = need('gUnk_03005220');
const A_4670P = need('gUnk_03004670'); // a POINTER; the record lives behind it
const A_TILEBUF = need('gBgTilemapBufs');
const A_QUEUE = need('gCallbackQueue');
console.log('UpdateHUDTimePanel', hex(FN), ' gUnk_03004C20', hex(A_C20), ' gUnk_03005220', hex(A_5220),
    ' gUnk_03004670', hex(A_4670P), ' gBgTilemapBufs', hex(A_TILEBUF), ' gCallbackQueue', hex(A_QUEUE));

const M = {
    level: member('Unk_03004C20', 'level'),
    unk10: member('Unk_03004C20', 'unk10'),
    lives: member('Unk_03005220', 'lives'),
    d: member('Unk_03005220', 'unk4D'),
    e: member('Unk_03005220', 'unk4E'),
    f: member('Unk_03005220', 'unk4F'),
    sub: member('Unk_03005220', 'unk60'),
    cur: member('CallbackQueue', 'current'),
    curN: member('CallbackQueue', 'currentCount'),
};
const rec = (i) => member('Unk_03004670', 'unk' + i);
const clock = () => [readField(bus, A_5220, M.d), readField(bus, A_5220, M.e), readField(bus, A_5220, M.f), readField(bus, A_5220, M.sub)];
const setClock = (d, e, f, sub = 0) => {
    writeField(bus, A_5220, M.d, d); writeField(bus, A_5220, M.e, e);
    writeField(bus, A_5220, M.f, f); writeField(bus, A_5220, M.sub, sub);
};
const recordPtr = () => {
    const p = bus.read32(A_4670P) >>> 0;
    if (!p || (p >>> 24) === 0) throw new Error('gUnk_03004670 does not point anywhere: ' + hex(p));
    return p;
};
const readRecord = () => { const p = recordPtr(); return [1, 2, 3, 4, 5, 6].map((i) => readField(bus, p, rec(i))); };
const writeRecord = (v) => { const p = recordPtr(); v.forEach((x, i) => writeField(bus, p, rec(i + 1), x)); };

// ===========================================================================
// Q1  the three fields are a clock: tick rate, carry chain, ceiling
// ===========================================================================
console.log('\n=== Q1 what are gUnk_03005220.unk4D / unk4E / unk4F? ===');
await eng.loadState(`${SAVES}/savestate-in-level-idle.json`);
await eng.wait({ frames: 5 });
await eng.press('a'); // leave the pause menu so the in-level state machine runs
await eng.wait({ frames: 90 });
await eng.takeScreenshot({ name: 'in-level-unpaused' });
console.log('  level =', readField(bus, A_C20, M.level), ' unk10 =', readField(bus, A_C20, M.unk10),
    ' lives =', readField(bus, A_5220, M.lives), ' clock =', clock().join('/'));

console.log('\n  CONTROL: unk10 left at 0, 120 frames of the same input');
let before = clock();
await eng.press('right', { hold: 120 });
console.log(`    unk4D/4E/4F/unk60  ${before.join('/')}  ->  ${clock().join('/')}`);

console.log('\n  INTERVENTION: gUnk_03004C20.unk10 := 1 (held), 180 frames of the same input');
const hold10 = () => writeField(bus, A_C20, M.unk10, 1);
hold10();
const log = [];
eng.onFrame(() => { hold10(); log.push(clock()); });
await eng.press('right', { hold: 180 });
eng.onFrame(null);
console.log(`    unk4D/4E/4F/unk60  0/0/0/0  ->  ${clock().join('/')}`);
console.log('    first 8 frames:', log.slice(0, 8).map((x) => x.join(':')).join('  '));
const subStep = log[1][3] - log[0][3];
console.log(`    unk60 step per frame: ${subStep};  unk4F == floor(unk60/100) every frame: ${log.every((x) => x[2] === Math.floor(x[3] / 100))}`);
let prev = log[0][1];
const secondEdges = [];
for (let i = 1; i < log.length; i++) if (log[i][1] !== prev) { secondEdges.push(i); console.log(`    unk4E ${prev} -> ${log[i][1]} at frame ${i}`); prev = log[i][1]; }
console.log(`    frames per unk4E step: ${secondEdges.slice(1).map((e, i) => e - secondEdges[i]).join(',')}  (GBA frame = 59.7275 Hz, so one step = one second)`);

console.log('\n  carry chain: force unk4E to 58 and watch the wrap into unk4D');
setClock(0, 58, 0, 0);
const log2 = [];
eng.onFrame(() => { hold10(); log2.push(clock()); });
await eng.press('right', { hold: 140 });
eng.onFrame(null);
const uniq = [];
for (const s of log2) { const k = s.slice(0, 2).join(':'); if (uniq[uniq.length - 1] !== k) uniq.push(k); }
console.log('    unk4D:unk4E sequence over those frames:', uniq.join(' -> '));
console.log(`    unk4E wrapped 59 -> 0 and unk4D became ${clock()[0]}  => unk4E is seconds (0..59), unk4D counts its overflow`);

console.log('\n  ceiling: force 98:59:xx and check what unk4D does at 99');
setClock(98, 59, 0, 0);
eng.onFrame(hold10);
await eng.press('right', { hold: 130 });
eng.onFrame(null);
console.log(`    after crossing the minute: ${clock().slice(0, 3).join(':')}  (unk4D capped at 99: ${clock()[0] === 99})`);

console.log('\n  the freeze guard: the ROM stops the clock when the triple reaches its ceiling');
for (const [label, trip] of [['at ceiling 99:59:99', [99, 59, 99]], ['one below 99:59:98', [99, 59, 98]], ['TRANSPOSED 59:99:99', [59, 99, 99]]]) {
    setClock(trip[0], trip[1], trip[2], 0);
    const b = clock();
    eng.onFrame(hold10);
    await eng.press('right', { hold: 90 });
    eng.onFrame(null);
    const a = clock();
    console.log(`    ${label.padEnd(22)} ${b.slice(0, 3).join(':')} -> ${a.slice(0, 3).join(':')}   ${a.join() === b.join() ? 'FROZEN' : 'still ticking'}`);
}

// ===========================================================================
// Q2  run the function itself and see which triple the re-arm writes
// ===========================================================================
console.log('\n=== Q2 which triple does the all-zero re-arm write? ===');

/** Append fn to the main loop's callback list for `frames` frames, then restore. */
async function runAsCallback(fnAddr, frames) {
    const base = A_QUEUE + M.cur.offset;
    const n = readField(bus, A_QUEUE, M.curN);
    const saved = Array.from({ length: n + 2 }, (_, i) => bus.read32(base + 4 * i) >>> 0);
    if (saved[n - 1] === 0) throw new Error('sentinel is 0 -- the queue is being swapped this frame, retry later');
    bus.write32(base + 4 * (n - 1), (fnAddr | 1) >>> 0); // our callback where the sentinel was
    bus.write32(base + 4 * n, saved[n - 1]); // sentinel one slot further out
    writeField(bus, A_QUEUE, M.curN, n + 1);
    await eng.wait({ frames });
    writeField(bus, A_QUEUE, M.curN, n);
    saved.forEach((v, i) => bus.write32(base + 4 * i, v));
}
const queueDump = () => {
    const n = readField(bus, A_QUEUE, M.curN);
    return Array.from({ length: n }, (_, i) => {
        const p = bus.read32(A_QUEUE + M.cur.offset + 4 * i) >>> 0;
        return p === 1 ? '<sentinel>' : (di.addressToSymbol(p & ~1)?.name ?? hex(p));
    }).join(', ');
};

// --- (a) the "else" path: the world map, where gUnk_03004C20.level == 0 ----
await eng.loadState(`${SAVES}/savestate-in-gameplay.json`);
await eng.wait({ frames: 8 });
console.log('  state: world map, gUnk_03004C20.level =', readField(bus, A_C20, M.level), '(!= 1, so the function takes the else path)');
console.log('  gCallbackQueue.current:', queueDump());
console.log('  save record behind gUnk_03004670 @', hex(recordPtr()));

console.log('\n  CONTROL C1: plant unk1..3 = 11,22,33 / unk4..6 = 0,0,0 and DO NOT install the function');
writeRecord([11, 22, 33, 0, 0, 0]);
await eng.wait({ frames: 4 });
console.log('    unk1..6 after 4 frames:', readRecord().join(','), '(unchanged: no re-arm happens by itself)');

console.log('\n  INTERVENTION I1: same planting, function installed for 2 frames (else path, selected triple all zero)');
writeRecord([11, 22, 33, 0, 0, 0]);
await runAsCallback(FN, 2);
const afterI1 = readRecord();
console.log('    unk1..6:', afterI1.join(','));
console.log(`    unk1..3 re-armed to 99,59,99: ${afterI1.slice(0, 3).join() === '99,59,99'}`);
console.log(`    unk4..6 (the triple it TESTED) left at 0,0,0: ${afterI1.slice(3).join() === '0,0,0'}`);

console.log('\n  CONTROL C2: else path with a NON-zero selected triple -- nothing may be re-armed');
writeRecord([11, 22, 33, 7, 8, 9]);
await runAsCallback(FN, 2);
console.log('    unk1..6:', readRecord().join(','), '(unchanged: the re-arm is guarded by the all-zero test)');

// --- what got drawn?  paint distinguishable digit strips first, so the panel
// --- can be read back unambiguously instead of guessed at ------------------
const tile = (i) => bus.read16(A_TILEBUF + 2 * i);
const setTile = (i, v) => bus.write16(A_TILEBUF + 2 * i, v);
console.log('\n  what the panel draws (digit strips replaced with markers 0xD00d / 0xE00d so the readout decodes uniquely):');
for (let d = 0; d <= 9; d++) { setTile(0x312 + d, 0xd000 + d); setTile(0x332 + d, 0xe000 + d); }
// blit markers too: the two DmaCopy16 rows come from row 0x16/0x17, col 0x12
for (let i = 0; i < 2; i++) for (let c = 0; c < 10; c++) setTile((0x16 + i) * 0x20 + 0x12 + c, 0xb000 + i * 16 + c);
writeRecord([11, 22, 33, 7, 8, 9]);
setClock(12, 34, 56, 0);
await runAsCallback(FN, 2);
const dec = (slots, base) => slots.map((s) => { const v = tile(s); return (v & 0xf000) === base ? String(v & 0xf) : '?' + hex(v); }).join('');
console.log('    row 0 slots 0x15,0x16 | 0x18,0x19 | 0x1B,0x1C :',
    dec([0x15, 0x16], 0xd000) + ':' + dec([0x18, 0x19], 0xd000) + ':' + dec([0x1b, 0x1c], 0xd000),
    ` <- gUnk_03004670 unk1..3 = ${readRecord().slice(0, 3).join(':')}, unk4..6 = ${readRecord().slice(3).join(':')}`);
console.log('    row 1 slots 0x35,0x36 | 0x38,0x39 | 0x3B,0x3C :',
    dec([0x35, 0x36], 0xe000) + ':' + dec([0x38, 0x39], 0xe000) + ':' + dec([0x3b, 0x3c], 0xe000),
    ' <- gUnk_03005220 unk4D:unk4E:unk4F = 12:34:56');
console.log('    panel frame blit, row 0 cols 0x14..0x1D:', Array.from({ length: 10 }, (_, c) => hex(tile(0x14 + c))).join(' '));
console.log('    panel frame blit, row 1 cols 0x14..0x1D:', Array.from({ length: 10 }, (_, c) => hex(tile(0x20 + 0x14 + c))).join(' '));
console.log('      (0xB00c / 0xB01c are the markers planted at row 0x16/0x17 col 0x12; digit slots are overwritten afterwards)');
console.log('    lives (offset 0x4C) is', readField(bus, A_5220, M.lives), 'and appears nowhere in the panel');

// --- (b) the "if" path: an in-level state, where level == 1 ----------------
console.log('\n  CONTROL C3: the level-1 path re-arms the triple it tested');
await eng.loadState(`${SAVES}/savestate-in-level-idle.json`);
await eng.wait({ frames: 5 });
await eng.press('a');
await eng.wait({ frames: 60 });
console.log('    state: in level, gUnk_03004C20.level =', readField(bus, A_C20, M.level));
console.log('    gCallbackQueue.current:', queueDump());
writeRecord([0, 0, 0, 7, 8, 9]);
await runAsCallback(FN, 2);
const afterC3 = readRecord();
console.log('    unk1..6:', afterC3.join(','));
console.log(`    unk1..3 re-armed to 99,59,99: ${afterC3.slice(0, 3).join() === '99,59,99'};  unk4..6 untouched: ${afterC3.slice(3).join() === '7,8,9'}`);
await eng.takeScreenshot({ name: 'after-panel-redraw' });
console.log('\nscreenshots in', OUT);
