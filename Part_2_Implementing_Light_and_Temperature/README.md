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

### 2.2 Set the correct pins for the Powering IOs and the Analog Signals
1. Look up which IO pin the Sensor element is connected to. The sensor element in the [FarmBug schematic](https://github.com/onethinx/Workshop_29May2023/blob/main/Assets/FarmBug_schematic.png?raw=true) is indicated with C+:<br>
