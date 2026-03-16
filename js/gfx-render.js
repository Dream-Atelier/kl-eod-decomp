/**
 * GBA 4bpp tile decode, tilemap compose, palette parse, canvas output.
 *
 * On GBA hardware, all BG layers share a single VRAM tile space. The game
 * loads tile data from all layers sequentially into VRAM, and each layer's
 * tilemap references tiles by absolute position in this combined space.
 * We replicate this by building a unified tile buffer from all 3 layers.
 */

import { readU16, readU32, romPtr, BG_TILE_TABLE, BG_TILEMAP_TABLE, BG_PALETTE_TABLE,
         BG_TILE_COUNT, BG_PALETTE_COUNT } from './rom-tables.js';
import { decompressAsset } from './decompress.js';

/** Convert GBA RGB555 (16-bit) to [r, g, b] array. */
function rgb555to888(c16) {
    return [
        (c16 & 0x1F) << 3,
        ((c16 >>> 5) & 0x1F) << 3,
        ((c16 >>> 10) & 0x1F) << 3,
    ];
}

/**
 * Parse GBA RGB555 palette bank into flat RGBA array.
 * @param {Uint8Array} data  raw palette bytes (32 bytes = 16 colors)
 * @param {number} numColors
 * @returns {Uint8Array} RGBA array (numColors * 4 bytes)
 */
function parsePaletteRGBA(data, numColors) {
    const dv = new DataView(data.buffer, data.byteOffset, data.byteLength);
    const rgba = new Uint8Array(numColors * 4);
    for (let i = 0; i < numColors; i++) {
        if (i * 2 + 1 >= data.length) break;
        const c16 = dv.getUint16(i * 2, true);
        const [r, g, b] = rgb555to888(c16);
        const off = i * 4;
        rgba[off] = r;
        rgba[off + 1] = g;
        rgba[off + 2] = b;
        rgba[off + 3] = 255;
    }
    rgba[3] = 0; // color index 0 = transparent
    return rgba;
}

/**
 * Decompress a BG asset and strip the 4-byte sub-header.
 * @param {Uint8Array} rom
 * @param {number} offset  file offset of compressed asset
 * @returns {Uint8Array}
 */
function decompressBgAsset(rom, offset) {
    const { result } = decompressAsset(rom, offset);
    return result.subarray(4);
}

// Cache for decompressed BG assets (keyed by offset)
const _decompCache = new Map();

function getCachedDecomp(rom, offset) {
    if (!_decompCache.has(offset)) {
        _decompCache.set(offset, decompressBgAsset(rom, offset));
    }
    return _decompCache.get(offset);
}

/** Clear the decompression cache (call when ROM changes). */
export function clearCache() {
    _decompCache.clear();
}

/**
 * Build a combined 1024-tile VRAM buffer from 3 layers' tile data.
 *
 * On GBA hardware, all BG layers share a single charblock (CBB=0).
 * The game loads each layer's tile data into VRAM at specific offsets.
 * We approximate this layout by placing:
 *   - L0 tiles at the start (tile 0)
 *   - L1 tiles right after L0
 *   - L2 tiles right-aligned at the end (tile 1024 - L2_count)
 * This ensures all 10-bit tile IDs (0-1023) used by any layer's tilemap
 * can resolve to valid tile data in the combined buffer.
 *
 * @param {Uint8Array[]} tileDatas  array of 3 tile data buffers
 * @returns {Uint8Array}  combined 1024-tile buffer (32768 bytes)
 */
function buildCombinedTiles(tileDatas) {
    const nt = tileDatas.map(d => (d.length / 32) | 0);
    const l2Base = 1024 - nt[2];
    const l1End = nt[0] + nt[1];

    // Build 1024-tile VRAM (32KB)
    const vram = new Uint8Array(1024 * 32);

    // L0 at tile 0
    if (nt[0] > 0) vram.set(tileDatas[0].subarray(0, nt[0] * 32), 0);
    // L1 right after L0
    if (nt[1] > 0) vram.set(tileDatas[1].subarray(0, nt[1] * 32), nt[0] * 32);
    // L2 right-aligned (may overlap L1 if space is tight — L2 wins)
    if (nt[2] > 0) vram.set(tileDatas[2].subarray(0, nt[2] * 32), l2Base * 32);

    return vram;
}

/**
 * Compose a GBA background from tiles + tilemap + palette.
 * Returns an offscreen canvas, or null on failure.
 * @param {Uint8Array} tilesRaw    4bpp tile charblock data (can be combined)
 * @param {Uint8Array} tilemapRaw  screenblock entries (u16 per cell)
 * @param {Uint8Array} paletteRaw  512 bytes, 16 banks x 16 colors
 * @param {number} mapW  tilemap width in tiles
 * @param {number} mapH  tilemap height in tiles
 * @returns {HTMLCanvasElement|null}
 */
export function composeBg(tilesRaw, tilemapRaw, paletteRaw, mapW = 32, mapH = 32) {
    if (tilesRaw.length < 32 || tilemapRaw.length < 2) return null;

    const palBanks = [];
    for (let bank = 0; bank < 16; bank++) {
        const bankData = paletteRaw.subarray(bank * 32, bank * 32 + 32);
        palBanks.push(parsePaletteRGBA(bankData, 16));
    }

    const width = mapW * 8;
    const height = mapH * 8;
    const canvas = document.createElement('canvas');
    canvas.width = width;
    canvas.height = height;
    const ctx = canvas.getContext('2d');
    const imgData = ctx.createImageData(width, height);
    const pixels = imgData.data;

    const numTiles = (tilesRaw.length / 32) | 0;
    const tmapDv = new DataView(tilemapRaw.buffer, tilemapRaw.byteOffset, tilemapRaw.byteLength);

    for (let my = 0; my < mapH; my++) {
        for (let mx = 0; mx < mapW; mx++) {
            const idx = my * mapW + mx;
            if (idx * 2 + 1 >= tilemapRaw.length) continue;
            const entry = tmapDv.getUint16(idx * 2, true);
            const tileId = entry & 0x3FF;
            const hflip = (entry >>> 10) & 1;
            const vflip = (entry >>> 11) & 1;
            const palBank = (entry >>> 12) & 0xF;

            if (tileId >= numTiles) continue;

            const pal = palBanks[palBank];
            const tileOffset = tileId * 32;
            const pxX = mx * 8;
            const pxY = my * 8;

            for (let ty = 0; ty < 8; ty++) {
                for (let tx = 0; tx < 8; tx += 2) {
                    const byteIdx = tileOffset + ty * 4 + (tx >>> 1);
                    if (byteIdx >= tilesRaw.length) continue;
                    const byteVal = tilesRaw[byteIdx];
                    const lo = byteVal & 0x0F;
                    const hi = (byteVal >>> 4) & 0x0F;

                    let dx0, dx1, c0, c1;
                    if (hflip) {
                        dx0 = 7 - tx - 1; dx1 = 7 - tx;
                        c0 = hi; c1 = lo;
                    } else {
                        dx0 = tx; dx1 = tx + 1;
                        c0 = lo; c1 = hi;
                    }
                    const dy = vflip ? (7 - ty) : ty;

                    if (c0 !== 0) {
                        const x0 = pxX + dx0, y0 = pxY + dy;
                        if (x0 >= 0 && x0 < width && y0 < height) {
                            const p = (y0 * width + x0) * 4;
                            const s = c0 * 4;
                            pixels[p] = pal[s]; pixels[p+1] = pal[s+1];
                            pixels[p+2] = pal[s+2]; pixels[p+3] = 255;
                        }
                    }
                    if (c1 !== 0) {
                        const x1 = pxX + dx1, y1 = pxY + dy;
                        if (x1 >= 0 && x1 < width && y1 < height) {
                            const p = (y1 * width + x1) * 4;
                            const s = c1 * 4;
                            pixels[p] = pal[s]; pixels[p+1] = pal[s+1];
                            pixels[p+2] = pal[s+2]; pixels[p+3] = 255;
                        }
                    }
                }
            }
        }
    }

    ctx.putImageData(imgData, 0, 0);
    return canvas;
}

/**
 * Composite 3 BG layer canvases (0=far, 1=mid, 2=near/foreground).
 * Tiles smaller layers to fill the max width. Color 0 = transparent.
 * @param {(HTMLCanvasElement|null)[]} layers  array of 3 canvases
 * @returns {HTMLCanvasElement|null}
 */
export function compositeLayers(layers) {
    let maxW = 0, maxH = 0;
    for (const c of layers) {
        if (c) { maxW = Math.max(maxW, c.width); maxH = Math.max(maxH, c.height); }
    }
    if (maxW === 0 || maxH === 0) return null;

    const canvas = document.createElement('canvas');
    canvas.width = maxW;
    canvas.height = maxH;
    const ctx = canvas.getContext('2d');

    ctx.fillStyle = '#000';
    ctx.fillRect(0, 0, maxW, maxH);

    for (const c of layers) {
        if (!c) continue;
        for (let x = 0; x < maxW; x += c.width) {
            for (let y = 0; y < maxH; y += c.height) {
                ctx.drawImage(c, x, y);
            }
        }
    }
    return canvas;
}

/**
 * Determine tilemap dimensions from decompressed data size.
 * @param {Uint8Array} tilemap  decompressed tilemap data
 * @returns {{ mapW: number, mapH: number }}
 */
function tilemapDims(tilemap) {
    const mapEntries = (tilemap.length / 2) | 0;
    if (mapEntries >= 2048) return { mapW: 64, mapH: 32 };
    if (mapEntries >= 1024) return { mapW: 32, mapH: 32 };
    return { mapW: 32, mapH: Math.max(1, (mapEntries / 32) | 0) };
}

/**
 * Render a composite of all 3 layers for a given vision/world.
 *
 * On GBA hardware, all BG layers share VRAM. The game loads each layer's
 * tile data into consecutive VRAM regions. We replicate this by building
 * a combined tile buffer (L0 tiles, then L1, then L2) so that each
 * layer's tilemap can reference tiles from any layer's data.
 *
 * @param {Uint8Array} rom
 * @param {DataView} dv
 * @param {number} vision  1-6
 * @param {number} world   0-8
 * @returns {{ composite: HTMLCanvasElement|null, layers: (HTMLCanvasElement|null)[] }}
 */
export function renderScene(rom, dv, vision, world) {
    const palIdx = (vision - 1) * 9 + world;
    const palOff = romPtr(dv, BG_PALETTE_TABLE + palIdx * 4);
    if (palOff === null) return { composite: null, layers: [null, null, null] };

    let palRaw;
    try { palRaw = getCachedDecomp(rom, palOff); }
    catch (e) { return { composite: null, layers: [null, null, null] }; }

    // Decompress all 3 layers' tile and tilemap data
    const tileDatas = [];
    const tilemapDatas = [];
    for (let layer = 0; layer < 3; layer++) {
        const bgIdx = (vision - 1) * 27 + world * 3 + layer;
        const tileOff = romPtr(dv, BG_TILE_TABLE + bgIdx * 4);
        const tmapOff = romPtr(dv, BG_TILEMAP_TABLE + bgIdx * 4);

        if (tileOff === null || tmapOff === null) {
            tileDatas.push(new Uint8Array(0));
            tilemapDatas.push(new Uint8Array(0));
            continue;
        }
        try {
            tileDatas.push(getCachedDecomp(rom, tileOff));
            tilemapDatas.push(getCachedDecomp(rom, tmapOff));
        } catch (e) {
            tileDatas.push(new Uint8Array(0));
            tilemapDatas.push(new Uint8Array(0));
        }
    }

    // Build combined 1024-tile VRAM buffer
    const combined = buildCombinedTiles(tileDatas);

    // Render each layer using the combined tile buffer
    const layerCanvases = [];
    for (let layer = 0; layer < 3; layer++) {
        if (tilemapDatas[layer].length < 2) {
            layerCanvases.push(null);
            continue;
        }
        const { mapW, mapH } = tilemapDims(tilemapDatas[layer]);
        layerCanvases.push(composeBg(combined, tilemapDatas[layer], palRaw, mapW, mapH));
    }

    return {
        composite: compositeLayers(layerCanvases),
        layers: layerCanvases,
    };
}

/**
 * Render a single BG layer from ROM (standalone, no combined tiles).
 * Used for individual layer display.
 * @param {Uint8Array} rom
 * @param {DataView} dv
 * @param {number} bgIdx   index into tile/tilemap tables (0-161)
 * @param {number} palIdx  index into palette table (0-53)
 * @returns {HTMLCanvasElement|null}
 */
export function renderBgLayer(rom, dv, bgIdx, palIdx) {
    const tileOff = romPtr(dv, BG_TILE_TABLE + bgIdx * 4);
    const tmapOff = romPtr(dv, BG_TILEMAP_TABLE + bgIdx * 4);
    const palOff  = romPtr(dv, BG_PALETTE_TABLE + palIdx * 4);

    if (tileOff === null || tmapOff === null || palOff === null) return null;

    try {
        const tiles   = getCachedDecomp(rom, tileOff);
        const tilemap = getCachedDecomp(rom, tmapOff);
        const palRaw  = getCachedDecomp(rom, palOff);
        const { mapW, mapH } = tilemapDims(tilemap);
        return composeBg(tiles, tilemap, palRaw, mapW, mapH);
    } catch (e) {
        console.warn(`Failed to render BG layer ${bgIdx}:`, e);
        return null;
    }
}
