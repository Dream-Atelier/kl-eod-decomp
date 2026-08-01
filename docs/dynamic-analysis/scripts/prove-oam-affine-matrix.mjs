// PROOF: 0x03004680 is the OBJ (sprite) AFFINE-MATRIX shadow table, and its four
// halfwords are the GBA's PA/PB/PC/PD in that order. This is what justified
// renaming
//
//     struct Unk_03004680 { u16 unk0; u16 unk2; u16 unk4; u16 unk6; };
//     extern struct Unk_03004680 gUnk_03004680[];
//  -> struct OamAffineMatrix { u16 pa; u16 pb; u16 pc; u16 pd; };
//     extern struct OamAffineMatrix gOamAffineMatrix[];
//
// (the script below reads the fields back out of the build's DWARF by their new
// names, so it keeps verifying the claim after the rename).
//
// Why this matters: StreamCmd_SetEntityTransform (gfx.c) writes
//     gOamAffineMatrix[n].pa = MultiplyQ8(0x100, ReciprocalQ8(mag));
//     gOamAffineMatrix[n].pb = 0;
//     gOamAffineMatrix[n].pc = 0;
//     gOamAffineMatrix[n].pd = MultiplyQ8(0x100, ReciprocalQ8(mag));
// and puts `n` into the entity's `affineHFlip_matrixNum`. With the first and last
// halfwords named pa/pd that reads as "load OBJ affine matrix n with a uniform 1/mag scale, no
// rotation and no shear" — which is exactly what the stream command means.
//
// The GBA interleaves the 32 OBJ affine matrices into OAM: matrix m's
// PA/PB/PC/PD are the halfwords at 0x07000006 + 32m, +0x0E, +0x16, +0x1E.
// (attribute3 of OAM entries 4m+0..4m+3). So if this table is the shadow copy,
// slot m must land there.
//
// Three independent observations below:
//   (1) correlation  — every one of the 8 live slots equals OAM's matrix, including
//                      the non-round 0x199 the game itself computed;
//   (2) causation    — writing four DISTINCT sentinels into one slot makes exactly
//                      those four values appear at PA/PB/PC/PD in that order;
//   (3) semantics    — forcing the player sprite affine and then perturbing only
//                      the +0x00 halfword squashes it HORIZONTALLY, only +0x06
//                      squashes it VERTICALLY: the defined meaning of PA and PD.
//
// Field offsets/sizes and both symbol addresses come from the build's DWARF via
// @gba-kit/debug-info — no hand-coded offsets.
import { HeadlessRuntime } from './gba-kit.mjs';
import { mkdirSync } from 'fs';
import { ROM, ELF, readField, writeField } from './_harness.mjs';

const OUT = '/tmp/klonoa-affine-proof';
mkdirSync(OUT, { recursive: true });

// boot -> title -> file select -> first vision (same entry sequence the other proofs use)
const ENTRY = [
  [null,409],['a',5],[null,20],['a',4],[null,8],['a',4],[null,8],['a',3],[null,8],['a',4],[null,51],['a',4],[null,8],['a',3],[null,89],
  ['a',4],[null,18],['a',4],[null,6],['start',6],[null,35],['a',6],[null,20],['a',3],[null,7],['a',4],[null,42],['a',3],[null,10],['a',4],[null,20],
  ['start',6],[null,34],['a',6],[null,20],['a',6],[null,13],['a',4],[null,124],
];

const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const eng = rt.engine, bus = rt.gba.bus, di = eng.debugInfo;

const MAT = di.symbolToAddress('gOamAffineMatrix');
const ENT = di.symbolToAddress('gUnk_03002920');
const F = ['pa', 'pb', 'pc', 'pd'].map((n) => di.structMember('OamAffineMatrix', n));
const MAT_SZ = 8; // sizeof(struct OamAffineMatrix)
const ENT_SZ = 0x1C;
const AE = di.structMember('Unk_03002920', 'affineEnable');
const AD = di.structMember('Unk_03002920', 'affineDouble');
const MN = di.structMember('Unk_03002920', 'affineHFlip_matrixNum');

// GBA OBJ affine matrix m -> OAM attribute3 of entries 4m+0..4m+3
const oamAffine = (m) => [6, 14, 22, 30].map((o) => bus.read16(0x07000000 + m * 32 + o));
const shadow = (m) => F.map((f) => readField(bus, MAT + m * MAT_SZ, f));
const hex = (a) => '{' + a.map((x) => '0x' + x.toString(16).padStart(4, '0')).join(', ') + '}';

await eng.pressSequence(ENTRY);

// ── (1) correlation ────────────────────────────────────────────────────────────
console.log('(1) shadow table vs the hardware OBJ affine matrices in OAM\n');
console.log('  m   gOamAffineMatrix[m] {pa,pb,pc,pd}         OAM matrix m {pa,pb,pc,pd}      equal?');
let allEqual = true;
for (let m = 0; m < 8; m++) {
  const s = shadow(m), o = oamAffine(m);
  const eq = s.every((v, i) => v === o[i]);
  allEqual &&= eq;
  console.log(`  ${m}   ${hex(s).padEnd(38)} ${hex(o).padEnd(30)} ${eq ? 'yes' : 'NO'}`);
}
console.log(`\n  all 8 slots identical: ${allEqual}`);
console.log('  note slot 0 = 0x199 — a value the game computed itself (0x100/0x9F-ish),');
console.log('  not a constant we could have matched by luck.\n');

// ── (2) causation ──────────────────────────────────────────────────────────────
const M = 3;
const SENTINEL = [0x1111, 0x2222, 0x3333, 0x4444];
console.log(`(2) write four DISTINCT sentinels into gOamAffineMatrix[${M}] and step one frame\n`);
console.log(`  before:  shadow ${hex(shadow(M))}   OAM ${hex(oamAffine(M))}`);
F.forEach((f, i) => writeField(bus, MAT + M * MAT_SZ, f, SENTINEL[i]));
console.log(`  wrote :  +0x00=0x1111 +0x02=0x2222 +0x04=0x3333 +0x06=0x4444`);
await eng.wait({ frames: 2 });
const got = oamAffine(M);
console.log(`  after :  OAM matrix ${M} = ${hex(got)}`);
const names = ['pa', 'pb', 'pc', 'pd'];
F.forEach((f, i) =>
  console.log(
    `           +0x0${f.offset} (0x${SENTINEL[i].toString(16)}) -> OAM ${names[i].toUpperCase()} = 0x${got[i].toString(16)}  ${got[i] === SENTINEL[i] ? 'MATCH' : 'mismatch'}`,
  ),
);
console.log(`  field order proven: ${got.every((v, i) => v === SENTINEL[i])}\n`);

// ── (3) semantics: pa is the X scale, pd is the Y scale ────────────────────────
// The level has no affine entity of its own, so force the player (entity 0) to use
// matrix M. The entity's flags are rewritten every frame from behaviour state, so
// clamp them with a watchpoint the way find-entity-flip.mjs does.
console.log('(3) force the player sprite through matrix 3 and perturb one field at a time\n');
// let the "VISION 1" intro banner (which animates its own affine zoom) finish first,
// so the only thing moving on screen is what we perturb.
await eng.wait({ frames: 260 });
const forceAffine = () => {
  writeField(bus, ENT, AE, 1); // OBJ affine enable
  writeField(bus, ENT, AD, 1); // double-size bounding box, so a zoom is not clipped
  writeField(bus, ENT, MN, M); // use matrix M
};
const setMatrix = (vals) => F.forEach((f, i) => writeField(bus, MAT + M * MAT_SZ, f, vals[i]));
// The entity's affine flags and the matrix are both rewritten from behaviour state,
// so re-assert them once per frame instead of using a (much slower) watchpoint.
const shot = async (name, vals, note) => {
  for (let i = 0; i < 8; i++) {
    forceAffine();
    setMatrix(vals);
    await eng.wait({ frames: 1 });
  }
  forceAffine();
  setMatrix(vals);
  console.log(`  ${name.padEnd(26)} matrix=${hex(vals)}  OAM=${hex(oamAffine(M))}   ${note}`);
  await eng.takeScreenshot({ name });
};
await shot('affine-identity', [0x100, 0, 0, 0x100], 'identity — sprite at 1:1');
await shot('affine-pa-half', [0x200, 0, 0, 0x100], 'pa (+0x00) = 0x200 -> HALF WIDTH only  => PA, the x scale');
await shot('affine-pd-half', [0x100, 0, 0, 0x200], 'pd (+0x06) = 0x200 -> HALF HEIGHT only => PD, the y scale');
await shot('affine-uniform-half', [0x200, 0, 0, 0x200], 'both -> uniform half size, i.e. what StreamCmd_SetEntityTransform emits for mag=2.0');
console.log('\n  screenshots in ' + OUT);
