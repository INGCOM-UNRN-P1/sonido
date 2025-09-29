#include "sonido.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TEST_SAMPLE_RATE 44100
#define TEST_CHANNELS 2
#define TEST_BITS_PER_SAMPLE 16
#define TEST_DURATION 1.0
#define TEST_FREQUENCY 440.0

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    printf("Running test: %s\n", #name); \
    tests_run++; \
    if (test_##name()) { \
        printf("  PASSED\n"); \
        tests_passed++; \
    } else { \
        printf("  FAILED\n"); \
    }

static void generate_test_data(int16_t* buffer, uint32_t num_samples, uint16_t channels) {
    for (uint32_t i = 0; i < num_samples; i++) {
        double t = (double)i / TEST_SAMPLE_RATE;
        int16_t sample = (int16_t)(sin(2 * M_PI * TEST_FREQUENCY * t) * 16000);
        
        for (uint16_t ch = 0; ch < channels; ch++) {
            buffer[i * channels + ch] = sample;
        }
    }
}

static int test_create_wav_file() {
    sonido_error_t error;
    const char* filename = "/tmp/test_create.wav";
    
    wav_file_t* wav = sonido_create(filename, TEST_CHANNELS, TEST_SAMPLE_RATE, 
                                   TEST_BITS_PER_SAMPLE, &error);
    if (!wav || error != SONIDO_OK) {
        printf("    Failed to create WAV file: %s\n", sonido_error_string(error));
        return 0;
    }
    
    uint32_t num_samples = (uint32_t)(TEST_SAMPLE_RATE * TEST_DURATION);
    int16_t* buffer = malloc(num_samples * TEST_CHANNELS * sizeof(int16_t));
    generate_test_data(buffer, num_samples, TEST_CHANNELS);
    
    error = sonido_write_samples(wav, buffer, num_samples);
    if (error != SONIDO_OK) {
        printf("    Failed to write samples: %s\n", sonido_error_string(error));
        free(buffer);
        sonido_close(wav);
        return 0;
    }
    
    free(buffer);
    sonido_close(wav);
    
    // Check if file was created
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("    File was not created\n");
        return 0;
    }
    fclose(file);
    
    return 1;
}

static int test_open_wav_file() {
    sonido_error_t error;
    const char* filename = "/tmp/test_create.wav";
    
    wav_file_t* wav = sonido_open(filename, &error);
    if (!wav || error != SONIDO_OK) {
        printf("    Failed to open WAV file: %s\n", sonido_error_string(error));
        return 0;
    }
    
    if (sonido_get_sample_rate(wav) != TEST_SAMPLE_RATE) {
        printf("    Sample rate mismatch: expected %d, got %d\n", 
               TEST_SAMPLE_RATE, sonido_get_sample_rate(wav));
        sonido_close(wav);
        return 0;
    }
    
    if (sonido_get_num_channels(wav) != TEST_CHANNELS) {
        printf("    Channel count mismatch: expected %d, got %d\n", 
               TEST_CHANNELS, sonido_get_num_channels(wav));
        sonido_close(wav);
        return 0;
    }
    
    if (sonido_get_bits_per_sample(wav) != TEST_BITS_PER_SAMPLE) {
        printf("    Bits per sample mismatch: expected %d, got %d\n", 
               TEST_BITS_PER_SAMPLE, sonido_get_bits_per_sample(wav));
        sonido_close(wav);
        return 0;
    }
    
    sonido_close(wav);
    return 1;
}

static int test_read_samples() {
    sonido_error_t error;
    const char* filename = "/tmp/test_create.wav";
    
    wav_file_t* wav = sonido_open(filename, &error);
    if (!wav || error != SONIDO_OK) {
        printf("    Failed to open WAV file: %s\n", sonido_error_string(error));
        return 0;
    }
    
    uint32_t num_samples = sonido_get_num_samples(wav);
    int16_t* buffer = malloc(num_samples * TEST_CHANNELS * sizeof(int16_t));
    uint32_t samples_read;
    
    error = sonido_read_samples(wav, buffer, num_samples, &samples_read);
    if (error != SONIDO_OK) {
        printf("    Failed to read samples: %s\n", sonido_error_string(error));
        free(buffer);
        sonido_close(wav);
        return 0;
    }
    
    if (samples_read != num_samples) {
        printf("    Sample count mismatch: expected %d, got %d\n", 
               num_samples, samples_read);
        free(buffer);
        sonido_close(wav);
        return 0;
    }
    
    free(buffer);
    sonido_close(wav);
    return 1;
}

static int test_get_duration() {
    sonido_error_t error;
    const char* filename = "/tmp/test_create.wav";
    
    wav_file_t* wav = sonido_open(filename, &error);
    if (!wav || error != SONIDO_OK) {
        printf("    Failed to open WAV file: %s\n", sonido_error_string(error));
        return 0;
    }
    
    double duration = sonido_get_duration(wav);
    double expected_duration = TEST_DURATION;
    
    if (fabs(duration - expected_duration) > 0.01) {
        printf("    Duration mismatch: expected %.2f, got %.2f\n", 
               expected_duration, duration);
        sonido_close(wav);
        return 0;
    }
    
    sonido_close(wav);
    return 1;
}

static int test_error_handling() {
    sonido_error_t error;
    
    // Test opening non-existent file
    wav_file_t* wav = sonido_open("/nonexistent/file.wav", &error);
    if (wav || error != SONIDO_ERROR_FILE_NOT_FOUND) {
        printf("    Should fail to open non-existent file\n");
        if (wav) sonido_close(wav);
        return 0;
    }
    
    // Test invalid parameters
    wav = sonido_create(NULL, TEST_CHANNELS, TEST_SAMPLE_RATE, 
                       TEST_BITS_PER_SAMPLE, &error);
    if (wav || error != SONIDO_ERROR_INVALID_PARAMETER) {
        printf("    Should fail with NULL filename\n");
        if (wav) sonido_close(wav);
        return 0;
    }
    
    wav = sonido_create("test.wav", 0, TEST_SAMPLE_RATE, 
                       TEST_BITS_PER_SAMPLE, &error);
    if (wav || error != SONIDO_ERROR_INVALID_PARAMETER) {
        printf("    Should fail with 0 channels\n");
        if (wav) sonido_close(wav);
        return 0;
    }
    
    return 1;
}

static int test_is_valid_wav() {
    const char* filename = "/tmp/test_create.wav";
    
    if (!sonido_is_valid_wav(filename)) {
        printf("    Valid WAV file not recognized\n");
        return 0;
    }
    
    if (sonido_is_valid_wav("/nonexistent/file.wav")) {
        printf("    Non-existent file should not be valid\n");
        return 0;
    }
    
    if (sonido_is_valid_wav(NULL)) {
        printf("    NULL filename should not be valid\n");
        return 0;
    }
    
    return 1;
}

static int test_mono_file() {
    sonido_error_t error;
    const char* filename = "/tmp/test_mono.wav";
    
    wav_file_t* wav = sonido_create(filename, 1, TEST_SAMPLE_RATE, 
                                   TEST_BITS_PER_SAMPLE, &error);
    if (!wav || error != SONIDO_OK) {
        printf("    Failed to create mono WAV file: %s\n", sonido_error_string(error));
        return 0;
    }
    
    uint32_t num_samples = (uint32_t)(TEST_SAMPLE_RATE * 0.5);
    int16_t* buffer = malloc(num_samples * sizeof(int16_t));
    generate_test_data(buffer, num_samples, 1);
    
    error = sonido_write_samples(wav, buffer, num_samples);
    if (error != SONIDO_OK) {
        printf("    Failed to write mono samples: %s\n", sonido_error_string(error));
        free(buffer);
        sonido_close(wav);
        return 0;
    }
    
    free(buffer);
    sonido_close(wav);
    
    // Read it back
    wav = sonido_open(filename, &error);
    if (!wav || error != SONIDO_OK) {
        printf("    Failed to open mono WAV file: %s\n", sonido_error_string(error));
        return 0;
    }
    
    if (sonido_get_num_channels(wav) != 1) {
        printf("    Mono file should have 1 channel, got %d\n", 
               sonido_get_num_channels(wav));
        sonido_close(wav);
        return 0;
    }
    
    sonido_close(wav);
    return 1;
}

int main() {
    printf("Running sonido library tests...\n\n");
    
    TEST(create_wav_file);
    TEST(open_wav_file);
    TEST(read_samples);
    TEST(get_duration);
    TEST(error_handling);
    TEST(is_valid_wav);
    TEST(mono_file);
    
    printf("\nTest Results: %d/%d tests passed\n", tests_passed, tests_run);
    
    if (tests_passed == tests_run) {
        printf("All tests PASSED!\n");
        return 0;
    } else {
        printf("Some tests FAILED!\n");
        return 1;
    }
}