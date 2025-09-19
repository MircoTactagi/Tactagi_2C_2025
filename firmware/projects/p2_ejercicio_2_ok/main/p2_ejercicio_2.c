/**
 * @file main.c
 * @brief Ejemplo de medición de distancia con HC-SR04 y visualización en LEDs y LCD.
 *
 * Este programa hace parpadear LEDs según la distancia medida, y permite "fijar"
 * o "detener" la medición mediante interrupciones de switches.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "gpio_mcu.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_MEDIR_DISTANCIA_PERIOD_US 1000000  /**< Período de medición (1s) */

/*==================[internal data definition]===============================*/
bool FijarMedicion = false;     /**< Flag para fijar medición en LCD */
bool DetenerMedicion = false;   /**< Flag para detener medición y LEDs */
TaskHandle_t medir_task_handle = NULL;   /**< Handle de la tarea de medición */

/*==================[internal functions declaration]=========================*/

/* Callbacks de switches */
void especta_tecla_1(void* param){
    DetenerMedicion = !DetenerMedicion;
}

void especta_tecla_2(void* param){
    FijarMedicion = !FijarMedicion;
}

/* Callback del timer */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
}

/* Actualiza LEDs según distancia */
void Actualiza_leds(uint16_t distancia){
    LedsOffAll();

    if (distancia >= 10 && distancia < 20) {
        LedOn(LED_1);
    } else if (distancia >= 20 && distancia < 30) {
        LedOn(LED_1);
        LedOn(LED_2);
    } else if (distancia >= 30) {
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);
    }
}

/*==================[tasks]==================================================*/

/**
 * @brief Tarea que mide distancia y actualiza LEDs/LCD
 */
static void TareaMedirDistancia(void *pvParameters) {
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (!DetenerMedicion) {
            uint16_t distancia = HcSr04ReadDistanceInCentimeters(); 
            Actualiza_leds(distancia);
            if (!FijarMedicion) {
                LcdItsE0803Write(distancia);
            }
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();
    LcdItsE0803Init();
    SwitchesInit();
    HcSr04Init(GPIO_3, GPIO_2); 

    /* Configuración del timer */
    timer_config_t timer_medicion = {
        .timer = TIMER_A,
        .period = CONFIG_MEDIR_DISTANCIA_PERIOD_US,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_medicion);

    /* Activar interrupciones en switches */
    SwitchActivInt(SWITCH_1, especta_tecla_1, NULL);
    SwitchActivInt(SWITCH_2, especta_tecla_2, NULL);

    /* Crear tarea */
    xTaskCreate(&TareaMedirDistancia, "TareaMedirDistancia", 512, NULL, 5, &medir_task_handle);

    /* Iniciar timer */
    TimerStart(timer_medicion.timer);
}