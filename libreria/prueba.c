#include "libreria.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void prueba_crear_wav_ok()
{
    printf("Ejecutando prueba: creacion de wav OK\n");
    wav_t *wav = wav_crear(44100, 1, 16, 1000);
    assert(wav != NULL);
    wav_destruir(wav);
}

void prueba_crear_wav_parametros_invalidos()
{
    printf("Ejecutando prueba: creacion de wav con parametros invalidos\n");
    wav_t *wav = wav_crear(0, 1, 16, 1000);
    assert(wav == NULL);
}

void prueba_guardar_leer_wav()
{
    printf("Ejecutando prueba: guardar y leer wav\n");
    const char *filename = "test.wav";
    wav_t *wav_out = wav_crear(44100, 1, 16, 100);
    assert(wav_out != NULL);

    for (int i = 0; i < 100; i++) {
        wav_out->samples[i] = i * 100;
    }

    int guardado = wav_guardar(wav_out, filename);
    assert(guardado == 0);

    wav_t *wav_in = wav_leer(filename);
    assert(wav_in != NULL);

    assert(memcmp(&wav_out->header, &wav_in->header, sizeof(wav_header_t)) == 0);
    assert(memcmp(wav_out->samples, wav_in->samples, 100 * sizeof(int16_t)) == 0);

    wav_destruir(wav_out);
    wav_destruir(wav_in);
    remove(filename);
}

void prueba_leer_no_existe()
{
    printf("Ejecutando prueba: leer archivo que no existe\n");
    wav_t *wav = wav_leer("no_existe.wav");
    assert(wav == NULL);
}

void prueba_generar_tono()
{
    printf("Ejecutando prueba: generar tono\n");
    wav_t *wav = wav_crear(44100, 1, 16, 44100);
    assert(wav != NULL);

    int resultado = wav_generar_tono(wav, 440, 1.0, 0.5);
    assert(resultado == 0);

    // La onda sinusoidal debe empezar en 0
    assert(wav->samples[0] == 0);

    wav_destruir(wav);
}

int main()
{
    prueba_crear_wav_ok();
    prueba_crear_wav_parametros_invalidos();
    prueba_guardar_leer_wav();
    prueba_leer_no_existe();
    prueba_generar_tono();

    printf("Todas las pruebas pasaron.\n");
    return 0;
}
