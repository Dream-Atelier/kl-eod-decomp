// PROOF: what UpdateSceneTransition (0x080487B4) actually drives, and what the byte at
// 0x03005498 that it ramps actually is.
//
// The byte has THREE names in this tree for ONE address:
//     include/structs/variables.h  #define gUnk_03005498 (*(u8 *)0x03005498)
//     include/game.h               #define gFrameCounter (*(u8 *)0x03005498)   <- used by
//                                                                   FadeOutController
//     src/code_1.c / src/code_3.c  extern u8 gBlendValue;                       /* 0x03005498 */
// "frame counter" and "blend level" cannot both be right, and this function's docstring
// is built on the answer: it ramps the byte 16 -> 0 and then 0 -> 15.
//
// METHOD — causal intervention with a control, on the boot run, where the whole sequence
// plays out with no input at all:
//   AgbMain installs [ReadKeyInput, FadeOutController, VBlankCallback_Gameplay];
//   FadeOutController fades out and swaps ITSELF for UpdateSceneTransition (slot 1);
//   UpdateSceneTransition runs its 0x140-frame timeline and swaps slots 1 and 2 for
//   [LoadBGPalette, VBlankCallback_TitleScreen].
//   Run 1: untouched (control) — record counter, byte, REG_BLDY, REG_BLDALPHA, REG_BLDCNT
//          and mean screen luminance every frame, and screenshot the presented picture.
//   Run 2: identical, except the byte is forced to 15 every frame during the HOLD window
//          (counter 0x40..0xF0), where the control leaves it at 0.
//   Run 3: identical, except the byte is forced to 0 every frame during the FADE-OUT
//          window (counter 0x101..0x13F), where the control ramps it 0 -> 15.
//   If the byte is a blend level, run 2 darkens a picture the control shows bright and
//   run 3 keeps bright a picture the control fades to black; if it were a frame counter,
//   neither would change what is on screen.
//
// Register labels are cross-checked against include/io_reg.h:
//     REG_BLDCNT 0x04000050, REG_BLDALPHA 0x04000052, REG_BLDY 0x04000054,
//     REG_DISPSTAT 0x04000004, REG_IE 0x04000200.
// Addresses come from the symbol table and struct offsets from DWARF.
import { HeadlessRuntime } from './gba-kit.mjs';
import { mkdirSync, readFileSync } from 'fs';
import { ROM, ELF, readField } from './_harness.mjs';

const OUT = '/tmp/klonoa-scene-transition';
mkdirSync(OUT, { recursive: true });
const hex = (v, n = 8) => '0x' + (v >>> 0).toString(16).padStart(n, '0');

const REG_DISPSTAT = 0x04000004,
  REG_BLDCNT = 0x04000050,
  REG_BLDALPHA = 0x04000052,
  REG_BLDY = 0x04000054,
  REG_IE = 0x04000200;

const mk = async () => {
  const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
  return [rt.engine, rt.gba.bus, rt.engine.debugInfo];
};
let [eng, bus, di] = await mk();
const need = (n) => {
  const a = di.symbolToAddress(n);
  if (a == null) throw new Error(`symbol ${n} unresolved — renamed?`);
  return a >>> 0;
};

const FN = need('UpdateSceneTransition');
const A_BYTE = need('gBlendValue'); // 0x03005498 — the cell under test
const A_SCENE = need('gUnk_03004C20');
const A_QUEUE = need('gCallbackQueue');
const A_STATE = need('gCallbackStateArray');
const A_SONGTAB = need('gSongTable');
const F_COUNTER = di.structMember('Unk_03004C20', 'sceneFrameCounter');
if (!F_COUNTER) throw new Error('Unk_03004C20.sceneFrameCounter not in DWARF');
const A_COUNTER = A_SCENE + F_COUNTER.offset;

console.log('UpdateSceneTransition', hex(FN), ' the byte under test', hex(A_BYTE));
console.log(
  `gCallbackStateArray ${hex(A_STATE)} and gCallbackQueue ${hex(A_QUEUE)} are the SAME address: ${A_STATE === A_QUEUE}` +
    ` -> gCallbackStateArray[1]/[2] are gCallbackQueue.current[1]/[2] (current[] is at +0x00, next[] at +0x28)`,
);
console.log(`sceneFrameCounter = gUnk_03004C20 + 0x${F_COUNTER.offset.toString(16)} (${F_COUNTER.size} bytes) = ${hex(A_COUNTER)}`);

// ---- who installs this function? ROM literal pools, not guesswork -----------
{
  const rom = readFileSync(ROM);
  const want = (FN | 1) >>> 0;
  const sites = [];
  for (let o = 0; o + 4 <= rom.length; o += 4) if (rom.readUInt32LE(o) === want) sites.push(0x08000000 + o);
  console.log(`\nliteral-pool references to ${hex(want)} (every installer): ${sites.length}`);
  for (const s of sites) {
    const sym = di.addressToSymbol(s);
    console.log(`   ${hex(s)}  in ${sym ? `${sym.name}+0x${sym.offset.toString(16)}` : '?'}`);
  }
}

// ---- one boot run, instrumented ---------------------------------------------
const lum = () => {
  const px = eng.getScreenRegion(0, 0, 240, 160);
  let s = 0;
  for (let i = 0; i < px.length; i += 4) s += px[i] + px[i + 1] + px[i + 2];
  return +(s / (240 * 160 * 3)).toFixed(1);
};

async function run(label, { force = null, shots = [] } = {}) {
  [eng, bus, di] = await mk();
  const wQueue = eng.watchMemory({
    address: A_QUEUE,
    length: 0x2c,
    maxHits: 200,
    filter: (h) => di.pcToFunction(h.instructionAddress)?.name === 'UpdateSceneTransition',
  });
  const wIe = eng.watchMemory({ address: REG_IE, length: 2, maxHits: 200, filter: (h) => di.pcToFunction(h.instructionAddress)?.name === 'UpdateSceneTransition' });
  const wStat = eng.watchMemory({ address: REG_DISPSTAT, length: 2, maxHits: 200, filter: (h) => di.pcToFunction(h.instructionAddress)?.name === 'UpdateSceneTransition' });
  const wByte = eng.watchMemory({ address: A_BYTE, length: 1, maxHits: 4000 });
  const wCounter = eng.watchMemory({ address: A_COUNTER, length: 4, maxHits: 4000 });
  const wBldy = eng.watchMemory({ address: REG_BLDY, length: 2, maxHits: 4000 });
  // the song: gSongTable[0x21].header written into any m4a player state
  const hdr21 = bus.read32(A_SONGTAB + 0x21 * 8) >>> 0;
  const slot21 = bus.read16(A_SONGTAB + 0x21 * 8 + 4);
  const wSong = eng.watchMemory({ address: 0x030065e0, length: 0x120, maxHits: 4000, filter: (h) => (h.value >>> 0) === hdr21 });

  // Frame-by-frame, so that a row is only recorded while UpdateSceneTransition is the
  // installed slot-1 callback: the same counter values recur in later scenes.
  const rows = [];
  const events = [];
  const pending = new Map(shots);
  // The counter value AT the frame each one-shot event fires — "the music starts
  // exactly 32 frames in" is a claim about the counter, not about a hit existing.
  const firedAt = {};
  for (let frame = 1; frame <= 900; frame++) {
    await eng.wait({ frames: 1 });
    const running = (bus.read32(A_QUEUE + 4) >>> 0) === ((FN | 1) >>> 0);
    const c = bus.read32(A_COUNTER) >>> 0;
    if (!running) continue;
    if (force && c >= force.from && c <= force.to) bus.write8(A_BYTE, force.value);
    const row = { f: frame, c, b: bus.read8(A_BYTE), bldy: eng.read16(REG_BLDY), bldalpha: eng.read16(REG_BLDALPHA), bldcnt: eng.read16(REG_BLDCNT), l: lum() };
    rows.push(row);
    for (const [k, w] of [['REG_IE', wIe], ['REG_DISPSTAT', wStat], ['song 0x21', wSong], ['callback swap', wQueue]])
      if (w.hits.length && firedAt[k] === undefined) firedAt[k] = c;
    for (const [name, want] of pending) {
      if (c === want) {
        await eng.takeScreenshot({ name: `${label}-${name}` });
        events.push(`${name}: counter ${hex(want, 3)} frame ${frame} byte ${row.b} REG_BLDY ${row.bldy} luminance ${row.l}`);
        pending.delete(name);
      }
    }
  }
  if (pending.size) throw new Error(`never reached counter(s) ${[...pending.keys()].join(',')} with UpdateSceneTransition installed`);

  const at = (c) => rows.find((r) => r.c === c) ?? {};
  console.log(`\n=== ${label} ===`);
  for (const e of events) console.log('   ' + e);
  console.log(
    '   counter :  ' + [0x10, 0x11, 0x18, 0x20, 0x2f, 0x30, 0x80, 0x100, 0x101, 0x120, 0x13f].map((c) => hex(c, 3)).join(' '),
  );
  console.log('   byte    :  ' + [0x10, 0x11, 0x18, 0x20, 0x2f, 0x30, 0x80, 0x100, 0x101, 0x120, 0x13f].map((c) => String(at(c).b ?? '?').padStart(5)).join(' '));
  console.log('   REG_BLDY:  ' + [0x10, 0x11, 0x18, 0x20, 0x2f, 0x30, 0x80, 0x100, 0x101, 0x120, 0x13f].map((c) => String(at(c).bldy ?? '?').padStart(5)).join(' '));
  console.log('   luminance: ' + [0x10, 0x11, 0x18, 0x20, 0x2f, 0x30, 0x80, 0x100, 0x101, 0x120, 0x13f].map((c) => String(at(c).l ?? '?').padStart(5)).join(' '));

  console.log('   counter at which each one-shot event fired: ' + Object.entries(firedAt).map(([k, v]) => `${k} @ ${hex(v, 3)}`).join(', '));
  const ieHit = wIe.hits[0],
    statHit = wStat.hits[0],
    songHit = wSong.hits[0];
  console.log(`   REG_IE written by UpdateSceneTransition: ${ieHit ? `${hex(ieHit.value, 4)} (bit0 set: ${(ieHit.value & 1) !== 0})` : 'never'}`);
  console.log(`   REG_DISPSTAT written by UpdateSceneTransition: ${statHit ? `${hex(statHit.value, 4)} (bit3 set: ${(statHit.value & 8) !== 0})` : 'never'}`);
  console.log(`   song 0x21 header ${hex(hdr21)} (player slot ${slot21}) written into m4a state: ${songHit ? 'yes' : 'no'}`);
  console.log(
    '   callback slots written by UpdateSceneTransition: ' +
      (wQueue.hits.map((h) => `+0x${(h.address - A_QUEUE).toString(16)} = ${di.addressToSymbol((h.value & ~1) >>> 0)?.name ?? hex(h.value)}`).join(', ') || 'none'),
  );
  const byteWriters = [...new Set(wByte.hits.map((h) => di.pcToFunction(h.instructionAddress)?.name ?? hex(h.instructionAddress)))];
  const bldyWriters = [...new Set(wBldy.hits.map((h) => di.pcToFunction(h.instructionAddress)?.name ?? hex(h.instructionAddress)))];
  console.log(`   writers of ${hex(A_BYTE)}: ${byteWriters.join(', ')}`);
  console.log(`   writers of REG_BLDY: ${bldyWriters.join(', ')}`);
  const counterWriters = new Map();
  for (const h of wCounter.hits) {
    const k = `${di.pcToFunction(h.instructionAddress)?.name ?? hex(h.instructionAddress)}${h.value === 0xffffffff ? ' (stores -1)' : ''}`;
    counterWriters.set(k, (counterWriters.get(k) ?? 0) + 1);
  }
  console.log(`   writers of the scene frame counter: ${[...counterWriters].map(([k, v]) => `${k} x${v}`).join(', ')}`);
  return { rows, at, wQueue, ieHit, statHit, songHit, firedAt };
}

const SHOTS = [
  ['blackout', 0x08],
  ['fade-in-mid', 0x1d],
  ['hold', 0x59],
  ['fade-out-mid', 0x13a],
];
const A = await run('A-control', { shots: SHOTS });
const B = await run('B-force-15-during-hold', { force: { from: 0x40, to: 0xf0, value: 15 }, shots: [['hold', 0x59]] });
const C = await run('C-force-0-during-fadeout', { force: { from: 0x101, to: 0x13f, value: 0 }, shots: [['fade-out-mid', 0x13a]] });

// after the handoff: what does the screen become?
console.log('\n--- after the sequence hands over (control run continued) ---');
[eng, bus, di] = await mk();
await eng.wait({ frames: 520 });
console.log(`   frame 520: counter ${hex(bus.read32(A_COUNTER))}, callbacks now ` +
  [0, 1, 2].map((i) => di.addressToSymbol((bus.read32(A_QUEUE + i * 4) & ~1) >>> 0)?.name ?? '?').join(', '));
await eng.takeScreenshot({ name: 'D-after-handoff' });

// ---- verdicts ---------------------------------------------------------------
console.log('\n=== VERDICT ===');
const ctlHold = A.at(0x59),
  frcHold = B.at(0x59);
const v1 = ctlHold.bldy === 0 && frcHold.bldy === 15 && frcHold.l < ctlHold.l * 0.6;
console.log(
  `0x03005498 is a BLDY blend level, not a frame counter: holding it at 15 through the hold window puts 15 in REG_BLDY ` +
    `(control ${ctlHold.bldy}) and drops screen luminance ${ctlHold.l} -> ${frcHold.l}: ${v1 ? 'CONFIRMED' : 'REFUTED'}`,
);
const ctlOut = A.at(0x13a),
  frcOut = C.at(0x13a);
const v2 = ctlOut.bldy > 10 && frcOut.bldy === 0 && frcOut.l > ctlOut.l * 1.5;
console.log(
  `the 0x101..0x13F ramp is a FADE-OUT performed by this byte: forcing it to 0 there keeps REG_BLDY at ${frcOut.bldy} ` +
    `(control ${ctlOut.bldy}) and the picture bright (${frcOut.l} vs control ${ctlOut.l}): ${v2 ? 'CONFIRMED' : 'REFUTED'}`,
);
const rampIn = [0x11, 0x18, 0x2f].map((c) => A.at(c).b);
const v3 = rampIn[0] === 15 && rampIn[2] === 0 && A.at(0x10).b === 16;
console.log(`the 0x11..0x2F ramp is a FADE-IN: byte ${A.at(0x10).b} (before) -> ${rampIn.join(' -> ')} at counters 0x11/0x18/0x2F: ${v3 ? 'CONFIRMED' : 'REFUTED'}`);
const v4 =
  !!A.ieHit && (A.ieHit.value & 1) !== 0 && !!A.statHit && (A.statHit.value & 8) !== 0 && !!A.songHit &&
  A.firedAt['REG_IE'] === 0x20 && A.firedAt['REG_DISPSTAT'] === 0x20 && A.firedAt['song 0x21'] === 0x20;
console.log(
  `at counter 0x20 (and only there) it re-arms the VBlank IRQ (REG_IE bit 0 + REG_DISPSTAT bit 3) and starts song 0x21 — ` +
    `fired at counters ${hex(A.firedAt['REG_IE'], 3)}/${hex(A.firedAt['REG_DISPSTAT'], 3)}/${hex(A.firedAt['song 0x21'], 3)}: ${v4 ? 'CONFIRMED' : 'REFUTED'}`,
);
console.log(
  `the callback swap is sampled at counter ${hex(A.firedAt['callback swap'], 3)}: it fires on the frame the counter passes 0x13F, ` +
    `but by the time this loop samples, the same call has already stored -1 and the VBlank increment has turned that into 0`,
);
const slots = A.wQueue.hits.map((h) => h.address - A_QUEUE);
const v5 = slots.includes(4) && slots.includes(8) && !slots.some((s) => s >= 0x28);
console.log(`the handoff writes gCallbackQueue.current[1] and current[2] (offsets +0x4/+0x8), NOT next[] (+0x28..): ${v5 ? 'CONFIRMED' : 'REFUTED'}`);
if (!(v1 && v2 && v3 && v4 && v5)) throw new Error('a prediction did not reproduce');
console.log('\nscreenshots in', OUT);
