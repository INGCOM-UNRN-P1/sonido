# sonido
Una librería simple para crear archivos de sonido 'wavefile' sin compresión.

## Características

- Apertura y lectura de archivos WAV existentes
- Creación de nuevos archivos WAV
- Manipulación de datos de audio (lectura/escritura de muestras)
- Soporte para diferentes formatos:
  - Mono/Estéreo/Multicanal
  - 8, 16, 24, 32 bits por muestra
  - Frecuencias de muestreo personalizables
- Validación de formato WAV
- Manejo de errores completo

## Estructura del proyecto

```
sonido/
├── include/        # Archivos de cabecera
│   └── sonido.h   # API principal
├── src/           # Código fuente
│   └── sonido.c   # Implementación
├── tests/         # Tests unitarios
│   └── test_sonido.c
├── examples/      # Ejemplos de uso
│   ├── sine_wave.c    # Genera onda senoidal
│   └── wav_info.c     # Muestra información de archivo WAV
└── Makefile       # Build system
```

## Compilación

```bash
# Compilar todo (librería, tests y ejemplos)
make all

# Solo la librería
make lib

# Solo los tests
make test

# Solo los ejemplos
make examples

# Ejecutar tests
make check

# Limpiar archivos compilados
make clean
```

## Uso básico

### Crear un archivo WAV

```c
#include "sonido.h"

sonido_error_t error;
wav_file_t* wav = sonido_create("archivo.wav", 2, 44100, 16, &error);
if (wav) {
    // Escribir datos de audio...
    int16_t samples[1000];
    sonido_write_samples(wav, samples, 1000);
    sonido_close(wav);
}
```

### Leer un archivo WAV

```c
#include "sonido.h"

sonido_error_t error;
wav_file_t* wav = sonido_open("archivo.wav", &error);
if (wav) {
    uint32_t sample_rate = sonido_get_sample_rate(wav);
    uint16_t channels = sonido_get_num_channels(wav);
    
    // Leer datos de audio...
    int16_t buffer[1000];
    uint32_t samples_read;
    sonido_read_samples(wav, buffer, 1000, &samples_read);
    
    sonido_close(wav);
}
```

## API

### Funciones principales

- `sonido_open()` - Abre un archivo WAV existente
- `sonido_create()` - Crea un nuevo archivo WAV
- `sonido_close()` - Cierra el archivo y libera recursos
- `sonido_read_samples()` - Lee muestras de audio
- `sonido_write_samples()` - Escribe muestras de audio

### Funciones de información

- `sonido_get_sample_rate()` - Obtiene la frecuencia de muestreo
- `sonido_get_num_channels()` - Obtiene el número de canales
- `sonido_get_bits_per_sample()` - Obtiene bits por muestra
- `sonido_get_num_samples()` - Obtiene el número total de muestras
- `sonido_get_duration()` - Obtiene la duración en segundos

### Funciones de utilidad

- `sonido_is_valid_wav()` - Verifica si un archivo es WAV válido
- `sonido_error_string()` - Convierte código de error a texto

## Códigos de error

- `SONIDO_OK` - Operación exitosa
- `SONIDO_ERROR_FILE_NOT_FOUND` - Archivo no encontrado
- `SONIDO_ERROR_INVALID_FORMAT` - Formato WAV inválido
- `SONIDO_ERROR_MEMORY_ALLOCATION` - Error de asignación de memoria
- `SONIDO_ERROR_IO` - Error de entrada/salida
- `SONIDO_ERROR_INVALID_PARAMETER` - Parámetro inválido
- `SONIDO_ERROR_UNSUPPORTED_FORMAT` - Formato no soportado

## Ejemplos

Ejecuta los ejemplos incluidos:

```bash
# Genera un archivo WAV con onda senoidal
./examples/sine_wave

# Muestra información de un archivo WAV
./examples/wav_info archivo.wav
```
