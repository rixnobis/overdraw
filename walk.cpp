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

#include "walk.hh"

namespace overdraw {

namespace {

// Apply whatever the walker just landed on. `allowSpring` exists so that a
// spring landing you on another spring resolves as ordinary ground for that
// tick instead of recursing.
void applySurface(const Tile* tiles, const uint8_t* order, uint8_t count, Walker& walker,
                  const Probe& probe, bool allowSpring) {
    switch (tiles[probe.tile].surface) {
        case Surface::Solid:
            break;
        case Surface::Hazard:
            walker.state = WalkState::LostHazard;
            break;
        case Surface::Tar:
            walker.state = WalkState::LostTar;
            break;
        case Surface::Goal:
            walker.state = WalkState::Won;
            break;
        case Surface::Spring: {
            if (!allowSpring) break;
            walker.x += kSpringJump;
            if (walker.x >= kScreenWidth) {
                walker.state = WalkState::LostEdge;
                return;
            }
            Probe landing = probeColumn(tiles, order, count, walker.x);
            if (landing.tile < 0) {
                walker.state = WalkState::LostPit;
                return;
            }
            walker.y = landing.y;
            applySurface(tiles, order, count, walker, landing, false);
            break;
        }
    }
}

}  // namespace

Walker walkStart(const Tile* tiles, const uint8_t* order, uint8_t count, int16_t startX) {
    Walker walker = {startX, 0, WalkState::Running};
    Probe probe = probeColumn(tiles, order, count, startX);
    if (probe.tile < 0) {
        walker.state = WalkState::LostPit;
        return walker;
    }
    walker.y = probe.y;
    return walker;
}

void walkStep(const Tile* tiles, const uint8_t* order, uint8_t count, Walker& walker) {
    if (walker.state != WalkState::Running) return;
    walker.x++;
    if (walker.x >= kScreenWidth) {
        walker.state = WalkState::LostEdge;
        return;
    }
    Probe probe = probeColumn(tiles, order, count, walker.x);
    if (probe.tile < 0) {
        walker.state = WalkState::LostPit;
        return;
    }
    if (probe.y < walker.y - kStepUpLimit) {
        walker.state = WalkState::LostWall;
        return;
    }
    walker.y = probe.y;
    applySurface(tiles, order, count, walker, probe, true);
}

WalkState walkAll(const Tile* tiles, const uint8_t* order, uint8_t count, int16_t startX) {
    Walker walker = walkStart(tiles, order, count, startX);
    // Bounded by construction: every step advances x by at least one and the
    // walker loses at the screen edge, so this cannot spin.
    for (int guard = 0; guard < kScreenWidth + 8; guard++) {
        if (walker.state != WalkState::Running) break;
        walkStep(tiles, order, count, walker);
    }
    return walker.state;
}

const char* walkStateName(WalkState state) {
    switch (state) {
        case WalkState::Running: return "RUNNING";
        case WalkState::Won: return "CLEAR";
        case WalkState::LostPit: return "FELL";
        case WalkState::LostHazard: return "BURNED";
        case WalkState::LostTar: return "STUCK";
        case WalkState::LostWall: return "BLOCKED";
        case WalkState::LostEdge: return "LOST";
    }
    return "?";
}

}  // namespace overdraw
