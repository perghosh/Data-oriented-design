
/*

#include <immintrin.h>
#include <cstdint>
#include <cstdio>

// Check if a byte is an alphabetic character
inline bool is_alpha(uint8_t c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// SIMD version to find non-alphabetic character
bool has_non_alpha_simd(const char* buffer, size_t length) {
    __m256i lower_mask = _mm256_set1_epi8('a');
    __m256i upper_mask = _mm256_set1_epi8('z');
    __m256i A_mask = _mm256_set1_epi8('A');
    __m256i Z_mask = _mm256_set1_epi8('Z');

    size_t i = 0;
    for (; i + 31 < length; i += 32) {
        __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(buffer + i));
        
        // Check if characters are within 'a' to 'z' or 'A' to 'Z'
        __m256i in_lower = _mm256_and_si256(_mm256_cmpgt_epi8(data, lower_mask), 
                                            _mm256_cmpgt_epi8(upper_mask, data));
        __m256i in_upper = _mm256_and_si256(_mm256_cmpgt_epi8(data, A_mask), 
                                            _mm256_cmpgt_epi8(Z_mask, data));
        
        __m256i is_alpha = _mm256_or_si256(in_lower, in_upper);
        
        // If any bit is zero, there's a non-alpha character
        if (!_mm256_testc_si256(is_alpha, _mm256_set1_epi8(0xFF))) {
            return true;
        }
    }

    // Check the remaining bytes without SIMD
    for (; i < length; ++i) {
        if (!is_alpha(buffer[i])) {
            return true;
        }
    }

    return false;
}

int main() {
    const char buffer[] = "ABCDEFGHijkl!MNOPQRSTUVWXYZ";
    size_t len = sizeof(buffer) - 1; // exclude null terminator
    bool result = has_non_alpha_simd(buffer, len);
    printf("Contains non-alphabetic character: %s\n", result ? "true" : "false");
    return 0;
}
*/