/*

MIT License

Copyright (c) 2026 Nicolas "Pixel" Noble

On-console self-test. Runs the same walker the game runs, on an R3000, over
the same level data and the same solutions the host solver found, and checks
the verdicts match.

The point is not that the walker is complicated. It is that the host solver
proves the levels solvable using x86 arithmetic, and the thing that actually
has to agree is a 32-bit MIPS part with different integer promotion behaviour
around the int16_t coordinates. A level being solvable on my desktop is not a
claim about the console until the console says so too.

*/

#include "common/hardware/pcsxhw.h"
#include "common/syscalls/syscalls.h"

#include "levels.hh"
#include "walk.hh"

#include "solutions.inc"

using namespace overdraw;

int main() {
    int failures = 0;
    ramsyscall_printf("OVERDRAW self-test: %u levels\n", g_levelCount);

    for (unsigned i = 0; i < g_levelCount; i++) {
        const Level& level = g_levels[i];

        WalkState shipped = walkAll(level.tiles, level.order, level.count, level.startX);
        WalkState solved = walkAll(level.tiles, g_solutions[i], level.count, level.startX);

        bool shippedOk = shipped != WalkState::Won;
        bool solvedOk = solved == WalkState::Won;

        ramsyscall_printf("  %u %-14s shipped=%-8s solved=%-8s %s\n", i + 1, level.name,
                          walkStateName(shipped), walkStateName(solved),
                          (shippedOk && solvedOk) ? "ok" : "FAIL");

        if (!shippedOk) {
            ramsyscall_printf("    FAIL: ships already solved\n");
            failures++;
        }
        if (!solvedOk) {
            ramsyscall_printf("    FAIL: host solution does not win on console\n");
            failures++;
        }
    }

    // A negative control. If probeColumn were stubbed out, degenerate, or
    // optimised into always returning the same tile, everything above could
    // still pass by accident. An empty column has to report a pit, and a
    // column owned by a tile has to report that tile.
    {
        static const Tile probeTiles[] = {
            {10, 100, 20, 10, Surface::Solid},
            {10, 100, 20, 10, Surface::Hazard},
        };
        static const uint8_t frontIsHazard[] = {1, 0};
        static const uint8_t frontIsSolid[] = {0, 1};

        Probe empty = probeColumn(probeTiles, frontIsHazard, 2, 5);
        Probe hazard = probeColumn(probeTiles, frontIsHazard, 2, 15);
        Probe solid = probeColumn(probeTiles, frontIsSolid, 2, 15);

        bool ok = empty.tile < 0 && hazard.tile == 1 && solid.tile == 0;
        ramsyscall_printf("  probe control: empty=%d hazard=%d solid=%d %s\n", empty.tile,
                          hazard.tile, solid.tile, ok ? "ok" : "FAIL");
        if (!ok) failures++;
    }

    if (failures) {
        ramsyscall_printf("OVERDRAW self-test: %d FAILURE(S)\n", failures);
        pcsx_exit(1);
    }
    ramsyscall_printf("OVERDRAW self-test: all good\n");
    pcsx_exit(0);
    return 0;
}
