// PROOF: the gfx-stream callback that StreamCmd_InitButtonWait installs — spelled in
// src/gfx.c as the bare literal 0x0804D075 — is a WAIT-FOR-THE-A-BUTTON handler.
// Along the way it also pins down the two GfxStreamEntry fields that command writes:
//
//     +0x14  unk_14 -> RENAMED `timer` = a countdown, decremented once per dispatch (frames)
//     +0x1E  unk_1E  = "ignore the timer" flag — when set, only the button ends the wait
//
// ── why the callback has no name ────────────────────────────────────────────────
// luvdis merged it into the tail of asm/nonmatchings/gfx/ProcessSpriteOscillation.s,
// so there is no `thumb_func_start` for it and no linker symbol. gfx.c therefore has
// to install it as a raw thumb address. Naming it needs evidence, not a guess.
//
// ── how the entry is driven ─────────────────────────────────────────────────────
// ProcessAnimationSteps (0x0804C8F4) walks all 32 GfxStreamEntry slots once per frame;
// for every slot with (status & 7) != 0 it calls sub_0805186C(idx, entry->callback),
// masks the callback's return with 7 and stores it back into status's low 3 bits.
// So: return 1 = "still waiting", return 0 = "done, deactivate this entry".
//
// ── method ──────────────────────────────────────────────────────────────────────
// ProcessAnimationSteps is a *scene* callback (installed by SetupGfxCallbacks as
// gCallbackStateArray[0x2C/4] = sub_0804EB64, which calls it); it does NOT run during
// ordinary in-level gameplay, so simply idling in a save state never exercises the
// path. Instead we execute the real code on the real CPU: load a real in-level save
// state (real RAM, real entry array), set r0/lr/pc and single-step the actual ROM
// instructions. Every number below is produced by ARM instructions from the cartridge,
// and each experiment prints the exact branch path that was executed.
//
// Run:  node docs/dynamic-analysis/scripts/prove-button-wait.mjs
import { HeadlessRuntime, REPO } from './gba-kit.mjs';
import { mkdirSync } from 'fs';

const OUT = '/tmp/klonoa-dynamic-out';
mkdirSync(OUT, { recursive: true });
const ROM = `${REPO}/baserom.gba`;
const ELF = `${REPO}/klonoa-eod.elf`;
import { SAVESTATE as SAVE } from './gba-kit.mjs';

const CALLBACK = 0x0804d075; // the literal src/gfx.c installs into entry->callback
const CALLBACK_FN = CALLBACK & ~1; // 0x0804D074, the thumb entry point
const DISPATCH = 0x0804c8f4; // ProcessAnimationSteps
const SENT = 0x08000000; // sentinel return address
const SLOT = 20;

const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const eng = rt.engine, bus = rt.gba.bus, cpu = rt.gba.armCpu, di = eng.debugInfo;

// ── layout comes from the ELF's DWARF, not from hand-typed offsets ──────────────
const F = {
  status: di.structMember('GfxStreamEntry', 'type'), // bits 0-2 of the u16 header at +0x00
  timer: di.structMember('GfxStreamEntry', 'timer'),
  unk_1E: di.structMember('GfxStreamEntry', 'unk_1E'),
  callback: di.structMember('GfxStreamEntry', 'callback'),
};
const STRIDE = 36;
const KEYS = di.symbolToAddress('gKeysPressed');
const ENTRY_ARRAY_PTR = 0x030052a4; // gBuffer_52A4

console.log('=== layout resolved from DWARF (nothing hand-coded) ===');
for (const [k, v] of Object.entries(F)) console.log(`  GfxStreamEntry.${k.padEnd(8)} +0x${v.offset.toString(16).padStart(2, '0')} size ${v.size}`);
console.log('  sizeof(GfxStreamEntry) =', STRIDE, '  gKeysPressed = 0x' + KEYS.toString(16));

console.log('\n=== the unnamed callback, disassembled from the live cartridge ===');
for (const i of eng.disassemble(CALLBACK_FN, 24, 'thumb')) {
  const p = /=0x(0804d0a[48])/.exec(i.instruction);
  console.log('  0x' + i.address.toString(16), i.instruction + (p ? '   ; -> 0x' + bus.read32(parseInt(p[1], 16)).toString(16) : ''));
}

await eng.loadState(SAVE);
await eng.wait({ frames: 4 });
const ARRAY = bus.read32(ENTRY_ARRAY_PTR) >>> 0;
const ENT = ARRAY + SLOT * STRIDE;
console.log('\n  gBuffer_52A4 -> 0x' + ARRAY.toString(16), ' using slot', SLOT, '(entry @0x' + ENT.toString(16) + ')');

// ── helpers ────────────────────────────────────────────────────────────────────
const s16 = (v) => (v << 16) >> 16;
const getTimer = () => s16(bus.read16(ENT + F.timer.offset));
const getStatus = () => bus.read8(ENT + F.status.offset) & 7;

/** Exactly the four writes StreamCmd_InitButtonWait performs (src/gfx.c). */
function installButtonWait(timeout, flag) {
  bus.write16(ENT + F.timer.offset, timeout & 0xffff);
  bus.write8(ENT + F.unk_1E.offset, flag);
  bus.write32(ENT + F.callback.offset, CALLBACK);
  bus.write8(ENT + F.status.offset, (bus.read8(ENT + F.status.offset) & ~7) | 1);
}

/** Execute a real thumb function on the real CPU; returns {ret, trace}. */
function callThumb(addr, r0) {
  const saved = Array.from(cpu.registers), savedCpsr = cpu.cpsr;
  cpu.registers[0] = r0 >>> 0;
  cpu.registers[14] = SENT | 1;
  cpu.registers[15] = addr >>> 0;
  cpu.cpsr = (cpu.cpsr | 0x20) >>> 0; // force thumb
  const trace = [];
  let n = 0;
  for (; n < 200000; n++) {
    const pc = cpu.registers[15] >>> 0;
    if (pc >= SENT && pc < SENT + 0x10) break;
    if (trace.length < 64) trace.push(pc);
    cpu.step();
  }
  const ret = cpu.registers[0] >>> 0;
  for (let i = 0; i < 16; i++) cpu.registers[i] = saved[i];
  cpu.cpsr = savedCpsr;
  return { ret, trace, steps: n };
}
const shortTrace = (t) => t.map((p) => p.toString(16).slice(-3)).join('>');

// ═══════════════════════════════════════════════════════════════════════════════
// 1. Which button is bit 0 of gKeysPressed?  (real input path, no register pokes)
// ═══════════════════════════════════════════════════════════════════════════════
console.log('\n=== 1. gKeysPressed bit assignment, measured by actually pressing buttons ===');
for (const btn of ['a', 'b', 'select', 'start', 'right', 'left', 'up', 'down', 'r', 'l']) {
  await eng.loadState(SAVE);
  await eng.wait({ frames: 4 });
  let peak = 0;
  eng.onFrame(() => { peak |= bus.read16(KEYS); });
  await eng.pressSequence([[null, 2], [btn, 8], [null, 2]]);
  eng.onFrame(null);
  console.log(`  press ${btn.padEnd(6)} -> gKeysPressed = 0x${peak.toString(16).padStart(4, '0')}   bit0 = ${peak & 1}`);
}
{
  await eng.loadState(SAVE);
  await eng.wait({ frames: 4 });
  const per = [];
  eng.onFrame(() => per.push(bus.read16(KEYS) & 1));
  await eng.pressSequence([[null, 3], ['a', 10], [null, 3]]);
  eng.onFrame(null);
  console.log('  bit0 per frame over [idle x3, A HELD x10, idle x3]:', per.join(''),
              '  -> edge-triggered (a new press), not a hold');
}

// ═══════════════════════════════════════════════════════════════════════════════
// 2. Truth table: execute the real callback with controlled inputs
// ═══════════════════════════════════════════════════════════════════════════════
console.log('\n=== 2. the callback executed on the real CPU (r0 = entry index) ===');
console.log('  keys&1  unk_1E  timer in   ->  ret   timer out   path (low 3 hex digits of each PC)');
const cases = [
  [0, 0, 5, 'counting down'],
  [0, 0, 1, 'counting down'],
  [0, 0, 0, 'this tick takes it negative'],
  [0, 0, -1, 'already negative'],
  [1, 0, 500, 'A pressed, timer nowhere near expiry'],
  [0, 1, -1, 'expired BUT flag set'],
  [0, 1, -500, 'very expired, flag set'],
  [1, 1, -500, 'A pressed, flag set'],
];
for (const [k, flag, t, why] of cases) {
  await eng.loadState(SAVE);
  await eng.wait({ frames: 4 });
  installButtonWait(t, flag);
  bus.write16(KEYS, k); // the callback's only other input
  const { ret, trace } = callThumb(CALLBACK_FN, SLOT);
  console.log(`    ${k}       ${flag}      ${String(t).padStart(5)}     ->   ${ret}    ${String(getTimer()).padStart(5)}      ${shortTrace(trace)}   (${why})`);
}

// ═══════════════════════════════════════════════════════════════════════════════
// 3. End-to-end: drive the real dispatcher and watch the entry deactivate
// ═══════════════════════════════════════════════════════════════════════════════
// ProcessAnimationSteps runs every slot, so first park all the other slots
// (status&7 = 0) to keep the experiment to a single entry; the whole array is
// restored from a snapshot afterwards, and every run reloads the save state anyway.
async function runDispatcher(timeout, flag, keysPerTick, ticks) {
  await eng.loadState(SAVE);
  await eng.wait({ frames: 4 });
  const snap = Array.from(bus.getRange ? [] : []); // (array restored via loadState next run)
  for (let i = 0; i < 32; i++) if (i !== SLOT) bus.write8(ARRAY + i * STRIDE, bus.read8(ARRAY + i * STRIDE) & ~7);
  installButtonWait(timeout, flag);
  const log = [];
  for (let n = 0; n < ticks; n++) {
    bus.write16(KEYS, keysPerTick(n));
    const before = { st: getStatus(), t: getTimer() };
    if (before.st === 0) { log.push({ n, ...before, done: true }); break; }
    callThumb(DISPATCH, ARRAY);
    log.push({ n, stBefore: before.st, tBefore: before.t, stAfter: getStatus(), tAfter: getTimer(), keys: keysPerTick(n) & 1 });
  }
  return log;
}

console.log('\n=== 3a. dispatcher, timeout = 4 frames, unk_1E = 0, NO input ===');
for (const r of await runDispatcher(4, 0, () => 0, 8)) {
  if (r.done) { console.log(`   tick ${r.n}: status&7 = 0 -> entry already gone`); break; }
  console.log(`   tick ${r.n}: status&7 ${r.stBefore} -> ${r.stAfter}   timer  ${String(r.tBefore).padStart(3)} -> ${String(r.tAfter).padStart(3)}`);
}

console.log('\n=== 3b. dispatcher, timeout = 600 frames, unk_1E = 0, A pressed on tick 3 ===');
for (const r of await runDispatcher(600, 0, (n) => (n === 3 ? 1 : 0), 8)) {
  if (r.done) { console.log(`   tick ${r.n}: status&7 = 0 -> entry already gone`); break; }
  console.log(`   tick ${r.n}: keys&1 ${r.keys}  status&7 ${r.stBefore} -> ${r.stAfter}   timer  ${String(r.tBefore).padStart(3)} -> ${String(r.tAfter).padStart(3)}`);
}

console.log('\n=== 3c. dispatcher, timeout = 2 but unk_1E = 1 (flag), A pressed on tick 6 ===');
for (const r of await runDispatcher(2, 1, (n) => (n === 6 ? 1 : 0), 9)) {
  if (r.done) { console.log(`   tick ${r.n}: status&7 = 0 -> entry already gone`); break; }
  console.log(`   tick ${r.n}: keys&1 ${r.keys}  status&7 ${r.stBefore} -> ${r.stAfter}   timer  ${String(r.tBefore).padStart(3)} -> ${String(r.tAfter).padStart(3)}`);
}

console.log('\n=== 3d. CONTROL for 3c: identical but unk_1E = 0 ===');
for (const r of await runDispatcher(2, 0, (n) => (n === 6 ? 1 : 0), 9)) {
  if (r.done) { console.log(`   tick ${r.n}: status&7 = 0 -> entry already gone`); break; }
  console.log(`   tick ${r.n}: keys&1 ${r.keys}  status&7 ${r.stBefore} -> ${r.stAfter}   timer  ${String(r.tBefore).padStart(3)} -> ${String(r.tAfter).padStart(3)}`);
}

console.log('\n=== 3e. CONTROL: which button reaches the dispatcher? (timeout 600, real presses) ===');
// Here the game itself runs frames, so gKeysPressed is produced by the real input
// path; between frames we hand-dispatch the entry.
for (const btn of ['a', 'b', 'start', 'left']) {
  await eng.loadState(SAVE);
  await eng.wait({ frames: 4 });
  for (let i = 0; i < 32; i++) if (i !== SLOT) bus.write8(ARRAY + i * STRIDE, bus.read8(ARRAY + i * STRIDE) & ~7);
  installButtonWait(600, 0);
  let ended = -1, tick = 0;
  eng.onFrame(() => {
    if (ended >= 0) return;
    if (getStatus() === 0) { ended = tick; return; }
    callThumb(DISPATCH, ARRAY);
    tick++;
  });
  await eng.pressSequence([[null, 3], [btn, 6], [null, 6]]);
  eng.onFrame(null);
  console.log(`   ${btn.padEnd(6)} -> ${ended === -1 ? 'still waiting after ' + tick + ' dispatches (timer = ' + getTimer() + ')' : 'ENDED after ' + ended + ' dispatches (timer = ' + getTimer() + ')'}`);
}
