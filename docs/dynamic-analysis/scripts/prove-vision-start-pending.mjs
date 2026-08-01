// PROOF: gUnk_030034B0.visionStartPending (byte 0x030034B0, bit 4) is the one-shot latch for the
// "vision start confirmed" delay — NOT a pause flag.
//
// The bit is written by exactly one function, InitPauseMenu, which despite its (legacy, wrong) name
// is the confirm delay between picking a vision on the vision-select map and loading it:
//     first frame  -> play confirm jingle (song 0x26), latch the bit, restart the scene counter
//     30 frames on -> queue TransitionGameOver (the generic fade-out) and drop the latch
//
// Two experiments:
//   A. REFUTATION of "isPaused": open the pause menu with START and show the bit stays 0 and the
//      byte is never written at all, while the PAUSE screen is demonstrably on-screen.
//   B. PROOF of the real meaning: replay a long run that enters three visions; the bit goes high
//      exactly when gCallbackQueue.current[1] becomes InitPauseMenu (scene counter 0) and low
//      exactly 31 frames later when it becomes TransitionGameOver — three times, and never else.
//      Screenshots show the vision-select map during the window and the loaded vision after it.
//
// Dogfoods @gba-kit/debug-info: `readVariable('gUnk_030034B0.visionStartPending')` decodes the
// packed bitfield straight from the decomp's DWARF (no hand-written >>4 & 1), `addressToSymbol`
// names the callback pointer, and each watch hit carries the writing C func/file:line.
//
// Requires an AGENT_INSTRUMENT=0 build (the instrumentation flags suppress agbcc's DWARF), and a
// full rebuild (`touch src/*.c && make AGENT_INSTRUMENT=0`) so every CU agrees on the struct layout.
import { HeadlessRuntime } from './gba-kit.mjs';
import { mkdirSync, readFileSync } from 'fs';
import { ROM, ELF, SAVES } from './_harness.mjs';

const OUT = '/tmp/klonoa-vision-start';
mkdirSync(OUT, { recursive: true });

const boot = async () => {
    const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
    return [rt.engine, rt.gba.bus];
};
const BYTE = 0x030034b0; // &gUnk_030034B0
const srcOf = (h) =>
    h.location ? `${h.location.func} ${h.location.file.replace(/^.*\/src\//, 'src/')}:${h.location.line}` : 'pc 0x' + h.instructionAddress.toString(16);

// ─── A. the pause menu does NOT touch the bit ────────────────────────────────
{
    const [eng, bus] = await boot();
    const pending = () => eng.readVariable('gUnk_030034B0.visionStartPending');
    await eng.loadState(`${SAVES}/savestate-fresh-gameplay.json`); // in a vision, playing
    await eng.pressSequence([[null, 30]]);

    const writes = [];
    const w = eng.watchMemory({ address: BYTE, length: 1, filter: (h) => (writes.push(srcOf(h)), false) });
    console.log('A. REFUTING "isPaused"');
    console.log('   in a vision, unpaused : visionStartPending =', pending(), ' byte = 0x' + bus.read8(BYTE).toString(16));
    await eng.pressSequence([['start', 6], [null, 60]]);
    console.log('   PAUSE menu on-screen  : visionStartPending =', pending(), ' byte = 0x' + bus.read8(BYTE).toString(16));
    await eng.takeScreenshot({ name: 'A-pause-menu-open' }); // shows PAUSE / Continue Game / World Map
    await eng.pressSequence([['start', 6], [null, 60]]);
    console.log('   unpaused again        : visionStartPending =', pending(), ' byte = 0x' + bus.read8(BYTE).toString(16));
    w.stop();
    console.log('   writes to byte 0x030034B0 across the whole pause/unpause:', writes.length === 0 ? 'NONE' : writes.join(', '));
}

// ─── B. what actually raises it ──────────────────────────────────────────────
// The input sequence is the project's established run (docs/dynamic-analysis/scripts/find-lives.mjs):
// boot -> title -> world map -> three visions entered and played.
const SRC = new URL('./find-lives.mjs', import.meta.url).pathname;
const txt = readFileSync(SRC, 'utf8');
const SEQ = eval(txt.slice(txt.indexOf('const SEQ = [') + 'const SEQ = '.length, txt.indexOf('];', txt.indexOf('const SEQ = [')) + 1));

console.log('\nB. WHAT ACTUALLY RAISES IT  (replaying the three-vision run)');

// B-pass 1: replay the sequence verbatim and tabulate every transition + every writer.
{
    const [eng, bus] = await boot();
    const QUEUE = eng.symbolToAddress('gCallbackQueue');
    const pending = () => eng.readVariable('gUnk_030034B0.visionStartPending');
    const cbName = () => {
        const v = bus.read32(QUEUE + 4) >>> 0; // gCallbackQueue.current[1]
        const s = v && eng.addressToSymbol(v & ~1);
        return s ? s.name : '0x' + v.toString(16);
    };

    const writers = new Map();
    eng.watchMemory({
        address: BYTE,
        length: 1,
        filter: (h) => {
            const k = srcOf(h);
            const e = writers.get(k) || { n: 0, vals: new Set() };
            e.n++;
            e.vals.add('0x' + bus.read8(BYTE).toString(16));
            writers.set(k, e);
            return false;
        },
    });

    let last = null;
    const flips = [];
    eng.onFrame((f) => {
        const b = pending();
        if (b !== last) {
            flips.push({ f, from: last, to: b, cb: cbName(), scene: bus.read32(0x03004c20) >>> 0 });
            last = b;
        }
    });
    for (let i = 0; i < SEQ.length; i += 40) await eng.pressSequence(SEQ.slice(i, i + 40));
    eng.onFrame(null);

    console.log('\n   every visionStartPending transition in the run:');
    for (const t of flips)
        console.log(`     frame ${String(t.f).padStart(6)}  ${t.from}->${t.to}   gCallbackQueue.current[1]=${t.cb}   sceneFrameCounter=${t.scene}`);
    console.log('\n   every writer of byte 0x030034B0 (source-annotated):');
    for (const [k, e] of writers) console.log(`     ${String(e.n).padStart(4)}x  ${k}   byte values: ${[...e.vals].join(',')}`);
}

// B-pass 2: a separate run that stops at the first rise so the window can be screenshotted.
// (Stepping frame-by-frame shifts the input timing, so this must NOT share pass 1's run.)
{
    const [eng, bus] = await boot();
    const QUEUE = eng.symbolToAddress('gCallbackQueue');
    const pending = () => eng.readVariable('gUnk_030034B0.visionStartPending');
    const cbName = () => {
        const v = bus.read32(QUEUE + 4) >>> 0;
        const s = v && eng.addressToSymbol(v & ~1);
        return s ? s.name : '0x' + v.toString(16);
    };
    let i = 0;
    while (i < SEQ.length && pending() === 0) await eng.pressSequence([SEQ[i++]]);
    console.log('\n   latch HIGH: gCallbackQueue.current[1] =', cbName(), ' sceneFrameCounter =', bus.read32(0x03004c20) >>> 0);
    await eng.takeScreenshot({ name: 'B1-latch-high-vision-select-map' }); // vision-select map, HUD "VISION 1-x"
    let held = 0;
    while (pending() === 1 && held < 200) {
        await eng.pressSequence([[null, 1]]);
        held++;
    }
    console.log('   latch LOW after', held, 'more frames: gCallbackQueue.current[1] =', cbName(), ' sceneFrameCounter =', bus.read32(0x03004c20) >>> 0);
    await eng.takeScreenshot({ name: 'B2-latch-low-fade-starts' });
    await eng.pressSequence([[null, 120]]);
    await eng.takeScreenshot({ name: 'B3-vision-loaded' }); // the "VISION 1-2" title card
    console.log('   120 frames later: gCallbackQueue.current[1] =', cbName(), '(the vision is loaded)');
}
