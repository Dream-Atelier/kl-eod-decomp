// Shared setup for the two scripts that need the player to actually die.
//
// The death has to be a real one — the point of both experiments is that a game
// event drives the code, so poking the timer that gates it would assume the
// answer. From `savestate-after-moo-tracking.json` the player stands next to a
// Moo, and walking into it costs a heart; three hits empty the bar and the death
// sequence starts on its own.
import { dirname } from 'node:path';
import { HeadlessRuntime, REPO, SAVESTATE } from './gba-kit.mjs';

export const SAVES = dirname(SAVESTATE);

/** Player hearts. Not a named symbol in the decomp yet; see find-health.mjs. */
export const HEARTS = 0x030055c0;

/** A runtime with the ROM and the build's ELF, parked next to the Moo. */
export async function bootNextToMoo(outputDir) {
  const rt = await HeadlessRuntime.create({
    romPath: `${REPO}/klonoa-eod.gba`,
    elfPath: `${REPO}/klonoa-eod.elf`,
    outputDir,
    logFn: () => {},
  });
  await rt.engine.loadState(`${SAVES}/savestate-after-moo-tracking.json`);
  await rt.engine.wait({ frames: 4 });
  return rt;
}

/**
 * Walk into the Moo until the death sequence arms, and report the frame budget
 * it took. Throws rather than returning quietly if the player never dies — a
 * script that silently measured a live player would "prove" whatever it liked.
 */
export async function walkIntoMooUntilDead(eng, { rounds = 16 } = {}) {
  for (let i = 0; i < rounds; i++) {
    if (eng.readVariable('gUnk_03005220.deathSequenceTimer') !== 0) return i;
    await eng.pressSequence([
      ['right', 25],
      [null, 20],
    ]);
  }
  throw new Error(`player still alive after ${rounds} walks into the Moo — the savestate or the route moved`);
}
