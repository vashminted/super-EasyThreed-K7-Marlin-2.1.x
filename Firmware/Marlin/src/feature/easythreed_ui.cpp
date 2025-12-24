/**
 * Marlin 3D Printer Firmware - Easythreed UI
 * Final: M125 para Pausa Manual / M108 para continuar M600 de G-Code.
 */

#include "../inc/MarlinConfigPre.h"

#if ENABLED(EASYTHREED_UI)

#include "easythreed_ui.h"
#include "pause.h"
#include "../module/temperature.h"
#include "../module/printcounter.h"
#include "../sd/cardreader.h"
#include "../gcode/queue.h"
#include "../module/motion.h"
#include "../module/planner.h"
#include "../MarlinCore.h"
#include "../module/stepper.h"
#include "../inc/MarlinConfig.h"

EasythreedUI easythreed_ui;

#define BTN_DEBOUNCE_MS 20
#define UI_MAX_TEMP 250

static uint8_t shared_flowrate = 100; 
static uint8_t shared_feedrate_index = 1;

enum LEDInterval : uint16_t {
  LED_OFF     =    0,
  LED_ON      = 4000, 
  LED_BLINK_2 = 1000,
  LED_BLINK_5 =  300,
  LED_BLINK_7 =   50
};

uint16_t blink_interval_ms = LED_ON;   

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
}

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

void EasythreedUI::blinkLED() {
  static millis_t prev_blink_interval_ms = 0, blink_start_ms = 0;
  if (blink_interval_ms == LED_OFF) { WRITE(EASYTHREED_LED_PIN, HIGH); return; }
  if (blink_interval_ms >= LED_ON)  { WRITE(EASYTHREED_LED_PIN,  LOW); return; }
  const millis_t ms = millis();
  if (prev_blink_interval_ms != blink_interval_ms) {
    prev_blink_interval_ms = blink_interval_ms;
    blink_start_ms = ms;
  }
  if (PENDING(ms, blink_start_ms, blink_interval_ms)) WRITE(EASYTHREED_LED_PIN, LOW);
  else if (PENDING(ms, blink_start_ms, 2 * blink_interval_ms)) WRITE(EASYTHREED_LED_PIN, HIGH);
  else blink_start_ms = ms;
}

// Boton Filamento: 220C. Funciona si la impresora está en IDLE o PAUSADA (M25 o M600)
void EasythreedUI::loadButton() {
  if (printingIsActive() && !print_job_timer.isPaused()) return;

  enum FilamentStatus : uint8_t { FS_IDLE, FS_PRESS, FS_CHECK, FS_PROCEED };
  static uint8_t filament_status = FS_IDLE;
  static millis_t filament_time = 0;

  switch (filament_status) {
    case FS_IDLE:
      if (!READ(BTN_RETRACT) || !READ(BTN_FEED)) {
        filament_status++;
        filament_time = millis();
      }
      break;

    case FS_PRESS:
      if (ELAPSED(millis(), filament_time, BTN_DEBOUNCE_MS)) {
        if (!READ(BTN_RETRACT) || !READ(BTN_FEED)) {
          thermalManager.setTargetHotend(220, 0); 
          blink_interval_ms = LED_BLINK_7;
          filament_status++;
        }
        else filament_status = FS_IDLE;
      }
      break;

    case FS_CHECK:
      if (READ(BTN_RETRACT) && READ(BTN_FEED)) {
        blink_interval_ms = LED_ON;
        filament_status = FS_IDLE;
        if (!print_job_timer.isPaused()) thermalManager.disable_all_heaters();
      }
      else if (thermalManager.degHotend(0) >= 220) {
        filament_status++;
        blink_interval_ms = LED_BLINK_5;
      }
      break;

    case FS_PROCEED: {
      static bool flag = false;
      if (READ(BTN_RETRACT) && READ(BTN_FEED)) {
        flag = false;
        filament_status = FS_IDLE;
        quickstop_stepper();
        if (!print_job_timer.isPaused()) thermalManager.disable_all_heaters();
        blink_interval_ms = LED_ON;
      }
      else if (!flag) {
        flag = true;
        // Inyecta carga o descarga usando modo relativo (M83)
        queue.inject(!READ(BTN_RETRACT) ? F("M83\nG1 E540 F2000\nG1 E60 F120\nM400") : F("M83\nG1 E3 F180\nG1 E-40 F3000\nG1 E-560 F1000\nM400"));
      }
    } break;
  }
}

#if HAS_STEPPER_RESET
  void disableStepperDrivers();
#endif

// Boton Print: Maneja la Pausa Manual (M125+M25) y libera el M600 (M108)
void EasythreedUI::printButton() {
  enum KeyStatus : uint8_t { KS_IDLE, KS_PRESS, KS_PROCEED };
  static uint8_t key_status = KS_IDLE;
  static millis_t key_time = 0;
  enum PrintFlag : uint8_t { PF_START, PF_PAUSE, PF_RESUME };
  static PrintFlag print_key_flag = PF_START;
  const millis_t ms = millis();

  switch (key_status) {
    case KS_IDLE:
      if (!READ(BTN_PRINT)) { key_time = ms; key_status++; }
      break;
    case KS_PRESS:
      if (ELAPSED(ms, key_time, BTN_DEBOUNCE_MS))
        key_status = READ(BTN_PRINT) ? KS_IDLE : KS_PROCEED;
      break;
    case KS_PROCEED:
      if (!READ(BTN_PRINT)) break;
      key_status = KS_IDLE;
      if (PENDING(ms, key_time, 1200 - BTN_DEBOUNCE_MS)) {

        // --- LOGICA DE DESBLOQUEO PARA M600 ---
        // Si hay una pausa activa, intentamos liberar a Marlin
        if (print_job_timer.isPaused()) {
            queue.inject(F("M108")); // Continúa M600 si está esperando
            queue.inject(F("M24"));  // Reanuda si era una pausa manual M25
            blink_interval_ms = LED_BLINK_2;
            print_key_flag = PF_PAUSE;
            return;
        }

        switch (print_key_flag) {
          case PF_START: {
            if (printingIsActive()) break;
            blink_interval_ms = LED_BLINK_2;
            print_key_flag = PF_PAUSE;
            card.mount();
            if (!card.isMounted()) { blink_interval_ms = LED_OFF; print_key_flag = PF_START; return; }
            card.ls();
            card.selectFileByIndex(card.get_num_items());
            card.openAndPrintFile(card.filename);
          } break;

          case PF_PAUSE: {
            blink_interval_ms = LED_ON;
            queue.inject(F("M125")); // Aparca el cabezal
            queue.inject(F("M25"));  // Pausa la ejecución desde SD
            print_key_flag = PF_RESUME;
          } break;

          case PF_RESUME: {
            // Este caso es un "fallback", la lógica de arriba con M108/M24 suele atraparlo
            blink_interval_ms = LED_BLINK_2;
            queue.inject(F("M24")); 
            print_key_flag = PF_PAUSE;
          } break;
        }
      }
      else {
        // Largo: Z+20 o Cancelar
        if (print_key_flag == PF_START && !printingIsActive())  {
          blink_interval_ms = LED_ON;
          queue.inject(F("G91\nG0 Z20 F600\nG90"));
        }
        else {
          card.abortFilePrintSoon();
          blink_interval_ms = LED_OFF;
        }
        planner.synchronize();
        TERN_(HAS_STEPPER_RESET, disableStepperDrivers());
        print_key_flag = PF_START;
        blink_interval_ms = LED_ON;
      }
      break;
  }
}

// Botones de movimiento y utilidades (Igual que antes)
void EasythreedUI::HomeButton() {
  enum KeyStatus : uint8_t { KS_IDLE, KS_PRESS, KS_PROCEED };
  static uint8_t key_status = KS_IDLE;
  static millis_t key_time = 0;
  constexpr uint8_t feedrates[] = {50, 100, 125, 160, 200};
  switch (key_status) {
    case KS_IDLE:
      if (!READ(BTN_HOME)) { key_time = millis(); key_status = KS_PRESS; }
      break;
    case KS_PRESS:
      if (ELAPSED(millis(), key_time, BTN_DEBOUNCE_MS)) key_status = READ(BTN_HOME) ? KS_IDLE : KS_PROCEED;
      break;
    case KS_PROCEED:
      if (!READ(BTN_HOME)) break;
      key_status = KS_IDLE;
      const millis_t ms = millis();
      if (printingIsActive()) {
        shared_feedrate_index = (shared_feedrate_index + 1) % COUNT(feedrates);
        char cmd[20]; sprintf_P(cmd, PSTR("M220 S%d"), feedrates[shared_feedrate_index]);
        queue.inject(cmd);
      } else {
        if (PENDING(ms, key_time + 1200 - BTN_DEBOUNCE_MS)) queue.inject(F("G28"));
        else queue.inject(F("G28\nM306 T\nM500")); 
      }
      break;
  }
}

void EasythreedUI::HandleButton1() {
  static uint8_t key1_status = 0;
  static millis_t key1_time = 0;
  const millis_t ms = millis();
  switch (key1_status) {
    case 0: if (!READ(BTN_1)) { key1_time = ms; key1_status = 1; } break;
    case 1: if (ELAPSED(ms, key1_time, BTN_DEBOUNCE_MS)) key1_status = READ(BTN_1) ? 0 : 2; break;
    case 2:
      if (READ(BTN_1)) {
        key1_status = 0;
        if (printingIsActive()) {
          shared_flowrate = min(shared_flowrate + 5, 200);
          char cmd[20]; sprintf_P(cmd, PSTR("M221 S%d"), shared_flowrate);
          queue.inject(cmd);
        } else queue.inject(F("G0 Z20\nG0 X115 Y120 F3000\nG0 Z30"));
      }
      break;
  }
}

void EasythreedUI::HandleButton2() {
  static uint8_t key2_status = 0;
  static millis_t key2_time = 0;
  const millis_t ms = millis();
  switch (key2_status) {
    case 0: if (!READ(BTN_2)) { key2_time = ms; key2_status = 1; } break;
    case 1: if (ELAPSED(ms, key2_time, BTN_DEBOUNCE_MS)) key2_status = READ(BTN_2) ? 0 : 2; break;
    case 2:
      if (READ(BTN_2)) {
        key2_status = 0;
        if (printingIsActive()) {
          shared_flowrate = max(shared_flowrate - 5, 50);
          char cmd[20]; sprintf_P(cmd, PSTR("M221 S%d"), shared_flowrate);
          queue.inject(cmd);
        } else queue.inject(F("G0 Z5\nG0 X107 Y7 F3000\nG0 Z0"));
      }
      break;
  }
}

void EasythreedUI::HandleButton3() {
  enum KeyStatus : uint8_t { KS_IDLE, KS_PRESS, KS_PROCEED };
  static uint8_t key_status = KS_IDLE;
  static millis_t key_time = 0;
  const millis_t ms = millis();
  switch (key_status) {
    case KS_IDLE: if (!READ(BTN_3)) { key_time = ms; key_status = KS_PRESS; } break;
    case KS_PRESS: if (ELAPSED(ms, key_time, BTN_DEBOUNCE_MS)) key_status = READ(BTN_3) ? KS_IDLE : KS_PROCEED; break;
    case KS_PROCEED:
      if (!READ(BTN_3)) break;
      key_status = KS_IDLE;
      if (!print_job_timer.isRunning()) queue.inject(F("G0 Z5\nG0 X107 Y105 F3000\nG0 Z0"));
      else {
        const int current = thermalManager.degTargetHotend(0);
        const int new_temp = constrain(current + 5, 0, UI_MAX_TEMP);
        char cmd[16]; sprintf_P(cmd, PSTR("M104 S%i"), new_temp);
        queue.inject(cmd);
      }
      break;
  }
}

void EasythreedUI::HandleButton4() {
  static uint8_t key4_status = 0;
  static millis_t key4_time = 0;
  const millis_t ms = millis();
  switch (key4_status) {
    case 0: if (!READ(BTN_4)) { key4_time = ms; key4_status = 1; } break;
    case 1: if (ELAPSED(ms, key4_time, BTN_DEBOUNCE_MS)) key4_status = READ(BTN_4) ? 0 : 2; break;
    case 2:
      if (READ(BTN_4)) {
        key4_status = 0;
        if (!print_job_timer.isRunning()) queue.inject(F("G0 Z5\nG0 X7 Y105 F3000\nG0 Z0"));
        else {
          const int current = thermalManager.degTargetHotend(0);
          const int new_temp = constrain(current - 5, 0, UI_MAX_TEMP);
          char cmd[16]; sprintf_P(cmd, PSTR("M104 S%i"), new_temp);
          queue.inject(cmd);
        }
      }
      break;
  }
}
#endif