/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "app.h"
#include "app_log.h"
#include <math.h>  // For pow() function
#include <stdbool.h> // For bool type

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

static void process_ibeacon(const sl_bt_evt_scanner_legacy_advertisement_report_t *report);
static bool is_ibeacon_packet(const uint8_t *data, uint8_t len);
static float calculate_distance(int8_t rssi, int8_t tx_power);

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/

void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  app_log("Event received: 0x%04X\n", SL_BT_MSG_ID(evt->header));  // Log all events

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id:
      app_log("System booted\n");

      // Verify Bluetooth stack version
      app_log("Bluetooth stack version: %d.%d.%d\n",
              evt->data.evt_system_boot.major,
              evt->data.evt_system_boot.minor,
              evt->data.evt_system_boot.patch);

      // Set scanning parameters
      sc = sl_bt_scanner_set_parameters(sl_bt_scanner_scan_phy_1m, 160, 160); // 100ms interval/window
      app_log("Scanner set params status: 0x%04X\n", sc);
      app_assert_status(sc);

      // Start passive scanning
      sc = sl_bt_scanner_start(0, sl_bt_scanner_discover_generic);
      app_log("Scanner start status: 0x%04X\n", sc);
      app_assert_status(sc);
      break;

    case sl_bt_evt_scanner_legacy_advertisement_report_id:
    {
      const sl_bt_evt_scanner_legacy_advertisement_report_t *adv_report =
          &evt->data.evt_scanner_legacy_advertisement_report;

      app_log("Advertisement received, RSSI: %d, Data len: %d\n",
              adv_report->rssi, adv_report->data.len);

      // Log raw advertisement data for debugging
      app_log("Advertisement data: ");
      for (int i = 0; i < adv_report->data.len; i++) {
        app_log("%02X ", adv_report->data.data[i]);
      }
      app_log("\n");

      if (is_ibeacon_packet(adv_report->data.data, adv_report->data.len)) {
        process_ibeacon(adv_report);
      } else {
        app_log("Not an iBeacon packet\n");
      }
      break;
    }

    case sl_bt_evt_connection_opened_id:
      app_log("Connection opened\n");
      sc = sl_bt_scanner_stop();
      app_assert_status(sc);
      break;

    case sl_bt_evt_connection_closed_id:
      app_log("Connection closed\n");
      sc = sl_bt_scanner_start(0, sl_bt_scanner_discover_generic);
      app_assert_status(sc);
      break;

    default:
      break;
  }
}

// Helper function to check if a packet is an iBeacon
static bool is_ibeacon_packet(const uint8_t *data, uint8_t len)
{
  /*
   * Minimum iBeacon packet structure:
   * Flags (3 bytes): 0x02, 0x01, flags
   * Manufacturer Data (26 bytes):
   *   0x1A, 0xFF,
   *   0x4C, 0x00 (Apple),
   *   0x02 (iBeacon type),
   *   0x15 (length),
   *   UUID (16 bytes),
   *   Major (2 bytes),
   *   Minor (2 bytes),
   *   TX Power (1 byte)
   * Total minimum: 29 bytes
   */
  if (len < 29) return false;

  // Scan through advertisement data for manufacturer specific data
  uint8_t pos = 0;
  while (pos < len - 2) {
    uint8_t field_len = data[pos];
    uint8_t field_type = data[pos+1];

    // Check for manufacturer specific data (0xFF)
    if (field_type == 0xFF && field_len >= 25) {
      // Check for Apple company ID (0x004C) and iBeacon type (0x02)
      if (data[pos+2] == 0x4C &&
          data[pos+3] == 0x00 &&
          data[pos+4] == 0x02 &&
          data[pos+5] == 0x15) {
        return true;
      }
    }
    pos += field_len + 1;
  }
  return false;
}

// Helper function to process iBeacon data
static void process_ibeacon(const sl_bt_evt_scanner_legacy_advertisement_report_t *report)
{
  const uint8_t *data = report->data.data;
  uint8_t len = report->data.len;
  uint8_t pos = 0;

  while (pos < len - 2) {
    uint8_t field_len = data[pos];
    uint8_t field_type = data[pos+1];

    if (field_type == 0xFF && field_len >= 25) {  // Manufacturer data
      if (data[pos+2] == 0x4C && data[pos+3] == 0x00 &&  // Apple
          data[pos+4] == 0x02 && data[pos+5] == 0x15) {  // iBeacon
        const uint8_t *ibeacon_data = &data[pos+6];

        // Extract fields
        // UUID starts at ibeacon_data[0] (16 bytes)
        uint16_t major = (ibeacon_data[16] << 8) | ibeacon_data[17];
        uint16_t minor = (ibeacon_data[18] << 8) | ibeacon_data[19];
        int8_t tx_power = (int8_t)ibeacon_data[20];
        int8_t rssi = report->rssi;

        // Calculate distance
        float distance = calculate_distance(rssi, tx_power);

        app_log("iBeacon detected: Major=%u, Minor=%u, RSSI=%d, TX=%d, Distance=%.2fm\n",
                major, minor, rssi, tx_power, distance);

        if (distance < 1.0f) {
          app_log("Salut %d, coleg de la masterul %d!\n", minor, major);
        }
        return;
      }
    }
    pos += field_len + 1;
  }
}

// Helper function to estimate distance from RSSI and TX power
static float calculate_distance(int8_t rssi, int8_t tx_power)
{
  // Simple distance calculation (can be refined)
  // tx_power is the RSSI at 1 meter
  if (rssi >= tx_power) {
    return 1.0f;
  }

  // Calculate ratio in dB
  float ratio = (tx_power - rssi) / 20.0f;
  // Calculate distance (simplified model)
  return pow(10, ratio);
}
