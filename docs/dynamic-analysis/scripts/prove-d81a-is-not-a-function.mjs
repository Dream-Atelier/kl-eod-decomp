// REFUTATION: `sub_0800D81A` was not a function. It is a label inside
// UpdatePlayerState at 0x0800D188, and a functions_merged.cfg entry
// declaring a boundary there is what used to put it in the symbol table.
//
// It was not inert. Anything that attributes a PC to a symbol — a profile, a
// caller map, a `pcToFunction` lookup — reported code belonging to 0x0800D188 as
// belonging to a function that does not exist, and truncated the real function's
// extent at 0x692 bytes.
//
// The cfg entry is gone; this script is now the guard on it staying gone. Five
// observations, the last of which is the one that generalises: 27 more cfg entries
// are labels in the same function, and this PR does not touch them.
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
// Expected output: five PASS lines.
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

const ENCLOSING = di.symbolToAddress('UpdatePlayerState') >>> 0;
if (ENCLOSING !== 0x0800d188) throw new Error(`UpdatePlayerState moved to 0x${ENCLOSING.toString(16)}`);

// ── 0. how big is the function it sits in? ───────────────────────────────────
// NOT `pcToFunction`: that stops at the next symbol, and the next symbol is the
// next cfg entry, which is itself a label inside this function. Scan for the
// first instruction that can return instead.
const rom = new Uint8Array(readFileSync(`${REPO}/klonoa-eod.gba`));
const BASE = 0x08000000;
const h16 = (o) => rom[o] | (rom[o + 1] << 8);

// The first instruction that can return, scanning forward from the entry. Every
// address between the two is code with no way out, so it is all one function.
let RET = null;
for (let a = ENCLOSING; a < ENCLOSING + 0x20000 && RET === null; a += 2) {
  const i = h16(a - BASE);
  if ((i & 0xff00) === 0xbd00 || (i & 0xff87) === 0x4700) RET = a;
}
if (RET === null) throw new Error('no return instruction found within 128 KB of the entry');
const END = RET + 2;
const cfgInside = di.symbols.symbols.filter((x) => x.type === 2 && ((x.address >>> 0) & ~1) > ENCLOSING && ((x.address >>> 0) & ~1) < END).length;
console.log(`  0x${ENCLOSING.toString(16)}: no return instruction until 0x${RET.toString(16)}, so the function is ${END - ENCLOSING} bytes`);
console.log(`  ${cfgInside} other symbols land strictly inside it; pcToFunction stops at the first, 0x${(di.pcToFunction(ENCLOSING).end >>> 0).toString(16)}`);
if (LABEL <= ENCLOSING || LABEL >= END) throw new Error('0x0800D81A is not inside the function after all');

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
const wLabel = eng.watchExecution(LABEL, { maxHits: 500 });
const wEntry = eng.watchExecution(ENCLOSING, { maxHits: 1 });
const wRet = eng.watchExecution(RET, { maxHits: 1 });
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
wEntry.stop();
wRet.stop();

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
const sampled = wLabel.hits.length;
const lrs = [...new Set(wLabel.hits.map((h) => (h.lr >>> 0) & ~1))];
const stale = lrs.filter((lr) => calleeOf(lr) === ENCLOSING);
if (stale.length !== lrs.length) {
  throw new Error(`some hits carried an lr that is not the return of a call to ${enclosing.name}: ${lrs.map((a) => '0x' + a.toString(16)).join(', ')}`);
}
console.log(
  `PASS  4. all ${sampled} sampled hits carried lr = ${lrs.map((a) => '0x' + a.toString(16)).join(', ')}, the return address of a \`bl\` to the function — nothing ever called the label`,
);

// ── 5. the whole span really is one function ─────────────────────────────────
if (wEntry.count === 0) throw new Error('the function never ran, so this says nothing');
if (wEntry.count !== wRet.count) {
  throw new Error(`the entry ran ${wEntry.count} times and the return at 0x${RET.toString(16)} ran ${wRet.count} — they are not one function`);
}
console.log(
  `PASS  5. the entry and that single return each ran exactly ${wEntry.count} times — one entry, one exit, ${END - ENCLOSING} bytes, and ${cfgInside} symbols (27 functions_merged.cfg entries, some with two names) inside it`,
);
