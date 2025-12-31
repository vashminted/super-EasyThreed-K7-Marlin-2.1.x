# super-EasyThreed-K7-Marlin-2.1.x

actualización completa para la EasyThreed k7

basado en el gran trabajo de schmttc para las impresoras EasyThreed con placas et4000+ y su documentación y esquemas proporcionados en https://github.com/schmttc/EasyThreeD-K7-STM32

Actualizado a Marlin 2.1.X bugfix (oct 2025)

Este firmware está destinado a usarse junto con mis mods para la EasyThreed K7, para reconstruir completamente la easythreed k7 y permitir el uso de motores paso a paso reales y un hotend. Los archivos STL y las imágenes de referencia se pueden encontrar en [Printables](https://www.printables.com/model/1527034-super-easythreed-k7) y los archivos originales en Tinkercad.

Este mod asume que estás usando al menos: 

-Motores paso a paso Nema 17 para todos los ejes y el extrusor.

-Fuente de alimentación de 12V 10AMP.

-Endstops para todos los ejes.

-Sin autonivelado.

-Heatbed (opcional).

-Extrusor Bowden Titan.

-Hotend MK8.

# Funciones incluidas
    -Autotune MPC para el hotend, mejor que PID y más estable.
    -Funcionalidad mejorada de los 4 botones de nivelación, botón home y el interruptor de feed/retract.
    -Linear advance e input shaping están habilitados por defecto.
    -Heatbed habilitado, establecer en cero en el slicer si no está instalado.
    -Pausa avanzada y comando m600 habilitados para cambio de filamento a mitad de impresión.

#  Comportamiento de los botones:

# PRINT Button

      Estado inactivo:
         -Pulsación corta: Imprime el último archivo en la tarjeta SD
         -Pulsación larga: Eleva el cabezal de impresión 2cm

      Impresión activa:
         -Pulsación corta: Pausa y mueve la boquilla a una zona segura
         -Pulsación larga: Cancela el trabajo de impresión actual

      Pausado / Esperando M600:
         -Pulsación corta: Reanuda la impresión

# FEED / RETRACT Buttons:

      Estado inactivo:
     -Precalienta la boquilla a 220C y carga/descarga el filamento
 
      Impresión activa:
     -Deshabilitado durante la impresión

      Pausado:
     -Precalienta la boquilla a 220C y carga/descarga el filamento, para cambios manuales de filamento
 
# Home button.

      Estado inactivo:
      -Pulsación corta: Home en todos los ejes (esquina 1 para nivelación)
      -Pulsación larga: Realiza el autotune de temperatura del hotend (M306)
  
      Impresión activa: Alterna la velocidad de impresión 100%, 125%, 160%, 200%} usando M220.

# Leveling buttons:
     
  # -Button1:
    -Inactivo: Mueve el cabezal de impresión a una zona segura
    -Imprimiendo: incrementa el flujo +5%
 #  -Button2:
    -Inactivo: Mueve a la esquina 2 para nivelación
    -Imprimiendo: reduce el flujo -5%
 # -Button3:
    -Inactivo: Mueve a la esquina 3 para nivelación
    -Imprimiendo: incrementa la temperatura +5 grados
#  -Button4:
    -Inactivo: Mueve a la esquina 4 para nivelación
    -Imprimiendo: reduce la temperatura -5 grados

Instalacion:
El bootloader de la placa es propietario de MKS, el cual lee un archivo binario de firmware llamado mksLite.bin desde la tarjeta SD al arrancar.

  Copia mksLite.bin a la tarjeta SD y reinicia la impresora
  Después de un corto tiempo (<30s) el firmware se escribe en la placa, y mksLite.bin en la tarjeta SD se renombra a mksLite.CUR

  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#  complete upgrade for the EasyThreed k7

based on the great work of schmttc for the EasyThreed printers with et4000+ boards and his documentation and schematics provided on https://github.com/schmttc/EasyThreeD-K7-STM32

Updated to Marlin 2.1.X bugfix (oct 2025)

This firmware is intended to be used with my EasyThreed K7 mods to completely rebuild the easythreed k7 to use real stepper mottors and hotend. Stl files and reference images can be found on printables and the original files on thinkercad.

This mod assumes you are using at least: 

-Nema 17 stepper motors for all axis and the extruder.

-12V 10AMP power supply.

-endstops for all axis.

-no autolevel.

-Heatbed (optional).

-Bowden titan extruder.

-MK8 hotend.

# Features included
    -MPC auto tune for the hoted, better than PID and more stable.
    -improved functionality of the 4 leveling buttons, home button, and feed/retract switch.
    -linear advance and input shaping are enabled by defaut.
    -Heatbed is enabled, set to zero on slicer if not installed.
    -Advanced pause and m600 command enabled for mid-print filament change.

#  Button behavior:

# PRINT Button

      Idle State:
         -Short Press: Prints the latest file on sd card
         -Long Press: Rises the printhead 2cm

      Active Printing:
         -Short Press: Pauses and moves the nozzle to a safe zone
         -Long Press: cancels current print job

      Paused / M600 Waiting:
         -Short Press: Resumes priniting

# FEED / RETRACT Buttons:

      Idle State:
     -preheat the nozzle to 220C and loads/unloads filament
 
      Active Printing:
     -disabled during printing

      Paused:
     -preheat the nozzle to 220C and loads/unloads filament, for manual filament changes
 
# Home button.

      Idle State:
      -Short Press: Home all axes (corner 1 for leveling)
      -Long Press: Performs hotend temperature autotune (M306)
  
      Active Printing: Cycles through printing speed 100%, 125%, 160%, 200%} using M220.

# Leveling buttons:
     
  # -Button1:
    -Idle: Moves printhead to safe zone
    -Printing: increases flow +5%
 #  -Button2:
    -Idle: Moves to corner 2 for leveling
    -Printing: decreases flow -5%
 # -Button3:
    -Idle: Moves to corner 3 for leveling
    -Printing: increases temp +5 degrees
#  -Button4:
    -Idle: Moves to corner 4 for leveling
    -Printing: decreases temp -5 degrees

Installation:
The board's bootloader is proprietary by MKS, which reads a binary firmware file mksLite.bin from the SD card on boot.

  Copy mksLite.bin to the SD card, and restart the printer
  After a short time (<30s) the firmware is written to the board, and mksLite.bin on the SD card is renamed to mksLite.CUR




