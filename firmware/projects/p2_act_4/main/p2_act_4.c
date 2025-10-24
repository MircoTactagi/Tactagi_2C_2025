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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define FS_MUESTREO_HZ 500
#define PERIODO_US (1000000 / FS_MUESTREO_HZ)
/*==================[internal data definition]===============================*/
TaskHandle_t tarea_adc_handle = NULL;
uint16_t valor_adc_mV = 0;

/*==================[internal functions declaration]=========================*/
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(tarea_adc_handle, pdFALSE);
}

static void TareaLecturaADC(void *pvParameters) {

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH1, &valor_adc_mV);
        UartSendString(UART_PC, (char*)UartItoa(valor_adc_mV, 10));
        UartSendString(UART_PC, "\r\n");
    }
}
/*==================[external functions definition]==========================*/
void app_main(void){
    serial_config_t uart_pc = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = NULL,
        .param_p = NULL
    };
    UartInit(&uart_pc);
	analog_input_config_t adc_cfg = {
        .input = CH1,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0
    };
	AnalogInputInit(&adc_cfg);
    timer_config_t timer_adc = {
        .timer = TIMER_A,
        .period = PERIODO_US,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_adc);
	xTaskCreate(&TareaLecturaADC, "TareaLecturaADC", 512, NULL, 5, &tarea_adc_handle);
	TimerStart(timer_adc.timer);
}
/*==================[end of file]============================================*/