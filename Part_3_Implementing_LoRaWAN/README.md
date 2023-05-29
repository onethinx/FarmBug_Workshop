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
6. Hit the `Build-And-Launch` button from the status bar at the bottom of VS Code
7. Hpefully your device will connect to the LoRaWAN network and thereby return from the `LoRaWAN_Join` function to hit the breapoint
