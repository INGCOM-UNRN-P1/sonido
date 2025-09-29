#include "sonido.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <wav_file>\n", argv[0]);
        printf("Displays information about a WAV file.\n");
        return 1;
    }
    
    const char* filename = argv[1];
    
    // First check if it's a valid WAV file
    if (!sonido_is_valid_wav(filename)) {
        printf("Error: '%s' is not a valid WAV file\n", filename);
        return 1;
    }
    
    printf("File: %s\n", filename);
    printf("Valid WAV format: Yes\n\n");
    
    // Open the file
    sonido_error_t error;
    wav_file_t* wav = sonido_open(filename, &error);
    
    if (!wav) {
        printf("Error opening file: %s\n", sonido_error_string(error));
        return 1;
    }
    
    // Display file information
    printf("WAV File Information:\n");
    printf("  Sample Rate:      %u Hz\n", sonido_get_sample_rate(wav));
    printf("  Channels:         %u\n", sonido_get_num_channels(wav));
    printf("  Bits per Sample:  %u\n", sonido_get_bits_per_sample(wav));
    printf("  Total Samples:    %u\n", sonido_get_num_samples(wav));
    printf("  Duration:         %.3f seconds\n", sonido_get_duration(wav));
    
    // Calculate additional information
    uint32_t byte_rate = sonido_get_sample_rate(wav) * 
                        sonido_get_num_channels(wav) * 
                        sonido_get_bits_per_sample(wav) / 8;
    printf("  Byte Rate:        %u bytes/sec\n", byte_rate);
    
    uint32_t total_bytes = sonido_get_num_samples(wav) * 
                          sonido_get_num_channels(wav) * 
                          sonido_get_bits_per_sample(wav) / 8;
    printf("  Audio Data Size:  %u bytes\n", total_bytes);
    
    // Read a small sample to verify we can read the data
    printf("\nReading first 10 samples...\n");
    
    uint16_t channels = sonido_get_num_channels(wav);
    uint16_t bits_per_sample = sonido_get_bits_per_sample(wav);
    
    if (bits_per_sample == 16) {
        int16_t* buffer = malloc(10 * channels * sizeof(int16_t));
        if (buffer) {
            uint32_t samples_read;
            error = sonido_read_samples(wav, buffer, 10, &samples_read);
            
            if (error == SONIDO_OK) {
                printf("Successfully read %u samples:\n", samples_read);
                for (uint32_t i = 0; i < samples_read && i < 5; i++) {
                    printf("  Sample %u: ", i + 1);
                    for (uint16_t ch = 0; ch < channels; ch++) {
                        printf("Ch%u=%d ", ch + 1, buffer[i * channels + ch]);
                    }
                    printf("\n");
                }
                if (samples_read > 5) {
                    printf("  ... (showing first 5 samples only)\n");
                }
            } else {
                printf("Error reading samples: %s\n", sonido_error_string(error));
            }
            free(buffer);
        }
    } else {
        printf("Note: Sample reading example only supports 16-bit samples\n");
    }
    
    sonido_close(wav);
    
    return 0;
}