# m4a — alignment with kleod's naming and C source

We share the same ROM (SHA1 `a0a298d9dba1ba15d04a42fc2eb35893d1a9569b`)
as [kleod](https://github.com/testyourmine/kleod), so functions at
identical ROM addresses are the same function — only labels and
implementation language differ.

## Status

This file documents the structural alignment with kleod and the
function-by-function porting progress on the `m4a-kleod-split` branch.

### Structural changes (Plan A — done)

- **`asm/m4a0.s`** — the hand-written Sappy/MP2K assembly blob
  (0x0804F284 → 0x0804FE9F) was copied verbatim from kleod and is
  `.include`d into `src/m4a.c`.  Same ROM ⇒ byte-identical.  This
  matches the file split used by kleod, pokeemerald, sa3.
- **`include/m4a_internal.h`** — header mirroring kleod's
  `include/gba/m4a_internal.h`, grown incrementally as each port
  needs new types/constants.  Currently defines:
    - `struct MP2KTrack` (full 0x00..0x2F field layout: status,
      keyShiftCalculated, pitchCalculated, keyShift, keyShiftPublic,
      tune, pitchPublic, bend, bendRange, volRightCalculated,
      volLeftCalculated, vol, volPublic, pan, panPublic,
      modCalculated, modDepth, modType, lfoSpeed, lfoSpeedCounter,
      chan, voicegroup, cmdPtr)
    - `struct MP2KPlayerState` (songHeader, status, trackCount,
      priority, cmd, checkSongPriority, clock, tempoRawBPM,
      tempoScale, tempoInterval, tempoCounter, fadeInterval,
      fadeCounter, fadeOV, tracks, voicegroup, lockStatus)
    - `struct MP2KVoiceGroup` (type)
    - `struct MP2KSongHeader` (trackCount, blockCount, priority,
      reverb, voicegroup, part[])
    - `struct MixerSource` (status, type, rightVol, leftVol,
      sustain, envelopeGoal, sustainGoal, pan, panMask)
    - `struct SoundMixerState` (lockStatus, freqOption,
      framesPerDmaCycle, maxScanlines, samplesPerFrame, sampleRate,
      sampleRateReciprocal, cgbChans, MPlayMainHead, musicPlayerHead,
      CgbSound, CgbOscOff, MidiKeyToCgbFreq function-pointer slots)
    - `struct WaveData` (type, status, freq, loopStart, size, data)
    - Constants: `ID_NUMBER`, `MPT_FLG_*`, `MUSICPLAYER_STATUS_*`,
      `FADE_VOL_SHIFT`, `FADE_IN`, `TEMPORARY_FADE`,
      `SOUND_MODE_REVERB_SET`, `PCM_DMA_BUF_SIZE`, `TIMER_ENABLE`,
      `TIMER_1CLK`, `REG_NR12/14/22/24/30/42/44/50` byte-level
      sound-channel register accessors, `SOUND_INFO_PTR` macro
- **`ldscript.in.txt`** — added ROM-data table addresses
  (`gMPlayJumpTableTemplate`, `gClockTable`, `gScaleTable`,
  `gFreqTable`, `gCgbScaleTable`, `gCgbFreqTable`, `gNoiseTable`,
  `gPcmSamplesPerVBlankTable`), plus the IWRAM `gMPlayJumpTable`
  pointer (0x03006450), `gMaxLines` (0).  6 `sub_<addr>` aliases for
  raw-name references from luvdis-emitted neighbouring `.s` into the
  m4a0 range.

### Port progress (40 → 23 INCLUDE_ASMs in m4a.c)

**Tier A — small per-track control helpers** (7 ports, all C-bodied):
1. `ClearModM` ← was `MidiUtilConvert` (0x080510B4)
2. `m4aMPlayTempoControl` ← `MPlayStateCheck1` (0x08050F4A)
3. `m4aMPlayVolumeControl` ← `MPlayStateCheck` (0x08050F70)
4. `m4aMPlayPitchControl` ← `MPlayStateCheck3` (0x08050FD8)
5. `m4aMPlayPanpotControl` ← `MidiNoteLookup` (0x0805104C)
6. `m4aMPlayModDepthSet` ← `MidiCommandEncode1` (0x080510D4)
   (also dropped hand-tuned register-asm hacks; clean kleod body
   produces matching bytes)
7. `m4aMPlayLFOSpeedSet` ← `MidiCommandEncode2` (0x08051148)

**Tier B — coupled fade family** (4 ports):
8. `MPlayContinue` + `MPlayFadeOut` ← `MPlayChannelReset` (single
   `.s` file holding two functions, both bodied as C)
9. `m4aMPlayFadeOut` ← `SappyStateVerify` (C-bodied, needed
   trailing `asm(".align 2, 0");` for missing 2-byte tail padding)
10. `m4aMPlayFadeOutTemporarily` ← `MPlayTrackVerify` (rename only)
11. `m4aMPlayFadeIn` ← `MPlayTrackFadeAndVerify` (rename only)

  Ports 10+11 are rename-only due to the **phantom-literal-pool
  blocker** (luvdis labels `MPlayTrackFadeAndVerify` 8 bytes too
  early, capturing the prior function's literal pool as fake
  instructions; C-porting the prior function emits its real pool
  and then INCLUDE_ASM emits the phantom pool again, shifting all
  downstream addresses).  A future generate_asm.py pass that strips
  the phantom prelude when the previous function is C-defined would
  unblock these.

**Tier C — song lookup wrappers** (6 ports, all rename-only):
12. `m4aSongNumStartOrChange` ← `m4aSongNumContinue` (0x0804FFF6)
13. `m4aSongNumStartOrContinue` ← `m4aSongNumLoad` (0x08050042)
14. `m4aSongNumContinue` ← our existing C `m4aSongNumStop` (0x080500C8)
15. `m4aSongNumStop` ← our existing C `m4aMPlayCommand` (0x08050094)
16. `m4aMPlayAllContinue` ← `StopSoundEffects` (0x08050134)
17. `m4aMPlayImmInit` ← `SoundEffectTrigger` (0x080501BA)

  Ports 14 ↔ 15 were a **3-way name swap** (our project had the
  names of `m4aSongNumStop` and `m4aSongNumContinue` semantically
  swapped relative to kleod).  Executed by sequencing the renames so
  each newly-claimed name was vacated by the preceding port.

**Tier D — bodies for already-(or-newly-)named functions** (8 ports):
18a. `CgbModVol` ← our `CgbOscOff` (0x08050A94) — name swap
18b. `CgbOscOff` ← `sub_08050A44` — name swap to free up the symbol
19. `CgbSound` ← `CgbChannelMix` (0x08050AFC) — rename only;
    body deferred (spans 0x08050AFC + 0x08050C70 = our existing
    "SoundMixerMain" region, since kleod's `CgbSound` is one big
    function ~250 lines)
20. `CgbOscOff` C body (0x08050A44) — switch over NRx2/NRx4 pairs
21. `MidiKeyToFreq` C body (0x0804FEA0) — needed
    `asm("bx lr\n.hword 0\n");` trailing trick to emit 4 dead bytes
    after the literal pool that agbcc doesn't generate naturally
22. `m4aMPlayImmInit` C body (0x080501BA)
23. `MidiKeyToCgbFreq` C body (0x0805099E)
24. `TrkVolPitSet` C body (0x080508E8)
25. `SampleFreqSet` C body (0x0805043C)
26. `FadeOutBody` C body (0x08050820)
27. `MPlayStart` C body (0x080506FC)
28. `MPlayExtender` C body ← `SoundHardwareInit` (0x08050200)
29. `SoundInit` ← `DirectSoundFifoSetup` (0x08050344) — rename only;
    body deferred (requires ~30 DMA/SOUNDCNT/SOUNDBIAS register
    constants and a `CpuFill32`-equivalent shim)
30. `CgbModVol` C body (0x08050A94)

Also a supporting rename:
- `m4aSoundMain` ← our existing wrapper `SoundInit` (0x0804FFBC).
  Required to free the `SoundInit` name for port 29.  Touched
  `math.c` and `globals.h`.

### Wrapper renames from earlier in this branch

The four "kleod name → our wrapper" ldscript aliases that we added
during Plan A's initial split have all been melted away by direct
TOML renames:
- `PlaySoundWithContext_D8` → `ClearChain` (0x0805031C)
- `PlaySoundWithContext_DC` → `Clear64byte` (0x08050330)
- `CgbModVol` ← our `CgbOscOff` (the misnamed one)
- `TrkVolPitSet` ← our `CgbSound` (the misnamed one)
- `FadeOutBody` ← our `CgbModVol` (the misnamed one)

## Still open

### Remaining post-range INCLUDE_ASMs in `src/m4a.c`

- `m4aMPlayFadeOutTemporarily` (0x08050172) — blocked by phantom
  literal pool in `MPlayTrackFadeAndVerify.s`
- `m4aMPlayFadeIn` (0x0805018A) — same blocker
- `SoundInit` (0x08050344) — needs DMA/SOUNDCNT/SOUNDBIAS register
  macros and a CpuFill32 shim (~30 new constants/defines)
- `CgbSound` (0x08050AFC) + `SoundMixerMain` (0x08050C70) — these
  are one function in kleod (~250 lines), would replace both
  INCLUDE_ASMs.  Largest single port remaining.
- `MPlayCommandDispatch` (0x080511BC) — likely `MP2K_event_memacc`
  in kleod; needs body verification
- `MPlayCmd_ReadU32Param` (0x08051348) — late-tail, kleod equivalent
  not yet confirmed

### Pre-range INCLUDE_ASMs (14 functions, 0x0804EB64 → 0x0804F283)

Kleod has none of these decompiled — they live in kleod's
`asm/code.s` as raw `sub_<addr>` blob.  Our project is **ahead** of
kleod in this range, so there's nothing kleod-side to adopt.  Future
decompilation work here would be original to this project.

### Generate_asm.py improvement (unblock Tier B remaining)

A pass that detects when a previous function is C-defined and strips
the phantom literal-pool prelude from the next INCLUDE_ASM's `.s`
file would unblock `m4aMPlayFadeOutTemporarily` and `m4aMPlayFadeIn`
(and likely future ports that hit the same pattern).
