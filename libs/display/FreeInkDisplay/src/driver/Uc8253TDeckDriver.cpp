#include <BoardConfig.h>

#if FREEINK_DRIVER_UC8253_TDECK

#include "Uc8253TDeckDriver.h"

namespace freeink {
namespace {
constexpr uint8_t CMD_PANEL_SETTING = 0x00;
constexpr uint8_t CMD_POWER_OFF = 0x02;
constexpr uint8_t CMD_POWER_ON = 0x04;
constexpr uint8_t CMD_DEEP_SLEEP = 0x07;
constexpr uint8_t CMD_DTM1 = 0x10;
constexpr uint8_t CMD_DISPLAY_REFRESH = 0x12;
constexpr uint8_t CMD_DTM2 = 0x13;
constexpr uint8_t CMD_VCOM_DATA_INTERVAL = 0x50;
constexpr uint8_t CMD_PARTIAL_WINDOW = 0x90;
constexpr uint8_t CMD_PARTIAL_IN = 0x91;
constexpr uint8_t CMD_PARTIAL_OUT = 0x92;
constexpr uint8_t CMD_CASCADE = 0xE0;
constexpr uint8_t CMD_FORCE_TEMP = 0xE5;

constexpr uint16_t W = 240;
constexpr uint16_t H = 320;
constexpr uint16_t WB = W / 8;  // 30
constexpr uint8_t kGhostClearInterval = 20;
}  // namespace

uint32_t Uc8253TDeckDriver::spiHz() const {
  return BoardConfig::ACTIVE.displaySpiHz != 0 ? BoardConfig::ACTIVE.displaySpiHz : 10000000;
}

PanelGeometry Uc8253TDeckDriver::geometry() const { return {W, H, WB, static_cast<uint32_t>(WB) * H}; }

int8_t Uc8253TDeckDriver::spiMiso() const { return BoardConfig::ACTIVE.sd.miso; }

int8_t Uc8253TDeckDriver::coCs() const { return BoardConfig::ACTIVE.sd.cs; }

void Uc8253TDeckDriver::initController(EpdBus& bus) {
  if (_hibernating) {
    bus.reset(10);
    _hibernating = false;
  } else {
    // Soft reset via PSR (RST_N bit low), as GxEPD2 does between refreshes.
    bus.cmd(CMD_PANEL_SETTING);
    bus.data(0x1E);
    bus.data(0x0D);
    delay(1);
  }
  _powerOn = false;
  bus.cmd(CMD_PANEL_SETTING);
  bus.data(0x1F);  // B/W, OTP LUT
  bus.data(0x0D);
}

void Uc8253TDeckDriver::powerOn(EpdBus& bus) {
  if (_powerOn) return;
  bus.cmd(CMD_POWER_ON);
  bus.waitBusy(" TDK_PON");
  _powerOn = true;
}

void Uc8253TDeckDriver::powerOff(EpdBus& bus) {
  if (!_powerOn) return;
  bus.cmd(CMD_POWER_OFF);
  bus.waitBusy(" TDK_POF");
  _powerOn = false;
}

void Uc8253TDeckDriver::writePlane(EpdBus& bus, uint8_t command, const uint8_t* fb) {
  bus.cmd(command);
  bus.beginTxn();
  for (uint16_t y = 0; y < H; y++) bus.rawWriteBytes(fb + static_cast<uint32_t>(y) * WB, WB);
  bus.endTxn();
}

void Uc8253TDeckDriver::begin(EpdBus& bus) {
  bus.reset(10);
  _hibernating = false;
  _initialRefresh = true;
  _fastCount = 0;
  initController(bus);
}

void Uc8253TDeckDriver::display(EpdBus& bus, const uint8_t* fb, const uint8_t* prev, RefreshMode mode, bool turnOff) {
  bool fast = (mode == RefreshMode::Fast) && !_initialRefresh;
  if (fast && ++_fastCount >= kGhostClearInterval) fast = false;
  if (!fast) _fastCount = 0;

  initController(bus);
  if (fast) {
    // Controller RAM still holds the previous frame (N2OCP copies new->old
    // after each refresh); a host-side previous frame, when present, is exact.
    bus.cmd(CMD_PARTIAL_IN);
    bus.cmd(CMD_PARTIAL_WINDOW);
    const uint8_t win[] = {0x00, static_cast<uint8_t>((W - 1) | 0x07), 0x00,
                           0x00, static_cast<uint8_t>((H - 1) >> 8),   static_cast<uint8_t>((H - 1) & 0xFF),
                           0x01};
    bus.data(win, sizeof(win));
    if (prev != nullptr) writePlane(bus, CMD_DTM1, prev);
    writePlane(bus, CMD_DTM2, fb);
    bus.cmd(CMD_CASCADE);
    bus.data(0x02);
    bus.cmd(CMD_FORCE_TEMP);
    bus.data(0x79);
    bus.cmd(CMD_VCOM_DATA_INTERVAL);
    bus.data(0xD7);
    powerOn(bus);
    bus.cmd(CMD_DISPLAY_REFRESH);
    bus.waitBusy(" TDK_DRF_FAST");
    bus.cmd(CMD_PARTIAL_OUT);
  } else {
    writePlane(bus, CMD_DTM1, fb);
    writePlane(bus, CMD_DTM2, fb);
    if (mode == RefreshMode::Half) {
      bus.cmd(CMD_CASCADE);
      bus.data(0x02);
      bus.cmd(CMD_FORCE_TEMP);
      bus.data(0x5A);
    }
    bus.cmd(CMD_VCOM_DATA_INTERVAL);
    bus.data(0x97);
    powerOn(bus);
    bus.cmd(CMD_DISPLAY_REFRESH);
    bus.waitBusy(" TDK_DRF_FULL");
    _initialRefresh = false;
  }
  if (turnOff) powerOff(bus);
}

void Uc8253TDeckDriver::deepSleep(EpdBus& bus) {
  powerOff(bus);
  bus.cmd(CMD_DEEP_SLEEP);
  bus.data(0xA5);
  _hibernating = true;
}

PanelDriver& uc8253TDeckDriver() {
  static Uc8253TDeckDriver instance;
  return instance;
}

}  // namespace freeink

#endif  // FREEINK_DRIVER_UC8253_TDECK
