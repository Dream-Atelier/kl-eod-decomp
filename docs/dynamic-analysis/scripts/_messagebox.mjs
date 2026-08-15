// Shared setup for the two message-box proofs (prove-message-box-wipe.mjs and
// prove-message-box-fade-in.mjs).
//
// The message box cannot be reached by idling in any shipped savestate: it is armed from
// three sites in InitGfxState and from UpdatePlayerInput / PlayerMovementPhysics, all of them
// gated on a progress bit that the savestates have already consumed. So instead of waiting
// for the game to do it, this replays what the ROM does at the arming site — InitGfxState's
// tail at 0x08048220 — and then simply lets the game run. Everything after the last line of
// `armMessageBox` is the cartridge's own code: InitFadeTransition builds the panel,
// UpdateMessageBoxWipe opens it, UpdateMessageBoxFadeIn ends it.
//
// The arming sequence, straight off that disassembly:
//     for (i = 0; i < 10; i++) gCallbackQueue.previous[i] = gCallbackQueue.current[i];
//     gCallbackQueue.previousCount = gCallbackQueue.currentCount;
//     gUnk_030051F0.unkE = gBlendValue;
//     gUnk_030051F0.unk4 = REG_BLDCNT;   .unk6 = REG_BG0CNT;  .unk8 = REG_BG1CNT;
//     gUnk_030051F0.unkA = REG_BG2CNT;   .unkC = REG_BG3CNT;
//     gUnk_030051F0.unk0 = gUnk_03004C20.sceneFrameCounter;
//     gUnk_03004D90.wipeState = 1;  gUnk_03004D90.unk9 = <variant>;  gBlendValue = 0;
//     gCallbackQueue.next[0] = InitFadeTransition;
//     gCallbackQueue.next[1] = VBlankCallback_Dialog;
//     gCallbackQueue.next[2] = NULL + 1;
//     gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
//     gCallbackQueue.nextCount = 3;
//
// Both function pointers are resolved through the ELF, and the gUnk_030051F0 member offsets
// are the ones src/code_3.c uses by name, so a rename breaks this instead of silently
// writing the wrong cell.

import { SAVES, writeField } from './_harness.mjs';
import { mustSymbol } from './_callrom.mjs';

/** Offsets of the three parallel arrays and their counts inside struct CallbackQueue. */
export const QUEUE_LAYOUT = (di) => ({
    current: di.structMember('CallbackQueue', 'current').offset,
    next: di.structMember('CallbackQueue', 'next').offset,
    previous: di.structMember('CallbackQueue', 'previous').offset,
    currentCount: di.structMember('CallbackQueue', 'currentCount').offset,
    nextCount: di.structMember('CallbackQueue', 'nextCount').offset,
    previousCount: di.structMember('CallbackQueue', 'previousCount').offset,
});

/**
 * Put a live, running gameplay state one frame away from popping the message box.
 *
 * @param ctx  { eng, bus, di } from the HeadlessRuntime
 * @param unk9 value for gUnk_03004D90.unk9 — which panel InitFadeTransition builds
 * @returns    the callback set that was running before, so a caller can check it comes back
 */
export async function armMessageBox({ eng, bus, di }, unk9) {
    const Q = mustSymbol(di, 'gCallbackQueue');
    const L = QUEUE_LAYOUT(di);
    const D90 = mustSymbol(di, 'gUnk_03004D90');
    const P = mustSymbol(di, 'gUnk_030051F0');
    const C20 = mustSymbol(di, 'gUnk_03004C20');
    const BLEND = mustSymbol(di, 'gBlendValue');
    const wipeState = di.structMember('Unk_03004D90', 'wipeState');
    const REG = { BLDCNT: 0x04000050, BG0CNT: 0x04000008, BG1CNT: 0x0400000a, BG2CNT: 0x0400000c, BG3CNT: 0x0400000e };

    await eng.loadState(`${SAVES}/savestate-in-gameplay.json`);
    await eng.wait({ frames: 4 });

    const before = [];
    for (let i = 0; i < bus.read8(Q + L.currentCount); i++) before.push(bus.read32(Q + L.current + i * 4) >>> 0);

    for (let i = 0; i < 10; i++) bus.write32(Q + L.previous + i * 4, bus.read32(Q + L.current + i * 4));
    bus.write8(Q + L.previousCount, bus.read8(Q + L.currentCount));

    bus.write8(P + di.structMember('Unk_030051F0', 'unkE').offset, bus.read8(BLEND));
    bus.write16(P + di.structMember('Unk_030051F0', 'unk4').offset, bus.read16(REG.BLDCNT));
    bus.write16(P + di.structMember('Unk_030051F0', 'unk6').offset, bus.read16(REG.BG0CNT));
    bus.write16(P + di.structMember('Unk_030051F0', 'unk8').offset, bus.read16(REG.BG1CNT));
    bus.write16(P + di.structMember('Unk_030051F0', 'unkA').offset, bus.read16(REG.BG2CNT));
    bus.write16(P + di.structMember('Unk_030051F0', 'unkC').offset, bus.read16(REG.BG3CNT));
    bus.write32(
        P + di.structMember('Unk_030051F0', 'unk0').offset,
        bus.read32(C20 + di.structMember('Unk_03004C20', 'sceneFrameCounter').offset),
    );

    writeField(bus, D90, wipeState, 1);
    bus.write8(D90 + 9, unk9);
    bus.write8(BLEND, 0);

    bus.write32(Q + L.next + 0, mustSymbol(di, 'InitFadeTransition') | 1);
    bus.write32(Q + L.next + 4, mustSymbol(di, 'VBlankCallback_Dialog') | 1);
    bus.write32(Q + L.next + 8, 1);
    bus.write32(Q + L.current + (bus.read8(Q + L.currentCount) - 1) * 4, 0);
    bus.write8(Q + L.nextCount, 3);

    return before;
}

/** The callback pointers the queue is running right now, as symbol names. */
export function queueNow({ eng, bus, di }) {
    const Q = mustSymbol(di, 'gCallbackQueue');
    const L = QUEUE_LAYOUT(di);
    const out = [];
    for (let i = 0; i < bus.read8(Q + L.currentCount); i++) {
        const p = bus.read32(Q + L.current + i * 4) >>> 0;
        const s = eng.addressToSymbol(p & ~1);
        out.push(s && s.offset === 0 ? s.name : '0x' + p.toString(16).padStart(8, '0'));
    }
    return out;
}
