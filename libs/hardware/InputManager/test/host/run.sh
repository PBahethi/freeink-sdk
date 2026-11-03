#!/bin/sh
# Builds and runs the allocation-free touch gesture math and slop tests.
set -e
cd "$(dirname "$0")"
BUILD_DIR="${TMPDIR:-/tmp}/freeink-input-tests"
mkdir -p "$BUILD_DIR"
c++ -std=c++17 -Wall -Wextra -Werror test_multitouch_gesture_math.cpp -o "$BUILD_DIR/test_multitouch_gesture_math"
"$BUILD_DIR/test_multitouch_gesture_math"
c++ -std=c++17 -Wall -Wextra -Werror test_touch_slop.cpp -o "$BUILD_DIR/test_touch_slop"
"$BUILD_DIR/test_touch_slop"
