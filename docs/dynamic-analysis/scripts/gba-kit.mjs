// Resolves the gba-kit headless emulator used by the prove-*.mjs scripts.
//
// gba-kit lives outside this repo, so its location is configurable:
//   GBA_KIT           path to a built gba-kit checkout (default: this repo's sibling
//                     ../gba-kit — resolved against the REPO below, not against the
//                     current directory, so the scripts work from a worktree too)
//   GBA_KIT_SAVESTATE savestate the scripts resume from
//                     (default: $GBA_KIT/klonoa-analysis/savestate-in-level-idle.json)
//
// gba-kit must be built first (`pnpm build` in that checkout) — these scripts
// import its compiled output, not its sources.

import { resolve } from 'node:path';
import { pathToFileURL } from 'node:url';

// Repo root, so scripts can find baserom.gba / klonoa-eod.elf regardless of
// where they are run from. Override with KLONOA_ROOT.
export const REPO = process.env.KLONOA_ROOT ?? resolve(new URL('../../..', import.meta.url).pathname);

// Resolved against REPO rather than the current directory: `resolve('../gba-kit')` used
// cwd, so running a script from a build worktree looked for gba-kit beside THAT tree and
// died with ERR_MODULE_NOT_FOUND.
const root = resolve(REPO, process.env.GBA_KIT ?? '../gba-kit');
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
