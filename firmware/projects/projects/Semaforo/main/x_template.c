/*! @mainpage Blinking
 *
 * \section genDesc General Description
 *
 * This example makes LED_1 blink.
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000
#define CONFIG_BLINK_PERIOD1 3000
#define CONFIG_BLINK_PERIOD2 500
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();
    while(true){
        printf("LED ON\n");
        LedOn(LED_3);
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
		LedOff(LED_3);
        LedOn(LED_2);
        vTaskDelay(CONFIG_BLINK_PERIOD2 / portTICK_PERIOD_MS);
		LedOff(LED_2);
        LedOn(LED_2);
        vTaskDelay(CONFIG_BLINK_PERIOD2 / portTICK_PERIOD_MS);
		LedOff(LED_2);
        LedOn(LED_1);
        vTaskDelay(CONFIG_BLINK_PERIOD1 / portTICK_PERIOD_MS);
		LedOff(LED_1);
        LedOn(LED_2);
        vTaskDelay(CONFIG_BLINK_PERIOD2/ portTICK_PERIOD_MS);
		LedOff(LED_2);
        LedOn(LED_2);
        vTaskDelay(CONFIG_BLINK_PERIOD2/ portTICK_PERIOD_MS);
		LedOff(LED_2);
        LedOn(LED_3);
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
		LedOff(LED_3);
        
        
    }
}
/*==================[end of file]============================================*/