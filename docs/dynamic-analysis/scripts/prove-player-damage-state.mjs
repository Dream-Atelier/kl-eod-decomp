// PROOF: 0x0800D188 is the player's per-frame damage/death step, not a VBlank
// callback and nothing to do with the credits. It was named
// `VBlankCallback_Credits`; this is what it actually is.
//
// Three observations, each of which the old name has to survive and does not:
//
//  1. NOT A CALLBACK. Its address never appears as a word in ROM or in RAM, so
//     nothing can ever install it in the callback queue or an interrupt vector.
//     Positive control: the same scan finds VBlankCallback_Gameplay, a real one,
//     sitting in RAM — so "found nowhere" is a measurement and not a broken scan.
//     Both of its callers reach it with a plain `bl`, from inside a per-frame
//     main-loop function.
//
//  2. IT IS GATED ON THE DEATH TIMER. The first thing it does is read
//     gUnk_03005220.deathSequenceTimer and return when it is 0. Across ordinary
//     play the entry runs every frame and the body runs zero times.
//
//  3. THE BODY IS THE DEATH SEQUENCE. Walk into the Moo until the hearts empty:
//     the body now runs, gBlendValue jumps to 0x10 and ramps back to 0 (the
//     screen fade), and gUnk_03005220.lives drops by one.
//
// Expected output: five PASS lines, then the observed ramp.
import { mkdirSync, readFileSync } from 'node:fs';

import { REPO } from './gba-kit.mjs';
import { bootNextToMoo, walkIntoMooUntilDead } from './_death.mjs';

const OUT = '/tmp/klonoa-player-damage-state';
mkdirSync(OUT, { recursive: true });

const rt = await bootNextToMoo(OUT);
const eng = rt.engine;
const di = eng.debugInfo;

const ENTRY = di.symbolToAddress('UpdatePlayerDamageState') >>> 0;
// The `bne` past the deathSequenceTimer==0 early-out. Asserted rather than
// assumed, so a shifted function fails here instead of measuring the wrong code.
const BODY = ENTRY + 0x26;
if (ENTRY !== 0x0800d188) throw new Error(`expected the function at 0x0800D188, ELF says 0x${ENTRY.toString(16)}`);

// ── 1. it is never anybody's function pointer ────────────────────────────────
const rom = new Uint8Array(readFileSync(`${REPO}/klonoa-eod.gba`));
const romHolds = (target) => {
  for (let o = 0; o + 4 <= rom.length; o += 4) {
    const w = (rom[o] | (rom[o + 1] << 8) | (rom[o + 2] << 16) | (rom[o + 3] << 24)) >>> 0;
    if (w === target || w === (target | 1) >>> 0) return o;
  }
  return null;
};
const ramHolds = (target) => [
  ...eng.searchMemory({ value: target, size: 32, region: 'both' }),
  ...eng.searchMemory({ value: (target | 1) >>> 0, size: 32, region: 'both' }),
];

const control = di.symbolToAddress('VBlankCallback_Gameplay') >>> 0;
const controlRam = ramHolds(control);
if (controlRam.length === 0) {
  throw new Error('positive control failed: VBlankCallback_Gameplay is a real callback but was found nowhere in RAM');
}
console.log(`  control  VBlankCallback_Gameplay is installed at ${controlRam.map((a) => '0x' + a.toString(16)).join(', ')}`);

const inRom = romHolds(ENTRY);
const inRam = ramHolds(ENTRY);
if (inRom !== null || inRam.length > 0) {
  throw new Error(`0x${ENTRY.toString(16)} IS stored as a pointer (rom offset ${inRom}, ram ${inRam})`);
}
console.log('PASS  1. no word in ROM or RAM holds its address — it is never installed as a callback');

// ── 2. gated on the death timer ──────────────────────────────────────────────
const wEntry = eng.watchExecution(ENTRY, { maxHits: 4 });
const wBody = eng.watchExecution(BODY, { maxHits: 1 });
await eng.wait({ frames: 120 });
const aliveEntries = wEntry.count;
const aliveBody = wBody.count;

const callers = [
  ...new Set(
    wEntry.hits.map((h) => {
      const s = di.addressToSymbol((h.lr >>> 0) & ~1);
      return s && s.exact ? `${s.name}+0x${s.offset.toString(16)}` : '0x' + (((h.lr >>> 0) & ~1) >>> 0).toString(16);
    }),
  ),
];
if (aliveEntries === 0) throw new Error('the function never ran at all — wrong savestate?');
if (aliveBody !== 0) throw new Error(`the body ran ${aliveBody} times while the player was alive`);
console.log(`PASS  2. alive: ${aliveEntries} entries in 120 frames, ${aliveBody} body runs. Called from ${callers.join(', ')}`);

// ── 3. the body is the death sequence ────────────────────────────────────────
const livesBefore = eng.readVariable('gUnk_03005220.lives');

// Sample from before the death, so the very first frame of the sequence is in
// the trace: the blend peak is set on the frame the timer arms and is gone a
// dozen frames later.
const blend = [];
const timer = [];
eng.onFrame(() => {
  blend.push(eng.readVariable('gBlendValue'));
  timer.push(eng.readVariable('gUnk_03005220.deathSequenceTimer'));
});
const bodyBefore = wBody.count;
const entryBefore = wEntry.count;
const walks = await walkIntoMooUntilDead(eng);
await eng.wait({ frames: 150 });
eng.onFrame(null);
wEntry.stop();
wBody.stop();
console.log(`  died after ${walks} walks into the Moo`);

const deadBody = wBody.count - bodyBefore;
const deadEntry = wEntry.count - entryBefore;
const armedFrames = timer.filter((t) => t !== 0).length;
if (deadBody === 0) throw new Error('the death sequence started but the body never ran');
// Every body run is a frame with the timer armed; the reverse does not hold,
// because the death sequence swaps the callback queue and the caller itself
// stops being scheduled part-way through.
if (deadBody > armedFrames) {
  throw new Error(`body ran ${deadBody} times but the timer was only armed on ${armedFrames} frames`);
}
console.log(`PASS  3. dead: ${deadEntry} entries, ${deadBody} body runs, ${armedFrames} frames with the timer armed`);

const peak = Math.max(...blend);
const peakAt = blend.indexOf(peak);
if (peak !== 0x10 || !blend.slice(peakAt).includes(0)) {
  throw new Error(`gBlendValue did not ramp 0x10 -> 0: peak 0x${peak.toString(16)} at frame ${peakAt}, trace ${blend.join(',')}`);
}
console.log(`PASS  4. gBlendValue jumped to 0x10 on the death frame and ramped back to 0 (the screen fade)`);

const livesAfter = eng.readVariable('gUnk_03005220.lives');
if (livesAfter !== livesBefore - 1) throw new Error(`lives went ${livesBefore} -> ${livesAfter}, expected one less`);
console.log(`PASS  5. lives ${livesBefore} -> ${livesAfter}`);

const dedupe = (xs) => xs.filter((v, i) => v !== xs[i - 1]);
console.log('\n  deathSequenceTimer:', dedupe(timer).map((v) => '0x' + v.toString(16)).join(' '));
console.log('  gBlendValue       :', dedupe(blend).join(' '));
