// PROOF: byte 0x1C of the graphics control buffer (*(u32 *)0x030034A0) carries two
// scene-transition flags, and they are what `struct GfxControlFlags` in include/gfx.h
// calls `blendRampDown` (bit 5) and `forceWindowsOpen` (bit 6).
//
//   bit 5 — the DIRECTION of the fade ProcessSceneTransitionOut runs.
//           clear -> gBlendValue ramps UP  (0 -> 16) and the scene is torn down at 16
//           set   -> gBlendValue ramps DOWN and, on underflow, REG_DISPCNT bit 10
//                    (BG2 enable) is cleared and the bit clears ITSELF (one-shot).
//   bit 6 — while the scene is exiting, the gfx tick (sub_0804EB64) forces
//           REG_WININ = REG_WINOUT = 0x3F and re-pins gBlendValue to 15 every frame.
//
// This is also the evidence of record for the stream command at 0x0804E5C6, whose whole
// body is "flip bit 5, advance the stream by 2" — so if bit 5 were mis-labelled that
// command would be too. It was called StreamCmd_ToggleLayerFlag (it touches no layer);
// this script is why it is now StreamCmd_ToggleFadeDirection.
//
// METHOD (A/B intervention). Reach a skippable cutscene, put it into the exiting state
// with the two writes StreamCmd_BeginSceneExit performs, and then run the SAME scene
// three times, changing exactly one bit of byte 0x1C between runs:
//     run A  bit5=0 bit6=0   (control)
//     run B  bit5=1 bit6=0   (blendRampDown)
//     run C  bit5=0 bit6=1   (forceWindowsOpen)
// Everything else — ROM, savestate-free boot path, input, frame counts — is identical.
//
// Bit positions are read out of the ELF's DWARF (`GfxControlFlags.blendRampDown`), not
// hand-written, so renaming the field breaks this script instead of silently drifting.
import { HeadlessRuntime } from './gba-kit.mjs';
import { mkdirSync } from 'fs';
import { ROM, ELF, readField, writeField } from './_harness.mjs';

const OUT = '/tmp/klonoa-gfx-flag-1c';
mkdirSync(OUT, { recursive: true });

// Same boot path the other gfx-stream proofs use: reach a running, skippable scene.
const TO_SKIPPABLE_SCENE = [
  [null, 409], ['a', 5], [null, 20], ['a', 4], [null, 8], ['a', 4], [null, 8], ['a', 3], [null, 8],
  ['a', 4], [null, 51], ['a', 4], [null, 8], ['a', 3], [null, 89], ['a', 4], [null, 18],
];

const PTR = 0x030034a0; // gGfxBufferPtr / gLevelStatePtr — the same heap buffer
const DISPCNT = 0x04000000;
const WININ = 0x04000048;
const WINOUT = 0x0400004a;

const mk = async () => {
  const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
  return [rt.engine, rt.gba.bus, rt.engine.debugInfo];
};

let F5, F6, BLEND;
{
  const [, , di] = await mk();
  F5 = di.structMember('GfxControlFlags', 'blendRampDown');
  F6 = di.structMember('GfxControlFlags', 'forceWindowsOpen');
  BLEND = di.symbolToAddress('gBlendValue');
  if (!F5 || !F6) throw new Error('GfxControlFlags.blendRampDown / .forceWindowsOpen not in DWARF (renamed?)');
  console.log(
    `DWARF: GfxControlFlags.blendRampDown = byte +0x${F5.offset.toString(16)} bit ${F5.bitOffset}` +
      `   .forceWindowsOpen = byte +0x${F6.offset.toString(16)} bit ${F6.bitOffset}`,
  );
  console.log(`ldscript: gBlendValue = 0x${BLEND.toString(16)}\n`);
  if (F5.offset !== 0x1c || F5.bitOffset !== 5 || F6.offset !== 0x1c || F6.bitOffset !== 6) {
    throw new Error('the two flags are no longer byte 0x1C bits 5/6 — update this script');
  }
}

/**
 * Run one trial: reach the scene, begin the exit, set byte 0x1C to the requested
 * flag combination, then sample every frame.
 */
async function trial(label, { bit5, bit6 }) {
  const [eng, bus] = await mk();
  await eng.pressSequence(TO_SKIPPABLE_SCENE);
  const p = bus.read32(PTR) >>> 0;

  // exactly StreamCmd_BeginSceneExit's two writes: clear the render mode, request exit
  bus.write8(p, bus.read8(p) & ~3);
  bus.write8(p + 2, (bus.read8(p + 2) & ~6) | 2);

  // the single variable under test
  writeField(bus, p, F5, bit5);
  writeField(bus, p, F6, bit6);
  bus.write8(BLEND, 8); // start both directions from the middle so either can move

  const blend = [];
  let win = null;
  for (let f = 0; f < 24; f++) {
    await eng.pressSequence([[null, 1]]);
    blend.push(bus.read8(BLEND));
    if (win === null && bus.read16(WININ) === 0x3f && bus.read16(WINOUT) === 0x3f) win = f + 1;
  }
  // Read DISPCNT at the END: bit 10 flickers within a frame on every run (the HBlank
  // handler toggles it), so only the settled state distinguishes the trials.
  const bg2On = (bus.read16(DISPCNT) & 0x400) !== 0;
  const after5 = readField(bus, bus.read32(PTR) >>> 0 || p, F5);
  console.log(`${label}`);
  console.log(`   gBlendValue per frame : ${blend.join(' ')}`);
  console.log(
    `   REG_WININ=0x${bus.read16(WININ).toString(16)} REG_WINOUT=0x${bus.read16(WINOUT).toString(16)}` +
      `   REG_DISPCNT=0x${bus.read16(DISPCNT).toString(16)} (BG2 enable = ${bg2On ? 1 : 0})`,
  );
  console.log(`   first frame WININ=WINOUT=0x3F: ${win ?? 'never'}   bit5 afterwards: ${after5}`);
  await eng.takeScreenshot({ path: `${OUT}/${label.split(' ')[0]}.png` });
  return { blend, bg2On, win, after5 };
}

console.log('=== A. CONTROL: bit5 = 0, bit6 = 0 ===');
const A = await trial('A-control', { bit5: 0, bit6: 0 });
console.log('\n=== B. bit5 = 1 — the ONLY difference from A ===');
const B = await trial('B-bit5', { bit5: 1, bit6: 0 });
console.log('\n=== C. bit6 = 1 — the ONLY difference from A ===');
const C = await trial('C-bit6', { bit5: 0, bit6: 1 });

// ---------------------------------------------------------------- verdicts
const rose = (s) => s[s.length - 1] > s[0];
// "ramped down" = it went below where it started before the routine re-pinned it
const fell = (s) => Math.min(...s) < s[0];

console.log('\n=== VERDICT ===');
const v1 = rose(A.blend) && fell(B.blend);
console.log(
  `bit5=0 ramps gBlendValue UP (${A.blend[0]} -> ${A.blend[A.blend.length - 1]}) while bit5=1 ramps it DOWN ` +
    `(min ${Math.min(...B.blend)}): ${v1 ? 'CONFIRMED' : 'REFUTED'}  -> the bit is a fade DIRECTION, i.e. blendRampDown`,
);
const v2 = !B.bg2On && A.bg2On;
console.log(
  `bit5=1 leaves REG_DISPCNT bit 10 (BG2) OFF after the ramp underflows (${B.bg2On ? 1 : 0}), ` +
    `control leaves it ON (${A.bg2On ? 1 : 0}): ${v2 ? 'CONFIRMED' : 'REFUTED'}`,
);
const v3 = B.after5 === 0 && B.blend.includes(255);
console.log(
  `bit5 underflows (gBlendValue wraps to 255) and then clears ITSELF — one-shot: ${v3 ? 'CONFIRMED' : 'REFUTED'}`,
);
const v4 = C.win !== null && A.win === null;
console.log(
  `bit6=1 forces REG_WININ = REG_WINOUT = 0x3F, control does not: ${v4 ? 'CONFIRMED' : 'REFUTED'}` +
    `  -> forceWindowsOpen`,
);
const v5 = C.blend[0] === 15 && A.blend[0] !== 15;
console.log(
  `bit6=1 also re-pins gBlendValue to 15 every frame (${C.blend[0]} vs control ${A.blend[0]}): ` +
    `${v5 ? 'CONFIRMED' : 'REFUTED'}`,
);
if (!(v1 && v2 && v3 && v4 && v5)) throw new Error('byte 0x1C bit 5 / bit 6 semantics did not reproduce');
