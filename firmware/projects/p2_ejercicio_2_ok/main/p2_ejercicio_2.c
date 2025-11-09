/*! @mainpage Medición de Distancia con HC-SR04 (Interrupciones)
 *
 * @section genDesc Descripción General
 *
 * Este programa mide la distancia utilizando el sensor ultrasónico HC-SR04.  
 * La distancia obtenida se refleja en un conjunto de LEDs y se muestra en el 
 * display LCD ITSE0803.  
 * A diferencia de versiones anteriores, esta implementación utiliza **interrupciones**
 * en los switches para fijar o detener la medición, y un **timer** para sincronizar
 * las adquisiciones periódicas.
 *
 * @section hardConn Conexiones de Hardware
 *
 * | Periférico   | ESP32 GPIO |
 * |:-------------:|:----------:|
 * | HC-SR04 TRIG  | GPIO_3     |
 * | HC-SR04 ECHO  | GPIO_2     |
 * | LED_1         | GPIO_X     |
 * | LED_2         | GPIO_Y     |
 * | LED_3         | GPIO_Z     |
 * | SWITCH_1 (Detener) | GPIO_A |
 * | SWITCH_2 (Fijar)    | GPIO_B |
 * | LCD ITSE0803  | GPIOs según interfaz I2C/SPI |
 *
 * @section changelog Registro de Versiones
 *
 * | Fecha       | Descripción                              |
 * |:------------:|:------------------------------------------|
 * | 26/09/2025   | Creación inicial del archivo              |
 * | 09/11/2025   | Versión documentada y comentada (final)   |
 *
 * @author Mirco Tactagi
 * @email mircotactagi@gmail.com
 */

/*==================[inclusiones]===========================================*/
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

/*==================[macros y definiciones]=================================*/
#define CONFIG_MEDIR_DISTANCIA_PERIOD_US 1000000  /**< Período de medición (1 s) */

/*==================[definición de datos internos]==========================*/
bool FijarMedicion = false;     /**< Flag para mantener medición actual en el LCD */
bool DetenerMedicion = false;   /**< Flag para pausar la medición y apagar LEDs */
TaskHandle_t medir_task_handle = NULL;   /**< Handle de la tarea de medición */

/*==================[declaración de funciones internas]=====================*/
/**
 * @brief Callback asociado al SWITCH_1.
 * 
 * Alterna el estado del flag @ref DetenerMedicion para pausar o reanudar la medición.
 */
void especta_tecla_1(void* param) {
    DetenerMedicion = !DetenerMedicion;
}

/**
 * @brief Callback asociado al SWITCH_2.
 * 
 * Alterna el estado del flag @ref FijarMedicion para mantener o liberar la lectura en LCD.
 */
void especta_tecla_2(void* param) {
    FijarMedicion = !FijarMedicion;
}

/**
 * @brief Callback del temporizador A.
 * 
 * Notifica a la tarea de medición para ejecutar una nueva adquisición de distancia.
 */
void FuncTimerA(void* param) {
    vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
}

/**
 * @brief Actualiza los LEDs de acuerdo con la distancia medida.
 * 
 * @param distancia Distancia medida en centímetros.
 * 
 * @details
 * - 0–9 cm: todos los LEDs apagados.  
 * - 10–19 cm: LED_1 encendido.  
 * - 20–29 cm: LED_1 y LED_2 encendidos.  
 * - 30 cm o más: LED_1, LED_2 y LED_3 encendidos.
 */
void Actualiza_leds(uint16_t distancia) {
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

/*==================[tareas FreeRTOS]=======================================*/
/**
 * @brief Tarea que mide la distancia y actualiza LEDs y LCD.
 * 
 * Esta tarea es notificada por el timer mediante interrupción.
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

/*==================[funciones externas]====================================*/
/**
 * @brief Función principal de la aplicación.
 * 
 * Inicializa periféricos, configura el timer, activa interrupciones en switches y 
 * crea la tarea principal de medición de distancia.
 */
void app_main(void) {
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

    /* Crear tarea principal */
    xTaskCreate(&TareaMedirDistancia, "TareaMedirDistancia", 512, NULL, 5, &medir_task_handle);

    /* Iniciar el temporizador */
    TimerStart(timer_medicion.timer);
}

/*==================[end of file]===========================================*/
