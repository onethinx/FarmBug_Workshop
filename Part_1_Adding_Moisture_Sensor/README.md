# FarmBug Workshop Part 1: Adding Moisture Sensor

**IMPORTANT: For the FarmBug, we use the setup project we created earlier with the blinking LED. If you don't have this project, please complete [these steps](https://github.com/onethinx/Workshop_29May2023#2-farmbug-project-chip-configuration-firmware-coding-and-debugging) first.**

## 1 Change LED functionality
We want to change the automatic blinking LED and have the firmware determine the state of the LED. This way we can give useful information to the user about the status of the Farmbug.

### 1.1 Configure the LED IO pin for use in firmware 
1. Open the Onethinx_Creator project in PSoC Creator. (double click `Onethinx_Creator.cyprj` inside the project folder `..\OTX-FarmBug\Onethinx_Creator.cydsn` or select the `Onethinx_Creator.cyprj` from PSoC Creator's recent project list)
1. Open the TopDesign.cysch schematic from the Workspace Explorer (left bar in PSoC Creator)
1. Remove the circuitry except the LED IO symbol (select and press `delete`)
1. Double click the LED IO symbol to go to the properties of the IO pin
1. Uncheck `HW Connection` as shown below:<br>
![LED HW connection](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/LED_HWconnect.png?raw=true)<br>
1. Also note that there's no circuitry left on the schematic except the LED_B IO pin
1. Build the project via the Build menu or by clicking the Build Symbol (or just press Shift+F6)
1. Wait for the build to succeed

### 1.2 Write code to flash the LED 3 times at startup
1. Open the Visual Studio Code project (if not already open)
2. Include the PSoC Creator project by adding the following line before the main function (line 42)<br>
```
#include "project.h"
```
3. Add the following code at the start of the main function to have the blue LED flash 3 times at startup:<br>
```
	/* Flash the LED 3 times at start */
	for (uint32_t cnt = 0; cnt < 3; cnt++)				// Make a loop which runs 3 times
	{
		Cy_GPIO_Write(LED_B_PORT, LED_B_NUM, 1);		// Turn LED on
		CyDelay(300);									// Wait 300 milliseconds
		Cy_GPIO_Write(LED_B_PORT, LED_B_NUM, 0);		// Turn LED off
		CyDelay(300);
	}
```
4. Hit the `Build-And-Launch` button from the status bar at the bottom of VS Code<br>
  Real time debugging is accomplished with the debug buttons as shown:<br>
![debug session](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/debug_session.png?raw=true)<br>
  You can play around with the debug buttons to see it's functionality (e.g. step into the CyDelay function)
  
## 2 Add the Capacitive moisture sensor
The moisture sensor consists of a capacitive element which can be perfectly implemented by the CapSense usermodule of the PSoC6. First we'll implement the CapSense chip configuration and second we will add the code for reading the CapSense value.

### 2.1 Adding the CapSense UserModule
1. Open the PSoC Creator project and go to the TopDesign.cysch schematic from the Workspace Explorer (left bar)
1. Select the CapSense usermodule from the Component Catalog (right bar: `Cypress -> CapSense -> CapSense [v3.0]`)
1. Drag and drop the CapSense usermodule onto the schematic page:<br>
![CapSense_UM](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/CapSense_UM.png?raw=true)<br><br>
1. Double click the CapSense usermodule open the configuration window:<br>
![add button](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/add_button.png?raw=true)<br>
1. Change the name of the component to: `CapSense`
1. Change the CSD tuning mode to: `Manual tuning`
1. Add the CapSense sensing element (Button) by hitting the '+' icon on the left
1. Click the `OK` button to make the changes.

### 2.1 Set the correct pins for the CapSense UserModule
Capsense requires one IO to connect to a modulation capacitor. The OTX-18 has an internal connection of P7[7] to P10[2]. To use Capsense with the Onethinx module, the modulation capacitor needs to be connected to P10[2] and the PSoC Creator project has to be configured for a modulation capacitor at P7[7].
1. Look up which IO pin the Sensor element is connected to. The sensor element in the [FarmBug schematic](https://github.com/onethinx/Workshop_29May2023/blob/main/Assets/FarmBug_schematic.png?raw=true) is indicated with C+:<br>
![IO_config](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/IO_config.png?raw=true)<br>
1. Open the Pins view inside PSoC Creator and set the modulation capacitor `\CapSense:Cmod\ (Cmod)` to P7[7]:<br>
![CapSense pin](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/CapSense_pin.png?raw=true)<br>
1. Set the CapSense sensor element `\CapSense:Sns\ (Cmod)` to the correct IO pin as found in the schematic above.
1. Build the project via the Build menu or by clicking the Build Symbol (or just press Shift+F6)
1. Wait for the build to succeed
1. Repeat the last build step (info for nerds: because PSoC Creator runs the IDE integration after the postbuild action, unfortunately).
2. 
### 2.2 Write code for the CapSense implementation
1. Switch to the Visual Studio Code project
2. Include the OTX18-EnableCapSense to enable CapSense for the module by adding the following line before the main function (line 43)<br>
```
#include "OTX18-EnableCapSense.h"
```
3. Add the following code at the start of the main function to initialize / configure CapSense at startup:<br>
```
	/* Enable Global Interrupts */
	__enable_irq();

	/* Enable CapSense on the OTX-18 */
	OTX18_EnableCapSense();
	/* Start the CapSense usermodule */
	CapSense_Start();
```
4. Add the following code between the curly braces of the Main Loop to process the CapSense functions and read the CapSense value (moisture):<br>
```
		/* Call functions to get CapSense value */
		CapSense_ProcessAllWidgets();
		CapSense_ScanAllWidgets();
		while (CapSense_IsBusy()) {}					// Wait till CapSense function is finished

		/* Read raw CapSense value directly from the register */
		uint16_t CapSense_Value = *CapSense_dsRam.snsList.button0[0].raw;
		/* Turn the LED on when the CapSense value exceeds 3800 */
		Cy_GPIO_Write(LED_B_PORT, LED_B_NUM,  CapSense_Value > 3800);
```
5. If the instructions are followed correctly, the code should look like [this](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/code_2.2.png?raw=true)
6. Hit the `Build-And-Launch` button from the status bar at the bottom of VS Code<br>
