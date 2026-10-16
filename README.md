# `sonido` - Creación de archivos de audio WAV

Esta librería crea, lee y guarda archivos de sonido `.wav` sin compresión (PCM). Un
WAV PCM es, por dentro, una cabecera fija de 44 bytes seguida de las muestras del
sonido una detrás de otra: con todo el código expuesto en el repositorio se puede ver
cómo se arma cada campo de la cabecera y cómo una onda se convierte en números.

> [!WARNING]
> Las muestras no tienen compresión: un segundo de sonido mono a 44.100 Hz y 16 bits
> ocupa unos 86 KB (44.100 × 2 bytes), y un minuto en estéreo, unos 10 MB.

## Objetivos del Makefile

1. `make` compila la librería y el ejercicio.
2. `make run` ejecuta el `main.c` del ejercicio (genera `tono.wav`).
3. `make test` compila y ejecuta las pruebas de la librería y del ejercicio.
4. `make clean` limpia los compilados e intermedios.
5. `make librerias` compila solo la librería.

Recuerden no enviar al repositorio los compilados (`test_bin`, `.exe`) e intermedios
(`.o`/`.a`); para eso usen `make clean` en la raíz del proyecto.

## Estructura

| Carpeta      | Contenido                                                                                   |
| :----------- | :------------------------------------------------------------------------------------------ |
| `libreria/`  | `libreria.h` y `libreria.c` (la librería, que se compila como `liblibreria.a`) y sus pruebas |
| `ejercicio/` | `main.c` de ejemplo (un tono de 440 Hz), `referencia.wav` y el lugar para el ejercicio      |

## Manual de la librería

### Compilación y enlazado

La librería usa `sin()` de `math.h`, así que hay que enlazar también con `-lm`:

```bash
make -C libreria
gcc -std=c11 -Wall -Wextra -o mi_programa mi_programa.c -L./libreria -llibreria -lm
```

### Tipos de datos

- `wav_header_t`: la cabecera del archivo, campo por campo como se guarda en disco
  (`"RIFF"`, tamaño, `"WAVE"`, el bloque `"fmt "` con el formato y el bloque `"data"`
  con el tamaño de las muestras).
- `wav_t`: un archivo en memoria, con su `header` y el arreglo `samples` de muestras
  `int16_t` (en estéreo, intercaladas: izquierda, derecha, izquierda…).

### Funciones

| Función | Qué hace | Devuelve |
| :-- | :-- | :-- |
| `wav_t *wav_crear(uint32_t sample_rate, uint16_t num_channels, uint16_t bits_per_sample, uint32_t num_samples)` | reserva un WAV con la cabecera completa y `num_samples` muestras por canal, sin inicializar | el WAV, o `NULL` si algún parámetro es 0 o no hay memoria |
| `void wav_destruir(wav_t *wav)` | libera el WAV y sus muestras | — |
| `int wav_guardar(const wav_t *wav, const char *path)` | escribe la cabecera y las muestras en `path` (lo crea o lo sobrescribe) | `0`, o `-1` si no se pudo escribir |
| `wav_t *wav_leer(const char *path)` | carga un WAV del disco | el WAV, o `NULL` si el archivo no existe o no se pudo leer |
| `int wav_generar_tono(wav_t *wav, float frecuencia, float duracion, float amplitud)` | reemplaza las muestras por una onda senoidal de `frecuencia` Hz durante `duracion` segundos, con `amplitud` entre 0,0 y 1,0 | `0`, o `-1` si los parámetros no son válidos |

Todo WAV que devuelven `wav_crear` y `wav_leer` lo libera quien lo pidió, con
`wav_destruir`.

### Ejemplo de uso

```c
#include <stdio.h>
#include "libreria/libreria.h"

int main(void)
{
    int resultado = 0;
    wav_t *wav = wav_crear(44100, 1, 16, 44100 * 2);   /* 2 segundos, mono, 16 bits */
    if (wav == NULL)
    {
        resultado = 1;
    }
    else
    {
        if (wav_generar_tono(wav, 440.0f, 2.0f, 0.5f) != 0 || wav_guardar(wav, "tono.wav") != 0)
        {
            fprintf(stderr, "No se pudo generar el tono\n");
            resultado = 1;
        }
        wav_destruir(wav);
    }
    return resultado;
}
```

El archivo resultante se puede escuchar con cualquier reproductor (`aplay tono.wav`
en Linux). `ejercicio/referencia.wav` es el mismo tono (440 Hz, 2 s, mono, 16 bits)
grabado con otro programa: trae un bloque `LIST` antes de `"data"`, así que comparar
las dos cabeceras byte a byte (`xxd -l 64`) es un buen primer ejercicio.

### Pruebas

`make test` corre las pruebas de `libreria/prueba.c`: creación con parámetros válidos
e inválidos, guardar y volver a leer, leer un archivo que no existe y generar un tono.

## Limitaciones

- Las muestras se guardan siempre como `int16_t`: aunque la cabecera acepta otros
  valores, el único formato que funciona de punta a punta es **16 bits por muestra**.
- `wav_leer` lee la cabecera tal cual: no valida que el archivo sea un WAV PCM ni
  soporta bloques extra (`LIST`, metadatos) entre `"fmt "` y `"data"`.

## Ideas

Con la onda en un arreglo se puede construir de todo: sumar tonos para hacer acordes,
generar una melodía a partir de una partitura en texto, aplicar un *fade in*/*fade
out*, invertir un sonido o mezclar dos archivos.
