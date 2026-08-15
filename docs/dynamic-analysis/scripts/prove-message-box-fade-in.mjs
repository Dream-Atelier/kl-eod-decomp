// PROOF: the unnamed function at 0x08047F80 is UpdateMessageBoxFadeIn — the callback that
// fades the screen back in after the message box has shut and then hands the game back to
// the callback set that was running before the box.
//
// It is the round's odd artefact: nothing CALLS it. Its address is only ever stored into a
// callback slot, so luvdis found no entry point for it and scripts/generate_asm.py absorbed
// it into its neighbour's INCLUDE_ASM slice (0x08047EC8..0x08048027 held two ROM functions,
// not one). It reached committed C with an address-derived name, which the project's naming
// policy forbids; this is the evidence for the name it has now.
//
// The claims under test:
//   G1. It really is a separate function and really is unreferenced: exactly one word in the
//       whole cartridge equals 0x08047F81, and no BL anywhere in the ROM targets it.
//   G2. While gBlendValue is non-zero it is a FADE step: it re-pins REG_WININ / REG_WINOUT
//       every call and steps gBlendValue DOWN by one, but only on frames where
//       gUnk_03004C20.globalFrameCounter & 3 == 0.
//   G3. At gBlendValue == 0 it RESTORES: REG_BLDCNT, REG_BG0CNT..REG_BG3CNT, gBlendValue and
//       gUnk_03004C20.sceneFrameCounter all come back from gUnk_030051F0, and
//       gCallbackQueue.previous is republished as .next — which is what ends the message box.
//   G4. It is the exact mirror of the fade-OUT at 0x08047ABC, which steps the same byte UP on
//       the same one-in-four cadence and then queues InitFadeTransition.
//   G5. Live: gBlendValue is the REG_BLDY level, so "fade in" is what the screen does.
//
// G2/G3 are causal interventions on a frozen savestate — no frames advance, so nothing but
// the single-stepped ROM code can touch a register — with distinctive sentinel values written
// into gUnk_030051F0 and gCallbackQueue.previous so a restore cannot be confused with a value
// that happened to be there. Each has its control: the same call from the other side of the
// gBlendValue == 0 test, which must do the other thing and only the other thing.
//
// Run: GBA_KIT=... node docs/dynamic-analysis/scripts/prove-message-box-fade-in.mjs
import { HeadlessRuntime, REPO } from './gba-kit.mjs';
import { ROM, ELF, SAVES, readField, writeField } from './_harness.mjs';
import { makeCaller, mustSymbol } from './_callrom.mjs';
import { armMessageBox, queueNow, QUEUE_LAYOUT } from './_messagebox.mjs';
import { mkdirSync, readFileSync } from 'node:fs';

const OUT = '/tmp/klonoa-message-box-fade';
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
const ctx = { eng, bus, di };
const call = makeCaller(cpu);
const h = (n, w = 4) => '0x' + (n >>> 0).toString(16).padStart(w, '0');

// Register names by ADDRESS out of include/io_reg.h, never typed in here.
const ioReg = readFileSync(`${REPO}/include/io_reg.h`, 'utf8');
const OFFSETS = new Map();
for (const m of ioReg.matchAll(/^#define\s+REG_OFFSET_(\w+)\s+(0x[0-9a-fA-F]+)/gm))
    OFFSETS.set(0x04000000 + parseInt(m[2], 16), m[1]);
const regAddr = (name) => {
    for (const [a, n] of OFFSETS) if (n === name) return a;
    throw new Error(`include/io_reg.h has no REG_OFFSET_${name}`);
};

const FADE_IN = mustSymbol(di, 'UpdateMessageBoxFadeIn');
const WIPE = mustSymbol(di, 'UpdateMessageBoxWipe');
const BLEND = mustSymbol(di, 'gBlendValue');
const P = mustSymbol(di, 'gUnk_030051F0');
const C20 = mustSymbol(di, 'gUnk_03004C20');
const Q = mustSymbol(di, 'gCallbackQueue');
const L = QUEUE_LAYOUT(di);
const FRAME = di.structMember('Unk_03004C20', 'globalFrameCounter');
const SCENE = di.structMember('Unk_03004C20', 'sceneFrameCounter');
const SAVED = Object.fromEntries(
    ['unk0', 'unk4', 'unk6', 'unk8', 'unkA', 'unkC', 'unkE'].map((k) => [k, di.structMember('Unk_030051F0', k)]),
);
console.log(`UpdateMessageBoxFadeIn = ${h(FADE_IN, 8)}   UpdateMessageBoxWipe = ${h(WIPE, 8)}`);

// ═══════════════════════════════════════════════════════════════════════════════════════
// G1. Nothing calls it, and exactly one word points at it.
// ═══════════════════════════════════════════════════════════════════════════════════════
console.log('\n=== G1. how 0x08047F80 is reached at all ===');
{
    const rom = readFileSync(ROM);
    const thumbPtr = (FADE_IN | 1) >>> 0;
    const words = [];
    for (let i = 0; i + 4 <= rom.length; i += 2) if (rom.readUInt32LE(i) === thumbPtr) words.push(0x08000000 + i);

    // Every Thumb BL in the cartridge, resolved to its target. A BL is a prefix halfword
    // (0xF000-0xF7FF, high 11 bits of a signed offset) followed by a suffix (0xF800-0xFFFF).
    const bls = [];
    for (let i = 0; i + 4 <= rom.length; i += 2) {
        const hi = rom.readUInt16LE(i);
        if ((hi & 0xf800) !== 0xf000) continue;
        const lo = rom.readUInt16LE(i + 2);
        if ((lo & 0xf800) !== 0xf800) continue;
        let off = ((hi & 0x7ff) << 12) | ((lo & 0x7ff) << 1);
        if (off & 0x400000) off -= 0x800000;
        const target = (0x08000000 + i + 4 + off) >>> 0;
        if (target === (FADE_IN >>> 0)) bls.push(0x08000000 + i);
    }
    console.log(`  words equal to ${h(thumbPtr, 8)} in the ROM: ${words.map((w) => h(w, 8)).join(', ') || 'none'}`);
    for (const w of words) {
        const owner = eng.addressToSymbol(w);
        console.log(`    ${h(w, 8)} is inside ${owner ? `${owner.name}+${owner.offset}` : '?'}`);
    }
    console.log(`  BL instructions targeting ${h(FADE_IN, 8)}: ${bls.length}`);
    check(words.length === 1, 'exactly one word in the cartridge points at it');
    check(
        eng.addressToSymbol(words[0]) && eng.addressToSymbol(words[0]).name === 'UpdateMessageBoxWipe',
        'and that word is in UpdateMessageBoxWipe (its literal pool)',
    );
    check(bls.length === 0, 'no BL anywhere in the ROM calls it — it is only ever installed');
}

// ═══════════════════════════════════════════════════════════════════════════════════════
// G2. The fade half.
// ═══════════════════════════════════════════════════════════════════════════════════════
const freeze = async () => {
    await eng.loadState(`${SAVES}/savestate-in-gameplay.json`);
    await eng.wait({ frames: 2 });
};

console.log('\n=== G2. gBlendValue != 0: one step down every fourth frame ===');
{
    await freeze();
    bus.write8(BLEND, 9);
    bus.write16(regAddr('WININ'), 0xffff);
    bus.write16(regAddr('WINOUT'), 0xffff);
    const rows = [];
    for (let f = 0; f < 16; f++) {
        writeField(bus, C20, FRAME, f);
        const before = bus.read8(BLEND);
        call(FADE_IN, []);
        rows.push({
            f,
            before,
            after: bus.read8(BLEND),
            winin: bus.read16(regAddr('WININ')),
            winout: bus.read16(regAddr('WINOUT')),
        });
    }
    for (const r of rows)
        console.log(
            `  globalFrameCounter=${String(r.f).padStart(2)} (&3=${r.f & 3})  gBlendValue ${r.before} -> ${r.after}` +
                `   REG_WININ=${h(r.winin)} REG_WINOUT=${h(r.winout)}`,
        );
    const stepped = rows.filter((r) => r.after !== r.before).map((r) => r.f);
    console.log(`  frames on which gBlendValue moved: ${stepped.join(', ')}`);
    check(
        stepped.every((f) => (f & 3) === 0) && stepped.length === 4,
        'gBlendValue steps only on frames where globalFrameCounter & 3 == 0',
    );
    check(
        rows.every((r) => r.after === r.before || r.after === r.before - 1),
        'and never by more than one, always downward',
    );
    check(
        rows.every((r) => r.winin === 0x0001 && r.winout === 0x003f),
        'every call re-pins REG_WININ = 0x0001 and REG_WINOUT = 0x003F',
    );
}

// ═══════════════════════════════════════════════════════════════════════════════════════
// G3. The restore half, with sentinels.
// ═══════════════════════════════════════════════════════════════════════════════════════
const SENTINELS = {
    unkE: 0x0b, // gBlendValue
    unk4: 0xbc01, // REG_BLDCNT
    unk6: 0xbc02, // REG_BG0CNT
    unk8: 0xbc03, // REG_BG1CNT
    unkA: 0xbc04, // REG_BG2CNT
    unkC: 0xbc05, // REG_BG3CNT
    unk0: 0x5eed0001, // gUnk_03004C20.sceneFrameCounter
};
async function restoreTrial(blendAtEntry) {
    await freeze();
    bus.write8(BLEND, blendAtEntry);
    for (const [k, v] of Object.entries(SENTINELS)) writeField(bus, P, SAVED[k], v);
    // distinctive previous[] so "restored" cannot be confused with "was already equal"
    for (let i = 0; i < 10; i++) {
        bus.write32(Q + L.previous + i * 4, (0xc0de0000 + i) >>> 0);
        bus.write32(Q + L.next + i * 4, 0);
    }
    bus.write8(Q + L.previousCount, 7);
    bus.write8(Q + L.nextCount, 0);
    bus.write16(regAddr('WININ'), 0x0001);
    const curCount = bus.read8(Q + L.currentCount);
    bus.write32(Q + L.current + (curCount - 1) * 4, 0xffffffff);
    writeField(bus, C20, FRAME, 1); // NOT a multiple of 4: the fade step must not fire
    call(FADE_IN, []);
    const next = [];
    for (let i = 0; i < 10; i++) next.push(bus.read32(Q + L.next + i * 4) >>> 0);
    return {
        bldcnt: bus.read16(regAddr('BLDCNT')),
        bg: [0, 1, 2, 3].map((i) => bus.read16(regAddr(`BG${i}CNT`))),
        blend: bus.read8(BLEND),
        scene: readField(bus, C20, SCENE) >>> 0,
        winin: bus.read16(regAddr('WININ')),
        next,
        nextCount: bus.read8(Q + L.nextCount),
        lastCurrent: bus.read32(Q + L.current + (curCount - 1) * 4) >>> 0,
    };
}

console.log('\n=== G3. gBlendValue == 0: the restore ===');
{
    const r = await restoreTrial(0);
    console.log(`  REG_BLDCNT   = ${h(r.bldcnt)}   (sentinel ${h(SENTINELS.unk4)})`);
    r.bg.forEach((v, i) =>
        console.log(`  REG_BG${i}CNT  = ${h(v)}   (sentinel ${h([SENTINELS.unk6, SENTINELS.unk8, SENTINELS.unkA, SENTINELS.unkC][i])})`),
    );
    console.log(`  gBlendValue  = ${r.blend}        (sentinel ${SENTINELS.unkE})`);
    console.log(`  sceneFrameCounter = ${h(r.scene, 8)}  (sentinel ${h(SENTINELS.unk0, 8)})`);
    console.log(`  REG_WININ    = ${h(r.winin)}   (0x0001 on entry, bit 5 set here)`);
    console.log(`  next[0..6]   = ${r.next.slice(0, 7).map((v) => h(v, 8)).join(' ')}`);
    console.log(`  nextCount    = ${r.nextCount}  (previousCount was 7)`);
    console.log(`  current[last]= ${h(r.lastCurrent, 8)}  (0xffffffff on entry)`);
    check(r.bldcnt === SENTINELS.unk4, 'REG_BLDCNT restored from gUnk_030051F0.unk4');
    check(
        JSON.stringify(r.bg) === JSON.stringify([SENTINELS.unk6, SENTINELS.unk8, SENTINELS.unkA, SENTINELS.unkC]),
        'REG_BG0CNT..REG_BG3CNT restored from unk6/unk8/unkA/unkC',
    );
    check(r.blend === SENTINELS.unkE, 'gBlendValue restored from unkE');
    check(r.scene === SENTINELS.unk0, 'gUnk_03004C20.sceneFrameCounter restored from unk0');
    check(r.winin === 0x0021, 'REG_WININ gains bit 5 (the colour-effect bit for window 0)');
    check(
        r.next.every((v, i) => v === ((0xc0de0000 + i) >>> 0)),
        'gCallbackQueue.previous is copied verbatim into .next — the box hands the game back',
    );
    check(r.nextCount === 7, 'nextCount takes previousCount');
    check(r.lastCurrent === 0, 'and the current set is terminated');
}

console.log('\n=== G3b. CONTROL: the same call with gBlendValue = 5 ===');
{
    const r = await restoreTrial(5);
    console.log(`  REG_BLDCNT=${h(r.bldcnt)} gBlendValue=${r.blend} nextCount=${r.nextCount}`);
    console.log(`  next[0..2] = ${r.next.slice(0, 3).map((v) => h(v, 8)).join(' ')}`);
    check(r.bldcnt !== SENTINELS.unk4, 'REG_BLDCNT is NOT restored');
    check(r.blend === 5, 'gBlendValue is untouched (frame counter & 3 != 0, so no step either)');
    check(r.nextCount === 0 && r.next.every((v) => v === 0), 'and the callback queue is NOT republished');
}

// ═══════════════════════════════════════════════════════════════════════════════════════
// G4. The mirror: the fade-OUT at 0x08047ABC.
// ═══════════════════════════════════════════════════════════════════════════════════════
// 0x08047ABC has no symbol of its own for the same reason 0x08047F80 had none — nothing calls
// it, it is only ever installed — and it sits inside the slice luvdis calls sub_0804786E. The
// address is therefore written out here, and guarded: the function is only run after checking
// that its first instruction loads the gUnk_03004D90 pool word, so a shifted ROM fails loudly
// instead of executing whatever happens to live there.
const FADE_OUT = 0x08047abc;
console.log('\n=== G4. the fade-OUT half at 0x08047ABC is the same ramp, upward ===');
{
    const owner = eng.addressToSymbol(FADE_OUT);
    const first = [...eng.disassemble(FADE_OUT, 2, 'thumb')];
    const lit = /=(0x[0-9a-f]+)/.exec(first[0].instruction);
    const poolWord = lit ? bus.read32(parseInt(lit[1], 16)) >>> 0 : 0;
    console.log(`  ${h(FADE_OUT, 8)} is inside ${owner ? `${owner.name}+${owner.offset}` : '?'}`);
    console.log(`  first instruction: ${first[0].instruction}  -> pool word ${h(poolWord, 8)}`);
    check(poolWord === mustSymbol(di, 'gUnk_03004D90'), 'guard: it really starts by loading gUnk_03004D90');

    await freeze();
    bus.write8(BLEND, 0);
    const rows = [];
    for (let f = 0; f < 40; f++) {
        writeField(bus, C20, FRAME, f);
        const before = bus.read8(BLEND);
        call(FADE_OUT | 1, []);
        rows.push({ f, before, after: bus.read8(BLEND), slot0: bus.read32(Q + L.current) >>> 0 });
    }
    const stepped = rows.filter((r) => r.after !== r.before);
    console.log(`  gBlendValue went ${rows[0].before} -> ${rows[rows.length - 1].after} in ${stepped.length} steps`);
    console.log(`  it stepped on frames: ${stepped.map((r) => r.f).join(', ')}`);
    const installed = rows.find((r) => (r.slot0 & ~1) === (mustSymbol(di, 'InitFadeTransition') & ~1));
    console.log(
        `  callback slot 0 became InitFadeTransition at gBlendValue = ${installed ? installed.before : '(never)'}`,
    );
    check(stepped.every((r) => (r.f & 3) === 0), 'the fade-out steps on the same one-in-four cadence');
    check(stepped.every((r) => r.after === r.before + 1), 'and steps UP where UpdateMessageBoxFadeIn steps down');
    check(!!installed, 'once it passes 8 it queues InitFadeTransition, which builds the message box');
}

// ═══════════════════════════════════════════════════════════════════════════════════════
// G5. Live: gBlendValue IS the REG_BLDY level, so the ramp is a fade.
// ═══════════════════════════════════════════════════════════════════════════════════════
console.log('\n=== G5. live: gBlendValue reaches REG_BLDY, and the screen comes back ===');
{
    const before = await armMessageBox(ctx, 3);
    await eng.wait({ frames: 40 }); // box opens
    await eng.press('a');
    const rows = [];
    for (let f = 0; f < 22; f++) {
        await eng.wait({ frames: 4 });
        rows.push({
            f: (f + 1) * 4,
            blend: bus.read8(BLEND),
            bldy: bus.read16(regAddr('BLDY')),
            bldcnt: bus.read16(regAddr('BLDCNT')),
            slot1: queueNow(ctx)[1],
        });
    }
    for (const r of rows)
        console.log(
            `  +${String(r.f).padStart(3)}f  gBlendValue=${String(r.blend).padStart(2)} REG_BLDY=${String(r.bldy).padStart(2)}` +
                `  REG_BLDCNT=${h(r.bldcnt)}  slot1=${r.slot1}`,
        );
    const duringFade = rows.filter((r) => r.slot1 === 'UpdateMessageBoxFadeIn');
    console.log(`  samples taken while UpdateMessageBoxFadeIn owned slot 1: ${duringFade.length}`);
    check(duringFade.length > 0, 'the fade-in callback did run');
    // REG_BLDY is written by the VBlank callback, so a sample taken between frames sees the
    // value gBlendValue had one push ago: the two differ by 0 or 1, never more, and never in
    // the other direction.
    const lag = duringFade.map((r) => r.bldy - r.blend);
    console.log(`  REG_BLDY - gBlendValue at each sample: ${lag.join(', ')}`);
    check(lag.every((d) => d === 0 || d === 1), 'REG_BLDY tracks gBlendValue to within one VBlank push');
    check(
        duringFade.every((r, i) => i === 0 || r.bldy <= duringFade[i - 1].bldy) &&
            duringFade[duringFade.length - 1].bldy < duringFade[0].bldy,
        'and falls monotonically while the fade-in runs',
    );
    check(
        duringFade.every((r) => (r.bldcnt & 0xc0) === 0xc0),
        'and REG_BLDCNT holds the brightness-DECREASE effect, so a falling value is the screen brightening',
    );
    const endQueue = queueNow(ctx);
    check(
        JSON.stringify(endQueue) === JSON.stringify(before.map((p) => queueNameOf(p))),
        `the queue ends up back at the pre-message-box set [${endQueue.join(', ')}]`,
    );
}

function queueNameOf(p) {
    const s = eng.addressToSymbol(p & ~1);
    return s && s.offset === 0 ? s.name : h(p, 8);
}

console.log(`\n=== ${failures.length ? `FAILED: ${failures.length}` : 'all checks passed'} ===`);
for (const f of failures) console.log(`  - ${f}`);
process.exit(failures.length ? 1 : 0);
