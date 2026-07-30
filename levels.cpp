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

#include "levels.hh"

namespace overdraw {

// Every level starts in an order that loses. The initial permutation is part
// of the puzzle statement, not a default.

// -- 1 -----------------------------------------------------------------
// One plank, one lava pool, exactly the same rectangle. The lava does not go
// anywhere; it just stops being the thing the pixel belongs to.
static const Tile s_tiles1[] = {
    {0, 170, 110, 60, Surface::Solid},
    {100, 170, 110, 60, Surface::Hazard},
    {100, 170, 110, 60, Surface::Solid},
    {200, 170, 70, 60, Surface::Solid},
    {268, 170, 52, 60, Surface::Goal},
};
static const uint8_t s_order1[] = {1, 2, 0, 3, 4};

// -- 2 -----------------------------------------------------------------
// Two problems, two planks. Teaches that each covering is independent, right
// before the next level teaches that they are not.
static const Tile s_tiles2[] = {
    {0, 170, 90, 60, Surface::Solid},
    {80, 170, 90, 60, Surface::Tar},
    {80, 170, 90, 60, Surface::Solid},
    {160, 170, 90, 60, Surface::Hazard},
    {160, 170, 90, 60, Surface::Solid},
    {240, 170, 40, 60, Surface::Solid},
    {275, 170, 45, 60, Surface::Goal},
};
static const uint8_t s_order2[] = {1, 3, 0, 2, 4, 5, 6};

// -- 3 -----------------------------------------------------------------
// A gap that geometry cannot close. Reordering never adds a surface, so the
// only way across is to stop hiding the spring.
static const Tile s_tiles3[] = {
    {0, 170, 80, 60, Surface::Solid},
    {56, 170, 24, 60, Surface::Spring},
    {56, 170, 24, 60, Surface::Solid},
    {120, 170, 90, 60, Surface::Solid},
    {180, 170, 60, 60, Surface::Hazard},
    {180, 170, 60, 60, Surface::Solid},
    {240, 170, 80, 60, Surface::Goal},
};
static const uint8_t s_order3[] = {2, 4, 0, 1, 3, 5, 6};

// -- 4 -----------------------------------------------------------------
// The plank has to be in front of the lava and behind the spring at the same
// time. It can be, because those are different tiles - but the player has to
// see the chain before they see the moves.
static const Tile s_tiles4[] = {
    {0, 170, 70, 60, Surface::Solid},
    {50, 170, 50, 60, Surface::Hazard},
    {50, 170, 90, 60, Surface::Solid},
    {110, 170, 30, 60, Surface::Spring},
    {180, 170, 60, 60, Surface::Solid},
    {200, 170, 40, 60, Surface::Hazard},
    {240, 170, 80, 60, Surface::Goal},
};
static const uint8_t s_order4[] = {1, 2, 5, 0, 3, 4, 6};

// -- 5 -----------------------------------------------------------------
static const Tile s_tiles5[] = {
    {0, 170, 60, 60, Surface::Solid},
    {40, 170, 60, 60, Surface::Hazard},
    {40, 170, 60, 60, Surface::Solid},
    {100, 170, 50, 60, Surface::Tar},
    {100, 170, 80, 60, Surface::Solid},
    {150, 170, 30, 60, Surface::Spring},
    {220, 170, 50, 60, Surface::Solid},
    {265, 170, 55, 60, Surface::Goal},
};
static const uint8_t s_order5[] = {1, 3, 4, 0, 2, 5, 6, 7};

const Level g_levels[] = {
    {"PLANK", "IT IS STILL LAVA", s_tiles1, s_order1, 5, 8},
    {"TWO PROBLEMS", "COVER BOTH", s_tiles2, s_order2, 7, 8},
    {"SPRING LOADED", "STOP HIDING IT", s_tiles3, s_order3, 7, 8},
    {"TOTAL ORDER", "FRONT OF ONE, BEHIND ANOTHER", s_tiles4, s_order4, 7, 8},
    {"OVERDRAW", "ALL OF IT AT ONCE", s_tiles5, s_order5, 8, 8},
};

const unsigned g_levelCount = sizeof(g_levels) / sizeof(g_levels[0]);

}  // namespace overdraw
