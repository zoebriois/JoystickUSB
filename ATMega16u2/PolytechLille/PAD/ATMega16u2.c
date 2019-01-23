/*
LUFA Library
Copyright (C) Dean Camera, 2017.

dean [at] fourwalledcubicle [dot] com
www.lufa-lib.org
*/

/*
Copyright 2010  OBinou (obconseil [at] gmail [dot] com)
Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

Permission to use, copy, modify, distribute, and sell this
software and its documentation for any purpose is hereby granted
without fee, provided that the above copyright notice appear in
all copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of the author not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

The author disclaims all warranties with regard to this
software, including all implied warranties of merchantability
and fitness.  In no event shall the author be liable for any
special, indirect or consequential damages or any damages
whatsoever resulting from loss of use, data or profits, whether
in an action of contract, negligence or other tortious action,
arising out of or in connection with the use or performance of
this software.
*/

#include "ATMega16u2.h"
#include <LUFA/Drivers/Peripheral/Serial.h>				// pour les fonctions serie

/**	Programme principal
--- appel de la fonction de configuration
--- boucle infini ou on s'occupe d'envoyer et recevoir les infos via les endpoints	**/
int main(void)	{
	SetupHardware();
	unsigned char data_led=0x80;
	for (;;)	{
		USB_USBTask();

		// si on recoit quelque chose sur le port serie
		// --- on lit la valeur recu
		// --- si ca correspond bien au joystick
		// --- --- on decoupe
		// --- --- on envoi sur les 2 endpoints IN
		if(Serial_IsCharReceived())	{
			uint8_t Received=Serial_ReceiveByte();
			if ((Received & 0b11100000)==0b00100000){
				uint8_t ToSend_IN_1 = (Received & 0b00000001);
				uint8_t ToSend_IN_2 = ((Received & 0b00011110) >> 1);
				Endpoint_SelectEndpoint(JOYSTICK_IN_1_EPADDR);
				Endpoint_Write_8(ToSend_IN_1);
				Endpoint_ClearIN();
				Endpoint_SelectEndpoint(JOYSTICK_IN_2_EPADDR);
				Endpoint_Write_8(ToSend_IN_2);
				Endpoint_ClearIN();
			}
		}

		// on lit les données que on a reçu sur le endpoint et on les renvoit sur le port serie
		Endpoint_SelectEndpoint(LED_OUT_1_EPADDR);
		if (Endpoint_IsOUTReceived())	{
			if (Endpoint_IsReadWriteAllowed())	{
				uint8_t data_led_13 = Endpoint_Read_8();
				data_led|=(data_led_13<<5);
				Serial_SendByte(data_led);
				Endpoint_ClearOUT();
			}
		}

		Endpoint_SelectEndpoint(LED_OUT_2_EPADDR);
		if (Endpoint_IsOUTReceived())	{
			if (Endpoint_IsReadWriteAllowed())	{
				uint8_t data_led_8_12 = Endpoint_Read_8();
				data_led|=data_led_8_12;
				Serial_SendByte(data_led);
				Endpoint_ClearOUT();
			}
		}
	}
}

/** Configures the board hardware and chip peripherals for the project's functionality. */
void SetupHardware(void)	{
	USB_Init();
	LEDs_Init();
	GlobalInterruptEnable();
}

/** Fonction de création des endpoints */
void EVENT_USB_Device_ControlRequest(void)	{
	Endpoint_ConfigureEndpoint(JOYSTICK_IN_1_EPADDR, EP_TYPE_INTERRUPT, TUTORAT_EPSIZE, 1);
	Endpoint_ConfigureEndpoint(JOYSTICK_IN_2_EPADDR, EP_TYPE_INTERRUPT, TUTORAT_EPSIZE, 1);
	Endpoint_ConfigureEndpoint(LED_OUT_1_EPADDR, EP_TYPE_INTERRUPT, TUTORAT_EPSIZE, 1);
	Endpoint_ConfigureEndpoint(LED_OUT_2_EPADDR, EP_TYPE_INTERRUPT, TUTORAT_EPSIZE, 1);
}
