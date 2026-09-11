/*
 * Programación 1 - Ingenieria en Computación - UNRN Andina
 */

#include <stdio.h>
#include "../libreria/libreria.h"
#include "ejercicio.h"

#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
#define NUM_CHANNELS 1
#define DURATION 2.0
#define FREQUENCY 440.0
#define AMPLITUDE 0.5
#define OUTPUT_FILENAME "tono.wav"

int main()
{
    uint32_t num_samples = (uint32_t)(SAMPLE_RATE * DURATION);
    wav_t *wav = NULL;
    int resultado = 0;

    printf("Creando archivo WAV con un tono de %.2f Hz\n", FREQUENCY);

    wav = wav_crear(SAMPLE_RATE, NUM_CHANNELS, BITS_PER_SAMPLE, num_samples);
    if (wav == NULL)
    {
        fprintf(stderr, "Error al crear la estructura WAV.\n");
        resultado = -1;
    }

    if (resultado == 0)
    {
        printf("Generando tono...\n");
        if (wav_generar_tono(wav, FREQUENCY, DURATION, AMPLITUDE) != 0)
        {
            fprintf(stderr, "Error al generar el tono.\n");
            resultado = -1;
        }
    }

    if (resultado == 0)
    {
        printf("Guardando archivo en '%s'...\n", OUTPUT_FILENAME);
        if (wav_guardar(wav, OUTPUT_FILENAME) != 0)
        {
            fprintf(stderr, "Error al guardar el archivo WAV.\n");
            resultado = -1;
        }
    }

    if (wav != NULL)
    {
        wav_destruir(wav);
    }

    if (resultado == 0)
    {
        printf("Archivo WAV creado exitosamente.\n");
    }

    return resultado;
}
