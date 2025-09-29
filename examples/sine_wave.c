#include "sonido.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100
#define CHANNELS 1
#define BITS_PER_SAMPLE 16
#define DURATION 2.0
#define FREQUENCY 440.0

int main() {
    printf("Creating a sine wave WAV file...\n");
    
    sonido_error_t error;
    wav_file_t* wav = sonido_create("sine_wave.wav", CHANNELS, SAMPLE_RATE, 
                                   BITS_PER_SAMPLE, &error);
    
    if (!wav) {
        printf("Error creating WAV file: %s\n", sonido_error_string(error));
        return 1;
    }
    
    printf("File created with:\n");
    printf("  Sample rate: %d Hz\n", SAMPLE_RATE);
    printf("  Channels: %d\n", CHANNELS);
    printf("  Bits per sample: %d\n", BITS_PER_SAMPLE);
    printf("  Duration: %.1f seconds\n", DURATION);
    printf("  Frequency: %.1f Hz\n", FREQUENCY);
    
    uint32_t num_samples = (uint32_t)(SAMPLE_RATE * DURATION);
    int16_t* buffer = malloc(num_samples * sizeof(int16_t));
    
    if (!buffer) {
        printf("Error allocating memory for audio buffer\n");
        sonido_close(wav);
        return 1;
    }
    
    // Generate sine wave
    for (uint32_t i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        double amplitude = 0.5; // 50% volume
        buffer[i] = (int16_t)(sin(2 * M_PI * FREQUENCY * t) * amplitude * 32767);
    }
    
    printf("Writing %d samples...\n", num_samples);
    
    error = sonido_write_samples(wav, buffer, num_samples);
    if (error != SONIDO_OK) {
        printf("Error writing samples: %s\n", sonido_error_string(error));
        free(buffer);
        sonido_close(wav);
        return 1;
    }
    
    free(buffer);
    sonido_close(wav);
    
    printf("WAV file 'sine_wave.wav' created successfully!\n");
    
    // Verify the file by reading it back
    printf("\nVerifying the created file...\n");
    
    wav = sonido_open("sine_wave.wav", &error);
    if (!wav) {
        printf("Error opening created file: %s\n", sonido_error_string(error));
        return 1;
    }
    
    printf("File properties:\n");
    printf("  Sample rate: %d Hz\n", sonido_get_sample_rate(wav));
    printf("  Channels: %d\n", sonido_get_num_channels(wav));
    printf("  Bits per sample: %d\n", sonido_get_bits_per_sample(wav));
    printf("  Number of samples: %d\n", sonido_get_num_samples(wav));
    printf("  Duration: %.2f seconds\n", sonido_get_duration(wav));
    
    sonido_close(wav);
    
    printf("\nFile verification successful!\n");
    
    return 0;
}