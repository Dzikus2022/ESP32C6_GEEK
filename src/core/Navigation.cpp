#include "core/Navigation.h"

#include <cstring>

#include "core/Log.h"

namespace {

void copyKey(char* dest, size_t destLen, const char* src) {
  strncpy(dest, src != nullptr ? src : "", destLen - 1);
  dest[destLen - 1] = '\0';
}

const char* wifiKey(const WifiNetwork& row) {
  return row.bssid[0] != '\0' ? row.bssid : row.ssid;
}

void applyIndex(AppState* state, uint8_t index) {
  if (state->browse.count == 0) {
    state->browse.index = 0;
    state->browse.selectedKey[0] = '\0';
    return;
  }
  if (index >= state->browse.count) {
    index = static_cast<uint8_t>(state->browse.count - 1);
  }
  state->browse.index = index;
  copyKey(state->browse.selectedKey, sizeof(state->browse.selectedKey),
          state->browse.keys[index]);
}

}  // namespace

void captureBleBrowse(AppState* state) {
  state->browse = BrowseNav{};
  uint8_t order[AppConfig::BROWSE_CAP];
  uint8_t n = state->bleShown < AppConfig::BROWSE_CAP ? state->bleShown
                                                      : AppConfig::BROWSE_CAP;
  for (uint8_t i = 0; i < n; ++i) {
    order[i] = i;
  }
  for (uint8_t i = 1; i < n; ++i) {
    const uint8_t key = order[i];
    int j = i - 1;
    while (j >= 0 && state->ble[order[j]].rssi < state->ble[key].rssi) {
      order[j + 1] = order[j];
      --j;
    }
    order[j + 1] = key;
  }
  state->browse.count = n;
  for (uint8_t i = 0; i < n; ++i) {
    copyKey(state->browse.keys[i], sizeof(state->browse.keys[i]),
            state->ble[order[i]].id);
  }
  state->browse.locked = true;
  applyIndex(state, 0);
}

void captureWifiBrowse(AppState* state) {
  state->browse = BrowseNav{};
  uint8_t order[AppConfig::BROWSE_CAP];
  uint8_t n = state->wifiShown < AppConfig::BROWSE_CAP ? state->wifiShown
                                                       : AppConfig::BROWSE_CAP;
  for (uint8_t i = 0; i < n; ++i) {
    order[i] = i;
  }
  for (uint8_t i = 1; i < n; ++i) {
    const uint8_t key = order[i];
    int j = i - 1;
    while (j >= 0 && state->wifi[order[j]].rssi < state->wifi[key].rssi) {
      order[j + 1] = order[j];
      --j;
    }
    order[j + 1] = key;
  }
  state->browse.count = n;
  for (uint8_t i = 0; i < n; ++i) {
    copyKey(state->browse.keys[i], sizeof(state->browse.keys[i]),
            wifiKey(state->wifi[order[i]]));
  }
  state->browse.locked = true;
  applyIndex(state, 0);
}

void resolveBrowseSelection(AppState* state) {
  if (state->browse.count == 0) {
    applyIndex(state, 0);
    return;
  }
  for (uint8_t i = 0; i < state->browse.count; ++i) {
    if (strncmp(state->browse.keys[i], state->browse.selectedKey,
                sizeof(state->browse.selectedKey)) == 0) {
      applyIndex(state, i);
      return;
    }
  }
  applyIndex(state, state->browse.index);
}

void browseNext(AppState* state) {
  if (state->browse.count == 0) {
    return;
  }
  const uint8_t next =
      static_cast<uint8_t>((state->browse.index + 1) % state->browse.count);
  applyIndex(state, next);
  state->uiDirty = true;
}

void nextTopLevel(AppState* state) {
  switch (state->screen) {
    case ScreenId::MainSystem:
      state->screen = ScreenId::MainWifi;
      break;
    case ScreenId::MainWifi:
      state->screen = ScreenId::MainBle;
      break;
    case ScreenId::MainBle:
    default:
      state->screen = ScreenId::MainSystem;
      break;
  }
  state->uiDirty = true;
  logLine("INPUT", "Top -> %u", static_cast<unsigned>(state->screen));
}

void enterCurrentFeature(AppState* state) {
  if (state->screen == ScreenId::MainBle) {
    captureBleBrowse(state);
    state->screen = ScreenId::BleList;
    state->uiDirty = true;
    logLine("INPUT", "Enter BLE list %u", state->browse.count);
  } else if (state->screen == ScreenId::MainWifi) {
    captureWifiBrowse(state);
    state->screen = ScreenId::WifiList;
    state->uiDirty = true;
    logLine("INPUT", "Enter WiFi list %u", state->browse.count);
  }
}

void openSelectedDetails(AppState* state) {
  if (state->browse.count == 0) {
    return;
  }
  resolveBrowseSelection(state);
  if (state->screen == ScreenId::BleList) {
    if (selectedBle(*state) == nullptr &&
        findBleIndex(*state, state->browse.selectedKey) < 0) {
      // Keep browsing a vanished device: details still show key-only empty
    }
    state->browse.detailPage = 0;
    state->screen = ScreenId::BleDetails;
    state->uiDirty = true;
    logLine("INPUT", "BLE details %s", state->browse.selectedKey);
  } else if (state->screen == ScreenId::WifiList) {
    state->browse.detailPage = 0;
    state->screen = ScreenId::WifiDetails;
    state->uiDirty = true;
    logLine("INPUT", "WiFi details %s", state->browse.selectedKey);
  }
}

void goBack(AppState* state) {
  if (state->screen == ScreenId::BleDetails) {
    state->screen = ScreenId::BleList;
    resolveBrowseSelection(state);
    state->uiDirty = true;
    logLine("INPUT", "Back BLE list @ %u", state->browse.index);
  } else if (state->screen == ScreenId::WifiDetails) {
    state->screen = ScreenId::WifiList;
    resolveBrowseSelection(state);
    state->uiDirty = true;
    logLine("INPUT", "Back WiFi list @ %u", state->browse.index);
  } else if (state->screen == ScreenId::BleList) {
    state->browse.locked = false;
    state->screen = ScreenId::MainBle;
    state->uiDirty = true;
    logLine("INPUT", "Back BLE dashboard");
  } else if (state->screen == ScreenId::WifiList) {
    state->browse.locked = false;
    state->screen = ScreenId::MainWifi;
    state->uiDirty = true;
    logLine("INPUT", "Back WiFi dashboard");
  }
}
