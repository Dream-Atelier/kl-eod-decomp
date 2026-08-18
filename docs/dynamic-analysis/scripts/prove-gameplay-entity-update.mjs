// PROOF: 0x0800A804 is the in-level per-frame entity update. It was named
// `HandlePauseMenuInput` after the guard it opens with; the guard is real, but it
// is not what the function is.
//
// Method: a level frame with the player standing still, then the same run again
// with START pressed.
//
//   1. It runs once on every frame of ordinary play.
//   2. Across those frames it writes gCallbackQueue zero times — the pause push
//      never happens — while dispatching per-kind entity handlers on every frame.
//   3. Press START and the pause push fires exactly once, and every write to
//      gCallbackQueue in the whole run comes from this function. That is the
//      positive control: observation 2 is a measurement, not a dead watchpoint.
//
// Expected output: three PASS lines and the handlers it dispatched.
import { mkdirSync } from 'node:fs';

import { bootNextToMoo } from './_death.mjs';

const OUT = '/tmp/klonoa-gameplay-entity-update';
mkdirSync(OUT, { recursive: true });

const FRAMES = 120;

/**
 * Run `input` for FRAMES frames and report what 0x0800A804 did: how often it ran,
 * what it dispatched, and every write it made to the callback queue.
 */
async function measure(input) {
  const rt = await bootNextToMoo(OUT);
  const eng = rt.engine;
  const di = eng.debugInfo;
  const self = di.pcToFunction(di.symbolToAddress('UpdateGameplayEntities') >>> 0);
  const queue = di.symbolToAddress('gCallbackQueue') >>> 0;
  const queueSize = eng.symbolExtent('gCallbackQueue');

  const STT_FUNC = 2;
  const entries = new Map();
  for (const s of di.symbols.symbols) {
    if (s.type !== STT_FUNC || (s.address >>> 0) < 0x08000000) continue;
    const a = (s.address >>> 0) & ~1;
    if (!entries.has(a)) entries.set(a, s.name);
  }

  const inSelf = (pc) => pc >= self.address && pc < self.end;
  const watches = [...entries].map(([addr, name]) => [name, eng.watchExecution(addr, { maxHits: 2000 })]);
  const wSelf = eng.watchExecution(self.address, { maxHits: 1 });
  const wQueue = eng.watchMemory({
    address: queue,
    length: queueSize.size,
    maxHits: 4000,
    filter: (h) => inSelf(h.instructionAddress >>> 0),
  });

  await eng.pressSequence(input);

  for (const [, w] of watches) w.stop();
  wSelf.stop();
  wQueue.stop();

  const dispatched = watches
    .map(([name, w]) => [name, w.hits.filter((h) => inSelf((h.lr >>> 0) & ~1)).length])
    .filter(([, n]) => n > 0)
    .sort((a, b) => b[1] - a[1]);

  return { self, queueSize, calls: wSelf.count, queueWrites: wQueue.hits.length, dispatched };
}

// ── ordinary play ────────────────────────────────────────────────────────────
const idle = await measure([[null, FRAMES]]);
console.log(`  0x${idle.self.address.toString(16)} is ${idle.self.end - idle.self.address} bytes; gCallbackQueue is ${idle.queueSize.size} bytes (from ${idle.queueSize.source})`);

if (idle.calls !== FRAMES) throw new Error(`it ran ${idle.calls} times in ${FRAMES} frames, expected one call per frame`);
console.log(`PASS  1. it ran ${idle.calls} times in ${FRAMES} frames — once per frame`);

if (idle.queueWrites !== 0) throw new Error(`it wrote gCallbackQueue ${idle.queueWrites} times without START being pressed`);
if (idle.dispatched.length === 0) throw new Error('it dispatched nothing at all — wrong scene?');
console.log(`PASS  2. ${idle.queueWrites} writes to gCallbackQueue, and ${idle.dispatched.reduce((n, [, c]) => n + c, 0)} calls out to ${idle.dispatched.length} entity handlers`);

// ── the control: press START and the guard does fire ─────────────────────────
const paused = await measure([
  [null, 10],
  ['start', 6],
  [null, FRAMES - 16],
]);
if (paused.queueWrites === 0) {
  throw new Error('positive control failed: START was pressed and the pause push still wrote nothing — the watchpoint proves nothing');
}
console.log(`PASS  3. control: with START pressed it wrote gCallbackQueue ${paused.queueWrites} times`);

console.log('\n  handlers it dispatched during ordinary play:');
for (const [name, n] of idle.dispatched) console.log(`    ${String(n).padStart(4)}  ${name}`);
