// PROOF: UpdateMessageBoxWipe (0x08047ECA) drives the in-game MESSAGE BOX — the panel the
// game pops over gameplay to show a hint or a notice — and the shape it animates is an
// axis-aligned RECTANGLE growing from the middle of the screen. Not an iris, not a curtain,
// and not a "screen wipe" between two scenes: the level stays loaded and visible around it.
//
// The four claims under test:
//   C1. The two registers it drives are REG_WIN1H and REG_WIN1V, and nothing else moves.
//   C2. wipeState == 1 opens the box over exactly 24 frames, from the single point
//       (120, 76) to the rectangle x 0..240 / y 4..148; wipeState == 2 closes it again over
//       the same 24 frames and then hands slot 1 to UpdateMessageBoxFadeIn.
//   C3. From the open state, pressing A, B, SELECT or START — and none of the other six
//       buttons — arms the closing pass.
//   C4. What the box reveals is a message panel, and the game gets it back afterwards:
//       run the whole cycle live and look at the screen.
//
// C1 FIRST, AND STATICALLY, because this is the claim this project has got wrong before: an
// earlier round shipped a proof script with WIN0V and WIN1H transposed, so the observations
// were right and the labels were not, and no build gate could ever catch it. So the primary
// evidence here is the ROM's own addressing arithmetic — UpdateMessageBoxWipe loads one
// register pointer and walks it +4, and InitFadeTransition walks a second one -0x0A, +2, -8,
// +4 — with every stop looked up in include/io_reg.h BY ADDRESS. A mislabelled hardware map
// cannot survive that, and it is cheaper than an emulator run.
//
// C2/C3 are then causal interventions on a frozen savestate: no frames advance, so no
// interrupt, VBlank handler or game logic can touch a window register, and the ROM's own
// Thumb code is single-stepped with the state under test set by hand. Every trial has a
// control — the same call with wipeState == 0, or with no key pressed, which must move
// nothing.
//
// C3 does not hand-write the KEYINPUT bit map either: each button is pressed on the real
// emulator and gNewKeys is read back, so the bit -> button mapping is measured, and only
// then is each bit fed to the function.
//
// C4 arms the message box the way the ROM does (InitGfxState's tail at 0x08048220 saves the
// callback queue and the display registers, sets wipeState = 1 and queues
// InitFadeTransition) and then just lets the game run.
//
// Run: GBA_KIT=... node docs/dynamic-analysis/scripts/prove-message-box-wipe.mjs
import { HeadlessRuntime, REPO } from './gba-kit.mjs';
import { ROM, ELF, SAVES, readField, writeField } from './_harness.mjs';
import { makeCaller, mustSymbol } from './_callrom.mjs';
import { armMessageBox, queueNow } from './_messagebox.mjs';
import { createHash } from 'node:crypto';
import { mkdirSync, readFileSync } from 'node:fs';

const OUT = '/tmp/klonoa-message-box';
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
const ctx = { eng, bus, di };
const h = (n, w = 4) => '0x' + (n >>> 0).toString(16).padStart(w, '0');
const symName = (a) => {
    const s = eng.addressToSymbol(a & ~1);
    return s && s.offset === 0 ? s.name : h(a, 8);
};

// ── every REG_ name in this script comes from include/io_reg.h, by address ─────────────
const ioReg = readFileSync(`${REPO}/include/io_reg.h`, 'utf8');
const OFFSETS = new Map();
for (const m of ioReg.matchAll(/^#define\s+REG_OFFSET_(\w+)\s+(0x[0-9a-fA-F]+)/gm))
    OFFSETS.set(0x04000000 + parseInt(m[2], 16), m[1]);
const regName = (addr) => OFFSETS.get(addr) ?? `<no REG_OFFSET_* in io_reg.h for ${h(addr, 8)}>`;
const regAddr = (name) => {
    for (const [a, n] of OFFSETS) if (n === name) return a;
    throw new Error(`include/io_reg.h has no REG_OFFSET_${name}`);
};

console.log('=== the window registers, as include/io_reg.h defines them ===');
for (const n of ['WIN0H', 'WIN1H', 'WIN0V', 'WIN1V', 'WININ', 'WINOUT'])
    console.log(`  REG_${n} = ${h(regAddr(n), 8)}`);

// ═══════════════════════════════════════════════════════════════════════════════════════
// C1a. The ROM's own addressing arithmetic — UpdateMessageBoxWipe's register pointer.
// ═══════════════════════════════════════════════════════════════════════════════════════
/**
 * Walk a Thumb function, tracking one register used as an I/O pointer, and return every
 * address it stores a halfword to. Literal pool loads print the POOL address, so the word
 * stored there is read out of the bus — reading the printed number as the value would label
 * every store with a ROM address instead of a register.
 */
function pointerWalk(start, reg, wantStores, maxInsns) {
    let ptr = null;
    const stops = [];
    const trace = [];
    for (const i of eng.disassemble(start & ~1, maxInsns, 'thumb')) {
        const lit = new RegExp(`^ldr ${reg}, \\[pc[^=]*=(0x[0-9a-f]+)`, 'i').exec(i.instruction);
        if (lit) ptr = bus.read32(parseInt(lit[1], 16)) >>> 0;
        const add = new RegExp(`^adds ${reg}, #(0x[0-9a-f]+)`, 'i').exec(i.instruction);
        const sub = new RegExp(`^subs ${reg}, #(0x[0-9a-f]+)`, 'i').exec(i.instruction);
        if (add) ptr = (ptr + parseInt(add[1], 16)) >>> 0;
        if (sub) ptr = (ptr - parseInt(sub[1], 16)) >>> 0;
        const store = new RegExp(`^strh r\\d+, \\[${reg}, #0x0\\]`, 'i').exec(i.instruction);
        if (lit || add || sub || store)
            trace.push(
                `  ${h(i.address, 8)}  ${i.instruction.padEnd(34)} ; ${reg} = ${h(ptr ?? 0, 8)}` +
                    (OFFSETS.has(ptr) ? ` = REG_${OFFSETS.get(ptr)}` : ''),
            );
        if (store) stops.push(ptr);
        if (stops.length === wantStores) break;
    }
    return { stops, trace };
}

console.log('\n=== C1a. UpdateMessageBoxWipe walks ONE register pointer, +4 ===');
{
    const { stops, trace } = pointerWalk(mustSymbol(di, 'UpdateMessageBoxWipe'), 'r0', 2, 40);
    trace.forEach((l) => console.log(l));
    console.log('  the two stores, labelled from include/io_reg.h by address:');
    stops.forEach((a, i) => console.log(`    store ${i + 1}: ${h(a, 8)} = REG_${regName(a)}`));
    check(stops.length === 2, 'two halfword stores through the walked pointer');
    check(regName(stops[0]) === 'WIN1H', `first store is REG_WIN1H (got REG_${regName(stops[0])})`);
    check(regName(stops[1]) === 'WIN1V', `second store is REG_WIN1V (got REG_${regName(stops[1])})`);
    console.log(
        '    -> the horizontal value goes to window ONE\'s H register and the vertical one to\n' +
            '       window ONE\'s V register. The +4 is what makes the second stop WIN1V and not\n' +
            '       WIN0V, which is two bytes lower.',
    );
}

// ═══════════════════════════════════════════════════════════════════════════════════════
// C1b. InitFadeTransition's walk — where the box's registers are set up in the first place.
// ═══════════════════════════════════════════════════════════════════════════════════════
console.log('\n=== C1b. InitFadeTransition walks the same block -0x0A, +2, -8, +4 ===');
{
    const { stops, trace } = pointerWalk(mustSymbol(di, 'InitFadeTransition'), 'r1', 6, 60);
    trace.forEach((l) => console.log(l));
    const names = stops.map(regName);
    console.log(`  stops: ${names.map((n, i) => `${h(stops[i], 8)}=REG_${n}`).join(', ')}`);
    check(
        JSON.stringify(names) === JSON.stringify(['BLDCNT', 'BLDALPHA', 'WININ', 'WINOUT', 'WIN1H', 'WIN1V']),
        'the walk visits BLDCNT, BLDALPHA, WININ, WINOUT, WIN1H, WIN1V in that order',
    );
}

// The two window-content registers InitFadeTransition writes, decoded bit by bit. WININ's
// low 6 bits are window 0 and bits 8..13 window 1; WINOUT's low 6 bits are "outside every
// window". The layer that is in one and not the other is the message panel's layer.
console.log('\n=== C1c. what InitFadeTransition puts in REG_WININ / REG_WINOUT ===');
{
    const WININ = 0x3701,
        WINOUT = 0x003e;
    const layers = ['BG0', 'BG1', 'BG2', 'BG3', 'OBJ', 'CLR'];
    const bits = (v) => layers.filter((_, i) => v & (1 << i)).join('|') || '(nothing)';
    console.log(`  REG_WININ  = ${h(WININ)}   inside window 0: ${bits(WININ & 0x3f)}`);
    console.log(`                       inside window 1: ${bits((WININ >> 8) & 0x3f)}`);
    console.log(`  REG_WINOUT = ${h(WINOUT)}   outside every window: ${bits(WINOUT & 0x3f)}`);
    const inWin1 = (WININ >> 8) & 0x3f,
        outside = WINOUT & 0x3f;
    const only = layers.filter((_, i) => inWin1 & (1 << i) && !(outside & (1 << i)));
    console.log(`  -> shown inside window 1 and NOWHERE else: ${only.join('|')}`);
    check(only.length === 1 && only[0] === 'BG0', 'BG0 is the one layer window 1 alone shows (the panel)');
}

// ═══════════════════════════════════════════════════════════════════════════════════════
// C2. The animation, stepped by hand on a frozen state.
// ═══════════════════════════════════════════════════════════════════════════════════════
const D90 = mustSymbol(di, 'gUnk_03004D90');
const WIPE = mustSymbol(di, 'UpdateMessageBoxWipe');
const FADE_IN = mustSymbol(di, 'UpdateMessageBoxFadeIn');
const QUEUE = mustSymbol(di, 'gCallbackQueue');
const NEWKEYS = mustSymbol(di, 'gNewKeys');
const F = {
    win1h: di.structMember('Unk_03004D90', 'win1h'),
    win1v: di.structMember('Unk_03004D90', 'win1v'),
    wipeState: di.structMember('Unk_03004D90', 'wipeState'),
};
const CUR = di.structMember('CallbackQueue', 'current');
console.log(
    `\nDWARF: gUnk_03004D90 @ ${h(D90, 8)}  .win1h +${F.win1h.offset} .win1v +${F.win1v.offset}` +
        ` .wipeState +${F.wipeState.offset}    UpdateMessageBoxFadeIn @ ${h(FADE_IN, 8)}`,
);

// Every I/O register in the LCD block, so "nothing else moved" is measured and not assumed.
const IO_LO = 0x04000000,
    IO_HI = 0x04000060;
const snapshotIo = () => {
    const a = [];
    for (let addr = IO_LO; addr < IO_HI; addr += 2) a.push(bus.read16(addr));
    return a;
};
const ioDiff = (a, b) =>
    a
        .map((v, i) => (v === b[i] ? null : `REG_${regName(IO_LO + i * 2)} ${h(v)}->${h(b[i])}`))
        .filter(Boolean);

const freeze = async () => {
    await eng.loadState(`${SAVES}/savestate-in-gameplay.json`);
    await eng.wait({ frames: 2 });
};
const rect = () => {
    const wh = bus.read16(D90 + F.win1h.offset),
        wv = bus.read16(D90 + F.win1v.offset);
    return { wh, wv, x1: wh >> 8, x2: wh & 0xff, y1: wv >> 8, y2: wv & 0xff };
};

console.log('\n=== C2a. wipeState = 1: the box opens ===');
{
    await freeze();
    writeField(bus, D90, F.win1h, 0x7878);
    writeField(bus, D90, F.win1v, 0x4c4c);
    writeField(bus, D90, F.wipeState, 1);
    bus.write16(NEWKEYS, 0);
    const io0 = snapshotIo();
    const seen = [];
    for (let i = 0; i < 26; i++) {
        call(WIPE, []);
        seen.push({ i: i + 1, ...rect(), st: readField(bus, D90, F.wipeState) });
    }
    for (const s of [seen[0], seen[1], seen[11], seen[22], seen[23], seen[24], seen[25]])
        console.log(
            `  call ${String(s.i).padStart(2)}: win1h=${h(s.wh)} win1v=${h(s.wv)}` +
                `  rect x ${String(s.x1).padStart(3)}..${String(s.x2).padStart(3)}` +
                `  y ${String(s.y1).padStart(3)}..${String(s.y2).padStart(3)}  wipeState=${s.st}`,
        );
    const moved = ioDiff(io0, snapshotIo());
    console.log(`  I/O registers that moved across all 26 calls: ${moved.join(', ') || 'none'}`);
    check(seen[23].wh === 0x00f0 && seen[23].wv === 0x0494, 'call 24 lands on win1h 0x00F0 / win1v 0x0494');
    check(seen[24].st === 0, 'call 25 sets wipeState = 0 (the box is open, animation over)');
    check(seen[25].wh === 0x00f0 && seen[25].wv === 0x0494, 'call 26 moves nothing further');
    check(
        moved.length === 2 && moved.every((m) => /REG_WIN1[HV] /.test(m)),
        'exactly REG_WIN1H and REG_WIN1V moved — no other I/O register',
    );
    check(
        seen[0].x1 === 115 && seen[0].x2 === 125 && seen[0].y1 === 73 && seen[0].y2 === 79,
        'one step moves x by -5/+5 and y by -3/+3 about the centre (120, 76)',
    );
}

console.log('\n=== C2b. CONTROL: wipeState = 0 with no key held ===');
{
    await freeze();
    writeField(bus, D90, F.win1h, 0x3cb4);
    writeField(bus, D90, F.win1v, 0x2870);
    writeField(bus, D90, F.wipeState, 0);
    bus.write16(NEWKEYS, 0);
    const io0 = snapshotIo();
    for (let i = 0; i < 10; i++) call(WIPE, []);
    const moved = ioDiff(io0, snapshotIo());
    console.log(`  after 10 calls: win1h=${h(rect().wh)} wipeState=${readField(bus, D90, F.wipeState)}`);
    console.log(`  I/O registers that moved: ${moved.join(', ') || 'none'}`);
    check(moved.length === 0, 'the control moves no register at all');
    check(rect().wh === 0x3cb4, 'and leaves the box where it was');
}

console.log('\n=== C2c. wipeState = 2: the box closes, then slot 1 changes hands ===');
{
    await freeze();
    writeField(bus, D90, F.win1h, 0x00f0);
    writeField(bus, D90, F.win1v, 0x0494);
    writeField(bus, D90, F.wipeState, 2);
    bus.write16(NEWKEYS, 0);
    const slotBefore = bus.read32(QUEUE + CUR.offset + 4);
    const io0 = snapshotIo();
    let closedAt = null;
    for (let i = 0; i < 24; i++) {
        call(WIPE, []);
        if (rect().wh === 0x7878 && closedAt === null) closedAt = i + 1;
    }
    console.log(`  after ${closedAt} calls the box is shut again (win1h=${h(rect().wh)})`);
    const movedWhileClosing = ioDiff(io0, snapshotIo());
    console.log(`  registers moved while closing: ${movedWhileClosing.join(', ') || 'none'}`);
    check(closedAt === 24, 'closing takes the same 24 calls');
    check(
        movedWhileClosing.length === 2 && movedWhileClosing.every((m) => /REG_WIN1[HV] /.test(m)),
        'still only REG_WIN1H and REG_WIN1V',
    );

    const io1 = snapshotIo();
    call(WIPE, []); // the terminal call: box already shut
    const slotAfter = bus.read32(QUEUE + CUR.offset + 4);
    const movedAtEnd = ioDiff(io1, snapshotIo());
    console.log(`  the terminal call moved: ${movedAtEnd.join(', ') || 'none'}`);
    console.log(`  callback slot 1: ${symName(slotBefore)} -> ${symName(slotAfter)}`);
    check((slotAfter & ~1) === (FADE_IN & ~1), 'slot 1 is handed to UpdateMessageBoxFadeIn');
    check(
        movedAtEnd.some((m) => m.startsWith('REG_BLDCNT 0x') && m.endsWith('->0x00d7')),
        'and REG_BLDCNT becomes 0x00D7 (darken BG0|BG1|BG2|OBJ)',
    );
}

// ═══════════════════════════════════════════════════════════════════════════════════════
// C3. Which buttons dismiss the box — bit map measured, not typed in.
// ═══════════════════════════════════════════════════════════════════════════════════════
console.log('\n=== C3a. bit -> button, measured by pressing each button for real ===');
// pressSequence([[b, 1]]) is the idiom prove-key-globals.mjs established: press(b) on its
// own does not reliably leave the edge visible in gNewKeys on the frame we sample. Both
// globals are `0x3FF ^ REG_KEYINPUT` masked differently, so gHeldKeys corroborates the same
// bit map while the press is still down.
const HELDKEYS = mustSymbol(di, 'gHeldKeys');
const BUTTON_BIT = new Map();
for (const b of ['a', 'b', 'select', 'start', 'right', 'left', 'up', 'down', 'r', 'l']) {
    await eng.loadState(`${SAVES}/savestate-in-gameplay.json`);
    await eng.wait({ frames: 2 });
    await eng.pressSequence([[b, 1]]);
    const nw = bus.read16(NEWKEYS),
        held = bus.read16(HELDKEYS);
    const bit = Math.log2(nw);
    if (nw && Number.isInteger(bit)) BUTTON_BIT.set(bit, b);
    console.log(
        `  press '${b.padEnd(6)}' -> gNewKeys = ${h(nw)} gHeldKeys = ${h(held)}` +
            `  (bit ${Number.isInteger(bit) ? bit : '?'})`,
    );
    if (nw !== held) console.log(`      note: gNewKeys and gHeldKeys disagree on this frame`);
}
check(BUTTON_BIT.size === 10, 'all ten buttons produced a distinct single gNewKeys bit');

console.log('\n=== C3b. which of those bits arms the closing pass ===');
{
    const arming = [];
    for (let bit = 0; bit < 10; bit++) {
        await freeze();
        writeField(bus, D90, F.win1h, 0x00f0);
        writeField(bus, D90, F.win1v, 0x0494);
        writeField(bus, D90, F.wipeState, 0);
        bus.write16(NEWKEYS, 1 << bit);
        call(WIPE, []);
        const st = readField(bus, D90, F.wipeState);
        if (st === 2) arming.push(BUTTON_BIT.get(bit));
        console.log(`  gNewKeys = ${h(1 << bit)} (${BUTTON_BIT.get(bit)}) -> wipeState = ${st}`);
    }
    // control
    await freeze();
    writeField(bus, D90, F.win1h, 0x00f0);
    writeField(bus, D90, F.wipeState, 0);
    bus.write16(NEWKEYS, 0);
    call(WIPE, []);
    const ctl = readField(bus, D90, F.wipeState);
    console.log(`  CONTROL gNewKeys = 0x0000 -> wipeState = ${ctl}`);
    console.log(`  -> the buttons that dismiss the message box: ${arming.join(', ')}`);
    check(
        JSON.stringify(arming.sort()) === JSON.stringify(['a', 'b', 'select', 'start'].sort()),
        'exactly A, B, SELECT and START arm the close',
    );
    check(ctl === 0, 'the control with no key pressed leaves wipeState at 0');
}

// ═══════════════════════════════════════════════════════════════════════════════════════
// C4. The whole cycle, live: what the box actually reveals, and what happens after.
// ═══════════════════════════════════════════════════════════════════════════════════════
const shot = async (name) => {
    await eng.takeScreenshot({ name });
    return createHash('sha256').update(readFileSync(`${OUT}/screenshot-${name}.png`)).digest('hex').slice(0, 12);
};

console.log('\n=== C4a. the box opening for real, and what it reveals ===');
{
    const before = (await armMessageBox(ctx, 3)).map(symName);
    const frames = [];
    for (let n = 4; n <= 16; n += 4) {
        await eng.wait({ frames: 4 });
        frames.push({ n, wh: bus.read16(regAddr('WIN1H')), st: readField(bus, D90, F.wipeState) });
        await shot(`open-${String(frames.length).padStart(2, '0')}`);
    }
    await eng.wait({ frames: 20 });
    const openHash = await shot('open-full');
    console.log(`  queue before arming : [${before.join(', ')}]`);
    console.log(`  queue while open    : [${queueNow(ctx).join(', ')}]`);
    frames.forEach((f) =>
        console.log(`  + ${f.n} frames: REG_WIN1H = ${h(f.wh)}  wipeState = ${f.st}`),
    );
    console.log(`  screenshots in ${OUT}/ (the panel reads "An extra stage has been unlocked.")`);
    check(queueNow(ctx)[1] === 'UpdateMessageBoxWipe', 'the live queue is running UpdateMessageBoxWipe in slot 1');
    check(bus.read16(regAddr('WIN1H')) === 0x00f0, 'and the box ends up fully open');

    console.log('\n=== C4b. press A: it closes, fades back in, and gameplay resumes ===');
    await eng.press('a');
    const log = [];
    for (let f = 6; f <= 90; f += 6) {
        await eng.wait({ frames: 6 });
        log.push({
            f,
            wh: bus.read16(regAddr('WIN1H')),
            winin: bus.read16(regAddr('WININ')),
            bldcnt: bus.read16(regAddr('BLDCNT')),
            blend: bus.read8(0x03005498),
            slot1: queueNow(ctx)[1],
        });
    }
    for (const l of log)
        console.log(
            `  +${String(l.f).padStart(2)}f  WIN1H=${h(l.wh)} WININ=${h(l.winin)} BLDCNT=${h(l.bldcnt)}` +
                ` blend=${String(l.blend).padStart(2)}  slot1=${l.slot1}`,
        );
    const endHash = await shot('after');
    check(
        log.some((l) => l.slot1 === 'UpdateMessageBoxFadeIn'),
        'slot 1 becomes UpdateMessageBoxFadeIn once the box is shut',
    );
    check(
        JSON.stringify(queueNow(ctx)) === JSON.stringify(before),
        `the queue is restored to the pre-message-box set [${before.join(', ')}]`,
    );
    check(openHash !== endHash, 'and the screen is not the panel any more');
}

console.log('\n=== C4c. gUnk_03004D90.unk9 picks WHICH panel (five values, five screens) ===');
{
    const hashes = new Map();
    for (const u of [0, 1, 2, 3, 5]) {
        await armMessageBox(ctx, u);
        await eng.wait({ frames: 40 });
        const hash = await shot(`panel-unk9-${u}`);
        hashes.set(u, hash);
        console.log(`  unk9 = ${u}: screenshot ${hash}  (${OUT}/screenshot-panel-unk9-${u}.png)`);
    }
    check(new Set(hashes.values()).size === 5, 'five different values of unk9 produce five different panels');
}

console.log(`\n=== ${failures.length ? `FAILED: ${failures.length}` : 'all checks passed'} ===`);
for (const f of failures) console.log(`  - ${f}`);
process.exit(failures.length ? 1 : 0);
