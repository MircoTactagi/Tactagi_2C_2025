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
/**
 * @file bcd_display.c
 * @brief Ejemplo de manejo de un display multiplexado con salida BCD mediante GPIOs.
 *
 * Este archivo contiene funciones para convertir números a BCD, mapear pines GPIO,
 * y mostrar valores numéricos en un display de 7 segmentos controlado por multiplexado.
 */

#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>

/*==================[macros and definitions]=================================*/

/**
 * @struct gpioConf_t
 * @brief Estructura de configuración para pines GPIO.
 *
 * Define un pin GPIO y su dirección (entrada/salida).
 */
typedef struct
{
    gpio_t pin;   /*!< Número de pin GPIO */
    io_t dir;     /*!< Dirección: 0 = IN, 1 = OUT */
} gpioConf_t;

/*==================[internal data definition]===============================*/

/**
 * @brief Mapeo de pines GPIO usados para representar dígitos en BCD.
 *
 * Cada elemento corresponde a un bit del número en formato BCD.
 */
gpioConf_t gpio_map[4] = {
    {GPIO_20, GPIO_OUTPUT},  /**< b0 -> GPIO_20 */
    {GPIO_21, GPIO_OUTPUT},  /**< b1 -> GPIO_21 */
    {GPIO_22, GPIO_OUTPUT},  /**< b2 -> GPIO_22 */
    {GPIO_23, GPIO_OUTPUT}   /**< b3 -> GPIO_23 */
};

/**
 * @brief Mapeo de pines GPIO usados para habilitar cada dígito del display.
 *
 * Cada elemento corresponde a un dígito del display multiplexado.
 */
gpioConf_t display_map[3] = {
    {GPIO_19, GPIO_OUTPUT},  /**< Dígito 1 */
    {GPIO_18, GPIO_OUTPUT},  /**< Dígito 2 */
    {GPIO_9, GPIO_OUTPUT}    /**< Dígito 3 */
};

/*==================[internal functions declaration]=========================*/

/**
 * @brief Convierte un número entero a un arreglo de dígitos en formato BCD.
 *
 * @param[in] data    Número entero a convertir.
 * @param[in] digits  Cantidad de dígitos a convertir.
 * @param[out] arr    Puntero al arreglo donde se almacenan los dígitos BCD.
 */
void convertir_a_BCD(uint32_t data, uint8_t digits, uint8_t *arr) {
    for (int i = digits - 1; i >= 0; i--) {
        arr[i] = data % 10;
        data /= 10;
    }
}

/**
 * @brief Configura el estado de los GPIOs en función de un número en BCD.
 *
 * @param[in] n_bcd  Número en BCD (0-9).
 * @param[in] gpios  Arreglo de pines GPIO configurados para salida BCD.
 */
void setear_gpio_del_bcd(uint8_t n_bcd, gpioConf_t *gpios){
    for (int i = 0; i < 4; i++) {
        if (((n_bcd >> i) & 1) == 1)
            GPIOOn(gpios[i].pin);
        else
            GPIOOff(gpios[i].pin);
    }
}

/**
 * @brief Muestra un número en el display multiplexado.
 *
 * Convierte el número en BCD, lo envía a los pines GPIO correspondientes,
 * y activa secuencialmente los dígitos del display.
 *
 * @param[in] numero     Número entero a mostrar.
 * @param[in] digitos    Cantidad de dígitos del display.
 * @param[in] bcd_pines  Arreglo de pines GPIO para salida BCD.
 * @param[in] display    Arreglo de pines GPIO para selección de dígitos.
 */
void mostrar_numeros(uint32_t numero, uint8_t digitos, gpioConf_t *bcd_pines, gpioConf_t *display){
    uint8_t bcd[digitos];
    convertir_a_BCD(numero, digitos, bcd);
    for (int i = 0; i < digitos; i++) {
        setear_gpio_del_bcd(bcd[i], bcd_pines);
        GPIOOn(display[i].pin);
        GPIOOff(display[i].pin);
    }
}

/*==================[external functions definition]==========================*/

/**
 * @brief Función principal de la aplicación.
 *
 * Inicializa los pines GPIO y muestra un número en el display.
 */
void app_main(void)
{
    for (int i = 0; i < 4; i++) {
        GPIOInit(gpio_map[i].pin, GPIO_OUTPUT);
    }

    for (int i = 0; i < 3; i++) {
        GPIOInit(display_map[i].pin, GPIO_OUTPUT);
    }

    uint32_t numero = 354;
    uint8_t digitos = 3;
    mostrar_numeros(numero, digitos, gpio_map, display_map);
}

/*==================[end of file]============================================*/
