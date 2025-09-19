/**
 * @mainpage Sistema de Control para Display BCD Multiplexado
 * 
 * @section genDesc Descripción General
 * 
 * Este proyecto implementa el control de un display de 7 segmentos multiplexado
 * con codificación BCD mediante GPIOs de un microcontrolador ESP32.
 * 
 * El sistema permite mostrar números de hasta 3 dígitos mediante multiplexación
 * temporal, activando secuencialmente cada dígito del display.
 * 
 * @section hardConn Conexiones Hardware
 * 
 * | Peripheral      | ESP32         | Descripción               |
 * |:---------------:|:-------------:|:-------------------------|
 * | BCD Bit 0 (LSB) | GPIO_20       | Bit menos significativo  |
 * | BCD Bit 1       | GPIO_21       |                           |
 * | BCD Bit 2       | GPIO_22       |                           |
 * | BCD Bit 3 (MSB) | GPIO_23       | Bit más significativo    |
 * | Dígito 1        | GPIO_19       | Control dígito unidades  |
 * | Dígito 2        | GPIO_18       | Control dígito decenas   |
 * | Dígito 3        | GPIO_9        | Control dígito centenas  |
 * 
 * 
 * @section changelog Registro de Cambios
 * 
 * |   Fecha      | Descripción                          | Autor                  |
 * |:------------:|:-------------------------------------|:-----------------------|
 * | 05/09/2025   | Creación del archivo                 | Mirco Tactagi          |
 * | 18/09/2025   | Mejora documentación funciones       | Mirco Tactagi          |
 * 
 * @author Mirco Tactagi (mircotactagi@gmail.com)
 */

/*==================[inclusions]=============================================*/

/**
 * @file bcd_display.c
 * @brief Sistema de control para display BCD multiplexado mediante GPIOs
 * @details Este módulo proporciona funciones para convertir números a formato BCD,
 * configurar pines GPIO, y mostrar valores numéricos en un display de 7 segmentos
 * controlado mediante técnica de multiplexación.
 * 
 * El sistema utiliza 4 pines para la codificación BCD (0-9) y 3 pines adicionales
 * para controlar qué dígito está activo en cada momento.
 */

#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>

/*==================[macros and definitions]=================================*/

/** Número máximo de dígitos soportados por el display */
#define MAX_DIGITOS 3

/** Número de bits para representación BCD */
#define BCD_BITS 4

/**
 * @struct gpioConf_t
 * @brief Estructura de configuración para pines GPIO
 * 
 * Esta estructura define la configuración básica de un pin GPIO,
 * incluyendo el número de pin y su dirección (entrada/salida).
 */
typedef struct
{
    gpio_t pin;   /*!< Número de pin GPIO según enumeración del sistema */
    io_t dir;     /*!< Dirección: GPIO_INPUT (0) o GPIO_OUTPUT (1) */
} gpioConf_t;

/*==================[internal data definition]===============================*/

/**
 * @brief Mapeo de pines GPIO para representación BCD
 * 
 * Este arreglo define la correspondencia entre los bits BCD (0-3)
 * y los pines GPIO físicos del microcontrolador.
 * 
 * @note El orden de los bits es: [0] = LSB, [3] = MSB
 */
gpioConf_t gpio_map[BCD_BITS] = {
    {GPIO_20, GPIO_OUTPUT},  /**< Bit 0 (LSB) -> GPIO_20 */
    {GPIO_21, GPIO_OUTPUT},  /**< Bit 1       -> GPIO_21 */
    {GPIO_22, GPIO_OUTPUT},  /**< Bit 2       -> GPIO_22 */
    {GPIO_23, GPIO_OUTPUT}   /**< Bit 3 (MSB) -> GPIO_23 */
};

/**
 * @brief Mapeo de pines GPIO para control de dígitos
 * 
 * Este arreglo define los pines GPIO utilizados para habilitar
 * cada uno de los dígitos del display multiplexado.
 * 
 * @note El orden de los dígitos es: [0] = unidades, [1] = decenas, [2] = centenas
 */
gpioConf_t display_map[MAX_DIGITOS] = {
    {GPIO_19, GPIO_OUTPUT},  /**< Control del dígito 1 (unidades) */
    {GPIO_18, GPIO_OUTPUT},  /**< Control del dígito 2 (decenas) */
    {GPIO_9, GPIO_OUTPUT}    /**< Control del dígito 3 (centenas) */
};

/*==================[internal functions definition]==========================*/

/**
 * @brief Convierte un número entero a un arreglo de dígitos BCD
 * 
 * Esta función toma un número entero y lo descompone en sus dígitos
 * individuales, almacenándolos en un arreglo en formato BCD.
 * 
 * @param[in] data    Número entero a convertir (0-999 para 3 dígitos)
 * @param[in] digits  Cantidad de dígitos a convertir (máximo MAX_DIGITOS)
 * @param[out] arr    Puntero al arreglo donde se almacenarán los dígitos BCD
 * 
 * @return void
 * 
 * @note Si el número excede la capacidad de dígitos, se truncará por la izquierda
 * 
 */
void convertir_a_BCD(uint32_t data, uint8_t digits, uint8_t *arr) {
    for (int i = digits - 1; i >= 0; i--) {
        arr[i] = data % 10;
        data /= 10;
    }
}

/**
 * @brief Establece el estado de los GPIOs según un dígito BCD
 * 
 * Esta función toma un dígito BCD (0-9) y establece el estado
 * de los pines GPIO correspondientes para representarlo.
 * 
 * @param[in] n_bcd  Dígito a representar en formato BCD (0-9)
 * @param[in] gpios  Arreglo de pines GPIO configurados para salida BCD
 * 
 * @return void
 * 
 * @warning El valor debe estar en el rango 0-9. Valores mayores pueden
 *          producir resultados inesperados.
 */
void setear_gpio_del_bcd(uint8_t n_bcd, gpioConf_t *gpios){
    for (int i = 0; i < BCD_BITS; i++) {
        if (((n_bcd >> i) & 1) == 1)
            GPIOOn(gpios[i].pin);
        else
            GPIOOff(gpios[i].pin);
    }
}

/**
 * @brief Muestra un número en el display multiplexado
 * 
 * Esta función convierte un número a BCD, establece los pines GPIO
 * correspondientes y activa secuencialmente cada dígito del display
 * mediante multiplexación temporal.
 * 
 * @param[in] numero     Número entero a mostrar (0-999 para 3 dígitos)
 * @param[in] digitos    Cantidad de dígitos del display (1-3)
 * @param[in] bcd_pines  Arreglo de pines GPIO para salida BCD
 * @param[in] display    Arreglo de pines GPIO para selección de dígitos
 * 
 * @return void
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
 * @brief Función principal de la aplicación
 * 
 * Esta función inicializa todos los pines GPIO necesarios y
 * muestra un número de ejemplo en el display multiplexado.
 * 
 * @return void
 */
void app_main(void)
{
    // Inicialización de pines para salida BCD
    for (int i = 0; i < BCD_BITS; i++) {
        GPIOInit(gpio_map[i].pin, GPIO_OUTPUT);
    }

    // Inicialización de pines para control de dígitos
    for (int i = 0; i < MAX_DIGITOS; i++) {
        GPIOInit(display_map[i].pin, GPIO_OUTPUT);
    }

    // Número de ejemplo a mostrar
    uint32_t numero = 354;
    uint8_t digitos = 3;
    
    mostrar_numeros(numero, digitos, gpio_map, display_map);
}

/*==================[end of file]============================================*/