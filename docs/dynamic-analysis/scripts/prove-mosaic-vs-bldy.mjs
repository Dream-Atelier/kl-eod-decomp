// PROOF: 0x030007D8 is the MOSAIC size, not a BLDY fade level.
//
// The codebase names the SAME address two ways, and they contradict each other:
//     include/structs/variables.h:78   extern u8 gMosaicSize;          // 0x030007D8
//     include/gfx.h:80                 #define gBldyFadeLevel (*(u8 *)0x030007D8)
// Two call sites (src/gfx.c:404, src/code_1.c:1084) use the `gBldyFadeLevel`
// spelling; everything else uses `gMosaicSize`.  GameplayFrameInit (the function
// decompiled in this round) clears BOTH candidate globals in one statement:
//     gBlendValue = gMosaicSize = 0;                 // src/code_1.c
// so getting the name right matters for this function's readability.
//
// METHOD — separate the two globals by INTERVENTION, not by reading code:
//   VBlankCallback_Gameplay (src/code_0.c:2370) writes, every frame:
//       REG_BLDY   = gUnk_03005498;                             // 0x03005498
//       REG_MOSAIC = MOSAIC_SET(gMosaicSize, ... );              // 0x030007D8
//   So force each RAM byte to 0x0F, one at a time, and observe the HARDWARE
//   register the emulator actually receives + what the screen does.
//   A BLDY fade darkens uniformly; a MOSAIC pixelates. They cannot be confused.
import { HeadlessRuntime } from './gba-kit.mjs';
import { mkdirSync } from 'fs';
import { ROM, ELF, SAVES } from './_harness.mjs';

const OUT = '/tmp/klonoa-dyn-gfi';
mkdirSync(OUT, { recursive: true });
const rt = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const eng = rt.engine;
const bus = rt.gba.bus;
const di = eng.debugInfo;

// Addresses come from the ELF symbol table / DWARF macro table — no magic numbers.
const A_MOSAIC_RAM = di.symbolToAddress('gMosaicSize'); // 0x030007D8
const A_BLEND_RAM = di.symbolToAddress('gBlendValue'); // 0x03005498
const REG_MOSAIC = 0x0400004c;
const REG_BLDY = 0x04000054;
const hex = (n) => '0x' + (n >>> 0).toString(16);

console.log('symbol gMosaicSize  ->', hex(A_MOSAIC_RAM));
console.log('symbol gBlendValue  ->', hex(A_BLEND_RAM));
console.log('DWARF macro info in ELF:', di.hasMacroInfo, ' line info:', di.hasLineInfo);

await eng.loadState(`${SAVES}/savestate-in-level-idle.json`);
await eng.wait({ frames: 30 });

// Which C statement writes REG_MOSAIC / REG_BLDY? Ask the watchpoints + DWARF.
const wM = eng.watchMemory({ address: REG_MOSAIC, length: 2, maxHits: 8 });
const wY = eng.watchMemory({ address: REG_BLDY, length: 2, maxHits: 8 });
await eng.wait({ frames: 4 });
wM.stop();
wY.stop();
const src = (pc) => {
  const l = di.pcToSource(pc);
  const f = di.pcToFunction(pc);
  return l ? `${l.file.split('/').pop()}:${l.line} in ${l.func ?? f?.name ?? '?'}` : (f?.name ?? 'unknown');
};
const fmt = (w) =>
  [...new Set(w.hits.map((h) => `${hex(h.value)} @${hex(h.instructionAddress)}  ${src(h.instructionAddress)}`))]
    .slice(0, 3)
    .join('\n      ');
console.log('\nREG_MOSAIC writers:\n      ' + fmt(wM));
console.log('REG_BLDY writers:\n      ' + fmt(wY));

const shot = async (name) => {
  await eng.takeScreenshot({ name });
  return name;
};
const sample = () => ({
  mosaicRam: bus.read8(A_MOSAIC_RAM),
  blendRam: bus.read8(A_BLEND_RAM),
  regMosaic: eng.read16(REG_MOSAIC),
  regBldy: eng.read16(REG_BLDY),
});
const show = (tag) => {
  const s = sample();
  console.log(
    `${tag.padEnd(26)} RAM[0x030007D8]=${String(s.mosaicRam).padStart(2)}  RAM[0x03005498]=${String(s.blendRam).padStart(2)}` +
      `   REG_MOSAIC=${hex(s.regMosaic).padEnd(8)} REG_BLDY=${hex(s.regBldy)}`,
  );
  return s;
};

console.log('');
const base = show('baseline (idle in level)');
await shot('mosaic-baseline');
const hBase = eng.hashRegion(0, 0, 240, 160);

// ---- Intervention A: force RAM[0x030007D8] = 0x0F every frame -------------
eng.onFrame(() => bus.write8(A_MOSAIC_RAM, 0x0f));
await eng.wait({ frames: 8 });
const a = show('force 0x030007D8 = 15');
await shot('mosaic-forced-15');
const hA = eng.hashRegion(0, 0, 240, 160);
eng.onFrame(null);
bus.write8(A_MOSAIC_RAM, base.mosaicRam);
await eng.wait({ frames: 8 });

// ---- Intervention B: force RAM[0x03005498] = 0x0F every frame -------------
eng.onFrame(() => bus.write8(A_BLEND_RAM, 0x0f));
await eng.wait({ frames: 8 });
const b = show('force 0x03005498 = 15');
await shot('bldy-forced-15');
const hB = eng.hashRegion(0, 0, 240, 160);
eng.onFrame(null);

const MOSAIC_SET = (v) => (v | (v << 4) | (v << 8) | (v << 12)) >>> 0;
console.log('\n--- verdict -------------------------------------------------');
console.log(`forcing 0x030007D8=15 -> REG_MOSAIC ${hex(a.regMosaic)} (MOSAIC_SET(15)=${hex(MOSAIC_SET(15))})`,
            `| REG_BLDY ${hex(a.regBldy)} unchanged: ${a.regBldy === base.regBldy}`);
console.log(`forcing 0x03005498=15 -> REG_BLDY   ${hex(b.regBldy)}`,
            `| REG_MOSAIC ${hex(b.regMosaic)} back to baseline: ${b.regMosaic === base.regMosaic}`);
console.log('screen hash  baseline / mosaic-forced / bldy-forced:', hBase, hA, hB);
console.log('screenshots in', OUT);
