// REFUTATION: `sub_0800D81A` was not a function. It is a label inside
// UpdatePlayerDamageState at 0x0800D188, and a functions_merged.cfg entry
// declaring a boundary there is what used to put it in the symbol table.
//
// It was not inert. Anything that attributes a PC to a symbol — a profile, a
// caller map, a `pcToFunction` lookup — reported code belonging to 0x0800D188 as
// belonging to a function that does not exist, and truncated the real function's
// extent at 0x692 bytes.
//
// The cfg entry is gone; this script is now the guard on it staying gone. Four
// observations:
//
//   1. Nothing can reach it. No `bl` anywhere in the ROM targets it, and no word
//      in ROM or RAM holds its address. Positive controls: the same scans do find
//      a `bl` to PlayerRespawnOrDeath and a stored pointer to VBlankCallback_Gameplay.
//   2. Control falls in. The instruction in front of it is `muls`, not a branch or
//      a return, so execution arrives from the instruction above.
//   3. Measured, that is exactly what happens: the address before it and the label
//      itself execute the same number of times, and every hit carries the `lr` of
//      the call to 0x0800D188 — nothing ever called the label, the enclosing
//      function was still running.
//
// Expected output: four PASS lines.
//
// The same defect class is reported wholesale by `scripts/find_dead_code.py`,
// under "mid-function splits".
import { mkdirSync, readFileSync } from 'node:fs';

import { HeadlessRuntime, REPO, SAVESTATE } from './gba-kit.mjs';

const OUT = '/tmp/klonoa-d81a';
mkdirSync(OUT, { recursive: true });

const LABEL = 0x0800d81a;
const BEFORE = LABEL - 2;

const rt = await HeadlessRuntime.create({
  romPath: `${REPO}/klonoa-eod.gba`,
  elfPath: `${REPO}/klonoa-eod.elf`,
  outputDir: OUT,
  loadSavePath: SAVESTATE,
  logFn: () => {},
});
const eng = rt.engine;
const di = eng.debugInfo;

// functions_merged.cfg no longer cuts here, so the symbol must be gone. This is
// the regression half: if it comes back, everything below says why it should not.
const sym = di.symbols.symbol('sub_0800D81A');
if (sym) throw new Error(`sub_0800D81A is back in the ELF at 0x${(sym.address >>> 0).toString(16)} — functions_merged.cfg is cutting mid-function again`);
const enclosing = di.pcToFunction(0x0800d188);
console.log(`  0x${LABEL.toString(16)} sits 0x${(LABEL - enclosing.address).toString(16)} bytes into ${enclosing.name} (0x${enclosing.address.toString(16)}..0x${enclosing.end.toString(16)})`);

// ── 1. nothing targets it ────────────────────────────────────────────────────
const rom = new Uint8Array(readFileSync(`${REPO}/klonoa-eod.gba`));
const BASE = 0x08000000;
const h16 = (o) => rom[o] | (rom[o + 1] << 8);

/** Every address a Thumb `bl` pair in the image would branch to. */
const blTargets = new Set();
for (let o = 0; o + 4 <= rom.length; o += 2) {
  const hi = h16(o);
  if ((hi & 0xf800) !== 0xf000) continue;
  const lo = h16(o + 2);
  if ((lo & 0xf800) !== 0xf800 && (lo & 0xf800) !== 0xe800) continue;
  let off = ((hi & 0x7ff) << 12) | ((lo & 0x7ff) << 1);
  if (off & 0x400000) off -= 0x800000;
  blTargets.add(((BASE + o + 4 + off) >>> 0) & ~1);
}
const callControl = di.symbolToAddress('PlayerRespawnOrDeath') >>> 0;
if (!blTargets.has(callControl & ~1)) throw new Error('positive control failed: no bl to PlayerRespawnOrDeath, so the bl decoder is wrong');
console.log(`  control  the bl scan found ${blTargets.size} distinct call targets, including PlayerRespawnOrDeath`);

const words = new Set();
for (let o = 0; o + 4 <= rom.length; o += 4) {
  const w = (rom[o] | (rom[o + 1] << 8) | (rom[o + 2] << 16) | (rom[o + 3] << 24)) >>> 0;
  if (w >= BASE && w < BASE + rom.length) words.add(w & ~1);
}
const ptrControl = di.symbolToAddress('VBlankCallback_Gameplay') >>> 0;
if (!words.has(ptrControl & ~1)) throw new Error('positive control failed: no stored pointer to VBlankCallback_Gameplay');
console.log(`  control  ${words.size} distinct code pointers in ROM, including VBlankCallback_Gameplay`);

const ram = [
  ...eng.searchMemory({ value: LABEL, size: 32, region: 'both' }),
  ...eng.searchMemory({ value: LABEL | 1, size: 32, region: 'both' }),
];
if (blTargets.has(LABEL) || words.has(LABEL) || ram.length > 0) {
  throw new Error(`something does reach 0x${LABEL.toString(16)}: bl=${blTargets.has(LABEL)} romPtr=${words.has(LABEL)} ram=${ram.length}`);
}
console.log('PASS  1. no bl targets it, no word in ROM or RAM holds its address — nothing can call it');

// ── 2. the instruction in front of it falls through ──────────────────────────
const prev = eng.disassemble(BEFORE, 1, 'thumb')[0];
if (/^(b|bx|blx|pop)\b/.test(prev.instruction)) {
  throw new Error(`0x${BEFORE.toString(16)} is \`${prev.instruction}\` — control does not fall through`);
}
console.log(`PASS  2. 0x${BEFORE.toString(16)} is \`${prev.instruction}\`, which falls through into it`);

// ── 3. and at run time, that is how it is reached ────────────────────────────
const wBefore = eng.watchExecution(BEFORE, { maxHits: 8 });
const wLabel = eng.watchExecution(LABEL, { maxHits: 8 });
await eng.pressSequence([
  ['right', 90],
  [null, 30],
  ['right+b', 40],
  [null, 60],
  ['left', 90],
  ['b', 10],
  [null, 120],
]);
wBefore.stop();
wLabel.stop();

if (wLabel.count === 0) throw new Error('the label never executed, so this run measures nothing');
if (wBefore.count !== wLabel.count) {
  throw new Error(`0x${BEFORE.toString(16)} ran ${wBefore.count} times and the label ${wLabel.count} — they are not one straight line`);
}
console.log(`PASS  3. both ran exactly ${wLabel.count} times — every execution of the label came from the instruction above it`);

/** The address the `bl` immediately before `ret` branched to, or null. */
const calleeOf = (ret) => {
  const o = ret - 4 - BASE;
  if (o < 0) return null;
  const hi = h16(o);
  const lo = h16(o + 2);
  if ((hi & 0xf800) !== 0xf000) return null;
  if ((lo & 0xf800) !== 0xf800 && (lo & 0xf800) !== 0xe800) return null;
  let off = ((hi & 0x7ff) << 12) | ((lo & 0x7ff) << 1);
  if (off & 0x400000) off -= 0x800000;
  return ((BASE + o + 4 + off) >>> 0) & ~1;
};
const lrs = [...new Set(wLabel.hits.map((h) => (h.lr >>> 0) & ~1))];
const stale = lrs.filter((lr) => calleeOf(lr) === (enclosing.address & ~1));
if (stale.length !== lrs.length) {
  throw new Error(`some hits carried an lr that is not the return of a call to ${enclosing.name}: ${lrs.map((a) => '0x' + a.toString(16)).join(', ')}`);
}
console.log(
  `PASS  4. every hit carried lr = ${lrs.map((a) => '0x' + a.toString(16)).join(', ')}, the return address of a \`bl ${enclosing.name}\` — nothing ever called the label`,
);
