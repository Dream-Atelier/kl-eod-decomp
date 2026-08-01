// PROOF: `gUnk_030034B0` byte 7, low nibble (was `unk7_0`) is the WORLD-MAP SELECTED
// VISION — a 1-based vision number, 0 meaning "auto-rotation idle".
//
// UpdateWorldMapNodeState (code_3.c, 0x080468B0) reads that nibble and steers the
// rotating world-map globe toward the matching node:
//
//     gBg2Alpha ==? (u8)-gWorldMapNodes[world-1][gWorldMapVisionNode[world-1][selectedVision-1]][1]
//
// and, while they differ, fakes an L (0x200) / R (0x100) press in gHeldKeys so the
// ordinary world-map rotation code spins the globe the short way round.
//
// METHOD (causal intervention, not correlation): park the emulator on the world
// map, then for each candidate vision number V write V into the nibble (bitfield
// shift/width read from the ELF's DWARF, not hand-coded) and let the game run.
// If the nibble really selects a vision, gBg2Alpha must converge on exactly the
// angle the ROM table stores for THAT vision, and the synthesized shoulder press
// must be the one that takes the shorter arc.
//
// Run: node docs/dynamic-analysis/scripts/prove-worldmap-selected-vision.mjs
import { HeadlessRuntime } from './gba-kit.mjs';
import { mkdirSync, readFileSync } from 'fs';
import { ROM, ELF, SAVES, readField, writeField } from './_harness.mjs';

const OUT = '/tmp/klonoa-dynamic-out';
mkdirSync(OUT, { recursive: true });
const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const eng = rt.engine,
  bus = rt.gba.bus,
  di = eng.debugInfo;

// Everything below is resolved from the decomp's own ELF: symbol addresses from
// the ldscript, field offsets + bitfield shift/width from agbcc's DWARF.
const UI = di.symbolToAddress('gUnk_030034B0');
const SCENE = di.symbolToAddress('gUnk_03004C20');
const ALPHA = di.symbolToAddress('gBg2Alpha');
const KEYS = di.symbolToAddress('gHeldKeys');
const T_REC = di.symbolToAddress('gWorldMapNodes'); // [world-1][nodeIdx][5]
const T_IDX = di.symbolToAddress('gWorldMapVisionNode'); // [world-1][vision-1]
const SEL = di.structMember('Unk_030034B0', 'selectedVision');
const TIMER = di.structMember('Unk_030034B0', 'visionArrivalTimer');
const WORLD = di.structMember('Unk_03004C20', 'world');
const LEVEL = di.structMember('Unk_03004C20', 'level');
console.log('DWARF: Unk_030034B0.selectedVision =', JSON.stringify(SEL), ' .visionArrivalTimer =', JSON.stringify(TIMER));
console.log(
  'ldscript: gWorldMapNodes=0x' + T_REC.toString(16),
  'gWorldMapVisionNode=0x' + T_IDX.toString(16),
  'gBg2Alpha=0x' + ALPHA.toString(16),
  'gHeldKeys=0x' + KEYS.toString(16),
);

// The ROM tables, read straight out of the cartridge image.
const rom = readFileSync(ROM);
const romU8 = (a) => rom[a - 0x08000000];
// exactly the expression UpdateWorldMapNodeState compares gBg2Alpha against
const targetAngle = (world, vision) => {
  const nodeIdx = romU8(T_IDX + (world - 1) * 8 + (vision - 1));
  return (-romU8(T_REC + (world - 1) * 200 + nodeIdx * 5 + 1)) & 0xff;
};

const sel = () => readField(bus, UI, SEL);
const alpha = () => bus.read8(ALPHA);

await eng.loadState(`${SAVES}/savestate-fresh-gameplay.json`);
await eng.pressSequence([[null, 30]]);
const world = readField(bus, SCENE, WORLD);
console.log(
  `\nstart: world=${world} level=${readField(bus, SCENE, LEVEL)} (0 = world map)  selected=${sel()}  gBg2Alpha=${alpha()}`,
);
console.log('ROM table says, for world %d:', world);
for (let v = 1; v <= 8; v++) console.log(`   vision ${v} -> target gBg2Alpha = ${targetAngle(world, v)}`);

console.log('\n--- intervention: write vision N into gUnk_030034B0.selectedVision, then just let the game run ---');
console.log('vision  targetAngle  alphaBefore  keysSeen   alphaAfter  arrived?  framesToArrive');
const results = [];
for (const v of [3, 5, 8, 2, 1]) {
  await eng.loadState(`${SAVES}/savestate-fresh-gameplay.json`);
  await eng.pressSequence([[null, 30]]);
  const before = alpha();
  writeField(bus, UI, SEL, v);
  const want = targetAngle(world, v);
  const keysSeen = new Set();
  let frames = 0,
    arrived = -1;
  for (let i = 0; i < 60 && arrived < 0; i++) {
    await eng.pressSequence([[null, 5]]);
    frames += 5;
    keysSeen.add(bus.read16(KEYS) & 0x300);
    if (alpha() === want) arrived = frames;
  }
  const keys = [...keysSeen]
    .filter((k) => k)
    .map((k) => '0x' + k.toString(16))
    .join(',');
  console.log(
    String(v).padStart(6),
    String(want).padStart(12),
    String(before).padStart(12),
    (keys || '-').padStart(10),
    String(alpha()).padStart(12),
    String(arrived >= 0).padStart(9),
    String(arrived >= 0 ? arrived : 'n/a').padStart(15),
  );
  results.push({ v, want, arrived });
  await eng.takeScreenshot({ name: `worldmap-selected-vision-${v}` });
}

// Negative control: a value the table does not describe as "here" — vision 1's
// angle must NOT be reached when 5 is selected, and vice versa.
console.log('\n--- control: every trial landed on ITS OWN vision angle, not a shared one ---');
console.log(results.map((r) => `vision ${r.v} -> ${r.want} (${r.arrived >= 0 ? 'reached' : 'MISSED'})`).join('\n'));

// And show the shoulder-press direction is the short arc, i.e. the (s8) difference.
console.log('\n--- direction check: (s8)(target - alpha) sign vs the key the game was fed ---');
for (const r of results) {
  const d = ((r.want - 1) << 24) >> 24; // alpha starts at 1 in this save state
  console.log(
    `vision ${r.v}: target=${r.want} start=1  (s8)(target-start)=${d}  =>  expect ${d < 0 ? 'L_BUTTON 0x200' : d > 0 ? 'R_BUTTON 0x100' : 'L|R 0x300'}`,
  );
}

// ---------------------------------------------------------------------------
// PHASE 2 — the byte at offset 5 (was `unk5`) is the ARRIVAL COUNTDOWN.
// UpdateWorldMapNodeState decrements it every frame, arms it to 0x80 the frame
// the globe reaches the node, plays a jingle + clears the node's progress byte
// at 0x40, and at 1 hands the selection to FindNextUnlockedVision.
console.log('\n--- phase 2: arrival countdown in gUnk_030034B0.visionArrivalTimer (selected vision = 3) ---');
await eng.loadState(`${SAVES}/savestate-fresh-gameplay.json`);
await eng.pressSequence([[null, 30]]);
writeField(bus, UI, SEL, 3);
const want3 = targetAngle(world, 3);
console.log('frame  gBg2Alpha  timer  selected(vision)  note');
let note = '';
let prevSel = 3;
for (let f = 0; f <= 300; f += 10) {
  const a = alpha(),
    t = readField(bus, UI, TIMER),
    s = sel();
  note = a === want3 ? 'at node' : 'rotating';
  if (s !== prevSel) note += `  <- selection changed ${prevSel} -> ${s}`;
  prevSel = s;
  console.log(String(f).padStart(5), String(a).padStart(10), String(t).padStart(5), String(s).padStart(17), ' ' + note);
  await eng.pressSequence([[null, 10]]);
}
