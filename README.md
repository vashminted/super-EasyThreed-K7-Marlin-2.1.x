# super-EasyThreed-K7-Marlin-2.1.x

### Complete upgrade for the EasyThreed K7
Based on the great work of [schmttc](https://github.com/schmttc/EasyThreeD-K7-STM32) for the EasyThreed printers with **et4000+** boards.

**Updated to Marlin 2.1.2.6**

This firmware is intended to be used with my EasyThreed K7 mods to completely rebuild the printer using Nema 17 stepper motors and a standard hotend. 

<img width="756" height="664" alt="Screenshot 2025-12-24 031906" src="https://github.com/user-attachments/assets/d533a417-b0a7-4f59-a185-824a3ec6577a" />


* **STL Files:** [Printables](https://www.printables.com/model/1527034-super-easythreed-k7)
* **Original Files:** [Tinkercad](https://www.tinkercad.com/things/4VaAuOSUpJB-easythreed-k7-complete-upgrade)
---

## Hardware Requirements
This mod assumes you are using at least:
* **Motors:** Nema 17 stepper motors for all axis and extruder.
* **Power:** 12V 10AMP power supply.
* **Sensors:** Endstops for all axis (No autolevel).
* **Extrusion:** Bowden Titan extruder & MK8 Hotend.
* **Optional:** Heatbed (Enabled by default).

## Features Included
* **MPC Auto Tune:** Advanced hotend temperature control, more stable than PID.
* **Linear Advance & Input Shaping:** Enabled by default for better print quality.
* **Smart Blink:** Status LED frequency scales dynamically with printing speed (M220).
* **Advanced Pause:** M600 command enabled for filament change.

---

## Button Behavior (English)

### [ PRINT Button ]
| State | Short Press | Long Press |
| :--- | :--- | :--- |
| **Idle** | **Selection Mode:** 1st click mounts SD. Extra clicks select file index. (5s timeout to start) | Rises the printhead 2cm |
| **Printing** | Pauses print and moves head to parking zone (M125/M25) | Cancels current print job |
| **Paused** | Resumes printing (M24) | Cancels current print job |

### [ FEED / RETRACT Switch ]
* **Idle / Paused:** Preheats nozzle to 230°C and starts filament load/unload.
* **Active Printing:** Disabled for safety.

### [ HOME Button ]
* **Idle:** Short Press for **Home All** (Corner 1). Long Press for **MPC Autotune** (M306).
* **Printing:** Cycles printing speed: **100% -> 125% -> 160% -> 200%** (LED blinks faster).

### [ Leveling Buttons 1-4 ]
| Button | Idle (Leveling) | Printing (Live Adjust) |
| :--- | :--- | :--- |
| **Button 1** | Move to safe zone | Increase Flow **+5%** |
| **Button 2** | Move to Corner 2 | Decrease Flow **-5%** |
| **Button 3** | Move to Corner 3 | Increase Temp **+5°C** (Max 250) |
| **Button 4** | Move to Corner 4 | Decrease Temp **-5°C** |

---

## Installation
The board's bootloader is proprietary by MKS. It reads a binary file named `mksLite.bin`.

1.  Copy `mksLite.bin` to the SD card root.
2.  Restart the printer.
3.  Wait (<30s). The firmware is written when `mksLite.bin` is renamed to `mksLite.CUR`.

---
---

# actualización completa para la EasyThreed k7

Basado en el trabajo de [schmttc](https://github.com/schmttc/EasyThreeD-K7-STM32) para placas **et4000+**.

**Actualizado a Marlin 2.1.X bugfix (Dic 2025)**

Este firmware permite reconstruir completamente la EasyThreed K7 para usar motores paso a paso reales y hotend estándar.

## Requisitos de Hardware
Este mod asume el uso de:
* **Motores:** Nema 17 en todos los ejes y extrusor.
* **Fuente:** 12V 10AMP.
* **Finales de carrera:** Endstops en todos los ejes (Sin autonivelado).
* **Extrusión:** Extrusor Bowden Titan y Hotend MK8.
* **Opcional:** Cama caliente (Habilitada por defecto).

## Funciones Incluidas
* **Autotune MPC:** Control de temperatura avanzado, más estable que PID.
* **Linear Advance e Input Shaping:** Habilitados por defecto.
* **Smart Blink:** El parpadeo del LED de estado escala con la velocidad de impresión (M220).
* **Pausa Avanzada:** Comando M600 habilitado para cambio de filamento.

---

## Comportamiento de los Botones (Español)

### [ Botón PRINT ]
| Estado | Pulsación Corta | Pulsación Larga |
| :--- | :--- | :--- |
| **Inactivo** | **Modo Selección:** 1er clic monta SD. Clics extra seleccionan archivo. (5s de espera para iniciar) | Eleva el cabezal 2cm |
| **Imprimiendo** | Pausa la impresión y mueve al parking (M125/M25) | Cancela el trabajo actual |
| **Pausado** | Reanuda la impresión (M24) | Cancela el trabajo actual |

### [ Interruptor FEED / RETRACT ]
* **Inactivo / Pausado:** Precalienta a 230°C y activa carga/descarga de filamento.
* **Imprimiendo:** Deshabilitado por seguridad.

### [ Botón HOME ]
* **Inactivo:** Corta para **Home** (Esquina 1). Larga para **Autotune MPC** (M306).
* **Imprimiendo:** Cicla velocidad: **100% -> 125% -> 160% -> 200%** (LED parpadea más rápido).

### [ Botones de Nivelación 1-4 ]
| Botón | Inactivo (Nivelación) | Imprimiendo (Ajuste Vivo) |
| :--- | :--- | :--- |
| **Botón 1** | Mover a zona segura | Incrementar Flujo **+5%** |
| **Botón 2** | Mover a Esquina 2 | Reducir Flujo **-5%** |
| **Botón 3** | Mover a Esquina 3 | Incrementar Temp **+5°C** (Máx 250) |
| **Botón 4** | Mover a Esquina 4 | Reducir Temp **-5°C** |

---

## Instalación
El bootloader de la placa lee un archivo binario llamado `mksLite.bin` desde la SD.

1.  Copia `mksLite.bin` a la raíz de la tarjeta SD.
2.  Reinicia la impresora.
3.  Espera (<30s). El firmware se escribe y el archivo se renombra a `mksLite.CUR`.
