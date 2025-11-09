/*! @mainpage Medición de Distancia con HC-SR04, UART e Interrupciones
 *
 * @section genDesc Descripción General
 *
 * Este programa mide la distancia mediante el sensor ultrasónico **HC-SR04**, 
 * y muestra el resultado tanto en un **display LCD ITSE0803** como por **UART**.  
 * Además, enciende LEDs según el rango de distancia y permite:
 * - Fijar o detener la medición usando **interruptores físicos (switches)**.  
 * - Cambiar entre **centímetros** y **pulgadas** mediante comandos por UART.  
 * - Consultar el **máximo valor medido**.  
 * - Aumentar o disminuir la **frecuencia de muestreo** mediante comandos seriales.  
 *
 * @section hardConn Conexiones de Hardware
 *
 * | Periférico    | ESP32 GPIO |
 * |:---------------:|:----------:|
 * | HC-SR04 TRIG    | GPIO_3     |
 * | HC-SR04 ECHO    | GPIO_2     |
 * | LED_1           | GPIO_X     |
 * | LED_2           | GPIO_Y     |
 * | LED_3           | GPIO_Z     |
 * | SWITCH_1        | GPIO_A     |
 * | SWITCH_2        | GPIO_B     |
 * | LCD ITSE0803    | GPIOs según interfaz I2C/SPI |
 * | UART TX/RX      | GPIO según configuración MCU |
 *
 * @section changelog Registro de Versiones
 *
 * | Fecha       | Descripción                                  |
 * |:------------:|:---------------------------------------------|
 * | 26/09/2025   | Creación inicial del archivo                 |
 * | 15/10/2025   | Versión con control UART y distancia máxima  |
 * | 09/11/2025   | Versión documentada y comentada (final)      |
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
#include "uart_mcu.h"

/*==================[macros y definiciones]=================================*/
#define CONFIG_MEDIR_DISTANCIA_PERIOD_US 1000000  /**< Período de medición (1 s) */

/*==================[definición de datos internos]==========================*/
bool FijarMedicion = false;      /**< Flag para fijar medición en LCD */
bool DetenerMedicion = false;    /**< Flag para detener medición y LEDs */
bool DistanciaenIn = false;      /**< Flag para cambiar unidades (cm/in) */
uint32_t periodo_lectura = CONFIG_MEDIR_DISTANCIA_PERIOD_US; /**< Período actual de lectura */
TaskHandle_t medir_task_handle = NULL; /**< Handle de la tarea de medición */
uint16_t distancia_max_in = 0; 
uint16_t distancia_max_cm = 0; 

/*==================[declaración de funciones internas]=====================*/
/**
 * @brief Callback del switch 1.
 * 
 * Alterna el estado de la bandera que detiene o reanuda la medición.
 */
void especta_tecla_1(void* param) {
    DetenerMedicion = !DetenerMedicion;
}

/**
 * @brief Callback del switch 2.
 * 
 * Alterna el estado de la bandera que fija la medición en pantalla.
 */
void especta_tecla_2(void* param) {
    FijarMedicion = !FijarMedicion;
}

/**
 * @brief Callback del temporizador.
 * 
 * Notifica a la tarea de medición para que realice una nueva lectura.
 */
void FuncTimerA(void* param) {
    vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
}

/**
 * @brief Envía la distancia medida a través de la UART.
 * 
 * @param distancia Valor de distancia a enviar.
 */
void EnviarDistanciaPorUART(uint16_t distancia) {
    UartSendString(UART_PC, (char*)UartItoa(distancia, 10));
    if (DistanciaenIn == false) {
        UartSendString(UART_PC, " cm\r\n");
    } else {
        UartSendString(UART_PC, " In\r\n");
    }
}

/**
 * @brief Interrupción de recepción por UART.
 * 
 * Gestiona comandos para controlar la aplicación:
 * - `O` / `o`: detener o reanudar medición  
 * - `H` / `h`: fijar o liberar valor en LCD  
 * - `I` / `i`: alternar entre cm e in  
 * - `M` / `m`: mostrar valor máximo medido  
 * - `F` / `f`: aumentar frecuencia de medición  
 * - `S` / `s`: disminuir frecuencia de medición  
 */
void InterrupcionPorUart(void) {
    uint8_t tecla; 
    UartReadByte(UART_PC, &tecla);

    switch(tecla) {
        case 'O': case 'o':
            DetenerMedicion = !DetenerMedicion;
            break;
        case 'H': case 'h':
            FijarMedicion = !FijarMedicion;
            break;
        case 'I': case 'i':
            DistanciaenIn = !DistanciaenIn;
            break;
        case 'M': case 'm': {
            uint8_t *num_str_cm = UartItoa(distancia_max_cm, 10);
            uint8_t *num_str_in = UartItoa(distancia_max_in, 10);
            
            if (DistanciaenIn == false) {
                UartSendString(UART_PC, (char*)num_str_cm);
                UartSendString(UART_PC, " MAXIMO en cm\r\n");
                LcdItsE0803Write(distancia_max_cm);
            } else {
                UartSendString(UART_PC, (char*)num_str_in);
                UartSendString(UART_PC, " MAXIMO en In\r\n");
                LcdItsE0803Write(distancia_max_in);
            }

            FijarMedicion = true;     
            vTaskDelay(pdMS_TO_TICKS(5000));  
            FijarMedicion = false;    
            break;
        }
        case 'F': case 'f':
            periodo_lectura -= 100000; 
            TimerUpdatePeriod(TIMER_A, periodo_lectura);
            break;
        case 'S': case 's':
            periodo_lectura += 100000; 
            TimerUpdatePeriod(TIMER_A, periodo_lectura);
            break;
    }
}

/**
 * @brief Actualiza los LEDs en función de la distancia.
 * 
 * @param distancia Valor de distancia medido.
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
 * @brief Tarea encargada de medir la distancia y actualizar las salidas.
 * 
 * Realiza la lectura del sensor, actualiza LEDs, LCD y envía datos por UART.
 */
static void TareaMedirDistancia(void *pvParameters) {
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (!DetenerMedicion) {
            uint16_t distancia = 0;

            if (DistanciaenIn == false) {
                distancia = HcSr04ReadDistanceInCentimeters();
                if (distancia > distancia_max_cm) {
                    distancia_max_cm = distancia;
                }
            } else {
                distancia = HcSr04ReadDistanceInInches();
                if (distancia > distancia_max_in) {
                    distancia_max_in = distancia;
                }
            }

            Actualiza_leds(distancia);

            if (!FijarMedicion) {
                LcdItsE0803Write(distancia);
            }

            EnviarDistanciaPorUART(distancia);
        }
    }
}

/*==================[definición de funciones externas]======================*/
/**
 * @brief Función principal del programa.
 * 
 * Inicializa los periféricos (LEDs, LCD, switches, UART, timer y sensor HC-SR04)
 * y crea la tarea principal de medición.
 */
void app_main(void) {
    LedsInit();
    LcdItsE0803Init();
    SwitchesInit();
    HcSr04Init(GPIO_3, GPIO_2); 
    
    serial_config_t uart_pc = {
        .port = UART_PC,
        .baud_rate = 9600,
        .func_p = InterrupcionPorUart,
        .param_p = NULL
    };
    UartInit(&uart_pc);

    timer_config_t timer_medicion = {
        .timer = TIMER_A,
        .period = CONFIG_MEDIR_DISTANCIA_PERIOD_US,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_medicion); 

    SwitchActivInt(SWITCH_1, especta_tecla_1, NULL);
    SwitchActivInt(SWITCH_2, especta_tecla_2, NULL);

    xTaskCreate(&TareaMedirDistancia, "TareaMedirDistancia", 512, NULL, 5, &medir_task_handle);

    TimerStart(timer_medicion.timer);
}

/*==================[end of file]===========================================*/
