/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2021 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "../inc/MarlinConfigPre.h"

#if ENABLED(EASYTHREED_UI)

#include "easythreed_ui.h"
#include "../module/temperature.h"
#include "../module/printcounter.h"
#include "../sd/cardreader.h"
#include "../gcode/queue.h"
#include "../module/motion.h"
#include "../module/planner.h"
#include "../MarlinCore.h"
#include "../module/stepper.h"
#include "../inc/MarlinConfig.h"
#include "../gcode/gcode.h"
#include "../feature/pause.h"

extern bool wait_for_user;

#if HAS_STEPPER_RESET
  void disableStepperDrivers();
#endif

enum LEDInterval : uint16_t {
  LED_OFF     = 0,
  LED_ON      = 4000,
  LED_BLINK_2 = 1000, // Impresión estándar
  LED_BLINK_5 = 300,  // Moviendo filamento
  LED_BLINK_7 = 50    // Calentando / Atención urgente
};

EasythreedUI easythreed_ui;

#define BTN_DEBOUNCE_MS 20
#define UI_MAX_TEMP 250

static uint8_t shared_flowrate = 100; 
static uint8_t shared_feedrate_index = 1;
uint16_t blink_interval_ms = LED_OFF; 

//
// Initialize UI Pins
//
void EasythreedUI::init() {
  SET_INPUT_PULLUP(BTN_HOME);     SET_OUTPUT(BTN_HOME_GND);
  SET_INPUT_PULLUP(BTN_FEED);     SET_OUTPUT(BTN_FEED_GND);
  SET_INPUT_PULLUP(BTN_RETRACT);  SET_OUTPUT(BTN_RETRACT_GND);
  SET_INPUT_PULLUP(BTN_PRINT);
  SET_OUTPUT(EASYTHREED_LED_PIN);
  SET_INPUT_PULLDOWN(BTN_1);
  SET_INPUT_PULLDOWN(BTN_2);
  SET_INPUT_PULLDOWN(BTN_3);
  SET_INPUT_PULLDOWN(BTN_4);
  
  blink_interval_ms = LED_OFF; 
}

//
// Main UI Loop
//
void EasythreedUI::run() {
  blinkLED();
  loadButton();
  printButton();
  HomeButton();
  HandleButton1();
  HandleButton2();
  HandleButton3();
  HandleButton4();
}

//
// Status LED logic (Inverted Logic Support)
//
void EasythreedUI::blinkLED() {
  static millis_t prev_blink_interval_ms = 0, blink_start_ms = 0;

  if (blink_interval_ms == LED_OFF) { WRITE(EASYTHREED_LED_PIN, HIGH); return; } // OFF (High pin)
  if (blink_interval_ms >= LED_ON)  { WRITE(EASYTHREED_LED_PIN, LOW); return; }  // ON (Low pin)

  const millis_t ms = millis();
  if (prev_blink_interval_ms != blink_interval_ms) {
    prev_blink_interval_ms = blink_interval_ms;
    blink_start_ms = ms;
  }

  uint16_t actual_interval = blink_interval_ms;

  // LED Blink frequency scale based on print speed
  if (printingIsActive() && blink_interval_ms == LED_BLINK_2) {
    actual_interval = (feedrate_percentage > 100) 
                      ? (740 - (feedrate_percentage * 3.4)) 
                      : 400;
  }

  if (PENDING(ms, blink_start_ms + actual_interval))
    WRITE(EASYTHREED_LED_PIN, LOW);
  else if (PENDING(ms, blink_start_ms + 2 * actual_interval))
    WRITE(EASYTHREED_LED_PIN, HIGH);
  else
    blink_start_ms = ms;
}

//
// Filament Load/Unload Button
//
void EasythreedUI::loadButton() {
  if (printingIsActive()) return;
  enum FilamentStatus : uint8_t { FS_IDLE, FS_PRESS, FS_CHECK, FS_PROCEED };
  static uint8_t filament_status = FS_IDLE;
  static millis_t filament_time = 0;

  switch (filament_status) {
    case FS_IDLE:
      if (!READ(BTN_RETRACT) || !READ(BTN_FEED)) { filament_status++; filament_time = millis(); }
      break;
    case FS_PRESS:
      if (ELAPSED(millis(), filament_time + BTN_DEBOUNCE_MS)) {
        if (!READ(BTN_RETRACT) || !READ(BTN_FEED)) {
          thermalManager.setTargetHotend(230, 0);
          blink_interval_ms = LED_BLINK_7; 
          filament_status++;
        } else filament_status = FS_IDLE;
      }
      break;
    case FS_CHECK:
      if (READ(BTN_RETRACT) && READ(BTN_FEED)) {
        blink_interval_ms = LED_OFF; filament_status = FS_IDLE;
        thermalManager.disable_all_heaters();
      }
      else if (thermalManager.degHotend(0) >= 230) {
        filament_status++; blink_interval_ms = LED_BLINK_5; 
      }
      break;
    case FS_PROCEED: {
      static bool flag = false;
      if (READ(BTN_RETRACT) && READ(BTN_FEED)) {
        flag = false; filament_status = FS_IDLE;
        quickstop_stepper(); thermalManager.disable_all_heaters();
        blink_interval_ms = LED_OFF;
      }
      else if (!flag) {
        flag = true;
        queue.inject(!READ(BTN_RETRACT) ? F("G91\nG0 E540 F2000\nG0 E60 F120\nG90\nM400\nM104 S0") : F("G91\nG1 E3 F180\nG1 E-40 F3000\nG1 E-560 F1000\nG90\nM400\nM104 S0"));
      }
    } break;
  }
}

//
// Print Start/Pause/Resume Button
//
void EasythreedUI::printButton() {
  enum KeyStatus : uint8_t { KS_IDLE, KS_PRESS, KS_PROCEED };
  static KeyStatus key_status = KS_IDLE;
  static millis_t key_time = 0;
  enum PrintFlag : uint8_t { PF_START, PF_PAUSE, PF_RESUME };
  static PrintFlag print_key_flag = PF_START;

  static bool selection_mode = false;
  static uint8_t click_count = 0;
  static millis_t last_click_time = 0;
  static int16_t file_count = 0;

  constexpr millis_t CLICK_TIMEOUT_MS = 5000;
  constexpr millis_t LONG_PRESS_MS    = 1200;
  const millis_t ms = millis();

  // Selection Timeout logic
  if (selection_mode && ELAPSED(ms, last_click_time + CLICK_TIMEOUT_MS)) {
    int16_t index = (file_count - 1) - click_count; 
    if (index < 0) index = 0; 
    wait_for_user = false; 
    print_key_flag = PF_PAUSE;
    card.selectFileByIndex(index);
    card.openAndPrintFile(card.filename);
    blink_interval_ms = LED_BLINK_2; 
    selection_mode = false;
    click_count = 0;
  }

  switch (key_status) {
    case KS_IDLE:
      if (!READ(BTN_PRINT)) { key_time = ms; key_status = KS_PRESS; }
      break;
    case KS_PRESS:
      if (ELAPSED(ms, key_time + BTN_DEBOUNCE_MS))
        key_status = READ(BTN_PRINT) ? KS_IDLE : KS_PROCEED;
      break;
    case KS_PROCEED:
      if (!READ(BTN_PRINT)) break;
      key_status = KS_IDLE;

      // SHORT PRESS REGISTER
      if (PENDING(ms, key_time + LONG_PRESS_MS - BTN_DEBOUNCE_MS)) {
        if (wait_for_user) {
          blink_interval_ms = LED_BLINK_2;
          wait_for_user = false; 
          // Restore normal acceleration and speed on resume
          queue.inject(F("M201 X1500 Y1500\nM203 X300 Y300\nM108\nM24")); 
          print_key_flag = PF_PAUSE;
          return;
        }
        if (!printingIsActive() && print_key_flag == PF_START) {
          if (!selection_mode) {
            card.mount();
            if (!card.isMounted()) return;
            card.ls();
            file_count = card.get_num_items();
            if (file_count <= 0) return;
            selection_mode = true;
            click_count = 0; 
            blink_interval_ms = LED_ON; 
          } else {
            click_count++;
            if (click_count > 4) click_count = 0;
            WRITE(EASYTHREED_LED_PIN, HIGH); safe_delay(60); 
          }
          last_click_time = ms;
          return;
        }
        if (printingIsActive() && print_key_flag != PF_RESUME) {
          blink_interval_ms = LED_BLINK_7;
          // Soft Parking: Limited acceleration and speed
          queue.inject(F("M201 X500 Y500\nM203 X60 Y60\nM125\nM25\nM0")); 
          print_key_flag = PF_RESUME;
          return;
        }
      } 
      // LONG PRESS REGISTER
      else {
        if (printingIsActive() || wait_for_user || selection_mode) {
          card.abortFilePrintSoon();
          wait_for_user = false; selection_mode = false;
          print_key_flag = PF_START; blink_interval_ms = LED_OFF;
        } else if (print_key_flag == PF_START) {
          queue.inject(F("M201 X800 Y800\nG91\nG1 Z20 F600\nG90"));
          blink_interval_ms = LED_ON;
        }
        planner.synchronize();
        TERN_(HAS_STEPPER_RESET, disableStepperDrivers());
        print_key_flag = PF_START;
      }
      break;
  }
}

//
// Home and Speed Adjustment Button
//
void EasythreedUI::HomeButton() {
  enum KeyStatus : uint8_t { KS_IDLE, KS_PRESS, KS_PROCEED };
  static uint8_t key_status = KS_IDLE;
  static millis_t key_time = 0;
  constexpr uint8_t feedrates[] = {100, 125, 160, 200};

  switch (key_status) {
    case KS_IDLE: if (!READ(BTN_HOME)) { key_time = millis(); key_status = KS_PRESS; } break;
    case KS_PRESS: if (ELAPSED(millis(), key_time + BTN_DEBOUNCE_MS)) key_status = READ(BTN_HOME) ? KS_IDLE : KS_PROCEED; break;
    case KS_PROCEED:
      if (!READ(BTN_HOME)) break;
      key_status = KS_IDLE;
      if (printingIsActive() || wait_for_user) {
        shared_feedrate_index = (shared_feedrate_index + 1) % COUNT(feedrates);
        char cmd[20]; sprintf_P(cmd, PSTR("M220 S%d"), feedrates[shared_feedrate_index]);
        queue.inject(cmd);
      } else {
        if (PENDING(millis(), key_time + 1200 - BTN_DEBOUNCE_MS)) queue.inject(F("G28"));
        else queue.inject(F("M306 T\nM500")); 
      }
      break;
  }
}

//
// XY Movement / Flow+ Button
//
void EasythreedUI::HandleButton1() {
  static uint8_t key1_status = 0; static millis_t key1_time = 0;
  switch (key1_status) {
    case 0: if (!READ(BTN_1)) { key1_time = millis(); key1_status = 1; } break;
    case 1: if (ELAPSED(millis(), key1_time + BTN_DEBOUNCE_MS)) key1_status = READ(BTN_1) ? 0 : 2; break;
    case 2:
      if (READ(BTN_1)) {
        key1_status = 0;
        if (printingIsActive() || wait_for_user) {
          shared_flowrate = min(shared_flowrate + 5, 200);
          char cmd[20]; sprintf_P(cmd, PSTR("M221 S%d"), shared_flowrate);
          queue.inject(cmd);
        } else queue.inject(F("G0 Z20\nG0 X124 Y115 F3000\nG0 Z30"));
      }
      break;
  }
}

//
// XY Movement / Flow- Button
//
void EasythreedUI::HandleButton2() {
  static uint8_t key2_status = 0; static millis_t key2_time = 0;
  switch (key2_status) {
    case 0: if (!READ(BTN_2)) { key2_time = millis(); key2_status = 1; } break;
    case 1: if (ELAPSED(millis(), key2_time + BTN_DEBOUNCE_MS)) key2_status = READ(BTN_2) ? 0 : 2; break;
    case 2:
      if (READ(BTN_2)) {
        key2_status = 0;
        if (printingIsActive() || wait_for_user) {
          shared_flowrate = max(shared_flowrate - 5, 50);
          char cmd[20]; sprintf_P(cmd, PSTR("M221 S%d"), shared_flowrate);
          queue.inject(cmd);
        } else queue.inject(F("G0 Z5\nG0 X107 Y7 F3000\nG0 Z0"));
      }
      break;
  }
}

//
// XY Movement / Temp+ Button
//
void EasythreedUI::HandleButton3() {
  enum KeyStatus : uint8_t { KS_IDLE, KS_PRESS, KS_PROCEED };
  static uint8_t key_status = KS_IDLE; static millis_t key_time = 0;
  switch (key_status) {
    case KS_IDLE: if (!READ(BTN_3)) { key_time = millis(); key_status = KS_PRESS; } break;
    case KS_PRESS: if (ELAPSED(millis(), key_time + BTN_DEBOUNCE_MS)) key_status = READ(BTN_3) ? KS_IDLE : KS_PROCEED; break;
    case KS_PROCEED:
      if (!READ(BTN_3)) break; key_status = KS_IDLE;
      if (printingIsActive() || wait_for_user) {
        const int new_temp = constrain(thermalManager.degTargetHotend(0) + 5, 0, UI_MAX_TEMP);
        char cmd[16]; sprintf_P(cmd, PSTR("M104 S%i"), new_temp);
        queue.inject(cmd);
      } else queue.inject(F("G0 Z5\nG0 X107 Y105 F3000\nG0 Z0"));
      break;
  }
}

//
// XY Movement / Temp- Button
//
void EasythreedUI::HandleButton4() {
  static uint8_t key4_status = 0; static millis_t key4_time = 0;
  switch (key4_status) {
    case 0: if (!READ(BTN_4)) { key4_time = millis(); key4_status = 1; } break;
    case 1: if (ELAPSED(millis(), key4_time + BTN_DEBOUNCE_MS)) key4_status = READ(BTN_4) ? 0 : 2; break;
    case 2:
      if (READ(BTN_4)) {
        key4_status = 0;
        if (printingIsActive() || wait_for_user) {
          const int new_temp = constrain(thermalManager.degTargetHotend(0) - 5, 0, UI_MAX_TEMP);
          char cmd[16]; sprintf_P(cmd, PSTR("M104 S%i"), new_temp);
          queue.inject(cmd);
        } else queue.inject(F("G0 Z5\nG0 X7 Y105 F3000\nG0 Z0"));
      }
      break;
  }
}

#endif // EASYTHREED_UI