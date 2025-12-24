
# super-EasyThreed-K7-Marlin-2.1.x
complete upgrade for the EasyThreed k7

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

Button behavior:

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




