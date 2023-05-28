# FarmBug Workshop Part 2: Implementing Light and Temperature

The Light (LDR) and Temperature sensors (NTC) are analog devices. They will be powered from an IO which can be switched to low (0) to save power and prevent the battery from being drain when the sensor isn't active. The analog lines (V_LDR for Light, V_NTC1 for the Soil Sensor and V_NTC2 for the Air Sensor) will be connected to the Analog to Digital Converter (ADC) which we will configure in this part of the workshop.

## 1 Implementing the Analog Light and Temperature Sensors
To implement the reading of the sensors, we have to add the ADC UserModule to the schematic page, add the IOs for powering the sensors and write the implementing code.

### 1.1 Add the ADC UserModule
1. Open the PSoC Creator project and go to the TopDesign.cysch schematic from the Workspace Explorer (left bar)
1. Select the CapSense usermodule from the Component Catalog (right bar: `Cypress -> Analog -> ADC -> Scanning SAR ADC [v3.10]`)
1. Drag and drop the ADC usermodule onto the schematic page:
1. Double click the ADC usermodule open the configuration window:<br>
![ADC config](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/ADC_config.png?raw=true)<br>
1. Change the name of the component to: `ADC`
1. Change the Vref select to: `Vdda/2` in order to be able to sample full voltage scale
1. We need to sample 3 channels, set the Number of channels to: `3`
1. As we want to measure non-differential signals (we want to measure the voltage to GND), set all the channel modes to: `Single ended`
1. Click the `OK` button to make the changes.

### 1.2 Add the analog and digital sensor pins:
1. Drag and drop 3 Analog Pins and connect them to the ADC UserModule:<br>
![Sensor IO pins](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/Sensor_IO_pins.png?raw=true)<br>
1. Drag and drop 3 Digital Output Pins<br>
1. Rename Pin_1 to: V_LDR<br>
1. Rename Pin_2 to: V_NTC1<br>
1. Rename Pin_3 to: V_NTC2<br>
1. Rename Pin_4 to: PWR_LDR and uncheck the `HW connection` option<br>
1. Rename Pin_5 to: PWR_NTC1 and uncheck the `HW connection` option<br>
1. Rename Pin_6 to: PWR_NTC2 and uncheck the `HW connection` option<br>
1. After configuration, the setup should look like this:<br>
![Sensor IO Pins Configured](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/Sensor_IO_Pins_Configured.png?raw=true)<br>
1. Build the project via the Build menu or by clicking the Build Symbol (or just press Shift+F6)
1. Wait for the build to succeed
1. Repeat the last build step.

### 1.3 Set the correct IO pins for powering the Sensors and interfacing the Analog Signals
1. Look up the following IO pins in the [FarmBug schematic](https://github.com/onethinx/Workshop_29May2023/blob/main/Assets/FarmBug_schematic.png?raw=true) and note the corresponding IO pin:<br>
  1.1 Light sensor power: PWR_LDR<br>
  1.2 Soil sensor power: PWR_NTC1<br>
  1.3 Air sensor power: PWR_NTC2<br>
  1.4 Light sensor power: V_LDR<br>
  1.5 Soil sensor signal: V_NTC1<br>
  1.6 Air sensor signal: V_NTC2<br>
1. Open the Pins view from the WorkSpace Explorer under Design Wide Resources
1. Configure the Sensor IO pins (dropdown arrow) wiith the schematic Pin numbering as just found.

### 1.4 Write code for the ADC implementation
1. Switch to the Visual Studio Code project
2. Add the following code after the CapSense_Start() function to initialize / configure the ADC at startup:<br>
```
	/* Start the ADC usermodule */
	ADC_Start();
```
3. Add the following code after the CapSense_Start() function to Power up the Analog sensors
```
		/* Power Sensors ON */
		Cy_GPIO_Write(PWR_LDR_PORT, PWR_LDR_NUM, 1);
		Cy_GPIO_Write(PWR_NTC1_PORT, PWR_NTC1_NUM, 1);
		Cy_GPIO_Write(PWR_NTC2_PORT, PWR_NTC2_NUM, 1);
		CyDelay(10);									// Allow some time to power sensors and have a stable voltage reading
```
4. Add the following code thereafter to sample the Analog sensors voltages and turn on the LED below a Light sensor value of 3000 (Light value is inversely proportional with light intensity)
```
		/* Sample Analog Sensors */
		ADC_StartConvert();
		ADC_IsEndConversion(CY_SAR_WAIT_FOR_RESULT);
		volatile uint32_t adcResult_Light = ADC_GetResult32(0);
		volatile uint32_t adcResult_temperatureSoil = ADC_GetResult32(1);
		volatile uint32_t adcResult_temperatureAir = ADC_GetResult32(2);
		/* Turn the LED on when the ADC light value is below 3000 */
		Cy_GPIO_Write(LED_B_PORT, LED_B_NUM,  adcResult_Light < 3000);
```
5. Add the following code thereafter to Power down the Analog sensors
```
		/* Power Sensors OFF */
		Cy_GPIO_Write(PWR_LDR_PORT, PWR_LDR_NUM, 0);
		Cy_GPIO_Write(PWR_NTC1_PORT, PWR_NTC1_NUM, 0);
		Cy_GPIO_Write(PWR_NTC2_PORT, PWR_NTC2_NUM, 0);
```
6. Comment (disable code by turning it into a comment) the CapSense LED switching code so it doesn't interfere with the LED light level switching (line 77, may be slightly different for you)
```
		//Cy_GPIO_Write(LED_B_PORT, LED_B_NUM,  CapSense_Value > 3800);
```
7. If the instructions are followed correctly, the code should look like [this](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/code_3.2.png?raw=true)
8. Hit the `Build-And-Launch` button from the status bar at the bottom of VS Code
9. Cross fingers and hopefully the Blue LED will turn on as enough light falls on the Light Sensor 🎉
10. Congratulations, you're now a qualified Onethinx Seasoned Engineer<br>🤓
