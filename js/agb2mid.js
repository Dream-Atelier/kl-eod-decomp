/**
 * M4A (GBA MusicPlayer2000 / Sappy) to Standard MIDI File converter.
 * Produces SMF Format 0, 24 ticks per quarter note.
 */

import { readU32, readU8, SONG_TABLE } from './rom-tables.js';

// Duration lookup table (49 entries, indices 0-48)
const LEN_TBL = [
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 28, 30, 32, 36, 40, 42, 44,
    48, 52, 54, 56, 60, 64, 66, 68, 72, 76, 78, 80, 84, 88, 90, 92, 96,
];

const TICKS_PER_BEAT = 24;

/** Write a MIDI variable-length quantity into an array. */
function writeVLQ(arr, value) {
    if (value < 0) value = 0;
    const bytes = [];
    bytes.push(value & 0x7F);
    value >>>= 7;
    while (value > 0) {
        bytes.push((value & 0x7F) | 0x80);
        value >>>= 7;
    }
    for (let i = bytes.length - 1; i >= 0; i--) arr.push(bytes[i]);
}

/** Write a big-endian 32-bit integer. */
function writeU32BE(arr, v) {
    arr.push((v >>> 24) & 0xFF, (v >>> 16) & 0xFF, (v >>> 8) & 0xFF, v & 0xFF);
}

/** Write a big-endian 16-bit integer. */
function writeU16BE(arr, v) {
    arr.push((v >>> 8) & 0xFF, v & 0xFF);
}

/**
 * Convert a song from the ROM to a Standard MIDI File (Uint8Array).
 * @param {DataView} dv  DataView over the ROM
 * @param {Uint8Array} rom  Uint8Array of the ROM
 * @param {number} songIndex  Song index (0-based into SONG_TABLE)
 * @returns {Uint8Array}  SMF bytes
 */
export function convertSong(dv, rom, songIndex) {
    // Read song pointer from table
    const songPtr = readU32(dv, SONG_TABLE + songIndex * 8);
    const songOff = songPtr - 0x08000000;

    // Song header
    const numTracks = rom[songOff];
    // const blockCount = rom[songOff + 1]; // unused for MIDI
    // const priority = dv.getUint16(songOff + 2, true); // unused
    // const voiceGroupPtr = readU32(dv, songOff + 4); // unused for MIDI

    // Read track pointers
    const trackPtrs = [];
    for (let t = 0; t < numTracks; t++) {
        const ptr = readU32(dv, songOff + 8 + t * 4);
        trackPtrs.push(ptr - 0x08000000);
    }

    // Parse all tracks, collecting timed MIDI events
    const events = []; // { tick, bytes[] }

    for (let ch = 0; ch < numTracks; ch++) {
        parseTrack(rom, dv, trackPtrs[ch], ch, events);
    }

    // Sort events by tick (stable: preserve insertion order for same tick)
    events.sort((a, b) => a.tick - b.tick);

    // Build MIDI track data
    const trackData = [];

    // Optional: Add a text marker
    const marker = 'Converted by kl-eod-decomp';
    trackData.push(0x00); // delta=0
    trackData.push(0xFF, 0x06); // text event
    writeVLQ(trackData, marker.length);
    for (let i = 0; i < marker.length; i++) trackData.push(marker.charCodeAt(i));

    // Write events as delta-time + MIDI bytes
    let lastTick = 0;
    for (const ev of events) {
        const delta = ev.tick - lastTick;
        writeVLQ(trackData, delta);
        for (const b of ev.bytes) trackData.push(b);
        lastTick = ev.tick;
    }

    // End of track
    writeVLQ(trackData, 0);
    trackData.push(0xFF, 0x2F, 0x00);

    // Build SMF
    const midi = [];
    // MThd
    midi.push(0x4D, 0x54, 0x68, 0x64); // 'MThd'
    writeU32BE(midi, 6);                 // header length
    writeU16BE(midi, 0);                 // format 0
    writeU16BE(midi, 1);                 // 1 track
    writeU16BE(midi, TICKS_PER_BEAT);    // division

    // MTrk
    midi.push(0x4D, 0x54, 0x72, 0x6B); // 'MTrk'
    writeU32BE(midi, trackData.length);
    for (const b of trackData) midi.push(b);

    return new Uint8Array(midi);
}

/**
 * Parse one m4a track and append timed MIDI events.
 *
 * Key m4a parsing rule: when a byte < 0x80 appears at the command position,
 * it is NOT a new command — it repeats the last note command (Nxx/TIE/EOT)
 * using the byte (and any subsequent bytes < 0x80) as new arguments.
 */
function parseTrack(rom, dv, startOff, channel, events) {
    let pos = startOff;
    let tick = 0;
    let lastKey = 60;
    let lastVel = 127;
    let keyShift = 0;
    let lastNoteCmd = -1; // last Nxx/TIE/EOT command byte for repeats

    // Subroutine return address (single-level)
    let returnPos = -1;
    let returnFlag = false;

    // Active notes: Map<key, true>
    const activeNotes = new Map();

    // Set of visited GOTO targets to prevent infinite loops
    const visitedGotos = new Set();

    // Emit initial reverb CC (many m4a games use reverb)
    events.push({ tick: 0, bytes: [0xB0 | channel, 0x5B, 0x47] });

    const maxIterations = 100000;
    let iterations = 0;

    /** Emit a Nxx note-on with auto-duration. */
    function emitNxx(duration) {
        let key = lastKey, vel = lastVel, gateExtra = 0;
        if (pos < rom.length && rom[pos] < 0x80) { key = rom[pos++]; lastKey = key; }
        if (pos < rom.length && rom[pos] < 0x80) { vel = rom[pos++]; lastVel = vel; }
        if (pos < rom.length && rom[pos] < 0x80) { gateExtra = rom[pos++]; }

        const midiKey = (key + keyShift) & 0x7F;
        const midiVel = vel & 0x7F;
        const totalDur = duration + gateExtra;

        if (activeNotes.has(midiKey)) {
            events.push({ tick, bytes: [0x80 | channel, midiKey, 0] });
            activeNotes.delete(midiKey);
        }
        events.push({ tick, bytes: [0x90 | channel, midiKey, midiVel] });
        events.push({ tick: tick + totalDur, bytes: [0x80 | channel, midiKey, 0] });
    }

    /** Emit a TIE note-on (indefinite duration). */
    function emitTie() {
        let key = lastKey, vel = lastVel;
        if (pos < rom.length && rom[pos] < 0x80) { key = rom[pos++]; lastKey = key; }
        if (pos < rom.length && rom[pos] < 0x80) { vel = rom[pos++]; lastVel = vel; }

        const midiKey = (key + keyShift) & 0x7F;
        const midiVel = vel & 0x7F;

        if (activeNotes.has(midiKey)) {
            events.push({ tick, bytes: [0x80 | channel, midiKey, 0] });
        }
        events.push({ tick, bytes: [0x90 | channel, midiKey, midiVel] });
        activeNotes.set(midiKey, true);
    }

    /** Emit an EOT note-off. */
    function emitEot() {
        let key = lastKey;
        if (pos < rom.length && rom[pos] < 0x80) { key = rom[pos++]; lastKey = key; }
        const midiKey = (key + keyShift) & 0x7F;
        if (activeNotes.has(midiKey)) {
            events.push({ tick, bytes: [0x80 | channel, midiKey, 0] });
            activeNotes.delete(midiKey);
        }
    }

    function endTrack() {
        for (const [key] of activeNotes) {
            events.push({ tick, bytes: [0x80 | channel, key, 0] });
        }
        activeNotes.clear();
    }

    while (iterations++ < maxIterations) {
        if (pos >= rom.length) break;
        const cmd = rom[pos];

        // Bytes < 0x80: repeat the last note command with new arguments
        if (cmd < 0x80) {
            if (lastNoteCmd >= 0xD0 && lastNoteCmd <= 0xFF) {
                // Don't consume the first byte — emitNxx reads args from pos
                emitNxx(LEN_TBL[lastNoteCmd - 0xD0]);
            } else if (lastNoteCmd === 0xCF) {
                emitTie();
            } else if (lastNoteCmd === 0xCE) {
                emitEot();
            } else {
                // No previous note command; skip byte
                pos++;
            }
            continue;
        }

        pos++; // consume the command byte

        if (cmd >= 0x80 && cmd <= 0xB0) {
            tick += LEN_TBL[cmd - 0x80];
            continue;
        }

        if (cmd >= 0xD0 && cmd <= 0xFF) {
            lastNoteCmd = cmd;
            emitNxx(LEN_TBL[cmd - 0xD0]);
            continue;
        }

        switch (cmd) {
            case 0xB1: endTrack(); return;

            case 0xB2: {
                const ptr = readU32(dv, pos) - 0x08000000;
                pos += 4;
                if (visitedGotos.has(ptr)) { endTrack(); return; }
                visitedGotos.add(ptr);
                pos = ptr;
                continue;
            }

            case 0xB3: {
                const ptr = readU32(dv, pos) - 0x08000000;
                pos += 4;
                returnPos = pos;
                returnFlag = true;
                pos = ptr;
                continue;
            }

            case 0xB4:
                if (returnFlag) { pos = returnPos; returnFlag = false; }
                continue;

            case 0xBB: {
                const tempoVal = rom[pos++];
                const usPerBeat = Math.round(60000000 / (tempoVal * 2));
                events.push({
                    tick,
                    bytes: [0xFF, 0x51, 0x03,
                            (usPerBeat >>> 16) & 0xFF,
                            (usPerBeat >>> 8) & 0xFF,
                            usPerBeat & 0xFF],
                });
                continue;
            }

            case 0xBC: {
                const val = rom[pos++];
                keyShift = (val > 127) ? val - 256 : val;
                continue;
            }

            case 0xBD: events.push({ tick, bytes: [0xC0 | channel, rom[pos++] & 0x7F] }); continue;
            case 0xBE: events.push({ tick, bytes: [0xB0 | channel, 0x07, rom[pos++] & 0x7F] }); continue;
            case 0xBF: events.push({ tick, bytes: [0xB0 | channel, 0x0A, rom[pos++] & 0x7F] }); continue;

            case 0xC0: {
                const bend = rom[pos++];
                const midiVal = Math.round(((bend - 0x40) / 0x40) * 0x2000 + 0x2000);
                const clamped = Math.max(0, Math.min(0x3FFF, midiVal));
                events.push({ tick, bytes: [0xE0 | channel, clamped & 0x7F, (clamped >>> 7) & 0x7F] });
                continue;
            }

            case 0xC1: {
                const range = rom[pos++];
                events.push({ tick, bytes: [0xB0 | channel, 0x65, 0x00] });
                events.push({ tick, bytes: [0xB0 | channel, 0x64, 0x00] });
                events.push({ tick, bytes: [0xB0 | channel, 0x06, range & 0x7F] });
                continue;
            }

            case 0xC2: pos++; continue; // LFOS
            case 0xC3: pos++; continue; // LFODL

            case 0xC4:
                events.push({ tick, bytes: [0xB0 | channel, 0x01, rom[pos++] & 0x7F] });
                continue;

            case 0xC5: pos++; continue; // MODT

            case 0xC8: {
                const tune = rom[pos++];
                events.push({ tick, bytes: [0xB0 | channel, 0x65, 0x00] });
                events.push({ tick, bytes: [0xB0 | channel, 0x64, 0x01] });
                events.push({ tick, bytes: [0xB0 | channel, 0x06, tune & 0x7F] });
                continue;
            }

            case 0xCE: lastNoteCmd = cmd; emitEot(); continue;
            case 0xCF: lastNoteCmd = cmd; emitTie(); continue;

            default:
                // Unknown commands: 0xB5-0xBA take 0 args, 0xC6-0xC7/0xC9-0xCD take 1 arg
                if (cmd >= 0xB5 && cmd <= 0xBA) continue;
                if ((cmd >= 0xC6 && cmd <= 0xC7) || (cmd >= 0xC9 && cmd <= 0xCD)) { pos++; continue; }
                continue;
        }
    }

    endTrack();
}
