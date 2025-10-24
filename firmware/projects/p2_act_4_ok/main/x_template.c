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
#define FS_MUESTREO_HZ 50
#define PERIODO_US (1000000 / FS_MUESTREO_HZ)

unsigned char ECG[] = {
17,17,17,17,17,17,17,17,17,17,17,18,18,18,17,17,17,17,17,17,17,18,18,18,18,18,18,18,17,17,16,16,16,16,17,17,18,18,18,17,17,17,17,
18,18,19,21,22,24,25,26,27,28,29,31,32,33,34,34,35,37,38,37,34,29,24,19,15,14,15,16,17,17,17,16,15,14,13,13,13,13,13,13,13,12,12,
10,6,2,3,15,43,88,145,199,237,252,242,211,167,117,70,35,16,14,22,32,38,37,32,27,24,24,26,27,28,28,27,28,28,30,31,31,31,32,33,34,36,
38,39,40,41,42,43,45,47,49,51,53,55,57,60,62,65,68,71,75,79,83,87,92,97,101,106,111,116,121,125,129,133,136,138,139,140,140,139,137,
133,129,123,117,109,101,92,84,77,70,64,58,52,47,42,39,36,34,31,30,28,27,26,25,25,25,25,25,25,25,25,24,24,24,24,25,25,25,25,25,25,25,
24,24,24,24,24,24,24,24,23,23,22,22,21,21,21,20,20,20,20,20,19,19,18,18,18,19,19,19,19,18,17,17,18,18,18,18,18,18,18,18,17,17,17,17,
17,17,17

} ;

#define NUM_MUESTRAS_ECG (sizeof(ECG)/sizeof(ECG[0]))
/*==================[internal data definition]===============================*/
TaskHandle_t tarea_adc_handle = NULL;
uint16_t valor_adc_mV = 0;

/*==================[internal functions declaration]=========================*/
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(tarea_adc_handle, pdFALSE);
}

static void TareaLecturaADC(void *pvParameters) {
    static size_t index = 0;

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        uint16_t valor = ECG[index]; 
        UartSendString(UART_PC, ">");
        UartSendString(UART_PC, "adcCH1:");
        UartSendString(UART_PC, (char*)UartItoa(valor, 10));
        UartSendString(UART_PC, "\r\n");
        index++;
        if (index >= NUM_MUESTRAS_ECG) {
            index = 0;
        }
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
    AnalogOutputInit();
}
/*==================[end of file]============================================*/