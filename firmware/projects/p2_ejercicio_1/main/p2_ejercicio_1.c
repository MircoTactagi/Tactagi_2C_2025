/**
 * @file main.c
 * @brief Ejemplo de medición de distancia con HC-SR04 y visualización en LEDs y LCD.
 *
 * Este programa hace parpadear LEDs según la distancia medida, y permite "fijar"
 * o "detener" la medición mediante switches.
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

/*==================[macros and definitions]=================================*/
#define TASK_PERIOD_MEDIR_DISTANCIA 1000  /**< Período de la tarea de medición (ms) */
#define TASK_PERIOD_LEER_TECLA 30         /**< Período de lectura de teclas (ms) */
#define TASK_PERIOD_TIEMPO_PRESION 100    /**< Tiempo de anti-rebote para switch (ms) */

/*==================[internal data definition]===============================*/
bool FijarMedicion = false;  /**< Flag para fijar medición en LCD */
bool DetenerMedicion = false;/**< Flag para detener medición y LEDs */

/*==================[internal functions declaration]=========================*/

/**
 * @brief Enciende LEDs según la distancia medida.
 * @return La distancia medida en centímetros.
 * @details
 * - 0-9 cm: todos los LEDs apagados.
 * - 10-19 cm: LED_1 encendido.
 * - 20-29 cm: LED_1 y LED_2 encendidos.
 * - 30 cm o más: LED_1, LED_2 y LED_3 encendidos.
 */
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

/**
 * @brief Tarea FreeRTOS que mide distancia y actualiza LEDs y LCD.
 * @param pvParameters Parámetros de tarea (no usados).
 */
static void TareaMedirDistancia(void *pvParameters) {
    
    while (true) {

        if (!DetenerMedicion) {
            uint16_t distancia = HcSr04ReadDistanceInCentimeters(); 
            Actualiza_leds(distancia);
            if (!FijarMedicion) {
                LcdItsE0803Write(distancia);
            }
        }
        vTaskDelay(TASK_PERIOD_MEDIR_DISTANCIA / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Tarea FreeRTOS que lee los switches y actualiza flags.
 * @param pvParameters Parámetros de tarea (no usados).
 */
static void LecturaTeclas(void *pvParameter){
    uint8_t teclas;
    while(true){
        teclas = SwitchesRead();

        switch(teclas){
            case SWITCH_1:
                DetenerMedicion = !DetenerMedicion;
                break;
            case SWITCH_2:
                FijarMedicion = !FijarMedicion;
                break;
        }
        
        vTaskDelay(TASK_PERIOD_LEER_TECLA / portTICK_PERIOD_MS);
    }
}

/*==================[external functions definition]==========================*/

/**
 * @brief Función principal de la aplicación.
 *
 * Inicializa LEDs, LCD, switches y sensor ultrasónico,
 * y crea las tareas FreeRTOS.
 */
void app_main(void){
    LedsInit();
    LcdItsE0803Init();
    SwitchesInit();
    HcSr04Init(GPIO_3, GPIO_2); 

    xTaskCreate(&TareaMedirDistancia, "TareaMedirDistancia", 512, NULL, 5, NULL);
    xTaskCreate(&LecturaTeclas, "LecturaTeclas", 512, NULL, 5, NULL);
}
