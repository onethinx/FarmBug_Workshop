
/********************************************************************************
 *    ___             _   _     _			
 *   / _ \ _ __   ___| |_| |__ (_)_ __ __  __
 *  | | | | '_ \ / _ \ __| '_ \| | '_ \\ \/ /
 *  | |_| | | | |  __/ |_| | | | | | | |>  < 
 *   \___/|_| |_|\___|\__|_| |_|_|_| |_/_/\_\
 *
 ********************************************************************************
 *
 * Copyright (c) 2019-2022 Onethinx BV <info@onethinx.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ********************************************************************************
 *
 * Created by: Rolf Nooteboom on 2023-05-27
 *
 * FarmBug Workshop project 

 * For a setup and description please see:
 * https://github.com/onethinx/Workshop_29May2023
 *
 ********************************************************************************/
#include "project.h"
#include "OTX18-EnableCapSense.h"
#include "OnethinxCore01.h"
#include "OnethinxExt01.h"
#include "LoRaWAN_keys.h"

struct __attribute__ ((__packed__))
{
	uint8_t 	humidityLevel;			// we have 8 bits of humidity data
	uint16_t	temperatureSoil;		// we have 16 bits of soil temperature data
	uint16_t	temperatureAir;			// we have 16 bits of air temperature data
	uint8_t		lightLevel;				// we have 8 bits of light level data
} loraPacket;

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

sleepConfig_t sleepConfig =
{
	.sleepMode = modeDeepSleep,
	.BleEcoON = false,
	.DebugON = true,
	.sleepCores = coresBoth,
	.wakeUpPin = wakeUpPinHigh(true),
	.wakeUpTime = wakeUpDelay(0, 0, 0, 30), // day, hour, minute, second
};

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

int main(void)
{
	/* Enable Global Interrupts */
	__enable_irq();

	/* Enable CapSense on the OTX-18 */
	OTX18_EnableCapSense();
	/* Start the CapSense usermodule */
	CapSense_Start();

	/* Start the ADC usermodule */
	ADC_Start();

	/* Flash the LED 3 times at start */
	for (uint32_t cnt = 0; cnt < 3; cnt++)				// Make a loop which runs 3 times
	{
		Cy_GPIO_Write(LED_B_PORT, LED_B_NUM, 1);		// Turn LED on
		CyDelay(300);									// Wait 300 milliseconds
		Cy_GPIO_Write(LED_B_PORT, LED_B_NUM, 0);		// Turn LED off
		CyDelay(300);
	}
	
	/* Initialize the OTX-18 stack with the configuration */
	LoRaWAN_Init(&coreConfig);
	/* Try to join and go in deepsleep while we do */
    LoRaWAN_Join(M4_WaitDeepSleep);

	/* Main Loop */
	for(;;)
	{
		/* Call functions to get CapSense value */
		CapSense_ProcessAllWidgets();
		CapSense_ScanAllWidgets();
		while (CapSense_IsBusy()) {}					// Wait till CapSense function is finished

		/* Read raw CapSense value directly from the register */
		uint16_t CapSense_Value = *CapSense_dsRam.snsList.button0[0].raw;
		/* Limit the raw values to prevent under-/ overflow */
		if (CapSense_Value < 3475) CapSense_Value = 3475;
		if (CapSense_Value > 4075) CapSense_Value = 4075;
		loraPacket.humidityLevel = (CapSense_Value - 3475) / 6;
		/* Turn the LED on when the CapSense value exceeds 3800 */
		//Cy_GPIO_Write(LED_B_PORT, LED_B_NUM,  CapSense_Value > 3800);
		

		/* Power Sensors ON */
		Cy_GPIO_Write(PWR_LDR_PORT, PWR_LDR_NUM, 1);
		Cy_GPIO_Write(PWR_NTC1_PORT, PWR_NTC1_NUM, 1);
		Cy_GPIO_Write(PWR_NTC2_PORT, PWR_NTC2_NUM, 1);
		CyDelay(10);									// Allow some time to power sensors and have a stable voltage reading

		/* Sample Analog Sensors */
		ADC_StartConvert();
		ADC_IsEndConversion(CY_SAR_WAIT_FOR_RESULT);
		volatile uint32_t adcResult_Light = ADC_GetResult32(0);
		/* Limit the raw values to prevent under-/ overflow */
		if (adcResult_Light < 1000) adcResult_Light = 1000;
		if (adcResult_Light > 4000) adcResult_Light = 4000;
		loraPacket.lightLevel = (4000 - adcResult_Light) / 30;

		volatile uint32_t adcResult_temperatureSoil = ADC_GetResult32(1);
		volatile uint32_t adcResult_temperatureAir = ADC_GetResult32(2);
		/* Convert raw ADC values into temperatures */
		loraPacket.temperatureSoil = NTCcalc(adcResult_temperatureSoil);
		loraPacket.temperatureAir = NTCcalc(adcResult_temperatureAir);

		/* Turn the LED on when the ADC light value is below 3000 */
		Cy_GPIO_Write(LED_B_PORT, LED_B_NUM,  adcResult_Light < 3000);

		/* Power Sensors OFF */
		Cy_GPIO_Write(PWR_LDR_PORT, PWR_LDR_NUM, 0);
		Cy_GPIO_Write(PWR_NTC1_PORT, PWR_NTC1_NUM, 0);
		Cy_GPIO_Write(PWR_NTC2_PORT, PWR_NTC2_NUM, 0);

		/* Send LoRaWAN sensor data */
		LoRaWAN_Send((uint8_t *) &loraPacket, sizeof (loraPacket), M4_WaitDeepSleep);
		/* Wait 30 seconds (30000 milliseconds) before sending reading data and sending again */
		LoRaWAN_Sleep(&sleepConfig);
	}
}


/* [] END OF FILE */