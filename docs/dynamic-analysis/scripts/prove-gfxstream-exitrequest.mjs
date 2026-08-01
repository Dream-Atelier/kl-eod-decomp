// PROOF: the 2-bit field at byte 2 (bits 1..2) of the GFX-stream control buffer
// (*(u32 *)0x030034A0, `gGfxBufferPtr`) is the cutscene *exit* control, and that
// the stream command at 0x0804E3D4 -- previously guessed to be
// `StreamCmd_SetBGModeTiled` -- is the "end this scene" command, NOT a BG-mode
// setter. Renamed to `StreamCmd_BeginSceneExit`; the two bits are now
// GFX_SCENE_EXITING / GFX_SCENE_SKIPPABLE in include/gfx.h.
//
// The function's whole body is:
//     *(s8 *)gGfxBufferPtr &= ~3;                              // clear render mode
//     gStreamPtr += 2;
//     ((s8 *)gGfxBufferPtr)[2] = (((s8 *)gGfxBufferPtr)[2] & ~6) | 2;   // field = 1
//
// Static context (asm):
//   * sub_0804EB64 (the gfx-stream tick) reads the same field:
//       field & 1  -> tear down windows, gUnk_03005498 = 0x0F, ProcessSceneTransitionOut()
//       field & 2  -> if (gKeyInput & 8 /* START */) do exactly the two writes above
//                     and clear REG_DISPCNT bit 14.
//   * InitWorldMapGfx sets the field to 2 ("skippable").
//   * StreamCmd_ToggleDisplayFlag flips the field's high bit (2).
//
// Runtime experiments below:
//   A) observe the field across the boot/opening sequence and across a START press
//   B) CAUSAL: from a running skippable scene, apply the function's two writes by
//      hand (no button press) and show the scene tears itself down exactly like the
//      START path, while an untouched control run keeps playing.
import { HeadlessRuntime, REPO } from './gba-kit.mjs';
import { mkdirSync } from 'fs';

const ROOT = REPO;
const OUT = '/tmp/klonoa-dyn-bgmode';
mkdirSync(OUT, { recursive: true });

// Input path from boot to the opening/world-map sequence (same prefix the other
// proof scripts use). [button, frames] pairs.
const TO_SKIPPABLE_SCENE = [
  [null, 409], ['a', 5], [null, 20], ['a', 4], [null, 8], ['a', 4], [null, 8], ['a', 3], [null, 8],
  ['a', 4], [null, 51], ['a', 4], [null, 8], ['a', 3], [null, 89], ['a', 4], [null, 18],
];

const mk = async () => {
  const rt = await HeadlessRuntime.create({
    romPath: `${ROOT}/baserom.gba`,
    elfPath: `${ROOT}/klonoa-eod.elf`,
    outputDir: OUT,
    logFn: () => {},
  });
  return rt;
};

const PTR = 0x030034a0; // gGfxBufferPtr / gLevelStatePtr
const STREAM = 0x03004d84; // gStreamPtr

const state = (bus) => {
  const p = bus.read32(PTR) >>> 0;
  if (!p) return { p: 0, txt: 'ptr=0' };
  const b0 = bus.read8(p), b2 = bus.read8(p + 2);
  return {
    p, b0, b2, field: (b2 >> 1) & 3,
    txt: `buf=0x${p.toString(16)} byte0=0x${b0.toString(16).padStart(2, '0')} byte2=0x${b2.toString(16).padStart(2, '0')} field=${(b2 >> 1) & 3} stream=0x${(bus.read32(STREAM) >>> 0).toString(16)}`,
  };
};

// ---------------------------------------------------------------- experiment A
console.log('=== A. observe the field; press START on a skippable scene ===');
{
  const rt = await mk(); const eng = rt.engine, bus = rt.gba.bus;
  await eng.pressSequence(TO_SKIPPABLE_SCENE);
  console.log('  scene running        ', state(bus).txt);

  // who writes byte 2 of the buffer?
  const p = bus.read32(PTR) >>> 0;
  const w = eng.watchMemory({ address: p + 2, length: 1 });
  await eng.pressSequence([['a', 4], [null, 6], ['start', 6], [null, 30]]);
  w.stop();
  console.log('  after START press    ', state(bus).txt);
  console.log('  writers of buf+2 while skipping:');
  const seen = new Set();
  for (const h of w.hits) {
    if (seen.has(h.instructionAddress)) continue;
    seen.add(h.instructionAddress);
    let fn = '?';
    try { fn = eng.pcToFunction(h.instructionAddress)?.name ?? '?'; } catch (e) { /* */ }
    console.log(`    wrote 0x${(h.value & 0xff).toString(16)} from pc=0x${h.instructionAddress.toString(16)}  (${fn})`);
  }
  await eng.wait({ frames: 120 });
  console.log('  +120f later          ', state(bus).txt);
}

// ---------------------------------------------------------------- experiment B
console.log('');
console.log('=== B. CAUSAL: apply the function\'s two writes by hand, no button ===');
for (const poke of [false, true]) {
  const rt = await mk(); const eng = rt.engine, bus = rt.gba.bus;
  await eng.pressSequence(TO_SKIPPABLE_SCENE);
  const before = state(bus);
  const label = poke ? 'POKED  ' : 'CONTROL';
  console.log(`  ${label} before        ${before.txt}`);
  if (poke) {
    // exactly StreamCmd_SetBGModeTiled's body (minus the gStreamPtr += 2)
    bus.write8(before.p, bus.read8(before.p) & ~3);
    bus.write8(before.p + 2, (bus.read8(before.p + 2) & ~6) | 2);
    console.log(`  ${label} after poke    ${state(bus).txt}`);
  }
  const dispcnt = () => '0x' + (bus.read16(0x04000000) & 0xffff).toString(16);
  await eng.wait({ frames: 40 });
  console.log(`  ${label} +40f          ${state(bus).txt} DISPCNT=${dispcnt()}`);
  await eng.wait({ frames: 120 });
  console.log(`  ${label} +160f         ${state(bus).txt} DISPCNT=${dispcnt()}`);
  await eng.takeScreenshot({ path: `${OUT}/exitrequest-${poke ? 'poked' : 'control'}.png` });
}

// ---------------------------------------------------------------- experiment C
// The skip path is gated on `gKeyInput (0x03004DA0) & 8`. Confirm bit 3 is START.
console.log('');
console.log('=== C. which key is bit 3 of gKeyInput (0x03004DA0)? ===');
{
  const rt = await mk(); const eng = rt.engine, bus = rt.gba.bus;
  await eng.pressSequence(TO_SKIPPABLE_SCENE);
  for (const key of ['a', 'b', 'select', 'start', 'up']) {
    let seen = 0;
    const stop = eng.onFrame(() => { seen |= bus.read16(0x03004da0) & 0xffff; });
    await eng.press([key], { hold: 6 });
    await eng.wait({ frames: 4 });
    if (typeof stop === 'function') stop();
    console.log(`  ${key.padEnd(7)} -> gKeyInput bits seen = 0x${seen.toString(16)}  (bit3 set: ${(seen & 8) !== 0})`);
  }
}
