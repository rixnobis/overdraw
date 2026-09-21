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
# pcsx-redux boots its built-in OpenBIOS when no -bios is given, which is what
# CI uses. A local openbios.bin still wins when there is one, so the gates run
# against the same kernel a developer is looking at.
if [ -f "$BIOS" ]; then
    BIOSARG="-bios $BIOS"
else
    BIOSARG=""
    echo "note: no BIOS at $BIOS, running on the emulator's built-in OpenBIOS"
fi
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
timeout 90 "$REDUX" -no-ui -run -stdout -testmode -interpreter $BIOSARG \
    -loadexe overdraw-selftest.ps-exe 2>&1 | grep -E "OVERDRAW|FAIL|  [0-9] |probe control"
SELFTEST=${PIPESTATUS[0]}
echo "  exit=$SELFTEST"
[ "$SELFTEST" -eq 0 ] || FAIL=1

grab() { # grab <exe> <port> <settle> <out>
    # -screen ...x24: xvfb-run defaults to an 8-bit screen, which has no GLX
    # visual a 3.2 core context can match.
    xvfb-run -a -s "-screen 0 1280x1024x24" "$REDUX" -run -stdout -webserver -webserver-port "$2" -interpreter \
        $BIOSARG -loadexe "$1" > /tmp/overdraw-grab.log 2>&1 &
    local launch=$!
    # Poll rather than sleep on a guess: the still endpoint answers 200 with a
    # valid blank PNG well before the guest has drawn, so the colour count is
    # the readiness test and the settle argument is only a floor.
    local i
    sleep "$3"
    GRABRC=none
    for i in $(seq 1 25); do
        curl -s -m 5 "http://localhost:$2/api/v1/screen/still" -o "$4"
        GRABRC=$?
        [ "$GRABRC" -eq 0 ] && \
            [ "$(convert "$4" -format %k info:- 2>/dev/null || echo 0)" -ge 5 ] && break
        sleep 1
    done
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
        # A dead emulator, a blank frame and a refused connection all land on
        # "none" above, so say which of the three this was.
        echo "    last curl exit: ${GRABRC:-none}"
        echo "    png: $(ls -la "$1" 2>&1)"
        echo "    --- emulator log ($(wc -c < /tmp/overdraw-grab.log 2>/dev/null || echo 0) bytes) ---"
        tail -20 /tmp/overdraw-grab.log 2>&1
        return 1
    fi
    return 0
}

RENDER=${RENDER:-1}
if [ "$RENDER" != 1 ]; then
    step "4/4 render and attract run SKIPPED (RENDER=0)"
    echo "  the frame grabs need the emulator's web server, which does not come"
    echo "  up on a runner - curl gets ECONNREFUSED for the whole poll. Board"
    echo "  #610; gates 1-3 still ran."
else
step "4/4 render and attract run"
grab "$PWD/overdraw.ps-exe" 8191 6 /tmp/overdraw-menu.png
check /tmp/overdraw-menu.png "edit screen" 5 || FAIL=1
grab "$PWD/overdraw-autoplay.ps-exe" 8192 8 /tmp/overdraw-attract.png
check /tmp/overdraw-attract.png "attract run" 5 || FAIL=1
fi

echo
if [ "$FAIL" -ne 0 ]; then echo "VERIFY: FAILED"; exit 1; fi
echo "VERIFY: all gates green"
