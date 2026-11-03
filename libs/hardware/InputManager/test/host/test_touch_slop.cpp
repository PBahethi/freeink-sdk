#include <cstdio>

#include "../../src/TouchSlop.h"

namespace {

using freeink::input_detail::TouchSlop;

int checksRun = 0;
int checksFailed = 0;

#define CHECK(condition)                                               \
  do {                                                                 \
    ++checksRun;                                                       \
    if (!(condition)) {                                                \
      ++checksFailed;                                                  \
      std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #condition); \
    }                                                                  \
  } while (0)

constexpr int kTap = 28;      // InputManager::TOUCH_TAP_SLOP_PX
constexpr int kRelease = 59;  // InputManager::TOUCH_TAP_RELEASE_SLOP_PX

void testStillFingerIsATap() {
  TouchSlop s;
  s.track(3, -4, kTap, kRelease);
  CHECK(!s.beyondTap);
  CHECK(!s.beyondRelease);
}

void testFingerRollKeepsTheTap() {
  TouchSlop s;
  s.track(0, 40, kTap, kRelease);  // past hold slop, short of a swipe
  CHECK(s.beyondTap);
  CHECK(!s.beyondRelease);
}

void testVerticalScrollSwipeIsNotATap() {
  // T-Deck Pro (CST328/CST3530): a 120 px upward scroll in a list.
  TouchSlop s;
  for (int dy = 0; dy >= -120; dy -= 20) s.track(2, dy, kTap, kRelease);
  CHECK(s.beyondTap);
  CHECK(s.beyondRelease);
}

void testFlagsLatchUntilReset() {
  TouchSlop s;
  s.track(-80, 0, kTap, kRelease);
  s.track(0, 0, kTap, kRelease);  // finger came back to the start
  CHECK(s.beyondRelease);
  s.reset();
  CHECK(!s.beyondTap);
  CHECK(!s.beyondRelease);
}

}  // namespace

int main() {
  testStillFingerIsATap();
  testFingerRollKeepsTheTap();
  testVerticalScrollSwipeIsNotATap();
  testFlagsLatchUntilReset();
  std::printf("%d checks, %d failed\n", checksRun, checksFailed);
  return checksFailed == 0 ? 0 : 1;
}
