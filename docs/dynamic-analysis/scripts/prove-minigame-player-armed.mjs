// PROOF: gMinigamePlayerArmed (0x0300549C, was `gUnk_0300549C`) is the ARMING FLAG for the player
// avatar of the hidden boot-menu minigame, and the function that sets it tests
// D-PAD UP — not Select, as its old name `IsSelectButtonPressed` claimed.
//
// Background (static, then confirmed below at runtime):
//   AgbMain @0x080005B8 swaps callback slot 1 (normally FadeOutController) for
//   MainGameFrameLoop when `gHeldKeys & 0x313 == 0x313` at boot — that is
//   A+B+RIGHT+L+R held on the very first frame. MainGameFrameLoop then calls
//   ConfigureInterruptsForGameplay once, which does
//       gMinigamePlayerArmed = IsDpadUpHeld();      // == (gHeldKeys & DPAD_UP) != 0
//   and then calls UpdatePlayerEntity every frame. UpdatePlayerEntity's ONLY use
//   of the flag is to gate the Select-press that spawns the player avatar at
//   (0x78, 0x9C) with animation 0x22; entities 14..19 rain down regardless.
//
// EXPERIMENT — three boots that differ only in ONE extra held button:
//   A. A+B+RIGHT+L+R              (no UP)      -> flag 0, Select does nothing
//   B. A+B+RIGHT+L+R + UP                      -> flag 1, Select spawns the avatar
//   C. A+B+RIGHT+L+R + SELECT     (not UP)     -> flag 0  => the test is NOT Select
// Everything else (ROM, frame counts, Select press) is identical, so the flag and
// the avatar spawn are attributable to UP alone.
//
// Dogfoods @gba-kit/debug-info: every address and field offset below is read from
// the decomp's DWARF (symbolToAddress / structMember) — no magic numbers.
import { HeadlessRuntime, REPO } from './gba-kit.mjs';
import { mkdirSync } from 'fs';

const OUT = '/tmp/klonoa-upe-out';
mkdirSync(OUT, { recursive: true });
const ROOT = REPO;
const ROM = ROOT + '/klonoa-eod.gba';
const ELF = ROOT + '/klonoa-eod.elf';

const BOOT = 'a+b+right+l+r'; // AgbMain's 0x313 loop-swap code

async function boot(extra) {
  const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
  const eng = rt.engine,
    bus = rt.gba.bus,
    di = eng.debugInfo;
  const sym = (n) => {
    const a = di.symbolToAddress(n);
    if (a === null || a === undefined) throw new Error('symbol not in ELF: ' + n);
    return a >>> 0;
  };
  const ENT = sym('gEntityInfo');
  const FLAG = sym('gMinigamePlayerArmed');
  const F = (n) => di.structMember('EntityInfo', n); // offsets straight from DWARF
  const e = (i) => ENT + i * 0x1c;
  const combo = extra ? BOOT + '+' + extra : BOOT;
  const writers = [];
  const w = eng.watchMemory({
    address: FLAG,
    length: 1,
    filter: (h) => {
      const f = eng.pcToFunction(h.instructionAddress);
      writers.push(`${f ? f.name : '?'} @0x${(h.instructionAddress >>> 0).toString(16)} -> ${bus.read8(FLAG)}`);
      return false;
    },
  });
  await eng.pressSequence([[combo, 150]]);
  w.stop();
  return { eng, bus, di, ENT, FLAG, F, e, combo, writers };
}

const show = (s, tag) =>
  console.log(
    `  ${tag}: gMinigamePlayerArmed=${s.bus.read8(s.FLAG)}  ` +
      `gEntityInfo[0].unk10=${s.bus.read8(s.e(0) + s.F('unk10').offset)} ` +
      `x=${s.bus.read16(s.e(0) + s.F('xPosBg2').offset)} ` +
      `y=${s.bus.read16(s.e(0) + s.F('yPosBg2').offset)}`,
  );

console.log('=== boot-code loop swap (AgbMain: gHeldKeys & 0x313 == 0x313) ===');
for (const extra of [null, 'up', 'select']) {
  const s = await boot(extra);
  console.log(`[${s.combo}]`);
  console.log('  callback slot 1 = 0x' + s.bus.read32(0x03003514).toString(16), '(0x80477a9 = MainGameFrameLoop|1)');
  console.log('  writers of gMinigamePlayerArmed (0x0300549C):', s.writers.join(', ') || '(none)');
  show(s, 'before Select');
  await s.eng.pressSequence([['select', 3], [null, 10]]);
  show(s, 'after  Select');
  await s.eng.takeScreenshot({ name: 'upe-' + (extra ?? 'noup') });
}

// ---------------------------------------------------------------------------
// Bonus evidence, same armed run: what the loop over entities 14..19 does.
// (Scoped observation, not a global rename -- see the report.)
console.log('\n=== armed run: per-frame behaviour of gEntityInfo[14..19] ===');
{
  const s = await boot('up');
  await s.eng.pressSequence([['select', 3], [null, 4]]);
  const OFF = { y: s.F('yPosBg2').offset, x: s.F('xPosBg2').offset, st: s.F('unkF').offset, v: s.F('unk8').offset + 1 };
  const rd = (i) => ({
    y: s.bus.read16(s.e(i) + OFF.y),
    x: s.bus.read16(s.e(i) + OFF.x),
    st: s.bus.read8(s.e(i) + OFF.st),
    v: s.bus.read8(s.e(i) + OFF.v),
  });
  let prev = {};
  for (let i = 14; i <= 19; i++) prev[i] = rd(i);
  let steps = 0,
    match = 0,
    respawns = 0;
  const speeds = new Set();
  for (let f = 0; f < 240; f++) {
    await s.eng.pressSequence([[null, 1]]);
    for (let i = 14; i <= 19; i++) {
      const c = rd(i);
      if (c.y === 0 && prev[i].y > 0xc0) {
        respawns++;
        speeds.add(c.v);
        if (respawns <= 3)
          console.log(
            `  respawn e${i}: y ${prev[i].y} -> 0, x ${prev[i].x} -> ${c.x}, offset+9 ${prev[i].v} -> ${c.v}, unkF ${prev[i].st}`,
          );
      } else if (c.y > prev[i].y) {
        steps++;
        if (c.y - prev[i].y === prev[i].v) match++;
        else if (steps - match <= 3) console.log(`  MISMATCH e${i}: dy=${c.y - prev[i].y} offset+9=${prev[i].v}`);
      }
      prev[i] = c;
    }
  }
  console.log(`  frames sampled: 240 | descending steps: ${steps} | steps where dy == byte@+0x09: ${match}`);
  console.log(`  respawns observed: ${respawns} | distinct speeds drawn at respawn: ${[...speeds].sort().join(', ')}`);
  console.log('  (UpdatePlayerEntity: e->yPosBg2 += e->unk8.split.unk9; respawn draws (rand%3)+2)');

  // L / R actually drive the avatar horizontally, and it is clamped to [0x10, 0xDF].
  const px = () => s.bus.read16(s.e(0) + OFF.x);
  console.log('\n=== armed run: L / R move the avatar (UpdatePlayerEntity R_BUTTON / L_BUTTON) ===');
  console.log('  x before      =', px());
  await s.eng.pressSequence([['r', 30]]);
  console.log('  x after 30f R =', px());
  await s.eng.pressSequence([['l', 30]]);
  console.log('  x after 30f L =', px());
  await s.eng.pressSequence([['l', 200]]);
  console.log('  x after 200 more frames of L (clamp low)  =', px(), '  (source clamp: while x > 0x10)');

  // The whole L/R block is gated on gEntityAnimationInfo[0].state != 0x0C, which the
  // collision test in the same function sets when a falling enemy overlaps the avatar.
  // Watch the avatar's animation slot: it should take the value 0x0C on a hit, and the
  // avatar should stop responding to L/R for as long as it holds.
  const ANIM = s.di.symbolToAddress('gEntityAnimationInfo') >>> 0;
  const st = () => s.bus.read8(ANIM + s.di.structMember('EntityAnimationInfo', 'state').offset);
  const seen = new Set();
  let hitFrames = 0,
    hitFrozen = 0,
    freeFrames = 0,
    freeMoved = 0;
  for (let f = 0; f < 400; f++) {
    const a0 = st(),
      x0 = px();
    await s.eng.pressSequence([['r', 1]]);
    seen.add(a0);
    if (a0 === 0xc) {
      hitFrames++;
      if (px() === x0) hitFrozen++;
    } else if (x0 < 0xdf) {
      freeFrames++;
      if (px() === x0 + 2) freeMoved++;
    }
  }
  console.log('  animation ids seen on slot 0 over 400 frames of R:', [...seen].map((v) => '0x' + v.toString(16)).join(', '));
  console.log(`  frames entered with anim == 0x0C (hit): ${hitFrames} | avatar x unchanged on ${hitFrozen} of them`);
  console.log(`  frames entered with anim != 0x0C and x < 0xDF: ${freeFrames} | x advanced by exactly +2 on ${freeMoved}`);
  await s.eng.takeScreenshot({ name: 'upe-armed-play' });
}

// ---------------------------------------------------------------------------
// CONFIRMS an existing name: EntityInfo.unkC_2 (Unk_03002920.flip, bit0 = hFlip).
// UpdatePlayerEntity writes 0 on R and 1 on L; if `flip` is right, the avatar's OAM
// entry must carry hFlip=0 after moving right and hFlip=1 after moving left.
console.log('\n=== armed run: gEntityInfo[0].unkC_2 == OAM hFlip (confirms Unk_03002920.flip) ===');
{
  const s = await boot('up');
  await s.eng.pressSequence([['select', 3], [null, 4]]);
  const FL = s.di.structMember('EntityInfo', 'unkC_2');
  const flip = () => (s.bus.read8(s.e(0) + FL.offset) >> FL.bitOffset) & ((1 << FL.bitWidth) - 1);
  const px = () => s.bus.read16(s.e(0) + s.F('xPosBg2').offset);
  // the avatar is the only sprite near y=0x9C at the avatar's x
  // the avatar is the enabled sprite nearest to (avatar x, 0x9C)
  const avatarOam = () => {
    const x = px();
    const d = (o) => Math.abs(o.x - x) + Math.abs(o.y - 0x9c);
    return s.eng
      .readOAM()
      .filter((o) => o.enabled)
      .sort((a, b) => d(a) - d(b))[0];
  };
  for (const [btn, label] of [['r', 'after R (moving right)'], ['l', 'after L (moving left)']]) {
    await s.eng.pressSequence([[btn, 24]]);
    const o = avatarOam();
    console.log(
      `  ${label}: gEntityInfo[0].xPosBg2=${px()} unkC_2=${flip()} | nearest OAM sprite (${o.x},${o.y}) hFlip=${Number(o.hFlip)}`,
    );
  }
}
