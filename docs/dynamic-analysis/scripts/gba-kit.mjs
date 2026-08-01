// Resolves the gba-kit headless emulator used by the prove-*.mjs scripts.
//
// gba-kit lives outside this repo, so its location is configurable:
//   GBA_KIT           path to a built gba-kit checkout (default: ../gba-kit)
//   GBA_KIT_SAVESTATE savestate the scripts resume from
//                     (default: $GBA_KIT/klonoa-analysis/savestate-in-level-idle.json)
//
// gba-kit must be built first (`pnpm build` in that checkout) — these scripts
// import its compiled output, not its sources.

import { resolve } from 'node:path';
import { pathToFileURL } from 'node:url';

const root = resolve(process.env.GBA_KIT ?? '../gba-kit');
const entry = pathToFileURL(`${root}/packages/gba-node/dist/index.js`).href;

let mod;
try {
    mod = await import(entry);
} catch (cause) {
    throw new Error(
        `Could not load gba-kit from ${root}. Set GBA_KIT to a built gba-kit checkout.`,
        { cause },
    );
}

export const { HeadlessRuntime } = mod;

export const SAVESTATE =
    process.env.GBA_KIT_SAVESTATE ?? `${root}/klonoa-analysis/savestate-in-level-idle.json`;

// Repo root, so scripts can find baserom.gba / klonoa-eod.elf regardless of
// where they are run from. Override with KLONOA_ROOT.
export const REPO = process.env.KLONOA_ROOT ?? resolve(new URL('../../..', import.meta.url).pathname);
