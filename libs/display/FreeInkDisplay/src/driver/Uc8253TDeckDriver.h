#pragma once

// UC8253 panel driver — LilyGo T-Deck Pro (Good Display GDEQ031T10, 3.1"
// 240x320 B/W, portrait, SPI shared with the SD card and the SX1262 radio).
//
// The GDEQ031T10 ships usable OTP waveforms, so unlike the X3/Murphy UC8253
// drivers no LUTs are loaded: the command sequence follows ZinggJM's GxEPD2
// GxEPD2_310_GDEQ031T10 (GPL-3.0; only the command bytes are reused, no code):
//   full  : 0x10 + 0x13 = frame, CDI 0x50=0x97, POWER ON, DRF 0x12
//   fast  : PTIN 0x91 + window 0x90, 0x13 = frame, force temp 0xE0/0xE5=0x79,
//           CDI 0x50=0xD7 (OTP differential), DRF, PTOUT 0x92
//   half  : full sequence with the "fast full" forced temperature (0xE5=0x5A)
// A FAST refresh is promoted to FULL every kGhostClearInterval refreshes.
// Framebuffer geometry equals controller RAM (240x320), so no rotation.
//
// Selection: linked only when -DFREEINK_DRIVER_UC8253_TDECK (T-Deck Pro env).

#include "PanelDriver.h"

namespace freeink {

class Uc8253TDeckDriver : public PanelDriver {
 public:
  uint32_t spiHz() const override;
  BusyPolarity busyPolarity() const override { return BusyPolarity::ActiveLow; }  // UC8253 BUSY low
  PanelGeometry geometry() const override;
  // MISO is shared with the SD card: hand it to the bus so SPI.begin() keeps it.
  int8_t spiMiso() const override;
  // SD card CS on the same bus: held high whenever the panel owns the bus.
  int8_t coCs() const override;

  void begin(EpdBus& bus) override;
  void deepSleep(EpdBus& bus) override;
  void display(EpdBus& bus, const uint8_t* fb, const uint8_t* prev, RefreshMode mode, bool turnOff) override;

 private:
  void initController(EpdBus& bus);
  void powerOn(EpdBus& bus);
  void powerOff(EpdBus& bus);
  void writePlane(EpdBus& bus, uint8_t command, const uint8_t* fb);

  bool _powerOn = false;
  bool _hibernating = false;
  bool _initialRefresh = true;
  uint8_t _fastCount = 0;
};

PanelDriver& uc8253TDeckDriver();

}  // namespace freeink
