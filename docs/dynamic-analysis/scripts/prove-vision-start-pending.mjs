// PROOF: gUnk_030034B0.visionStartPending (byte 0x030034B0, bit 4) is the one-shot latch of a
// ~31-frame confirm delay — NOT a pause flag.
//
// The bit is written by exactly one function, RunVisionStartConfirmDelay (0x0804539A, formerly and
// wrongly called InitPauseMenu), the confirm delay between picking a vision and loading it:
//     first frame  -> play confirm jingle (song 0x26), latch the bit, restart the scene counter
//     30 frames on -> queue TransitionGameOver (the generic fade-out) and drop the latch
//
// Two experiments:
//   A. REFUTATION of "isPaused": open the pause menu with START and show the bit stays 0 and the
//      byte is never written at all, while the PAUSE screen is demonstrably on-screen.
//   B. CAUSAL PROOF of the real meaning, as an A/B intervention: from one savestate, run the game
//      twice for 60 frames changing exactly one thing — the CONTROL run is left alone, the other has
//      RunVisionStartConfirmDelay written into gCallbackQueue.current[1] by hand. Only the poked run
//      raises the latch, it holds it ~31 frames, and it drops it as the callback becomes
//      TransitionGameOver. Every write to the byte is source-annotated from DWARF line info.
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

// ─── B. CAUSAL: install the function, change nothing else ──────────────────
{
    console.log('\nB. CAUSAL A/B — install RunVisionStartConfirmDelay by hand, change nothing else');
    const runs = {};
    for (const poke of [false, true]) {
        const [eng, bus] = await boot();
        await eng.loadState(`${SAVES}/savestate-fresh-gameplay.json`);
        await eng.pressSequence([[null, 30]]);

        const QUEUE = eng.symbolToAddress('gCallbackQueue');
        const FN = eng.symbolToAddress('RunVisionStartConfirmDelay');
        if (QUEUE == null || FN == null) throw new Error('gCallbackQueue / RunVisionStartConfirmDelay missing from the ELF');
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

        const label = poke ? 'POKED  ' : 'CONTROL';
        console.log(`\n   ${label} before: cb=${cbName()} visionStartPending=${pending()} byte=0x${bus.read8(BYTE).toString(16)}`);
        if (poke) bus.write32(QUEUE + 4, (FN | 1) >>> 0); // the ONE difference between the runs

        const series = [];
        let highFor = 0;
        for (let f = 0; f < 60; f++) {
            await eng.pressSequence([[null, 1]]);
            const b = pending();
            series.push(b);
            if (b === 1) highFor++;
            if (f === 4) await eng.takeScreenshot({ name: `B-${poke ? 'poked' : 'control'}-f5` });
        }
        console.log(`   ${label} latch per frame: ${series.join('')}`);
        console.log(`   ${label} frames high: ${highFor}   cb afterwards: ${cbName()}`);
        console.log(`   ${label} writers of 0x030034B0:`,
            writers.size ? [...writers].map(([k, e]) => `${e.n}x ${k} -> ${[...e.vals].join(',')}`).join(' | ') : 'NONE');
        runs[poke ? 'poked' : 'control'] = { series, highFor, cb: cbName(), writers };
    }

    const c = runs.control, p = runs.poked;
    console.log('\n=== VERDICT ===');
    const v1 = c.highFor === 0 && p.highFor > 0;
    console.log(`   only the poked run raises the latch (control ${c.highFor} frames, poked ${p.highFor}): ${v1 ? 'CONFIRMED' : 'REFUTED'}`);
    const v2 = p.highFor >= 28 && p.highFor <= 34;
    console.log(`   it stays high for the ~31-frame confirm delay (${p.highFor}): ${v2 ? 'CONFIRMED' : 'REFUTED'}`);
    const v3 = p.series[p.series.length - 1] === 0;
    console.log(`   it drops again once the delay is spent (cb afterwards = ${p.cb}): ${v3 ? 'CONFIRMED' : 'REFUTED'}`);
    const own = [...p.writers.keys()].filter((k) => k.includes('RunVisionStartConfirmDelay'));
    console.log(`   RunVisionStartConfirmDelay is among the writers: ${own.length ? 'CONFIRMED' : 'REFUTED'}  (${own.join(', ') || 'none'})`);
    if (!(v1 && v2 && v3 && own.length)) throw new Error('visionStartPending semantics did not reproduce');
}
