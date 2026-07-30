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

#pragma once

#include <stdint.h>

// OVERDRAW - the level model.
//
// The whole game is a list of axis-aligned rectangles and a permutation of
// that list. The permutation is the ordering table: slot 0 is drawn last and
// therefore wins every pixel it touches. Nothing here stores a "height map" or
// a "collision layer" - the surface the walker touches is derived from the
// same two facts the GPU uses, the rectangles and their order.

namespace overdraw {

// What a tile does to a walker standing on it. This is the ONLY thing draw
// order decides: which tile owns the topmost visible pixel of a column.
enum class Surface : uint8_t {
    Solid,   // walkable, does nothing
    Hazard,  // kills on contact
    Spring,  // launches the walker forward over a gap
    Tar,     // walker sticks and stops - a loss, not a hazard
    Goal,    // level complete
};

struct Tile {
    int16_t x, y, w, h;
    Surface surface;
};

struct Level {
    const char* name;
    const char* hint;
    const Tile* tiles;
    // Initial draw order, front (index 0, drawn last, wins pixels) to back.
    // Entries are indices into `tiles`. Levels start in a losing order on
    // purpose; that is the puzzle.
    const uint8_t* order;
    uint8_t count;
    int16_t startX;
};

// Result of asking "what is at the top of this column, and who owns it".
struct Probe {
    int16_t y;      // topmost visible pixel row; undefined when tile < 0
    int8_t tile;    // index into Level::tiles, or -1 for empty column (a pit)
};

// Both the renderer and the walker read the world through this one function.
// Keeping it a single definition is the point: if the walker and the GPU ever
// disagreed about who owns a pixel, the game would be lying to the player.
//
// `order` is front-to-back, same convention as Level::order.
Probe probeColumn(const Tile* tiles, const uint8_t* order, uint8_t count, int16_t x);

}  // namespace overdraw
