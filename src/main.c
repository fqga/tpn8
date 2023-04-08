/* Copyright 2017, Esteban Volentini - Facet UNT, Fi UNER
 * Copyright 2014, 2015 Mariano Cerdeiro
 * Copyright 2014, Pablo Ridolfi
 * Copyright 2014, Juan Cecconi
 * Copyright 2014, Gustavo Muro
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** @file main.c
 **
 ** @brief Ejemplo de un led parpadeando
 **
 ** Ejemplo de un led parpadeando utilizando la capa de abstraccion de
 ** hardware y con sistema operativo FreeRTOS.
 **
 ** | RV | YYYY.MM.DD | Autor       | Descripción de los cambios              |
 ** |----|------------|-------------|-----------------------------------------|
 ** |  3 | 2023.04.08 | fquiroga    | tpn8     |
 ** |  2 | 2017.10.16 | evolentini  | Correción en el formato del archivo     |
 ** |  1 | 2017.09.21 | evolentini  | Version inicial del archivo             |
 **
 ** @defgroup ejemplos Proyectos de ejemplo
 ** @brief Proyectos de ejemplo de la Especialización en Sistemas Embebidos
 ** @{
 */

/* === Inclusiones de cabeceras ============================================ */
#include "FreeRTOS.h"
#include "bsp.h"
#include "task.h"
#include <stdbool.h>

/* === Definicion y Macros ================================================= */

/* === Declaraciones de tipos de datos internos ============================ */

typedef struct parametros_s {
    digital_output_t led;
    uint16_t delay;
} * parametros_t;

/* === Declaraciones de funciones internas ================================= */

/* === Definiciones de variables internas ================================== */

static board_t board;

/* === Definiciones de variables externas ================================== */

/* === Definiciones de funciones internas ================================== */

void Rojo(void * parameters) {
    while (true) {
        DigitalOutputToggle(board->led_rojo);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void Verde(void * parameters) {
    while (true) {
        DigitalOutputToggle(board->led_verde);
        vTaskDelay(pdMS_TO_TICKS(750));
    }
}

void Amarillo(void * parameters) {
    while (true) {
        DigitalOutputToggle(board->led_amarillo);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

void Azul(void * parameters) {
    while (true) {
        if (DigitalInputHasActivated(board->boton_cambiar)) {
            DigitalOutputToggle(board->led_azul);
        }
    }
}

void Blinking(void * parameters) {
    parametros_t parametros = (parametros_t)parameters;

    while (true) {
        DigitalOutputToggle(parametros->led);
        vTaskDelay(pdMS_TO_TICKS(parametros->delay));
    }
}

void BlinkingSync(void * parameters) {
    parametros_t parametros = (parametros_t)parameters;
    TickType_t ultimo_valor;

    // Initialise the ultimo_valor variable with the current time
    ultimo_valor = xTaskGetTickCount();
    while (true) {
        DigitalOutputToggle(parametros->led);
        vTaskDelayUntil(&ultimo_valor, pdMS_TO_TICKS(parametros->delay));
    }
}

void TecScan(void * state) {
    TaskHandle_t tarea;
    bool taskState = state;

    tarea = xTaskGetHandle("Rojo");

    while (true) {
        if (DigitalInputHasActivated(board->boton_cambiar)) {
            if (taskState) {
                vTaskSuspend(tarea);
                taskState = false;
            } else {
                vTaskResume(tarea);
                taskState = true;
            }
        }

        if (DigitalInputHasActivated(board->boton_prender)) {
            DigitalOutputToggle(board->led_azul);
        }

        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

/* === Definiciones de funciones externas ================================== */

/** @brief Función principal del programa
 **
 ** @returns 0 La función nunca debería termina
 **
 ** @remarks En un sistema embebido la función main() nunca debe terminar.
 **          El valor de retorno 0 es para evitar un error en el compilador.
 */
int main(void) {
    /* Inicializaciones y configuraciones de dispositivos */
    static struct parametros_s parametros[3];
    static bool taskState = true;

    board = BoardCreate();

    parametros[0].led = board->led_rojo;
    parametros[0].delay = 500;

    parametros[1].led = board->led_verde;
    parametros[1].delay = 750;

    parametros[2].led = board->led_amarillo;
    parametros[2].delay = 250;

    /* Creación de las tareas */

    xTaskCreate(Blinking, "Rojo", configMINIMAL_STACK_SIZE, &parametros[0], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(Blinking, "Verde", configMINIMAL_STACK_SIZE, &parametros[1], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(BlinkingSync, "Amarillo", configMINIMAL_STACK_SIZE, &parametros[2], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(TecScan, "TecScan", configMINIMAL_STACK_SIZE, &taskState, tskIDLE_PRIORITY + 2, NULL);

    /* Arranque del sistema operativo */
    vTaskStartScheduler();

    /* vTaskStartScheduler solo retorna si se detiene el sistema operativo */
    while (1) {
    };

    /* El valor de retorno es solo para evitar errores en el compilador*/
    return 0;
}
/* === Ciere de documentacion ============================================== */
/** @} Final de la definición del modulo para doxygen */
