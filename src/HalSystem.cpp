#include "HalSystem.h"

#include <Arduino.h>
#include <BoardConfig.h>

void HalSystem::begin() {}
void HalSystem::restart() { exit(0); }
void HalSystem::checkPanic() {}
void HalSystem::clearPanic() {}
std::string HalSystem::getPanicInfo(bool full) { return {}; }
bool HalSystem::isRebootFromPanic() { return false; }
const char* HalSystem::getDeviceModel() { return BoardConfig::ACTIVE.name; }
bool HalSystem::getDeviceId(DeviceId& out) {
  out = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
  return true;
}
HalSystem::HeapInfo HalSystem::getHeapInfo() {
  return {ESP.getFreeHeap(), ESP.getHeapSize(), ESP.getMaxAllocHeap()};
}
