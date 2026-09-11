#include "libreria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

wav_t *wav_crear(uint32_t sample_rate, uint16_t num_channels, uint16_t bits_per_sample, uint32_t num_samples)
{
    wav_t *wav = NULL;
    int exito = 1;

    if (sample_rate == 0 || num_channels == 0 || bits_per_sample == 0 || num_samples == 0)
    {
        exito = 0;
    }

    if (exito)
    {
        wav = (wav_t *)malloc(sizeof(wav_t));
        if (wav == NULL)
        {
            exito = 0;
        }
    }

    if (exito)
    {
        wav->samples = (int16_t *)malloc(num_samples * num_channels * sizeof(int16_t));
        if (wav->samples == NULL)
        {
            free(wav);
            wav = NULL;
            exito = 0;
        }
    }

    if (exito)
    {
        uint32_t subchunk2_size = num_samples * num_channels * bits_per_sample / 8;
        uint32_t chunk_size = 36 + subchunk2_size;

        memcpy(wav->header.riff, "RIFF", 4);
        wav->header.chunk_size = chunk_size;
        memcpy(wav->header.wave, "WAVE", 4);
        memcpy(wav->header.fmt, "fmt ", 4);
        wav->header.subchunk1_size = 16;
        wav->header.audio_format = 1;
        wav->header.num_channels = num_channels;
        wav->header.sample_rate = sample_rate;
        wav->header.byte_rate = sample_rate * num_channels * bits_per_sample / 8;
        wav->header.block_align = num_channels * bits_per_sample / 8;
        wav->header.bits_per_sample = bits_per_sample;
        memcpy(wav->header.data, "data", 4);
        wav->header.subchunk2_size = subchunk2_size;
    }

    return wav;
}

void wav_destruir(wav_t *wav)
{
    if (wav != NULL)
    {
        if (wav->samples != NULL)
        {
            free(wav->samples);
        }
        free(wav);
    }
}

int wav_guardar(const wav_t *wav, const char *path)
{
    int valor_retorno = 0;
    FILE *file = NULL;

    if (wav == NULL || path == NULL)
    {
        valor_retorno = -1;
    }

    if (valor_retorno == 0)
    {
        file = fopen(path, "wb");
        if (file == NULL)
        {
            valor_retorno = -1;
        }
    }

    if (valor_retorno == 0)
    {
        size_t written = fwrite(&wav->header, sizeof(wav_header_t), 1, file);
        if (written < 1)
        {
            valor_retorno = -1;
        }
    }

    if (valor_retorno == 0)
    {
        size_t written = fwrite(wav->samples, sizeof(int16_t), wav->header.subchunk2_size / sizeof(int16_t), file);
        if (written < wav->header.subchunk2_size / sizeof(int16_t))
        {
            valor_retorno = -1;
        }
    }

    if (file != NULL)
    {
        fclose(file);
    }

    return valor_retorno;
}

wav_t *wav_leer(const char *path)
{
    wav_t *wav = NULL;
    FILE *file = NULL;
    int exito = 1;

    if (path == NULL)
    {
        exito = 0;
    }

    if (exito)
    {
        file = fopen(path, "rb");
        if (file == NULL)
        {
            exito = 0;
        }
    }

    if (exito)
    {
        wav = (wav_t *)malloc(sizeof(wav_t));
        if (wav == NULL)
        {
            exito = 0;
        }
    }

    if (exito)
    {
        size_t read = fread(&wav->header, sizeof(wav_header_t), 1, file);
        if (read < 1)
        {
            free(wav);
            wav = NULL;
            exito = 0;
        }
    }

    if (exito)
    {
        wav->samples = (int16_t *)malloc(wav->header.subchunk2_size);
        if (wav->samples == NULL)
        {
            free(wav);
            wav = NULL;
            exito = 0;
        }
    }

    if (exito)
    {
        size_t read = fread(wav->samples, wav->header.subchunk2_size, 1, file);
        if (read < 1)
        {
            free(wav->samples);
            free(wav);
            wav = NULL;
            exito = 0;
        }
    }

    if (file != NULL)
    {
        fclose(file);
    }

    return wav;
}

int wav_generar_tono(wav_t *wav, float frecuencia, float duracion, float amplitud)
{
    int valor_retorno = 0;
    uint32_t num_samples = 0;

    if (wav == NULL || wav->samples == NULL || frecuencia <= 0 || duracion <= 0 || amplitud < 0 || amplitud > 1.0)
    {
        valor_retorno = -1;
    }

    if (valor_retorno == 0)
    {
        num_samples = (uint32_t)(wav->header.sample_rate * duracion);
        if (num_samples * wav->header.num_channels * sizeof(int16_t) > wav->header.subchunk2_size)
        {
            valor_retorno = -1;
        }
    }

    if (valor_retorno == 0)
    {
        double max_amplitude = pow(2, wav->header.bits_per_sample - 1) - 1;
        for (uint32_t i = 0; i < num_samples; i++)
        {
            double value = amplitud * sin(2 * M_PI * frecuencia * i / wav->header.sample_rate);
            int16_t sample = (int16_t)(value * max_amplitude);
            for (uint16_t j = 0; j < wav->header.num_channels; j++)
            {
                wav->samples[i * wav->header.num_channels + j] = sample;
            }
        }
    }

    return valor_retorno;
}
