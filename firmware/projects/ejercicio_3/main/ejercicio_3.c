/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <led.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/*==================[macros and definitions]=================================*/

enum {ON, OFF, TOGGLE};

/*==================[internal data definition]===============================*/

struct leds
{
    uint8_t mode;       /*ON, OFF, TOGGLE*/
	uint8_t n_led;      /*indica el número de led a controlar*/  
	uint8_t n_ciclos;   /*indica la cantidad de ciclos de encendido/apagado*/
	uint16_t periodo;    /*indica el tiempo de cada ciclo*/
};

struct leds my_leds; 
/*==================[internal functions declaration]=========================*/

void administrar_led (struct leds *mis_leds) {
	switch (mis_leds->mode){
		case ON:
			LedOn(mis_leds->n_led);
			break;
		case OFF:
			LedOff(mis_leds->n_led);
			break;
		case TOGGLE:
			uint8_t i = 0;
			while (i < 2*mis_leds->n_ciclos) {
				LedToggle(mis_leds->n_led);
				i++;
				vTaskDelay(mis_leds->periodo/ portTICK_PERIOD_MS);
			}
			break;

	}
}
/*==================[external functions definition]==========================*/


void app_main(void){
    LedsInit();
    my_leds.mode = TOGGLE;
    my_leds.n_ciclos = 10;
    my_leds.n_led = LED_1;
	my_leds.periodo= 500;
    administrar_led(&my_leds);
}


/*==================[end of file]============================================*/