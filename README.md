# OVERDRAW

A PlayStation 1 puzzle game where the ordering table is the board.

The PS1 has no depth buffer. Three-dimensional scenes are sorted into an
ordering table and drawn back to front, and whichever primitive is drawn last
owns the pixel. Every 3D game of that era spent real effort hiding the seams
that fall out of this: coplanar faces that argue about which is in front, large
polygons that pop between orderings, the whole familiar wobble.

This game does not hide it. The ordering table is the only thing you can touch.

## The rules

A level is a list of rectangles and a permutation of that list. Slot 0 is drawn
last, so it wins every pixel it covers. The only verb is sliding one tile one
slot forward or backward.

Then a walker crosses the screen left to right. For each column it asks the
same question the GPU answers - which tile owns the topmost visible pixel here -
and does what that tile says. Planks are safe, lava kills, tar stops you dead,
springs launch you, and the green one is the exit.

The consequence that makes it a game: **reordering never moves anything.** The
silhouette of the level is fixed, because the topmost visible pixel of a column
is decided by geometry alone. What order decides is who *owns* that pixel. You
do not remove the lava. You put a plank in front of it and walk over the lava,
because the GPU has no idea the lava is still there.

And draw order is a *total* order. A tile cannot be in front of another on the
left and behind it on the right. That constraint is not a design conceit, it is
the actual limitation that makes painter's-algorithm rendering hard, and the
later levels are built out of it.

## Controls

| Button | |
|---|---|
| Up / Down | pick a tile in the order strip |
| Left / Right | slide the picked tile toward the front / back |
| Cross | run the walker |
| Circle | retry after a failure |
| Start | reset the level to its shipped order |
| Triangle | skip to the next level |

Par is the fewest slides from the shipped order to any winning order. It is not
a guess; see below.

## Building

Needs a MIPS toolchain (`mipsel-none-elf-gcc`) and the PSYQo SDK, either from
[nugget](https://github.com/pcsx-redux/nugget) or from the `src/mips` directory
of a pcsx-redux checkout.

```sh
make PSYQO_ROOT=/path/to/nugget          # -> overdraw.ps-exe
make verify                              # host-side level check
./verify.sh                              # everything, including on-console
```

## How the levels are checked

`solver.cpp` builds natively and links the *same* `overdraw.cpp`, `walk.cpp`
and `levels.cpp` the console binary does. For every level it brute-forces all
n! orderings and reports how many win, then breadth-first searches the
permutation graph over adjacent transpositions - the player's actual verb - to
find the minimum move count. It fails the build if a level is unsolvable or if
it ships already solved, because a level that ships solved is not a puzzle.

It then emits `solutions.inc`, which `selftest.cpp` compiles into a small
`.ps-exe` that re-runs the same walker on the console and checks it reaches the
same verdicts. A level being solvable on a desktop is not a claim about the
console until the console says so; the two targets disagree about integer
promotion around the `int16_t` coordinates if you are careless, and this is
what would catch it.

`verify.sh` runs the whole chain, boots both binaries in pcsx-redux headless,
and grades the screenshots on their colour histogram rather than on whether
`curl` succeeded - the screenshot endpoint returns a perfectly valid PNG of an
entirely blank frame if you grab before the guest has drawn, and `file` cannot
tell the difference.

Current state: 5 levels, par 1 / 5 / 6 / 7 / 8.

## Layout

| | |
|---|---|
| `overdraw.hh` / `.cpp` | the level model and `probeColumn`, the one function both the renderer and the walker read the world through |
| `walk.hh` / `.cpp` | the walker |
| `levels.cpp` | level data |
| `main.cpp` | PSYQo application, rendering, input |
| `solver.cpp` | host-side solvability and par verifier |
| `selftest.cpp` | on-console cross-check |
| `verify.sh` | the whole pipeline |
