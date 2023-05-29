# FarmBug Workshop Part 3: LoRaWAN Implementation

This final part will cover how to implement the LoRaWAN functionality to our FarmBug.

## 1 Implementing the LoRaWAN core functionality

LoRaWAN needs several parameters to funtion correctly e.g:
- region (US, EU, India, Australia, China etc)
- speed and power, also called datarate (automatic = ADR or fixed = manual)
- Keys (to identify to the network).

When the device first comes to air, it needs to exchange the (security) keys and channel parameters. This procedure is called "joining". A device can be pre-joined (ABP = activation by personalization) or join as soon as it goes to air (OTAA = over the air activation).
The OTX-18 needs additional configuration for basic system functionality:
- M0 mode: the separate Cortex M0 core can be put in (deep) sleep while being idle to save energy
- BleEco: the External Crystal Oscillator can be set to 'always on" to suit specific BLE functionality
- DebugOn: the debug port can be set to 'always on" to suit debugging while the module goes to sleep.

### 1.1 Configuring the LoRaWAN parameters, the basic system and the LoRaWAN keys
For this demo we use the following LoRaWAN settings:
- region: Europe (EU)
- datarate: automatic (ADR).

The following basic system settings will be used:
- M0 mode: M0_DeepSleep (put M0 in DeepSleep when waiting in order to save battery)
- BleEcoON: off (= false: no specific BLE functionality is needed to have the BLE ECO on)
- DebugON: on (= true: we like to be able to debug even with the device in sleep).

The personal keys for joining the network will be handed out at the beginning of the workshop.

1. Put the following code after `#include "OTX18-EnableCapSense.h"` (line 42) to implement the configuration as described above:
```
#include "OnethinxCore01.h"
#include "OnethinxExt01.h"
#include "LoRaWAN_keys.h"

coreConfiguration_t	coreConfig = {
	.Join =
	{
		.KeysPtr = 			&TTN_OTAAkeys,
		.DataRate =			DR_AUTO,
		.Power =			PWR_MAX,
		.MAXTries = 		100,
		.SubBand_1st =     	EU_SUB_BANDS_DEFAULT,
		.SubBand_2nd =     	EU_SUB_BANDS_DEFAULT
	},
	.TX =
	{
		.Confirmed = 		false,
		.DataRate = 		DR_ADR,
		.Power = 			PWR_ADR,
		.FPort = 			1
	},
	.RX =
	{
		.Boost = 			true
	},
	.System =
	{
		.Idle =
		{
			.Mode = 		M0_DeepSleep,
			.BleEcoON =		false,
			.DebugON =		true,
		}
	}
};
```
2. Open the LoRaWAN_keys.h file by clicking the `Explorer` icon at the left panel of Visual Studio Code<br>
![LoRaWAN keys](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/LoRaWAN_keys.png?raw=true)<br>
3. Fill in the keys you got at the beginning of the workshop.
 
### 1.2 Implementing the LoRaWAN function calls
Implementing LoRaWAN with the OTX-18 is really simple, we only need to have three function calls:
- `LoRaWAN_Init(...)` to initialize and configure the LoRaWAN stack
- `LoRaWAN_Join(...)` to join to the LoRaWAN network
- `LoRaWAN_Send(...)` to send data to the LoRaWAN network

1. We want to initialize LoRaWAN and join the network before going into the main loop. Paste this code just before the main loop:
```
	/* Initialize the OTX-18 stack with the configuration */
	LoRaWAN_Init(&coreConfig);
	/* Try to join and go in deepsleep while we do */
	LoRaWAN_Join(M4_WaitDeepSleep);
```
2. Now let's say we only want to send the raw ADC light data. Paste the following code just after powering the sensors off:
```
		/* Send LoRaWAN sensor data */
		LoRaWAN_Send((uint8_t *) &adcResult_Light, sizeof (adcResult_Light), M4_WaitDeepSleep);
		/* Wait 30 seconds (30000 milliseconds) before sending reading data and sending again */
		CyDelay(30000);
```
3. If the instructions are followed correctly, the code should look like [this](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/code_3.3.png?raw=true)
4. Put a breakpoint on the main loop `for` statement<br>
![breakpoint](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/breakpoint.png?raw=true)<br> 
5. Hit the `Build-And-Launch` button from the status bar at the bottom of VS Code
6. Hopefully your device will connect to the LoRaWAN network and thereby return from the `LoRaWAN_Join` function to hit the breakpoint.

### 1.3 Preparing the real world data and adding Sleep functionality
This part finalizes the firmware of the FarmBug. Currently the device only sends the raw ADC light data and does just wait at the end of the loop to repeat. We want the device to send all data in a formatted way and have it in sleep mode when it's idle. Let's start with the data formatting first.
1. The following code will make a fixed format data packet to send over LoRaWAN. Put this code just after the `#include "LoRaWAN_keys.h"` code line:
```
struct __attribute__ ((__packed__))
{
	uint8_t 	humidityLevel;			// we have 8 bits of humidity data
	uint16_t	temperatureSoil;		// we have 16 bits of soil temperature data
	uint16_t	temperatureAir;			// we have 16 bits of air temperature data
	uint8_t		lightLevel;			// we have 8 bits of light level data
} loraPacket;
```
2. Next, let's format the CapSense moisture/humidity data into a percentage (0-100%). The raw data roughly ranges from 3450-4075. Let's make it ourselves easy and use the following formula: humidity = (value - 3475) / 6.
  Insert the following code just after `uint16_t CapSense_Value = *CapSense_dsRam.snsList.button0[0].raw;` to set the humidityLevel in the loraPacket:
```
		/* Limit the raw values to prevent under-/ overflow */
		if (CapSense_Value < 3475) CapSense_Value = 3475;
		if (CapSense_Value > 4075) CapSense_Value = 4075;
		loraPacket.humidityLevel = (CapSense_Value - 3475) / 6;
```
3. For the Light sensor we also want a percentage (0-100%). The raw data roughly ranges from 1000 (light) to 4000 (dark). Let's make it ourselves easy and use the following formula: light = (4000 - value) / 30.
  Insert the following code just after `uint16_t CapSense_Value = *CapSense_dsRam.snsList.button0[0].raw;` to set the humidityLevel in the loraPacket:
```
		/* Limit the raw values to prevent under-/ overflow */
		if (adcResult_Light < 1000) adcResult_Light = 1000;
		if (adcResult_Light > 4000) adcResult_Light = 4000;
		loraPacket.lightLevel = (4000 - adcResult_Light) / 30;
```
4. The raw ADC values for the temperature are not linear, so a nifty look up and interpolation function would come in handy. Add this brilliant code + function just before the main loop:
```
#define		NTC_THopen		4000	// Threshold to return OPEN (approx -25C)
#define		NTC_THshort		300		// Threshold to return SHORT
#define		NTC_SHORT		9999
#define		NTC_OPEN		-9999

const uint16_t	NTClookup[] = {   6,  7, 8, 10,12, 14, 16, 17, 18, 19, 20, 20, 20, 19, 18, 17, 16, 15, 13, 12, 10, 10,  8,  7,  7,  6,  5,  4,  4,  2  };
//								-40    -30    -20 -15 -10  -5   0   5  10  15  20  25  30  35  40  45  50  55  60  65  70  75  80  85  90  95 100 105

int16_t NTCcalc(uint16_t NTCvalue)
{
	uint8 ADCcompCount = 0;
	uint8 ADCcompPoint = 0;
	uint16 NTCcount = 3923;	// First reference @ -40 C
	int TempOut = - 405;	//	Start with - 40.5 C
	if (NTCvalue > NTC_THopen) return NTC_OPEN;
	if (NTCvalue < NTC_THshort) return NTC_SHORT;
	do {
		if (NTCvalue > NTCcount) return TempOut;		// Value lower than -40.5 C or not connected error
		NTCcount -= NTClookup[ADCcompPoint] ;			// Get .5C step and step compare value towards NTC value
		if (++ADCcompCount == 10) {
			ADCcompCount = 0;
			ADCcompPoint++;
		}
		TempOut += 5;
	} while (NTCcount > 342);
	return 1095;// Value higher than 109.5 C or not connected error
}
```
5. Implement the ADC functions on the RAW ADC data just after this line `volatile uint32_t adcResult_temperatureAir = ADC_GetResult32(2);`:
```
		/* Convert raw ADC values into temperatures */
		loraPacket.temperatureSoil = NTCcalc(adcResult_temperatureSoil);
		loraPacket.temperatureAir = NTCcalc(adcResult_temperatureAir);
```
6. Change the data of the `LoRaWAN_Send(..)` function to implement the sensor data:
```
 LoRaWAN_Send((uint8_t *) &loraPacket, sizeof (loraPacket), M4_WaitDeepSleep);
 ```
7. Finally we need to replace the delay function (CyDelay) with the Sleep function. Add the configuration for the sleep mode just after the `coreConfig` structure
```
sleepConfig_t sleepConfig =
{
	.sleepMode = modeDeepSleep,
	.BleEcoON = false,
	.DebugON = true,
	.sleepCores = coresBoth,
	.wakeUpPin = wakeUpPinHigh(true),
	.wakeUpTime = wakeUpDelay(0, 0, 0, 30), // day, hour, minute, second
};
```
8. And change the last line of the main loop (CyDelay(...)) to:
```
 LoRaWAN_Sleep(&sleepConfig);
```
9. If the instructions are followed correctly, the code should look like [this](https://github.com/onethinx/FarmBug_Workshop/blob/main/Assets/main.c?raw=true)
10. Hit the `Build-And-Launch` button from the status bar at the bottom of VS Code
11. Cross fingers and hopefully the FarmBug will work as expected 🎉
10. Congratulations, you're now a qualified Onethinx Expert Engineer<br>🤓
