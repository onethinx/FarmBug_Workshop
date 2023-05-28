# FarmBug Workshop Part 1: Adding Moisture Sensor

**IMPORTANT: For the FarmBug, we use the setup project we created earlier with the blinking LED. If you don't have this project, please complete [these steps](https://github.com/onethinx/Workshop_29May2023#2-farmbug-project-chip-configuration-firmware-coding-and-debugging) first.**

## 1 Change LED functionality
We want to change the automatic blinking LED and have the firmware determine the state of the LED. This way we can give useful information to the user about the status of the Farmbug.

1. Open the Onethinx_Creator project in PSoC Creator. (double click `Onethinx_Creator.cyprj` inside the project folder `..\OTX-FarmBug\Onethinx_Creator.cydsn` or select the `Onethinx_Creator.cyprj` from PSoC Creator's recent project list)
1. Open the TopDesign.cysch schematic from the Workspace Explorer (left bar in PSoC Creator)
1. Remove the circuitry except the LED IO symbol (select and press `delete`)
1. Double click the LED IO symbol to go to the properties of the IO pin
1. Uncheck `HW Connection` as shown below<br>
![LED HW connection](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/LED_HWconnect.png?raw=true)<br>
1. Also note that there's no circuitry left on the schematic except the LED_B IO pin
1. Build the project via the Build menu or by clicking the Build Symbol (or just press Shift+F6)
1. After building, open the Visual Studio Code project (if not already open)
1. Hit the Build-And-Launch button from the status bar at the bottom of VS Code
2. <br>
![debug session](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/debug_session.png?raw=true)<br>
