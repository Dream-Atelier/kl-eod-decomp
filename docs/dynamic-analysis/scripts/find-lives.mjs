// PROOF: the extra-life count is gUnk_03005220.lives — a plain u8 at offset 0x4C
// (i.e. 0x0300526C). Unlike hearts/stars/dreamStones (packed bits in byte 0),
// lives is a whole byte, so a plain value scan works — and its run is the
// distinctive non-monotonic 3 -> 4 -> 5 -> 4 (two 1-up pickups, then a death).
//
// Dogfoods @gba-kit/debug-info: pass `elfPath`, then each watch hit carries
// `.location` (the writer's C func/file:line), naming the gain/lose writers.
//
// Run from a fresh boot. The input sequence (from the user) has three marked
// life changes; we split it at those markers (matched by surrounding context so
// we don't depend on duplicate-value anchors), snapshot at each, find the byte
// reading [3,4,5,4], then watch it to name the writers.
import { HeadlessRuntime, REPO } from './gba-kit.mjs';
import { mkdirSync } from 'fs';
const OUT = '/tmp/klonoa-dynamic-out';
mkdirSync(OUT, { recursive: true });
const ROM = `${REPO}/baserom.gba`;
const ELF = `${REPO}/klonoa-eod.elf`;

// The user's input, with the three life-change markers (the change lands a few
// frames AFTER each commented line).
const SEQ = [
  [null,387],['a',6],[null,10],['a',3],[null,9],['a',3],[null,26],['a',6],[null,13],['a',5],[null,58],['a',6],[null,7],['a',5],[null,96],
  ['start',6],[null,66],['a',8],[null,48],['a',7],[null,14],['start',7],[null,21],['a',7],[null,28],['a',5],[null,26],['a',6],[null,112],
  ['right',82],['b+right',5],['right',94],['a+right',8],['right',38],['a+right',3],['right',41],['a+right',7],['right',55],['a+right',12],['right',19],['a+right',6],['right',105],['b+right',5],['right',73],['a+right',8],['right',9],['a+right',8],['right',53],['a+right',8],['right',178],['a+right',27],['right',43],['right+left',1],['left',2],['right+left',2],['right',19],['a+right',6],['right',24],['right+left',1],['left',4],[null,13],['a',5],[null,3],['b',6],[null,8],['a',7],[null,15],['b',5],[null,16],['a',9],[null,6],['a',9],[null,11],
  ['right',155],['a+right',6],['right',35],['a+right',7],['right',35],['a+right',8],['right',37],['a+right',12],['right',7],['b+right',8],['right',1],['left',1],['a+left',14],['left',18],['right+left',2],['right',263],['b+right',6],['right',93],['a+right',12],['right',25],['a+right',9],['right',56],['a+right',6],['right',41],['a+right',8],['right',59],['a+right',9],['right',55],['a+right',7],['right',8],['b+right',6],['right',57],['a+right',5],['right',53],[null,4],['a',7],[null,10],['a+right',90],['right',12],[null,14],['left',8],[null,30],['left',4],[null,19],['b',6],[null,7],['right',36],['a+right',5],['right',61],['a+right',9],['right',46],['a+right',6],['right',16],['a+right',7],['right',37],['right+left',1],['left',61],['a+left',7],['left',69],['a+left',8],['left',69],['b+left',6],['left',26],['right+left',2],['right',11],['b+right',5],['right',78],['a+right',6],['right',66],['up',8],[null,391],['right',8],[null,38],['a',6],[null,85],['right',29],[null,2],['left',16],['right+left',1],['right',18],['b+right',6],['right',98],['a+right',5],['right',42],['a+right',11],['right',8],['a+right',7],['right',96],[null,1],['left',16],['right+left',1],['right',6],['b+right',8],['right',13],['a+right',9],['right',44],['b+right',1],['right',14],['b+right',2],['right',4],['right+left',1],['left',48],['b+left',7],['left',14],['right',33],['a+right',9],['right',36],['a+right',7],['right',34],[null,3],['a',7],[null,10],['a',7],[null,16],['right',312],['right+left',2],['left',6],['b+left',6],['left',8],['a+left',8],['left',32],['a+left',6],['left',21],[null,5],['right',16],['a+right',12],['right',21],[null,1],['left',8],['a+left',7],['left',20],[null,17],['a',5],[null,15],['left',12],[null,11],['right',6],['a+right',10],['right',27],['a+right',11],['right',45],['a+right',5],['right',5],[null,6],
  ['right',23], // gain one life (3 -> 4)
  [null,4],['left',14],[null,262],['left',76],['right+left',1],['right',11],['right+left',2],['left',24],['right+left',2],['right',40],['right+left',2],['left',9],['b+left',6],['left',16],['a+left',6],['left',29],[null,7],['a',8],[null,12],['left',6],[null,25],['a',10],[null,9],['a',12],['left',42],['a+left',6],['left',36],['a+left',5],['left',15],[null,10],['left',5],['a+left',6],['left',1],['b+left',5],['left',23],['right+left',1],['right',10],['a+right',14],['right',8],['a+right',8],['right',55],['a+right',10],['right',83],['left',10],['right+left',2],['right',19],['b+right',6],['right',60],['a+right',6],['right',43],['a+right',14],['right',10],['a+right',12],['right',414],['b+right',7],['right',72],['a+right',9],['right',34],['a+right',1],['a',8],[null,7],['a',11],[null,14],['left',29],['right+left',2],['right',14],['b+right',7],['right',11],['right+left',1],['left',12],['a+left',7],['left',10],['a+left',14],['left',36],['b+left',8],['left',32],['a+left',6],['left',11],['a+left',8],['left',135],['a+left',7],['left',40],['up',9],[null,386],['right',9],[null,31],['a',7],[null,91],['right',64],['a+right',12],['right',36],['a+right',12],['right',93],['left',8],['b+left',5],['right+left',1],['right',127],['a+right',3],['a',7],[null,8],['left',1],['a+left',14],['left',84],['b+left',6],['left',2],['right',6],['a+right',14],['right',46],['a+right',9],['right',77],['a+right',11],['right',30],['a+right',6],['right',6],['a+right',10],['right',31],['right+left',3],['left',35],['right+left',2],['right',154],['a+right',7],['right',36],['a+right',12],['right',122],['right+left',1],['left',39],['right+left',3],['right',6],[null,30],['a',14],[null,1],['b',4],['b+right',2],['right',21],[null,11],['a',7],[null,7],['a',13],[null,7],['right',33],[null,4],['left',8],['a+left',7],['left',2],['b+left',7],['left',6],['a+left',17],['left',17],[null,14],['left',10],[null,6],['right',10],['b',5],[null,11],['left',5],[null,232],['a',6],[null,5],['a',3],['a+left',5],
  ['left',20], // gain one life (4 -> 5)
  [null,4],['right',15],[null,139],['a+right',7],['right',4],[null,20],['right',6],[null,59],['right',6],[null,97],['a',18],[null,48],['left',12],[null,1],['right',24],[null,18],['left',9],[null,18],['right',5],[null,16],['left',3],[null,59],['a',16],[null,31],['left',21],['right',25],[null,1],['left',10],[null,13],['right',3],
  [null,114], // lost 1 life (5 -> 4)
  ['a',12],[null,473],
];

// Split at the three marked changes, matched by surrounding context.
const eq = (e, b, f) => e !== undefined && e[0] === b && e[1] === f;
const i1 = SEQ.findIndex((e, i) => eq(e, 'right', 23) && eq(SEQ[i + 1], null, 4) && eq(SEQ[i + 2], 'left', 14));
const i2 = SEQ.findIndex((e, i) => eq(e, 'left', 20) && eq(SEQ[i - 1], 'a+left', 5) && eq(SEQ[i + 1], null, 4));
const i3 = SEQ.findIndex((e, i) => eq(e, null, 114) && eq(SEQ[i - 1], 'right', 3) && eq(SEQ[i + 1], 'a', 12));
const A = SEQ.slice(0, i1), B = SEQ.slice(i1, i2), C = SEQ.slice(i2, i3), D = SEQ.slice(i3);

// --- Pass 1: snapshot at each change, find the byte reading [3,4,5,4] ---
const rt1 = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const bus1 = rt1.gba.bus;
const snap = () => { const m = new Uint8Array(0x8000); for (let k = 0; k < 0x8000; k++) m[k] = bus1.read8(0x03000000 + k); return m; };
await rt1.engine.pressSequence(A); const S0 = snap();
await rt1.engine.pressSequence(B); const S1 = snap();
await rt1.engine.pressSequence(C); const S2 = snap();
await rt1.engine.pressSequence(D); const S3 = snap();
const found = [];
for (let k = 0; k < 0x8000; k++) if (S0[k] === 3 && S1[k] === 4 && S2[k] === 5 && S3[k] === 4) found.push(0x03000000 + k);
console.log('bytes reading [3,4,5,4]:', found.map((a) => '0x' + a.toString(16)).join(', '));
const LIVES = found[0]; // 0x0300526c == gUnk_03005220.lives (offset 0x4C)

// --- Pass 2: watch that byte, name the writers via debug info ---
const rt2 = await HeadlessRuntime.create({ romPath: ROM, elfPath: ELF, outputDir: OUT, logFn: () => {} });
const bus2 = rt2.gba.bus, eng2 = rt2.engine, r8 = (a) => bus2.read8(a);
await eng2.pressSequence(A);
console.log('\nlives @0x' + LIVES.toString(16), 'after entering level:', r8(LIVES), ' hasDebugInfo:', eng2.hasDebugInfo);
let last = r8(LIVES); const evs = [];
const w = eng2.watchMemory({ address: LIVES, length: 1, filter: (h) => {
  const n = r8(LIVES); if (n !== last) { evs.push({ from: last, to: n, pc: h.instructionAddress, loc: h.location }); last = n; }
  return false;
}});
await eng2.pressSequence(B); console.log('after "gain (3->4)":', r8(LIVES));
await eng2.pressSequence(C); console.log('after "gain (4->5)":', r8(LIVES));
await eng2.pressSequence(D); console.log('after "lost (5->4)":', r8(LIVES));
w.stop();
console.log('\nlives changes (debug-info annotated writer):');
for (const e of evs) {
  const loc = e.loc ? `${e.loc.func} ${e.loc.file.replace(/^.*\/src\//, 'src/')}:${e.loc.line}` : '(no source)';
  console.log(`  ${e.from} -> ${e.to}  by 0x${e.pc.toString(16)}  ${loc}`);
}
await eng2.takeScreenshot({ name: 'lives-4remaining' });
