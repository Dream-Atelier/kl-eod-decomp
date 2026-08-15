// PROOF: ProcessSceneTransitionOut (0x0804E0E8) really is the "out" half of a gfx-stream
// scene, and its four-arm switch selects a DESTINATION — which screen comes next — rather
// than a phase of one transition.
//
// A four-arm switch named for a direction is a claim about all four arms, so all four are
// checked here, and the one that cannot be reached is reported as unreached rather than
// quietly counted as evidence.
//
// The claims under test:
//   S1. gUnk_0805769C is one byte per gfx-stream scene, packed (world << 4) | level. Decoded
//       straight out of the cartridge: which indices are populated and which low nibbles the
//       shipped data actually contains.
//   S2. For every populated entry, running the function to completion sets
//       gUnk_03004C20.world / .level from that byte and installs a specific callback set. The
//       arm is therefore chosen by the scene's destination, not by how far along a fade is.
//   S3. The default direction is OUT: gBlendValue and gMosaicSize ramp UP, once per two
//       frames, and the teardown happens when the blend passes 15.
//       GfxControlFlags.blendRampDown reverses only the ramp and clears itself.
//   S4. CONTROLS: an odd globalFrameCounter must move nothing at all, and a table byte with a
//       zero high nibble must leave the callback queue alone.
//
// Method: causal intervention on a frozen savestate. gLevelStatePtr (which the function reads
// as its GfxControlFlags block) and gUnk_03005284 are pointed at scratch EWRAM this script
// owns, so the only thing that differs between trials is the one byte under test; the state
// is reloaded from the savestate before every trial, and no frames advance, so nothing but
// the single-stepped ROM code can touch anything.
//
// Bit positions come from the ELF's DWARF (GfxControlFlags.blendRampDown,
// Unk_03004C20.world/.level), never hand-written.
//
// Run: GBA_KIT=... node docs/dynamic-analysis/scripts/prove-scene-exit-destination.mjs
import { HeadlessRuntime } from './gba-kit.mjs';
import { ROM, ELF, SAVES, readField, writeField } from './_harness.mjs';
import { makeCaller, mustSymbol } from './_callrom.mjs';
import { QUEUE_LAYOUT } from './_messagebox.mjs';
import { mkdirSync, readFileSync } from 'node:fs';

const OUT = '/tmp/klonoa-scene-exit';
mkdirSync(OUT, { recursive: true });

const failures = [];
const check = (ok, what) => {
    console.log(`    ${ok ? 'OK  ' : 'FAIL'}  ${what}`);
    if (!ok) failures.push(what);
};

const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const eng = rt.engine,
    bus = rt.gba.bus,
    cpu = rt.gba.armCpu,
    di = eng.debugInfo;
const call = makeCaller(cpu);
const h = (n, w = 4) => '0x' + (n >>> 0).toString(16).padStart(w, '0');
const symName = (a) => {
    const s = eng.addressToSymbol(a & ~1);
    return s && s.offset === 0 ? s.name : h(a, 8);
};

const PSTO = mustSymbol(di, 'ProcessSceneTransitionOut');
const TABLE = mustSymbol(di, 'gUnk_0805769C');
const C20 = mustSymbol(di, 'gUnk_03004C20');
const S284 = mustSymbol(di, 'gUnk_03005284');
const Q = mustSymbol(di, 'gCallbackQueue');
const L = QUEUE_LAYOUT(di);
const BLEND = mustSymbol(di, 'gBlendValue');
const MOSAIC = mustSymbol(di, 'gMosaicSize');
const CTLPTR = 0x030034a0; // gLevelStatePtr / gGfxBufferPtr — the same heap cell
const RAMP = di.structMember('GfxControlFlags', 'blendRampDown');
const FRAME = di.structMember('Unk_03004C20', 'globalFrameCounter');
const WORLD = di.structMember('Unk_03004C20', 'world');
const LEVEL = di.structMember('Unk_03004C20', 'level');
const IDX = di.structMember('Unk_03005284', 'unk4');
console.log(
    `ProcessSceneTransitionOut = ${h(PSTO, 8)}   gUnk_0805769C = ${h(TABLE, 8)}\n` +
        `DWARF: GfxControlFlags.blendRampDown = byte +${RAMP.offset} bit ${RAMP.bitOffset}` +
        `   Unk_03004C20.world +${WORLD.offset} .level +${LEVEL.offset}   Unk_03005284.unk4 +${IDX.offset}`,
);

// ═══════════════════════════════════════════════════════════════════════════════════════
// S1. The table, decoded out of the cartridge.
// ═══════════════════════════════════════════════════════════════════════════════════════
console.log('\n=== S1. gUnk_0805769C decoded: (world << 4) | level ===');
const rom = readFileSync(ROM);
const tableByte = (i) => rom[TABLE - 0x08000000 + i];
const POPULATED = [];
{
    for (let i = 0; i < 26; i++) {
        const b = tableByte(i);
        if ((b & 0xf0) !== 0) POPULATED.push(i);
        console.log(
            `  [${String(i).padStart(2)}] = ${h(b, 2)}   world ${b >> 4}  low nibble ${b & 0x0f}` +
                (i >= 3 && i % 3 === 0 ? `   <- index world*3 for world ${i / 3}; code_1.c sets exactly this` : ''),
        );
    }
    // The table does not run to a round 32: nine zero bytes follow the populated block and
    // then unrelated data starts. Printed rather than assumed, so "the table" is not silently
    // extended over whatever lives after it.
    console.log(
        `  [26..31] = ${[26, 27, 28, 29, 30, 31].map((i) => h(tableByte(i), 2)).join(' ')}` +
            '   <- no longer zero: this is other data, not more table',
    );
    const nibbles = [...new Set(POPULATED.map((i) => tableByte(i) & 0x0f))].sort((a, b) => a - b);
    console.log(`  populated indices: ${POPULATED.join(', ')}`);
    console.log(`  low nibbles that actually occur: ${nibbles.join(', ')}`);
    check(POPULATED.length === 17, '17 entries are populated');
    check(
        JSON.stringify(nibbles) === JSON.stringify([0, 2, 4, 8]),
        'the shipped low nibbles are 0, 2, 4 and 8 — the switch has an arm for each',
    );
    check(
        !nibbles.includes(5),
        'no populated entry has low nibble 5, so `case 5` is NOT reachable through this table',
    );
    let zeros = 0;
    for (let i = 17; i < 26; i++) if (tableByte(i) === 0) zeros++;
    check(zeros === 9, 'indices 17..25 are zero (the populated block ends at 16)');
    // The only writer of gUnk_03005284->unk4 in decompiled C is code_1.c:
    // `gUnk_03005284->unk4 = gUnk_03004C20.world * 3`, and its neighbours +/-1 are the other
    // two entries of each world's group, so worlds 1..6 span indices 2..19 — all inside the
    // populated block or its zero tail.
    console.log('  world*3 for worlds 1..6 spans indices 2..19, i.e. the populated block plus zeros');
}

// ═══════════════════════════════════════════════════════════════════════════════════════
// S2. Run it to completion once per entry and read back where the game is being sent.
// ═══════════════════════════════════════════════════════════════════════════════════════
const SCRATCH_CTL = 0x0203c000,
    SCRATCH_284 = 0x0203b000;
const QUEUE_SENTINEL = 0xdeadbee0;
const COUNT_SENTINEL = 0xee;

async function trial({ idx, rampDown = 0, blend = 0x0f, frame = 0, calls = 1 }) {
    await eng.loadState(`${SAVES}/savestate-in-gameplay.json`);
    await eng.wait({ frames: 2 });
    for (let k = 0; k < 0x40; k++) bus.write8(SCRATCH_CTL + k, 0);
    for (let k = 0; k < 0x40; k++) bus.write8(SCRATCH_284 + k, 0);
    bus.write32(CTLPTR, SCRATCH_CTL);
    bus.write32(S284, SCRATCH_284);
    writeField(bus, SCRATCH_284, IDX, idx);
    writeField(bus, SCRATCH_CTL, RAMP, rampDown);
    writeField(bus, C20, FRAME, frame);
    bus.write8(BLEND, blend);
    for (let i = 0; i < 10; i++) bus.write32(Q + L.next + i * 4, (QUEUE_SENTINEL + i) >>> 0);
    bus.write8(Q + L.nextCount, COUNT_SENTINEL);
    const before = {
        world: readField(bus, C20, WORLD),
        level: readField(bus, C20, LEVEL),
        dispcnt: bus.read16(0x04000000),
        mosaic: bus.read8(MOSAIC),
        blend: bus.read8(BLEND),
    };
    const runs = [];
    for (let i = 0; i < calls; i++) {
        const r = call(PSTO, [], 3_000_000);
        if (r.timedOut) throw new Error(`ProcessSceneTransitionOut did not return (idx ${idx})`);
        runs.push({ blend: bus.read8(BLEND), mosaic: bus.read8(MOSAIC), ramp: readField(bus, SCRATCH_CTL, RAMP) });
        writeField(bus, C20, FRAME, frame + (i + 1) * 2);
    }
    const next = [];
    for (let i = 0; i < 5; i++) next.push(bus.read32(Q + L.next + i * 4) >>> 0);
    return {
        before,
        runs,
        byte: tableByte(idx),
        world: readField(bus, C20, WORLD),
        level: readField(bus, C20, LEVEL),
        dispcnt: bus.read16(0x04000000),
        nextCount: bus.read8(Q + L.nextCount),
        next,
        nextNames: next.map(symName),
        ramp: readField(bus, SCRATCH_CTL, RAMP),
    };
}

console.log('\n=== S2. every populated entry, run to its teardown ===');
{
    const seen = new Map();
    for (const idx of POPULATED) {
        const t = await trial({ idx });
        const arm = t.byte & 0x0f;
        const queued = t.nextCount === COUNT_SENTINEL ? '(queue untouched)' : t.nextNames.slice(0, t.nextCount).join(', ');
        console.log(
            `  [${String(idx).padStart(2)}] ${h(t.byte, 2)} -> world ${t.before.world}->${t.world}` +
                ` level ${t.before.level}->${t.level}  nextCount=${t.nextCount === COUNT_SENTINEL ? '-' : t.nextCount}` +
                `  case ${arm}: ${queued}`,
        );
        if (!seen.has(arm)) seen.set(arm, { idx, t });
    }
    console.log(`\n  arms actually exercised: ${[...seen.keys()].sort((a, b) => a - b).join(', ')}`);

    const a0 = seen.get(0).t,
        a8 = seen.get(8).t,
        a4 = seen.get(4).t,
        a2 = seen.get(2).t;
    check(a0.world === a0.byte >> 4 && a0.level === 0, 'case 0 sends the game to level 0 of the byte\'s world');
    check(a8.level === 8, 'case 8 sends it to level 8');
    check(
        a0.nextNames.slice(0, 2).join(',') === 'InitLevelBG,ResetVideoRegisters' &&
            a8.nextNames.slice(0, 2).join(',') === 'InitLevelBG,ResetVideoRegisters',
        'both rebuild a level (InitLevelBG, ResetVideoRegisters)',
    );
    check(
        a4.level === 4 &&
            a4.nextNames.slice(0, 3).join(',') === 'ReadKeyInput,TransitionToGameplayScreen,VBlankCallback_Gameplay',
        'case 4 goes straight into gameplay behind TransitionToGameplayScreen',
    );
    check(
        (a2.t ?? a2).byte === 0x52 && a2.world === 6 && a2.level === 8,
        'case 2 is world 5\'s hand-off: destination world 5 is overridden to world 6, level 8',
    );
    check(
        seen.get(2).idx === 16 && POPULATED.filter((i) => (tableByte(i) & 0x0f) === 2).length === 1,
        'and only one entry (index 16, byte 0x52) selects it, so its `world == 5` guard always holds',
    );
    console.log(
        '  case 5 was NOT exercised: no populated byte selects it, and gba-kit\'s bus ignores writes\n' +
            '  to cartridge ROM, so the entry cannot be synthesised either. It stays an unobserved arm.',
    );
}

console.log('\n=== S2b. CONTROL: a table byte with a zero high nibble ===');
{
    const t = await trial({ idx: 17 });
    console.log(
        `  [17] ${h(t.byte, 2)} -> world ${t.before.world}->${t.world} level ${t.before.level}->${t.level}` +
            `  nextCount=${t.nextCount}  next[0]=${h(t.next[0], 8)}`,
    );
    check(t.world === t.before.world && t.level === t.before.level, 'world and level are left alone');
    check(t.nextCount === COUNT_SENTINEL, 'and the callback queue is not republished');
    check(t.next.every((v, i) => v === ((QUEUE_SENTINEL + i) >>> 0)), 'next[] still holds this script\'s sentinels');
}

// ═══════════════════════════════════════════════════════════════════════════════════════
// S3. The direction of the ramp, and the flag that reverses it.
// ═══════════════════════════════════════════════════════════════════════════════════════
console.log('\n=== S3a. blendRampDown = 0: the blend and the mosaic ramp UP (fade OUT) ===');
{
    const t = await trial({ idx: 2, blend: 0, calls: 8 });
    t.runs.forEach((r, i) => console.log(`  call ${i + 1}: gBlendValue=${r.blend} gMosaicSize=${r.mosaic}`));
    check(
        t.runs.every((r, i) => r.blend === i + 1),
        'gBlendValue climbs by one per call (i.e. once per two frames)',
    );
    check(
        t.runs.every((r, i) => i === 0 || r.mosaic >= t.runs[i - 1].mosaic) && t.runs[7].mosaic > t.runs[0].mosaic,
        'gMosaicSize climbs with it',
    );
    check(bus.read16(0x04000050) === 0x00ff, 'REG_BLDCNT is held at "darken every layer" (0x00FF)');
}

console.log('\n=== S3b. blendRampDown = 1: the same ramp, downward, and self-clearing ===');
{
    const t = await trial({ idx: 2, rampDown: 1, blend: 3, calls: 6 });
    t.runs.forEach((r, i) =>
        console.log(`  call ${i + 1}: gBlendValue=${r.blend} blendRampDown=${r.ramp} REG_DISPCNT=${h(bus.read16(0x04000000))}`),
    );
    console.log(`  REG_DISPCNT before: ${h(t.before.dispcnt)}   after: ${h(t.dispcnt)}   (bit 10 = BG2 enable)`);
    check(t.runs[0].blend === 2 && t.runs[1].blend === 1 && t.runs[2].blend === 0, 'gBlendValue counts DOWN');
    check(t.runs[3].blend === 0xff, 'and underflows to 0xFF on the next step');
    check(t.runs[3].ramp === 0, 'at which point the flag clears itself — it is one-shot');
    check(
        (t.before.dispcnt & 0x0400) !== 0 && (t.dispcnt & 0x0400) === 0,
        'and BG2 is switched off in REG_DISPCNT',
    );
}

console.log('\n=== S3c. CONTROL: an odd globalFrameCounter ===');
{
    const t = await trial({ idx: 2, blend: 5, frame: 1, calls: 4 });
    t.runs.forEach((r, i) => console.log(`  call ${i + 1}: gBlendValue=${r.blend} gMosaicSize=${r.mosaic}`));
    check(
        t.runs.every((r) => r.blend === 5),
        'nothing ramps on odd frames',
    );
    check(t.nextCount === COUNT_SENTINEL, 'and nothing is queued');
}

console.log(`\n=== ${failures.length ? `FAILED: ${failures.length}` : 'all checks passed'} ===`);
for (const f of failures) console.log(`  - ${f}`);
process.exit(failures.length ? 1 : 0);
