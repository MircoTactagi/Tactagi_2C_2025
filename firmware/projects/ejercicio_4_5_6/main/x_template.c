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
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/


void convertir_a_BCD(uint32_t data, uint8_t digits, uint8_t *arr) {
    for (int i = digits - 1; i >= 0; i--) {
        arr[i] = data % 10;   
        data /= 10;           
    }
}
typedef struct
{
    gpio_t pin;   /*!< GPIO pin number */
    io_t dir;     /*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

gpioConf_t gpio_map[4] = {
    {GPIO_20, GPIO_OUTPUT},  // b0 -> GPIO_20
    {GPIO_21, GPIO_OUTPUT},  // b1 -> GPIO_21
    {GPIO_22, GPIO_OUTPUT},  // b2 -> GPIO_22
    {GPIO_23, GPIO_OUTPUT}   // b3 -> GPIO_23
};

void setear_gpio_del_bcd(uint8_t n_bcd, gpioConf_t *gpios){
	for (int i=0; i<4;  i++)
	
	if (((n_bcd >> i) & 1) == 1)

		GPIOOn(gpios[i].pin);
	
	else
		GPIOOff(gpios[i].pin);

}

gpioConf_t display_map[3] = {
    {GPIO_19, GPIO_OUTPUT},  // Dígito 1
    {GPIO_18, GPIO_OUTPUT},  // Dígito 2
    {GPIO_9, GPIO_OUTPUT}    // Dígito 3
};
//for (int j = 0; j < digitos; j++) GPIOOff(display[j].pin);
			
void mostrar_numeros(uint32_t numero, uint8_t digitos, gpioConf_t *bcd_pines, gpioConf_t *display){
	uint8_t bcd[digitos];
	convertir_a_BCD(numero, digitos, bcd); 
    for (int i = 0; i < digitos; i++)
    {
        
        setear_gpio_del_bcd(bcd[i], bcd_pines);
        GPIOOn(display[i].pin);
		GPIOOff(display[i].pin);

    }
}

void app_main(void)
{
    for (int i = 0; i < 4; i++)
    {
        GPIOInit(gpio_map[i].pin, GPIO_OUTPUT);
    }

    for (int i = 0; i < 3; i++)
    {
        GPIOInit(display_map[i].pin, GPIO_OUTPUT);
    }

    uint32_t numero = 354;
    uint8_t digitos = 3;
	mostrar_numeros(numero, digitos, gpio_map, display_map);

}

/*==================[end of file]============================================*/