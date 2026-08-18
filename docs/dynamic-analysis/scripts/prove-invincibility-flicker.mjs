// PROOF: gUnk_03005220 +0x3E is the post-hit invincibility timer, and the
// flicker it drives is written by the player damage/death step at 0x0800D188 —
// the same function whose other branch runs the death sequence.
//
// Method: walk into the Moo once (hearts 3 -> 2, no death) and sample every
// frame. The observations:
//
//   * +0x3E is 0 in ordinary play, jumps above 0x80 on the hit frame and
//     decrements by exactly 1 per frame back to 0.
//   * While it counts, gUnk_03002920[0].onScreen toggles; in an equally long
//     window before the hit it never changes at all (the positive control that
//     makes "it flickers" a measurement). The flicker runs at two rates: fast
//     while the timer is above 0x43, then four times slower below it.
//   * Every write to that flag in the window comes from an instruction inside
//     0x0800D188.
//
// Expected output: four PASS lines and the flicker trace.
import { mkdirSync } from 'node:fs';

import { bootNextToMoo, HEARTS } from './_death.mjs';

const OUT = '/tmp/klonoa-invincibility-flicker';
mkdirSync(OUT, { recursive: true });

const rt = await bootNextToMoo(OUT);
const eng = rt.engine;
const di = eng.debugInfo;
const bus = rt.gba.bus;

const DAMAGE_STEP = di.pcToFunction(di.symbolToAddress('UpdatePlayerDamageState') >>> 0);
const TIMER = 'gUnk_03005220.invincibilityTimer';

const sample = () => ({
  timer: eng.readVariable(TIMER),
  onScreen: eng.readVariable('gUnk_03002920[0].onScreen'),
  hearts: bus.read8(HEARTS),
});

// ── control: an untouched player does not flicker ────────────────────────────
const before = [];
eng.onFrame(() => before.push(sample()));
await eng.wait({ frames: 60 });
eng.onFrame(null);

const flips = (xs) => xs.filter((s, i) => i > 0 && s.onScreen !== xs[i - 1].onScreen).length;
if (before.some((s) => s.timer !== 0)) throw new Error(`${TIMER} was already set before the hit`);
if (flips(before) !== 0) throw new Error(`the player already flickered ${flips(before)} times before being hit`);
console.log(`PASS  1. control: ${before.length} frames untouched, ${TIMER} = 0, onScreen never changed`);

// ── the hit ──────────────────────────────────────────────────────────────────
const heartsBefore = before[before.length - 1].hearts;
const flagAddr = di.resolveVariable('gUnk_03002920[0].onScreen').address >>> 0;
const w = eng.watchMemory({ address: flagAddr, length: 1, maxHits: 4000 });

const after = [];
eng.onFrame(() => after.push(sample()));
let walks = 0;
while (bus.read8(HEARTS) === heartsBefore) {
  if (++walks > 8) throw new Error('walked into the Moo 8 times without losing a heart — the savestate or the route moved');
  await eng.pressSequence([
    ['right', 25],
    [null, 20],
  ]);
}
await eng.wait({ frames: 150 });
eng.onFrame(null);
w.stop();

const heartsAfter = after[after.length - 1].hearts;
if (heartsAfter !== heartsBefore - 1) throw new Error(`hearts went ${heartsBefore} -> ${heartsAfter}, expected exactly one hit`);

const armed = after.filter((s) => s.timer !== 0);
const peak = Math.max(...after.map((s) => s.timer));
if (peak < 0x70) throw new Error(`${TIMER} only reached 0x${peak.toString(16)} after the hit`);
console.log(`PASS  2. hearts ${heartsBefore} -> ${heartsAfter}; ${TIMER} armed to 0x${peak.toString(16)} and ran for ${armed.length} frames`);

const timers = after.map((s) => s.timer);
const start = timers.indexOf(peak);
const end = timers.indexOf(0, start);
for (let i = start + 1; i < end; i++) {
  if (timers[i] !== timers[i - 1] - 1) {
    throw new Error(`${TIMER} went 0x${timers[i - 1].toString(16)} -> 0x${timers[i].toString(16)} at frame ${i}, not a per-frame countdown`);
  }
}
console.log(`PASS  3. it decremented by exactly 1 on each of the ${end - start} frames from 0x${peak.toString(16)} to 0`);

const flickers = flips(after.slice(start, end));
if (flickers < 10) throw new Error(`onScreen only changed ${flickers} times while the timer ran`);

const writers = new Set(
  w.hits.map((h) => {
    const a = h.instructionAddress >>> 0;
    return a >= DAMAGE_STEP.address && a < DAMAGE_STEP.end ? 'damage step' : '0x' + a.toString(16);
  }),
);
if (writers.size !== 1 || !writers.has('damage step')) {
  throw new Error(`onScreen was written from outside 0x0800D188: ${[...writers].join(', ')}`);
}
console.log(`PASS  4. onScreen flipped ${flickers} times while the timer ran, every write from inside 0x0800D188`);

const trace = after.slice(start, end).map((s) => (s.onScreen ? '#' : '.')).join('');
console.log(`\n  onScreen while invincible (# = shown): ${trace}`);
