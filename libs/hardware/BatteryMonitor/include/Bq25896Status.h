#pragma once

#include <cstdint>

// BQ25896 charger status register REG0B: VBUS_STAT [7:5], CHRG_STAT [4:3], PG_STAT [2].
struct Bq25896Status {
  bool charging;   // CHRG_STAT 01 pre-charge or 10 fast charge
  bool powerGood;  // input source is good
  uint8_t vbus;    // 0 = no input, 1 = USB host SDP, 2 = adapter, 3 = USB CDP/DCP, 7 = OTG

  // A charger or USB host is supplying power, including after charging has finished.
  constexpr bool externalPower() const { return powerGood && vbus != 0 && vbus != 7; }
};

constexpr Bq25896Status decodeBq25896Reg0B(const uint8_t reg) {
  return Bq25896Status{((reg >> 3) & 0x03) == 0x01 || ((reg >> 3) & 0x03) == 0x02, (reg & 0x04) != 0,
                       static_cast<uint8_t>(reg >> 5)};
}
