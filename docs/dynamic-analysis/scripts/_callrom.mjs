// Shared "run a real ROM function on the real CPU" helper for the prove-*.mjs
// scripts, factored out of prove-button-wait.mjs / prove-gfxstream-motion-fields.mjs
// (both had their own copy).
//
// Why direct calls at all: the gfx-stream dispatcher (ProcessAnimationSteps,
// 0x0804C8F4) and the script executor (StreamCmd_RunScript, 0x0804EA94) do not run
// in any savestate reachable by idling in a level, so "play until the game does it"
// observes nothing. Instead we load a real, fully-initialised savestate, stop
// advancing frames (so no interrupt, VBlank handler or game logic can touch the
// state under test), and single-step the cartridge's own Thumb code with r0/lr/pc
// set by hand. Every number a caller prints is then produced by ARM instructions
// out of baserom.gba.
//
// The sentinel return address is 0x08000000 (the ROM header, never executed), so a
// function returning to it is unambiguous.

export const SENTINEL = 0x08000000;

/**
 * Build a caller bound to one emulator CPU.
 * @param cpu rt.gba.armCpu
 */
export function makeCaller(cpu) {
    /**
     * Call a Thumb ROM function. Registers and CPSR are saved and restored, so the
     * emulator is left exactly as it was found — only the memory the callee wrote
     * persists, which is the point.
     *
     * @param addr  function entry (thumb bit optional)
     * @param args  up to four u32 arguments (r0..r3)
     * @param maxSteps instruction budget; exceeding it is reported, never silently ignored
     * @returns {{ret:number, steps:number, timedOut:boolean}}
     */
    return function callThumb(addr, args = [], maxSteps = 300000) {
        if (addr == null) throw new Error('callThumb: null address (a symbol lookup returned null)');
        const saved = Array.from(cpu.registers);
        const savedCpsr = cpu.cpsr;
        for (let i = 0; i < 4; i++) cpu.registers[i] = (args[i] ?? 0) >>> 0;
        cpu.registers[14] = SENTINEL | 1;
        cpu.registers[15] = (addr & ~1) >>> 0;
        cpu.cpsr = (cpu.cpsr | 0x20) >>> 0; // Thumb
        let n = 0;
        let timedOut = true;
        for (; n < maxSteps; n++) {
            const pc = cpu.registers[15] >>> 0;
            if (pc >= SENTINEL && pc < SENTINEL + 0x10) {
                timedOut = false;
                break;
            }
            cpu.step();
        }
        const ret = cpu.registers[0] >>> 0;
        for (let i = 0; i < 16; i++) cpu.registers[i] = saved[i];
        cpu.cpsr = savedCpsr;
        return { ret, steps: n, timedOut };
    };
}

/** Resolve a linker/ELF symbol, refusing the silent `null >>> 0 === 0` read-at-zero. */
export function mustSymbol(di, name) {
    const a = di.symbolToAddress(name);
    if (a == null) throw new Error(`symbol "${name}" did not resolve — it would have read memory at 0`);
    return a >>> 0;
}
