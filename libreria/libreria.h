#ifndef WAV_H
#define WAV_H

#include <stdint.h>
#include <stddef.h>

// Estructura para la cabecera de un archivo WAV
typedef struct {
    // RIFF Chunk Descriptor
    uint8_t riff[4];              // "RIFF"
    uint32_t chunk_size;          // Tamaño del archivo - 8
    uint8_t wave[4];              // "WAVE"
    // "fmt" sub-chunk
    uint8_t fmt[4];               // "fmt "
    uint32_t subchunk1_size;      // 16 for PCM
    uint16_t audio_format;        // 1 for PCM
    uint16_t num_channels;        // Mono = 1, Stereo = 2, etc.
    uint32_t sample_rate;         // 8000, 44100, etc.
    uint32_t byte_rate;           // sample_rate * num_channels * bits_per_sample/8
    uint16_t block_align;         // num_channels * bits_per_sample/8
    uint16_t bits_per_sample;     // 8 bits = 8, 16 bits = 16, etc.
    // "data" sub-chunk
    uint8_t data[4];              // "data"
    uint32_t subchunk2_size;      // num_samples * num_channels * bits_per_sample/8
} wav_header_t;

// Estructura para manejar un archivo WAV en memoria
typedef struct {
    wav_header_t header;
    int16_t *samples;
} wav_t;

/**
 * Crea una estructura wav_t en memoria.
 *
 * @param sample_rate Tasa de muestreo (e.g., 44100).
 * @param num_channels Número de canales (1 para mono, 2 para estéreo).
 * @param bits_per_sample Bits por muestra (e.g., 16).
 * @param num_samples Número de muestras por canal.
 * @pre sample_rate, num_channels, bits_per_sample y num_samples deben ser
 *      mayores a 0.
 * @returns Un puntero a la estructura wav_t creada, o NULL si falla la
 *          asignación de memoria. El llamador es responsable de liberar
 *          la memoria con wav_destruir().
 * @post La estructura wav_t está inicializada y lista para ser usada.
 *       Las muestras de audio no están inicializadas.
 */
wav_t *wav_crear(uint32_t sample_rate, uint16_t num_channels,
                 uint16_t bits_per_sample, uint32_t num_samples);

/**
 * Libera la memoria de una estructura wav_t.
 *
 * @param wav Puntero a la estructura wav_t a destruir.
 * @pre wav no debe ser NULL.
 * @post La memoria asociada a wav y sus muestras es liberada.
 */
void wav_destruir(wav_t *wav);

/**
 * Guarda una estructura wav_t a un archivo.
 *
 * @param wav Puntero a la estructura wav_t a guardar.
 * @param path Ruta del archivo donde se guardará el WAV.
 * @pre wav y path no deben ser NULL.
 * @returns 0 si se guardó correctamente, -1 si hubo un error.
 * @post El archivo WAV es creado o sobrescrito en la ruta especificada.
 */
int wav_guardar(const wav_t *wav, const char *path);

/**
 * Lee un archivo WAV y lo carga en una estructura wav_t.
 *
 * @param path Ruta del archivo WAV a leer.
 * @pre path no debe ser NULL.
 * @returns Un puntero a la estructura wav_t leída, o NULL si hubo un error.
 *          El llamador es responsable de liberar la memoria con wav_destruir().
 * @post La estructura wav_t contiene la información y muestras del archivo.
 */
wav_t *wav_leer(const char *path);

/**
 * Genera un tono de onda sinusoidal y lo carga en una estructura wav_t.
 *
 * @param wav Puntero a la estructura wav_t donde se generará el tono.
 * @param frecuencia Frecuencia del tono en Hz.
 * @param duracion Duración del tono en segundos.
 * @param amplitud Amplitud del tono (0.0 a 1.0).
 * @pre wav no debe ser NULL. frecuencia, duracion y amplitud deben ser
 *      valores positivos.
 * @returns 0 si se generó correctamente, -1 si hubo un error.
 * @post Las muestras en la estructura wav_t son reemplazadas por el tono
 *       generado.
 */
int wav_generar_tono(wav_t *wav, float frecuencia, float duracion, float amplitud);

#endif // WAV_H
