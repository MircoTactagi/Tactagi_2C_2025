/*! @mainpage Medición de Distancia con HC-SR04
 *
 * @section genDesc Descripción General
 *
 * Este programa mide la distancia utilizando el sensor ultrasónico HC-SR04. 
 * Según la distancia detectada, enciende una cantidad proporcional de LEDs y 
 * muestra el valor medido en un display LCD ITSE0803. 
 * Mediante los switches se puede **fijar** o **detener** la medición.
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
 * | SWITCH_1      | GPIO_A     |
 * | SWITCH_2      | GPIO_B     |
 * | LCD ITSE0803  | GPIOs según interfaz I2C/SPI |
 *
 * @section changelog Registro de Versiones
 *
 * | Fecha       | Descripción                              |
 * |:------------:|:------------------------------------------|
 * | 12/09/2025   | Creación inicial del archivo              |
 * | 25/09/2025   | Versión intermedia con pruebas funcionales|
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

/*==================[macros y definiciones]=================================*/
#define TASK_PERIOD_MEDIR_DISTANCIA 1000  /**< Período de la tarea de medición (ms) */
#define TASK_PERIOD_LEER_TECLA 30         /**< Período de lectura de teclas (ms) */
#define TASK_PERIOD_TIEMPO_PRESION 100    /**< Tiempo de anti-rebote para switch (ms) */

/*==================[definición de datos internos]==========================*/
bool FijarMedicion = false;   /**< Flag para mantener la medición actual en el LCD */
bool DetenerMedicion = false; /**< Flag para pausar la medición y apagar LEDs */

/*==================[declaración de funciones internas]=====================*/
/**
 * @brief Actualiza el estado de los LEDs en función de la distancia medida.
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

/**
 * @brief Tarea que mide la distancia y actualiza LEDs y LCD.
 * 
 * @param pvParameters Parámetros de tarea (no utilizados).
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
 * @brief Tarea que lee el estado de los switches y actualiza las banderas de control.
 * 
 * @param pvParameter Parámetros de tarea (no utilizados).
 */
static void LecturaTeclas(void *pvParameter) {
    uint8_t teclas;
    while (true) {
        teclas = SwitchesRead();

        switch (teclas) {
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

/*==================[definición de funciones externas]======================*/
/**
 * @brief Función principal del programa.
 * 
 * Inicializa periféricos (LEDs, LCD, switches, HC-SR04) y crea las tareas de FreeRTOS.
 */
void app_main(void) {
    LedsInit();
    LcdItsE0803Init();
    SwitchesInit();
    HcSr04Init(GPIO_3, GPIO_2); 

    xTaskCreate(&TareaMedirDistancia, "TareaMedirDistancia", 512, NULL, 5, NULL);
    xTaskCreate(&LecturaTeclas, "LecturaTeclas", 512, NULL, 5, NULL);
}

/*==================[end of file]===========================================*/
