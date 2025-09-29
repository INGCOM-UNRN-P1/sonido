#ifndef SONIDO_H
#define SONIDO_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* WAV file format structures */
typedef struct {
    char     chunk_id[4];       // "RIFF"
    uint32_t chunk_size;        // File size - 8
    char     format[4];         // "WAVE"
} wav_riff_header_t;

typedef struct {
    char     subchunk_id[4];    // "fmt "
    uint32_t subchunk_size;     // Size of this subchunk (16 for PCM)
    uint16_t audio_format;      // 1 for PCM
    uint16_t num_channels;      // Number of channels
    uint32_t sample_rate;       // Sample rate in Hz
    uint32_t byte_rate;         // Bytes per second
    uint16_t block_align;       // Bytes per sample frame
    uint16_t bits_per_sample;   // Bits per sample
} wav_fmt_header_t;

typedef struct {
    char     subchunk_id[4];    // "data"
    uint32_t subchunk_size;     // Size of audio data
} wav_data_header_t;

typedef struct {
    wav_riff_header_t riff;
    wav_fmt_header_t  fmt;
    wav_data_header_t data;
    FILE*             file;
    char*             filename;
    int               mode;     // 0 = read, 1 = write
    uint32_t          data_pos; // Position where audio data starts
} wav_file_t;

/* Error codes */
typedef enum {
    SONIDO_OK = 0,
    SONIDO_ERROR_FILE_NOT_FOUND,
    SONIDO_ERROR_INVALID_FORMAT,
    SONIDO_ERROR_MEMORY_ALLOCATION,
    SONIDO_ERROR_IO,
    SONIDO_ERROR_INVALID_PARAMETER,
    SONIDO_ERROR_UNSUPPORTED_FORMAT
} sonido_error_t;

/* Function declarations */

/* File operations */
wav_file_t* sonido_open(const char* filename, sonido_error_t* error);
wav_file_t* sonido_create(const char* filename, uint16_t num_channels, 
                         uint32_t sample_rate, uint16_t bits_per_sample, 
                         sonido_error_t* error);
sonido_error_t sonido_close(wav_file_t* wav);

/* Audio data operations */
sonido_error_t sonido_read_samples(wav_file_t* wav, void* buffer, 
                                  uint32_t num_samples, uint32_t* samples_read);
sonido_error_t sonido_write_samples(wav_file_t* wav, const void* buffer, 
                                   uint32_t num_samples);

/* File information */
uint32_t sonido_get_sample_rate(const wav_file_t* wav);
uint16_t sonido_get_num_channels(const wav_file_t* wav);
uint16_t sonido_get_bits_per_sample(const wav_file_t* wav);
uint32_t sonido_get_num_samples(const wav_file_t* wav);
double   sonido_get_duration(const wav_file_t* wav);

/* Utility functions */
const char* sonido_error_string(sonido_error_t error);
int sonido_is_valid_wav(const char* filename);

#ifdef __cplusplus
}
#endif

#endif /* SONIDO_H */