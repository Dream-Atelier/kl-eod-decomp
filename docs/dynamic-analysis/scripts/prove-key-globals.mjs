// PROOF: 0x03004DA0 is gNewKeys (edge-detected) and 0x030051E4 is gHeldKeys
// (raw held state) — NOT `gKeysPressed` / `gKeysPrevious`, the names they also
// carried, which imply the opposite meanings.
//
// Static reading first. ReadKeyInput @0x080006CC does:
//     r1 = 0x3FF ^ REG_KEYINPUT     ; active-high current keys
//     r2 = [0x030051E4]             ; read BEFORE overwriting
//     [0x03004DA0] = r1 & ~r2       ; edge
//     [0x030051E4] = r1             ; held
// so 0x030051E4 only holds "the previous frame" for the two instructions
// between those loads. By the time any other function reads it, it is the
// CURRENT held state — and 30 of the 34 call sites use it that way.
//
// EXPERIMENT: hold A for six frames, then release for two, sampling both
// addresses every frame. An edge-detected global is high on frame 1 only; a
// held-state global is high for all six.
import { HeadlessRuntime, REPO } from './gba-kit.mjs';
import { mkdirSync } from 'fs';

const OUT = '/tmp/klonoa-key-globals';
mkdirSync(OUT, { recursive: true });

const NEW_KEYS = 0x03004da0;
const HELD_KEYS = 0x030051e4;
const A_BUTTON = 0x001;

const rt = await HeadlessRuntime.create({
  romPath: REPO + '/klonoa-eod.gba',
  elfPath: REPO + '/klonoa-eod.elf',
  outputDir: OUT,
  logFn: () => {},
});
const eng = rt.engine,
  bus = rt.gba.bus,
  di = eng.debugInfo;

// The addresses come from the ELF, so a rename in ldscript.in.txt that
// contradicts this experiment fails here instead of silently drifting.
for (const [name, expected] of [
  ['gNewKeys', NEW_KEYS],
  ['gHeldKeys', HELD_KEYS],
]) {
  const got = di.symbolToAddress(name);
  if ((got >>> 0) !== expected) {
    throw new Error(`${name} is 0x${(got >>> 0).toString(16)} in the ELF, expected 0x${expected.toString(16)}`);
  }
  console.log(`  ${name.padEnd(10)} = 0x${expected.toString(16).toUpperCase()}  (from DWARF)`);
}

await eng.pressSequence([[null, 300]]); // boot to the title screen

const sample = () => ({ nw: bus.read16(NEW_KEYS), held: bus.read16(HELD_KEYS) });
const newSeries = [];
const heldSeries = [];

console.log('\n=== holding A for 6 frames, then releasing for 2 ===');
for (let i = 0; i < 8; i++) {
  await eng.pressSequence([[i < 6 ? 'a' : null, 1]]);
  const s = sample();
  newSeries.push(s.nw & A_BUTTON ? 1 : 0);
  heldSeries.push(s.held & A_BUTTON ? 1 : 0);
  console.log(
    `  frame ${i + 1} (${i < 6 ? 'A held ' : 'release'}):  gNewKeys=${s.nw & A_BUTTON ? 1 : 0}  gHeldKeys=${s.held & A_BUTTON ? 1 : 0}`,
  );
}

console.log('\n  gNewKeys  series:', newSeries.join(','), '  <- high on the press frame only => EDGE');
console.log('  gHeldKeys series:', heldSeries.join(','), '  <- high for the whole hold      => HELD');

const edgeOk = newSeries.join(',') === '1,0,0,0,0,0,0,0';
const heldOk = heldSeries.join(',') === '1,1,1,1,1,1,0,0';
console.log(`\n  gNewKeys is edge-detected : ${edgeOk ? 'CONFIRMED' : 'REFUTED'}`);
console.log(`  gHeldKeys is the held state: ${heldOk ? 'CONFIRMED' : 'REFUTED'}`);
if (!edgeOk || !heldOk) {
  throw new Error('key-global semantics did not reproduce');
}
