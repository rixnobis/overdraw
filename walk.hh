#pragma once

#include "overdraw.hh"

namespace overdraw {

static constexpr int16_t kScreenWidth = 320;
static constexpr int16_t kSpringJump = 72;
// A step up taller than this is a wall, not a step. Nothing in the shipped
// levels uses it yet, but the walker must not silently teleport up a cliff the
// first time a level does have height in it.
static constexpr int16_t kStepUpLimit = 16;

enum class WalkState : uint8_t {
    Running,
    Won,
    LostPit,      // walked into a column no tile covers
    LostHazard,
    LostTar,
    LostWall,     // ran into a face too tall to step onto
    LostEdge,     // reached the right edge without finding the goal
};

struct Walker {
    int16_t x;
    int16_t y;
    WalkState state;
};

Walker walkStart(const Tile* tiles, const uint8_t* order, uint8_t count, int16_t startX);
void walkStep(const Tile* tiles, const uint8_t* order, uint8_t count, Walker& walker);
// Runs to completion. Used by the host-side solver to decide whether a given
// permutation wins, which is how level solvability is verified rather than
// assumed.
WalkState walkAll(const Tile* tiles, const uint8_t* order, uint8_t count, int16_t startX);

const char* walkStateName(WalkState state);

}  // namespace overdraw
