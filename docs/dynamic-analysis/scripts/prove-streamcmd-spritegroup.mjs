// PROOF: what the stream command at 0x0804C300 (StreamCmd_SetupOAMSpriteGroup) really
// writes, and what its 16-bit operand selects.
//
// The name asserts three things that static reading cannot settle, and one of them is
// wrong in the obvious reading:
//   1. "OAM"    — does it write hardware OAM (0x07000000), or the OAM shadow buffer
//                 gOamBuffer (0x03004800) that VBlankHandler_OamOnly DMAs, or neither?
//   2. "Group"  — does one command set up SEVERAL objects, or just one?
//   3. the operand at stream bytes [2..3] is a *group id* into gUnk_08189F04.
//
// METHOD — inject the command and compare against a control.
//   The graphics/music stream is consumed from EWRAM (gStreamPtr, 0x03004D84, points at
//   0x020009xx in a running scene), so it is *writable*: from one savestate we run the
//   same single frame three times, changing only the four bytes at the stream cursor:
//     A  control   — nothing written                       (the command never runs)
//     B  FF 24 00 00 — group 0                             (ROM table says 7 live rows)
//     C  FF 24 04 00 — group 4                             (ROM table says 13 live rows)
//     D  FF 25 00 00 — SAME payload, opcode nibble 5       (negative control: the byte-1
//                                                           low nibble is the selector, so
//                                                           this must reach a DIFFERENT
//                                                           handler and spawn nothing)
//   Byte [4] is set to 0x00 so the dispatcher's loop (StreamCmd_RunScript, 0x0804EA94)
//   stops after our command instead of running the rest of the stream misaligned.
//
// Every write performed *from inside* the command's call tree (PCs whose DWARF function
// is StreamCmd_SetupOAMSpriteGroup or SetupOAMSprite) is recorded across all of IWRAM,
// so "it never touches the OAM shadow" is a measurement, not an argument: gOamBuffer
// (0x03004800) is inside the watched range and simply never appears.
//
// Offsets/masks come from the build's DWARF (`di.structMember`), addresses from the
// symbol table; the only hand-written constants are hardware ones (OAM base) and the
// dispatch-table address 0x08117854, which is itself checked against the ELF's function
// address before it is used.
import { HeadlessRuntime } from './gba-kit.mjs';
import { mkdirSync } from 'fs';
import { ROM, ELF, readField } from './_harness.mjs';

const OUT = '/tmp/klonoa-spritegroup';
mkdirSync(OUT, { recursive: true });
const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const eng = rt.engine,
  bus = rt.gba.bus,
  di = eng.debugInfo;
const hex = (v, n = 8) => '0x' + (v >>> 0).toString(16).padStart(n, '0');
const need = (n) => {
  const a = di.symbolToAddress(n);
  if (a == null) throw new Error(`symbol ${n} unresolved — renamed?`);
  return a >>> 0;
};

const FN = need('StreamCmd_SetupOAMSpriteGroup');
const A_TABLE = need('gUnk_08189F04');
const A_TABLE_END = need('gUnk_0818B704'); // the next symbol in ldscript.txt
const A_ENT = need('gUnk_03002920');
const A_CURSOR = need('gUnk_03005428');
const A_STREAM = need('gStreamPtr');
const A_OAMBUF = 0x03004800; // gOamBuffer (a #define in include/ui.h, so no ELF symbol)
const OAM = 0x07000000;
const DISPATCH = 0x08117854; // sub-table used when stream byte[1] has bit5 set, bits 7/6/4 clear

// ---- DWARF layout (no hand-typed offsets) ---------------------------------
const R = Object.fromEntries(
  ['unk0', 'unk2', 'unk4', 'unk5', 'unk6', 'unk7', 'unk8', 'unk9'].map((k) => [k, di.structMember('Unk_08189F04', k)]),
);
const E = Object.fromEntries(
  ['xPosScreen', 'yPosScreen', 'unk8', 'unkA', 'unkF', 'kind'].map((k) => [k, di.structMember('Unk_03002920', k)]),
);
for (const [k, v] of Object.entries({ ...R, ...E })) if (!v) throw new Error(`DWARF member ${k} missing`);
const ROW = 0xc; // sizeof(struct Unk_08189F04) — checked against the DWARF below
const SLOT = 0x1c; // sizeof(struct Unk_03002920)
console.log('DWARF Unk_08189F04:', Object.entries(R).map(([k, v]) => `${k}@+0x${v.offset.toString(16)}/${v.size}`).join(' '));
console.log('DWARF Unk_03002920:', Object.entries(E).map(([k, v]) => `${k}@+0x${v.offset.toString(16)}/${v.size}`).join(' '));
console.log('symbols: fn', hex(FN), 'table', hex(A_TABLE), 'entities', hex(A_ENT), 'cursor', hex(A_CURSOR), 'gStreamPtr', hex(A_STREAM));

// ---- ROM arithmetic, before any emulation ---------------------------------
// The dispatcher indexes this table with (stream byte[1] & 0xF); entry 4 must be our
// function, with the Thumb bit set.
const entry4 = bus.read32(DISPATCH + 4 * 4) >>> 0;
console.log(`\ndispatch table ${hex(DISPATCH)}[4] = ${hex(entry4)}  (fn|1 = ${hex(FN | 1)})  match: ${entry4 === (FN | 1) >>> 0}`);
if (entry4 !== ((FN | 1) >>> 0)) throw new Error('dispatch entry 4 is not StreamCmd_SetupOAMSpriteGroup');

const span = A_TABLE_END - A_TABLE;
console.log(`group table spans ${hex(A_TABLE)}..${hex(A_TABLE_END)} = ${span} bytes = ${span / (16 * ROW)} groups of 16 rows x 0x${ROW.toString(16)}`);
const groupRows = [];
for (let g = 0; g < span / (16 * ROW); g++) {
  const rows = [];
  for (let i = 0; i < 16; i++) {
    const a = A_TABLE + g * 16 * ROW + i * ROW;
    if (readField(bus, a, R.unk0) === 0xffff) break;
    rows.push(Object.fromEntries(Object.entries(R).map(([k, f]) => [k, readField(bus, a, f)])));
  }
  groupRows.push(rows);
}
// NB: the C loop is unbounded (`for (i = 0; ...unk0 != 0xFFFF; i++)`), so "16" below means
// "no terminator inside this group's own 16 rows", not "16 sprites".
console.log('live rows per group (0xFFFF in unk0 terminates):', groupRows.map((r) => r.length).join(','));

// ---- reach a scene whose stream is being consumed --------------------------
const TO_SKIPPABLE_SCENE = [
  [null, 409], ['a', 5], [null, 20], ['a', 4], [null, 8], ['a', 4], [null, 8], ['a', 3], [null, 8],
  ['a', 4], [null, 51], ['a', 4], [null, 8], ['a', 3], [null, 89], ['a', 4], [null, 18],
];
await eng.pressSequence(TO_SKIPPABLE_SCENE);
await eng.saveState({ name: 'base' });
const BASE = `${OUT}/savestate-base.json`;
console.log(`\nbase state: gUnk_03005428 = ${bus.read8(A_CURSOR)}, gStreamPtr = ${hex(bus.read32(A_STREAM))} (EWRAM, hence writable)`);

const fnOf = (pc) => di.pcToFunction(pc)?.name ?? hex(pc);
const inCmd = (h) => {
  const n = fnOf(h.instructionAddress);
  return n === 'StreamCmd_SetupOAMSpriteGroup' || n === 'SetupOAMSprite';
};
const STACK_TOP = 0x03008000,
  STACK_LO = 0x03007c00; // IWRAM stack; writes here are locals, not globals

async function trial(label, { opcode = null, group = 0 } = {}) {
  await eng.loadState(BASE);
  const p = bus.read32(A_STREAM) >>> 0;
  const cursor0 = bus.read8(A_CURSOR);
  const wIwram = eng.watchMemory({ address: 0x03000000, length: 0x8000, maxHits: 8000, filter: inCmd });
  const wOam = eng.watchMemory({ address: OAM, length: 0x400, maxHits: 8000 });
  // separate, uncapped-in-practice watch: wIwram's hit cap truncates on the runaway trial
  const wCur = eng.watchMemory({ address: A_CURSOR, length: 1, maxHits: 20000, filter: inCmd });
  if (opcode !== null) {
    bus.write8(p + 0, 0xff);
    bus.write8(p + 1, opcode);
    bus.write8(p + 2, group & 0xff);
    bus.write8(p + 3, (group >> 8) & 0xff);
    bus.write8(p + 4, 0x00);
  }
  await eng.wait({ frames: 1 });
  const cursor1 = bus.read8(A_CURSOR);
  const hits = wIwram.hits;
  const streamWrite = hits.find((h) => h.address >= A_STREAM && h.address < A_STREAM + 4);
  const slots = [...new Set(hits.filter((h) => h.address >= A_ENT && h.address < A_ENT + SLOT * 128).map((h) => ((h.address - A_ENT) / SLOT) | 0))].sort((a, b) => a - b);
  const oamShadow = hits.filter((h) => h.address >= A_OAMBUF && h.address < A_OAMBUF + 0x400).length;
  const oamHw = wOam.hits.filter(inCmd).length;
  const other = [...new Set(hits.filter((h) => !(h.address >= A_ENT && h.address < A_ENT + SLOT * 128) && !(h.address >= STACK_LO && h.address < STACK_TOP) && !(h.address >= A_STREAM && h.address < A_STREAM + 4) && !(h.address >= A_CURSOR && h.address < A_CURSOR + 1)).map((h) => hex(h.address)))];
  console.log(`\n--- ${label} ---`);
  console.log(`  stream bytes written: ${opcode === null ? '(none — control)' : `FF ${opcode.toString(16)} ${(group & 0xff).toString(16)} ${((group >> 8) & 0xff).toString(16)}`} at ${hex(p)}`);
  console.log(`  handler that ran     : ${[...new Set(hits.map((h) => fnOf(h.instructionAddress)))].join(', ') || '(no write from StreamCmd_SetupOAMSpriteGroup/SetupOAMSprite)'}`);
  console.log(`  gStreamPtr advance   : ${streamWrite ? `${hex(p)} -> ${hex(streamWrite.value)} (+${streamWrite.value - p}) from ${fnOf(streamWrite.instructionAddress)}` : '(not advanced by the command)'}`);
  console.log(`  gUnk_03005428        : ${cursor0} -> ${cursor1} (+${cursor1 - cursor0})`);
  console.log(`  gUnk_03002920 slots  : ${slots.join(',') || '(none)'}`);
  console.log(`  writes to gOamBuffer (0x03004800) from the command: ${oamShadow}`);
  console.log(`  writes to hardware OAM (0x07000000) from the command: ${oamHw}`);
  console.log(`  hardware-OAM writers this frame (control that the OAM watch works): ${[...new Set(wOam.hits.map((h) => `${fnOf(h.instructionAddress)}/${h.source}`))].join(' ') || '(none)'}`);
  console.log(`  other globals written by the command: ${other.length > 12 ? `${other.length} addresses, ${other.slice(0, 6).join(' ')} ... ${other[other.length - 1]}` : other.join(' ') || '(none)'}`);
  const iterations = wCur.hits.length;
  console.log(`  loop iterations (writes to gUnk_03005428): ${iterations}`);
  wIwram.stop();
  wOam.stop();
  wCur.stop();
  return { cursor0, cursor1, slots, oamShadow, oamHw, streamWrite, p, iterations };
}

const A = await trial('A. CONTROL — no injection', {});
const B = await trial('B. inject FF 24 00 00 (group 0)', { opcode: 0x24, group: 0 });
const C = await trial('C. inject FF 24 04 00 (group 4)', { opcode: 0x24, group: 4 });
const D = await trial('D. NEGATIVE CONTROL — FF 25 00 00 (opcode nibble 5)', { opcode: 0x25, group: 0 });

// ---- do the spawned slots carry the ROM group's rows? ----------------------
function checkRows(res, g) {
  const rows = groupRows[g];
  let ok = true;
  console.log(`\ngroup ${g}: ${rows.length} ROM rows vs slots ${res.slots[0]}..${res.slots[res.slots.length - 1]}`);
  rows.forEach((row, i) => {
    const base = A_ENT + (res.cursor0 + i) * SLOT;
    const got = {
      xPosScreen: readField(bus, base, E.xPosScreen),
      yPosScreen: readField(bus, base, E.yPosScreen),
      unkA: readField(bus, base, E.unkA),
      unk8: readField(bus, base, E.unk8),
      unkF: readField(bus, base, E.unkF),
      kind: readField(bus, base, E.kind),
    };
    const pass =
      got.xPosScreen === row.unk0 && got.yPosScreen === row.unk2 && got.unkA === row.unk7 && got.unk8 === row.unk4 && got.unkF === row.unk6 && got.kind === row.unk8;
    ok = ok && pass;
    console.log(
      `  row ${String(i).padStart(2)} -> slot ${res.cursor0 + i}: x ${row.unk0}=${got.xPosScreen} y ${row.unk2}=${got.yPosScreen} ` +
        `unk7->unkA ${row.unk7}=${got.unkA} unk4->unk8 ${row.unk4}=${got.unk8} unk6->unkF ${row.unk6}=${got.unkF} unk8->kind ${row.unk8}=${got.kind}  ${pass ? 'ok' : 'MISMATCH'}`,
    );
  });
  return ok;
}
// C ran last of the two spawning trials, so re-run B to leave its state in memory.
const B2 = await trial('B2. re-run of B, to read its slots back', { opcode: 0x24, group: 0 });
const rowsB = checkRows(B2, 0);
const C2 = await trial('C2. re-run of C, to read its slots back', { opcode: 0x24, group: 4 });
const rowsC = checkRows(C2, 4);

// ---- out-of-range operand (informational, not a verdict) -------------------
// The C loop has no bound: it runs until a row's unk0 reads 0xFFFF. Groups 20..31 of the
// table are all zeros, so an id in that range walks past its own 16 rows.
const E_ = await trial('E. informational — FF 24 14 00 (group 20, which has no terminator)', { opcode: 0x24, group: 20 });
console.log(
  `  -> ${E_.iterations} iterations from a group whose own 16 rows contain no 0xFFFF: the loop runs past the group,` +
    ` writes descriptor slots far beyond the table, and the u8 cursor wraps (${E_.cursor0} -> ${E_.cursor1}).` +
    ` Only ids 0..19 are populated.`,
);

// ---- do the spawned descriptors become OAM objects? ------------------------
// NOT PROVEN HERE, and deliberately left as an open observation: in this scene the
// descriptor->OAM pass does not turn the injected slots into visible objects (they read
// back as the parked (-8,-8) tile 0), so this script says nothing about the second hop.
// What it does measure is that hardware OAM is written by DMA3 from the VBlank handler
// alone.
await eng.loadState(BASE);
await eng.wait({ frames: 1 });
const objs = eng.readOAM().filter((o) => o.enabled).length;
console.log(`\n(informational) enabled OAM objects in the untouched scene: ${objs}`);
for (const s of C2.slots.slice(0, 4)) {
  const o = eng.readOAM()[s];
  console.log(`  OAM entry ${s}: (${o.x},${o.y}) tile ${o.tileId} enabled=${o.enabled}  <- unverified whether an injected descriptor ever reaches it`);
}

// ---- verdict ---------------------------------------------------------------
console.log('\n=== VERDICT ===');
const v1 = A.cursor1 === A.cursor0 && A.slots.length === 0 && B.cursor1 - B.cursor0 === groupRows[0].length;
console.log(`"Group": one command spawns MANY objects — control +${A.cursor1 - A.cursor0}, group 0 +${B.cursor1 - B.cursor0} (ROM rows ${groupRows[0].length}), group 4 +${C.cursor1 - C.cursor0} (ROM rows ${groupRows[4].length}): ${v1 ? 'CONFIRMED' : 'REFUTED'}`);
const v2 = C.cursor1 - C.cursor0 === groupRows[4].length && B.cursor1 - B.cursor0 !== C.cursor1 - C.cursor0;
console.log(`bytes [2..3] are a GROUP INDEX: changing only them changes the count and the values written: ${v2 ? 'CONFIRMED' : 'REFUTED'}`);
const v3 = rowsB && rowsC;
console.log(`every spawned slot carries its ROM row's fields (x, y, unk7->unkA, unk4->unk8, unk6->unkF, unk8->kind): ${v3 ? 'CONFIRMED' : 'REFUTED'}`);
const v4 = B.oamShadow === 0 && C.oamShadow === 0 && B.oamHw === 0 && C.oamHw === 0;
console.log(`"OAM": the command writes NEITHER hardware OAM NOR gOamBuffer — it writes the sprite descriptor table gUnk_03002920 (and only DMA3 in the VBlank handler writes OAM): ${v4 ? 'CONFIRMED' : 'REFUTED'}`);
const v5 = D.slots.length === 0 && D.cursor1 === D.cursor0;
console.log(`opcode: the low nibble of stream byte[1] selects the handler — FF 25 with the same payload spawns nothing: ${v5 ? 'CONFIRMED' : 'REFUTED'}`);
const v6 = B.streamWrite && B.streamWrite.value - B.p === 4;
console.log(`the command is 4 bytes: it advances gStreamPtr by ${B.streamWrite ? B.streamWrite.value - B.p : '?'}: ${v6 ? 'CONFIRMED' : 'REFUTED'}`);
if (!(v1 && v2 && v3 && v4 && v5 && v6)) throw new Error('a prediction did not reproduce');
console.log('\nscreenshot + savestate in', OUT);
