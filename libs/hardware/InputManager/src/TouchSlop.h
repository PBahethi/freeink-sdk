#pragma once

// Single-contact slop tracking shared by the touch backends: how far the contact
// has moved from where it went down. Allocation-free and Arduino-free so the
// host tests can drive it (test/host/test_touch_slop.cpp).

namespace freeink::input_detail {

struct TouchSlop {
  // Past the stationary slop: ends hold / long-press detection.
  bool beyondTap = false;
  // Past the release slop (one short of the swipe distance): the release is not
  // a tap. Without this a swipe also fired a tap at its touch-down point.
  bool beyondRelease = false;

  void reset() {
    beyondTap = false;
    beyondRelease = false;
  }

  // dx/dy: current sample minus touch-down point, in panel pixels.
  void track(const int dx, const int dy, const int tapSlop, const int releaseSlop) {
    const int adx = dx < 0 ? -dx : dx;
    const int ady = dy < 0 ? -dy : dy;
    if (adx > tapSlop || ady > tapSlop) beyondTap = true;
    if (adx > releaseSlop || ady > releaseSlop) beyondRelease = true;
  }
};

}  // namespace freeink::input_detail
