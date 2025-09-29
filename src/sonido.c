#include "sonido.h"
#include <string.h>
#include <errno.h>

/* Helper function to read little-endian integers */
static uint32_t read_uint32_le(FILE* file) {
    uint8_t bytes[4];
    if (fread(bytes, 1, 4, file) != 4) {
        return 0;
    }
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) | 
           ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

static uint16_t read_uint16_le(FILE* file) {
    uint8_t bytes[2];
    if (fread(bytes, 1, 2, file) != 2) {
        return 0;
    }
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

/* Helper function to write little-endian integers */
static int write_uint32_le(FILE* file, uint32_t value) {
    uint8_t bytes[4] = {
        (uint8_t)(value & 0xFF),
        (uint8_t)((value >> 8) & 0xFF),
        (uint8_t)((value >> 16) & 0xFF),
        (uint8_t)((value >> 24) & 0xFF)
    };
    return fwrite(bytes, 1, 4, file) == 4 ? 0 : -1;
}

static int write_uint16_le(FILE* file, uint16_t value) {
    uint8_t bytes[2] = {
        (uint8_t)(value & 0xFF),
        (uint8_t)((value >> 8) & 0xFF)
    };
    return fwrite(bytes, 1, 2, file) == 2 ? 0 : -1;
}

wav_file_t* sonido_open(const char* filename, sonido_error_t* error) {
    if (!filename) {
        if (error) *error = SONIDO_ERROR_INVALID_PARAMETER;
        return NULL;
    }

    FILE* file = fopen(filename, "rb");
    if (!file) {
        if (error) *error = SONIDO_ERROR_FILE_NOT_FOUND;
        return NULL;
    }

    wav_file_t* wav = malloc(sizeof(wav_file_t));
    if (!wav) {
        fclose(file);
        if (error) *error = SONIDO_ERROR_MEMORY_ALLOCATION;
        return NULL;
    }

    wav->file = file;
    wav->filename = malloc(strlen(filename) + 1);
    if (!wav->filename) {
        fclose(file);
        free(wav);
        if (error) *error = SONIDO_ERROR_MEMORY_ALLOCATION;
        return NULL;
    }
    strcpy(wav->filename, filename);
    wav->mode = 0; // read mode

    /* Read RIFF header */
    if (fread(wav->riff.chunk_id, 1, 4, file) != 4 ||
        strncmp(wav->riff.chunk_id, "RIFF", 4) != 0) {
        sonido_close(wav);
        if (error) *error = SONIDO_ERROR_INVALID_FORMAT;
        return NULL;
    }

    wav->riff.chunk_size = read_uint32_le(file);
    
    if (fread(wav->riff.format, 1, 4, file) != 4 ||
        strncmp(wav->riff.format, "WAVE", 4) != 0) {
        sonido_close(wav);
        if (error) *error = SONIDO_ERROR_INVALID_FORMAT;
        return NULL;
    }

    /* Read fmt chunk */
    if (fread(wav->fmt.subchunk_id, 1, 4, file) != 4 ||
        strncmp(wav->fmt.subchunk_id, "fmt ", 4) != 0) {
        sonido_close(wav);
        if (error) *error = SONIDO_ERROR_INVALID_FORMAT;
        return NULL;
    }

    wav->fmt.subchunk_size = read_uint32_le(file);
    wav->fmt.audio_format = read_uint16_le(file);
    wav->fmt.num_channels = read_uint16_le(file);
    wav->fmt.sample_rate = read_uint32_le(file);
    wav->fmt.byte_rate = read_uint32_le(file);
    wav->fmt.block_align = read_uint16_le(file);
    wav->fmt.bits_per_sample = read_uint16_le(file);

    /* Validate format */
    if (wav->fmt.audio_format != 1) {  // Only PCM supported
        sonido_close(wav);
        if (error) *error = SONIDO_ERROR_UNSUPPORTED_FORMAT;
        return NULL;
    }

    /* Read data chunk header */
    if (fread(wav->data.subchunk_id, 1, 4, file) != 4 ||
        strncmp(wav->data.subchunk_id, "data", 4) != 0) {
        sonido_close(wav);
        if (error) *error = SONIDO_ERROR_INVALID_FORMAT;
        return NULL;
    }

    wav->data.subchunk_size = read_uint32_le(file);
    wav->data_pos = ftell(file);

    if (error) *error = SONIDO_OK;
    return wav;
}

wav_file_t* sonido_create(const char* filename, uint16_t num_channels, 
                         uint32_t sample_rate, uint16_t bits_per_sample, 
                         sonido_error_t* error) {
    if (!filename || num_channels == 0 || sample_rate == 0 || 
        (bits_per_sample != 8 && bits_per_sample != 16 && bits_per_sample != 24 && bits_per_sample != 32)) {
        if (error) *error = SONIDO_ERROR_INVALID_PARAMETER;
        return NULL;
    }

    FILE* file = fopen(filename, "wb");
    if (!file) {
        if (error) *error = SONIDO_ERROR_IO;
        return NULL;
    }

    wav_file_t* wav = malloc(sizeof(wav_file_t));
    if (!wav) {
        fclose(file);
        if (error) *error = SONIDO_ERROR_MEMORY_ALLOCATION;
        return NULL;
    }

    wav->filename = malloc(strlen(filename) + 1);
    if (!wav->filename) {
        fclose(file);
        free(wav);
        if (error) *error = SONIDO_ERROR_MEMORY_ALLOCATION;
        return NULL;
    }
    strcpy(wav->filename, filename);

    wav->file = file;
    wav->mode = 1; // write mode

    /* Initialize headers */
    memcpy(wav->riff.chunk_id, "RIFF", 4);
    wav->riff.chunk_size = 36; // Will be updated when closing
    memcpy(wav->riff.format, "WAVE", 4);

    memcpy(wav->fmt.subchunk_id, "fmt ", 4);
    wav->fmt.subchunk_size = 16;
    wav->fmt.audio_format = 1; // PCM
    wav->fmt.num_channels = num_channels;
    wav->fmt.sample_rate = sample_rate;
    wav->fmt.bits_per_sample = bits_per_sample;
    wav->fmt.block_align = num_channels * bits_per_sample / 8;
    wav->fmt.byte_rate = sample_rate * wav->fmt.block_align;

    memcpy(wav->data.subchunk_id, "data", 4);
    wav->data.subchunk_size = 0; // Will be updated when closing

    /* Write headers */
    if (fwrite(wav->riff.chunk_id, 1, 4, file) != 4 ||
        write_uint32_le(file, wav->riff.chunk_size) != 0 ||
        fwrite(wav->riff.format, 1, 4, file) != 4 ||
        fwrite(wav->fmt.subchunk_id, 1, 4, file) != 4 ||
        write_uint32_le(file, wav->fmt.subchunk_size) != 0 ||
        write_uint16_le(file, wav->fmt.audio_format) != 0 ||
        write_uint16_le(file, wav->fmt.num_channels) != 0 ||
        write_uint32_le(file, wav->fmt.sample_rate) != 0 ||
        write_uint32_le(file, wav->fmt.byte_rate) != 0 ||
        write_uint16_le(file, wav->fmt.block_align) != 0 ||
        write_uint16_le(file, wav->fmt.bits_per_sample) != 0 ||
        fwrite(wav->data.subchunk_id, 1, 4, file) != 4 ||
        write_uint32_le(file, wav->data.subchunk_size) != 0) {
        sonido_close(wav);
        if (error) *error = SONIDO_ERROR_IO;
        return NULL;
    }

    wav->data_pos = ftell(file);

    if (error) *error = SONIDO_OK;
    return wav;
}

sonido_error_t sonido_close(wav_file_t* wav) {
    if (!wav) {
        return SONIDO_ERROR_INVALID_PARAMETER;
    }

    /* Update file size for write mode */
    if (wav->mode == 1 && wav->file) {
        long current_pos = ftell(wav->file);
        uint32_t data_size = current_pos - wav->data_pos;
        uint32_t file_size = current_pos - 8;

        /* Update data chunk size */
        fseek(wav->file, wav->data_pos - 4, SEEK_SET);
        write_uint32_le(wav->file, data_size);

        /* Update file size */
        fseek(wav->file, 4, SEEK_SET);
        write_uint32_le(wav->file, file_size);
    }

    if (wav->file) {
        fclose(wav->file);
    }
    if (wav->filename) {
        free(wav->filename);
    }
    free(wav);

    return SONIDO_OK;
}

sonido_error_t sonido_read_samples(wav_file_t* wav, void* buffer, 
                                  uint32_t num_samples, uint32_t* samples_read) {
    if (!wav || !buffer || wav->mode != 0) {
        return SONIDO_ERROR_INVALID_PARAMETER;
    }

    uint32_t bytes_per_sample = wav->fmt.bits_per_sample / 8 * wav->fmt.num_channels;
    uint32_t bytes_to_read = num_samples * bytes_per_sample;
    
    size_t bytes_read = fread(buffer, 1, bytes_to_read, wav->file);
    
    if (samples_read) {
        *samples_read = bytes_read / bytes_per_sample;
    }

    return SONIDO_OK;
}

sonido_error_t sonido_write_samples(wav_file_t* wav, const void* buffer, 
                                   uint32_t num_samples) {
    if (!wav || !buffer || wav->mode != 1) {
        return SONIDO_ERROR_INVALID_PARAMETER;
    }

    uint32_t bytes_per_sample = wav->fmt.bits_per_sample / 8 * wav->fmt.num_channels;
    uint32_t bytes_to_write = num_samples * bytes_per_sample;
    
    size_t bytes_written = fwrite(buffer, 1, bytes_to_write, wav->file);
    
    if (bytes_written != bytes_to_write) {
        return SONIDO_ERROR_IO;
    }

    return SONIDO_OK;
}

uint32_t sonido_get_sample_rate(const wav_file_t* wav) {
    return wav ? wav->fmt.sample_rate : 0;
}

uint16_t sonido_get_num_channels(const wav_file_t* wav) {
    return wav ? wav->fmt.num_channels : 0;
}

uint16_t sonido_get_bits_per_sample(const wav_file_t* wav) {
    return wav ? wav->fmt.bits_per_sample : 0;
}

uint32_t sonido_get_num_samples(const wav_file_t* wav) {
    if (!wav) return 0;
    uint32_t bytes_per_sample = wav->fmt.bits_per_sample / 8 * wav->fmt.num_channels;
    return bytes_per_sample > 0 ? wav->data.subchunk_size / bytes_per_sample : 0;
}

double sonido_get_duration(const wav_file_t* wav) {
    if (!wav || wav->fmt.sample_rate == 0) return 0.0;
    return (double)sonido_get_num_samples(wav) / wav->fmt.sample_rate;
}

const char* sonido_error_string(sonido_error_t error) {
    switch (error) {
        case SONIDO_OK:
            return "Success";
        case SONIDO_ERROR_FILE_NOT_FOUND:
            return "File not found";
        case SONIDO_ERROR_INVALID_FORMAT:
            return "Invalid WAV format";
        case SONIDO_ERROR_MEMORY_ALLOCATION:
            return "Memory allocation error";
        case SONIDO_ERROR_IO:
            return "I/O error";
        case SONIDO_ERROR_INVALID_PARAMETER:
            return "Invalid parameter";
        case SONIDO_ERROR_UNSUPPORTED_FORMAT:
            return "Unsupported format";
        default:
            return "Unknown error";
    }
}

int sonido_is_valid_wav(const char* filename) {
    if (!filename) return 0;
    
    FILE* file = fopen(filename, "rb");
    if (!file) return 0;
    
    char chunk_id[4];
    char format[4];
    
    int valid = (fread(chunk_id, 1, 4, file) == 4) &&
                (strncmp(chunk_id, "RIFF", 4) == 0) &&
                (fseek(file, 4, SEEK_CUR) == 0) &&  // Skip chunk size
                (fread(format, 1, 4, file) == 4) &&
                (strncmp(format, "WAVE", 4) == 0);
    
    fclose(file);
    return valid;
}