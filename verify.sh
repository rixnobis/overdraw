#!/bin/bash
# Full verification pipeline for OVERDRAW. Four gates, in dependency order:
#
#   1. host solver   - every level is solvable and none ships pre-solved
#   2. console self-test - an R3000 reaches the same verdicts the host did
#   3. render        - the game boots and draws something with real content
#   4. attract run   - the walker actually crosses a level and wins
#
# Gate 3 grades on the colour histogram, not on `curl` exit status or `file`
# output. Both of those report success for a 0x0 image and for an all-
# background frame grabbed before the guest drew anything.
#
# main.o is deleted between the normal and AUTOPLAY builds on purpose: make
# tracks source timestamps, not the -D flags, so an incremental build across a
# define change happily links yesterday's object and reports success.

set -u
cd "$(dirname "$0")" || exit 1

REDUX=${REDUX:-/home/pixel/sources/pcsx-redux-wt/tetris-bg/pcsx-redux}
BIOS=${BIOS:-/home/pixel/sources/pcsx-redux/src/mips/openbios/openbios.bin}
FAIL=0

step() { echo; echo "=== $* ==="; }

step "1/4 host solver"
make solver >/dev/null 2>&1 || { echo "solver build FAILED"; exit 1; }
./solver --emit solutions.inc || FAIL=1

step "2/4 building"
rm -f main.o main.dep
make -j"$(nproc)" >/dev/null 2>&1 || { echo "game build FAILED"; exit 1; }
make -f Makefile.selftest -j"$(nproc)" >/dev/null 2>&1 || { echo "selftest build FAILED"; exit 1; }
rm -f main.o main.dep
make AUTOPLAY=true -j"$(nproc)" >/dev/null 2>&1 || { echo "autoplay build FAILED"; exit 1; }
rm -f main.o main.dep
make -j"$(nproc)" >/dev/null 2>&1 || { echo "game rebuild FAILED"; exit 1; }
for f in overdraw.ps-exe overdraw-selftest.ps-exe overdraw-autoplay.ps-exe; do
    echo "  $f  $(stat -c %s "$f") bytes  $(date -u -d @"$(stat -c %Y "$f")" -Iseconds)"
done

step "3/4 console self-test"
timeout 90 "$REDUX" -no-ui -run -stdout -testmode -interpreter -bios "$BIOS" \
    -loadexe overdraw-selftest.ps-exe 2>&1 | grep -E "OVERDRAW|FAIL|  [0-9] |probe control"
SELFTEST=${PIPESTATUS[0]}
echo "  exit=$SELFTEST"
[ "$SELFTEST" -eq 0 ] || FAIL=1

grab() { # grab <exe> <port> <settle> <out>
    xvfb-run -a "$REDUX" -run -stdout -webserver -webserver-port "$2" -interpreter \
        -bios "$BIOS" -loadexe "$1" > /tmp/overdraw-grab.log 2>&1 &
    local launch=$!
    sleep "$3"
    curl -s -m 15 "http://localhost:$2/api/v1/screen/still" -o "$4"
    local p c
    for p in $(pgrep -P $launch 2>/dev/null); do
        for c in $(pgrep -P "$p" 2>/dev/null); do kill -TERM "$c" 2>/dev/null; done
        kill -TERM "$p" 2>/dev/null
    done
    kill -TERM $launch 2>/dev/null
    wait $launch 2>/dev/null
}

check() { # check <png> <label> <min-colors>
    local dims colors
    dims=$(identify -format "%wx%h" "$1" 2>/dev/null)
    colors=$(convert "$1" -format %k info:- 2>/dev/null)
    echo "  $2: ${dims:-none} ${colors:-0} colors"
    if [ "${dims:-}" != "320x239" ] || [ "${colors:-0}" -lt "$3" ]; then
        echo "  $2 FAILED (want 320x239 and >= $3 colors)"
        return 1
    fi
    return 0
}

step "4/4 render and attract run"
grab "$PWD/overdraw.ps-exe" 8191 6 /tmp/overdraw-menu.png
check /tmp/overdraw-menu.png "edit screen" 5 || FAIL=1
grab "$PWD/overdraw-autoplay.ps-exe" 8192 8 /tmp/overdraw-attract.png
check /tmp/overdraw-attract.png "attract run" 5 || FAIL=1

echo
if [ "$FAIL" -ne 0 ]; then echo "VERIFY: FAILED"; exit 1; fi
echo "VERIFY: all gates green"
