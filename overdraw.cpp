/*

MIT License

Copyright (c) 2026 Nicolas "Pixel" Noble

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "overdraw.hh"

namespace overdraw {

Probe probeColumn(const Tile* tiles, const uint8_t* order, uint8_t count, int16_t x) {
    // Pass one: how high does the silhouette reach in this column. This is a
    // property of the geometry alone. Reordering cannot raise or lower a
    // surface, which is exactly why the game is about ownership and not about
    // stacking things up.
    int16_t topmost = 0x7fff;
    for (uint8_t i = 0; i < count; i++) {
        const Tile& tile = tiles[i];
        if (x < tile.x) continue;
        if (x >= tile.x + tile.w) continue;
        if (tile.y < topmost) topmost = tile.y;
    }
    if (topmost == 0x7fff) return {0, -1};

    // Pass two: walk the draw order front to back and take the first tile that
    // actually contains that pixel. This is the painter's algorithm read
    // backwards - instead of drawing back to front and letting the last write
    // win, we look front to back and let the first hit win. Same answer, one
    // pass instead of a framebuffer.
    for (uint8_t slot = 0; slot < count; slot++) {
        const Tile& tile = tiles[order[slot]];
        if (x < tile.x) continue;
        if (x >= tile.x + tile.w) continue;
        if (topmost < tile.y) continue;
        if (topmost >= tile.y + tile.h) continue;
        return {topmost, static_cast<int8_t>(order[slot])};
    }

    // Unreachable: some tile produced `topmost`, so some tile contains it.
    return {topmost, -1};
}

}  // namespace overdraw
