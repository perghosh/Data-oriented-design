/**
 * @file gd_math_type.cpp
 */

#include "gd_math_type.h"

_GD_MATH_TYPE_BEGIN

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string represents any valid numeric value.
 *
 * This function determines whether the string contains a valid number in any format,
 * including integers, decimals, hexadecimal (0x prefix), binary (0b prefix), or
 * octal (0 prefix) representations. Leading and trailing whitespace is ignored.
 * The function accepts optional '+' or '-' signs for signed numbers.
 *
 * @param stringText The string to validate as a number.
 * @return bool True if the string represents a valid number, false otherwise.
 * 
 * @code
 * bool result1 = is_number("123");     // true
 * bool result2 = is_number("3.14");    // true  
 * bool result3 = is_number("0xFF");    // true
 * bool result4 = is_number("0b1010");  // true
 * bool result5 = is_number("abc");     // false
 * @endcode
 */
bool is_number(const std::string_view& stringText) noexcept 
{
    // ## Handle empty string
    if(stringText.empty()) { return false; }
    
    // ## Trim leading and trailing whitespace
    size_t uStart = 0;
    size_t uEnd = stringText.length();
    
    while(uStart < uEnd && (stringText[uStart] == ' ' || stringText[uStart] == '\t')) { uStart++; }
    while(uEnd > uStart && (stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t')) { uEnd--; }
    
    if(uStart >= uEnd) { return false; }
    
    std::string_view trimmed = stringText.substr(uStart, uEnd - uStart);
    
    // ## Check for different number formats
    return is_integer(trimmed) || is_decimal(trimmed) || is_hex(trimmed) || is_binary(trimmed) || is_octal(trimmed);
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string represents a valid integer.
 *
 * This function validates whether the string contains a valid integer representation.
 * It accepts optional '+' or '-' signs at the beginning and allows only digit characters.
 * Leading and trailing whitespace is ignored. Empty strings return false.
 *
 * @param stringText The string to validate as an integer.
 * @return bool True if the string represents a valid integer, false otherwise.
 * 
 * @code
 * bool result1 = is_integer("123");    // true
 * bool result2 = is_integer("-456");   // true
 * bool result3 = is_integer("+789");   // true
 * bool result4 = is_integer("12.3");   // false
 * bool result5 = is_integer("abc");    // false
 * @endcode
 */
bool is_integer(const std::string_view& stringText) noexcept 
{
    // ## Handle empty string
    if(stringText.empty()) { return false; }
    
    // ## Trim leading and trailing whitespace
    size_t uStart = 0;
    size_t uEnd = stringText.length();
    
    while(uStart < uEnd && (stringText[uStart] == ' ' || stringText[uStart] == '\t')) { uStart++; }
    while(uEnd > uStart && (stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t')) { uEnd--; }
    
    if(uStart >= uEnd) { return false; }
    
    size_t uIndex = uStart;
    
    // ## Check for optional sign
    if(stringText[uIndex] == '+' || stringText[uIndex] == '-') 
    {
        uIndex++;
        if(uIndex >= uEnd) { return false; } // Sign without digits
    }
    
    // ## Validate remaining characters are digits
    bool bFoundDigit = false;
    while(uIndex < uEnd) 
    {
        char iChar = stringText[uIndex];
        if(iChar < '0' || iChar > '9') { return false; }
        bFoundDigit = true;
        uIndex++;
    }
    
    return bFoundDigit;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string represents a valid unsigned integer.
 *
 * This function validates whether the string contains a valid unsigned integer
 * representation. It accepts only positive integers without any sign prefix.
 * The string must contain only digit characters (0-9). Leading and trailing 
 * whitespace is ignored. Empty strings and negative numbers return false.
 *
 * @param stringText The string to validate as an unsigned integer.
 * @return bool True if the string represents a valid unsigned integer, false otherwise.
 * 
 * @code
 * bool result1 = is_unsigned("123");   // true
 * bool result2 = is_unsigned("0");     // true
 * bool result3 = is_unsigned("-456");  // false (negative)
 * bool result4 = is_unsigned("+789");  // false (has sign)
 * bool result5 = is_unsigned("12.3");  // false (decimal)
 * @endcode
 */
bool is_unsigned(const std::string_view& stringText) noexcept 
{
    // ## Handle empty string
    if(stringText.empty()) { return false; }
    
    // ## Trim leading and trailing whitespace
    size_t uStart = 0;
    size_t uEnd = stringText.length();
    
    while(uStart < uEnd && (stringText[uStart] == ' ' || stringText[uStart] == '\t')) { uStart++; }
    while(uEnd > uStart && (stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t')) { uEnd--; }
    
    if(uStart >= uEnd) { return false; }
    
    // ## Reject strings with signs
    if(stringText[uStart] == '+' || stringText[uStart] == '-') { return false; }
    
    // ## Validate all characters are digits
    bool bFoundDigit = false;
    for(size_t uIndex = uStart; uIndex < uEnd; uIndex++) 
    {
        char iChar = stringText[uIndex];
        if(iChar < '0' || iChar > '9') { return false; }
        bFoundDigit = true;
    }
    
    return bFoundDigit;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string represents a valid decimal number.
 *
 * This function validates whether the string contains a valid decimal (floating-point)
 * representation. It accepts optional '+' or '-' signs, digits before and after the
 * decimal point, and requires at least one digit and exactly one decimal point.
 * Leading and trailing whitespace is ignored.
 *
 * @param stringText The string to validate as a decimal number.
 * @return bool True if the string represents a valid decimal, false otherwise.
 * 
 * @code
 * bool result1 = is_decimal("3.14");   // true
 * bool result2 = is_decimal("-2.5");   // true
 * bool result3 = is_decimal(".123");   // true
 * bool result4 = is_decimal("456.");   // true
 * bool result5 = is_decimal("12.34.5"); // false
 * @endcode
 */
bool is_decimal(const std::string_view& stringText) noexcept 
{
    // ## Handle empty string
    if(stringText.empty()) { return false; }
    
    // ## Trim leading and trailing whitespace
    size_t uStart = 0;
    size_t uEnd = stringText.length();
    
    while(uStart < uEnd && (stringText[uStart] == ' ' || stringText[uStart] == '\t')) { uStart++; }
    while(uEnd > uStart && (stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t')) { uEnd--; }
    
    if(uStart >= uEnd) { return false; }
    
    size_t uIndex = uStart;
    bool bFoundDecimalPoint = false;
    bool bFoundDigit = false;
    
    // ## Check for optional sign
    if(stringText[uIndex] == '+' || stringText[uIndex] == '-') 
    {
        uIndex++;
        if(uIndex >= uEnd) { return false; } // Sign without content
    }
    
    // ## Validate characters
    while(uIndex < uEnd) 
    {
        char iChar = stringText[uIndex];
        
        if(iChar == '.') 
        {
            if(bFoundDecimalPoint) { return false; } // Multiple decimal points
            bFoundDecimalPoint = true;
        }
        else if(iChar >= '0' && iChar <= '9') 
        {
            bFoundDigit = true;
        }
        else 
        {
            return false; // Invalid character
        }
        
        uIndex++;
    }
    
    // ## Must have decimal point and at least one digit
    return bFoundDecimalPoint && bFoundDigit;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string represents a valid hexadecimal number.
 *
 * This function validates whether the string contains a valid hexadecimal representation.
 * It requires the "0x" or "0X" prefix followed by valid hexadecimal digits (0-9, a-f, A-F).
 * Optional '+' or '-' signs are accepted before the prefix. Leading and trailing 
 * whitespace is ignored.
 *
 * @param stringText The string to validate as a hexadecimal number.
 * @return bool True if the string represents a valid hexadecimal, false otherwise.
 * 
 * @code
 * bool result1 = is_hex("0xFF");     // true
 * bool result2 = is_hex("0x123");    // true
 * bool result3 = is_hex("-0xABC");   // true
 * bool result4 = is_hex("0xGHI");    // false
 * bool result5 = is_hex("123");      // false (no 0x prefix)
 * @endcode
 */
bool is_hex(const std::string_view& stringText) noexcept 
{
    // ## Handle empty string
    if(stringText.empty()) { return false; }
    
    // ## Trim leading and trailing whitespace
    size_t uStart = 0;
    size_t uEnd = stringText.length();
    
    while(uStart < uEnd && (stringText[uStart] == ' ' || stringText[uStart] == '\t')) { uStart++; }
    while(uEnd > uStart && (stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t')) { uEnd--; }
    
    if(uStart >= uEnd) { return false; }
    
    size_t uIndex = uStart;
    
    // ## Check for optional sign
    if(stringText[uIndex] == '+' || stringText[uIndex] == '-') 
    {
        uIndex++;
        if(uIndex >= uEnd) { return false; } // Sign without content
    }
    
    // ## Check for "0x" or "0X" prefix
    if(uIndex + 1 >= uEnd || stringText[uIndex] != '0') { return false; }
    uIndex++;
    
    if(stringText[uIndex] != 'x' && stringText[uIndex] != 'X') { return false; }
    uIndex++;
    
    if(uIndex >= uEnd) { return false; } // No digits after prefix
    
    // ## Validate remaining characters are hex digits
    bool bFoundDigit = false;
    while(uIndex < uEnd) 
    {
        char iChar = stringText[uIndex];
        if(!((iChar >= '0' && iChar <= '9') || (iChar >= 'a' && iChar <= 'f') || (iChar >= 'A' && iChar <= 'F'))) 
        {
            return false;
        }
        bFoundDigit = true;
        uIndex++;
    }
    
    return bFoundDigit;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string represents a valid binary number.
 *
 * This function validates whether the string contains a valid binary representation.
 * It requires the "0b" or "0B" prefix followed by valid binary digits (0 and 1 only).
 * Optional '+' or '-' signs are accepted before the prefix. Leading and trailing 
 * whitespace is ignored.
 *
 * @param stringText The string to validate as a binary number.
 * @return bool True if the string represents a valid binary number, false otherwise.
 * 
 * @code
 * bool result1 = is_binary("0b1010");  // true
 * bool result2 = is_binary("0B0101");  // true
 * bool result3 = is_binary("-0b1100"); // true
 * bool result4 = is_binary("0b1012");  // false (contains '2')
 * bool result5 = is_binary("1010");    // false (no 0b prefix)
 * @endcode
 */
bool is_binary(const std::string_view& stringText) noexcept 
{
    // ## Handle empty string
    if(stringText.empty()) { return false; }
    
    // ## Trim leading and trailing whitespace
    size_t uStart = 0;
    size_t uEnd = stringText.length();
    
    while(uStart < uEnd && (stringText[uStart] == ' ' || stringText[uStart] == '\t')) { uStart++; }
    while(uEnd > uStart && (stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t')) { uEnd--; }
    
    if(uStart >= uEnd) { return false; }
    
    size_t uIndex = uStart;
    
    // ## Check for optional sign
    if(stringText[uIndex] == '+' || stringText[uIndex] == '-') 
    {
        uIndex++;
        if(uIndex >= uEnd) { return false; } // Sign without content
    }
    
    // ## Check for "0b" or "0B" prefix
    if(uIndex + 1 >= uEnd || stringText[uIndex] != '0') { return false; }
    uIndex++;
    
    if(stringText[uIndex] != 'b' && stringText[uIndex] != 'B') { return false; }
    uIndex++;
    
    if(uIndex >= uEnd) { return false; } // No digits after prefix
    
    // ## Validate remaining characters are binary digits
    bool bFoundDigit = false;
    while(uIndex < uEnd) 
    {
        char iChar = stringText[uIndex];
        if(iChar != '0' && iChar != '1') { return false; }
        bFoundDigit = true;
        uIndex++;
    }
    
    return bFoundDigit;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string represents a valid octal number.
 *
 * This function validates whether the string contains a valid octal representation.
 * It requires a leading '0' followed by valid octal digits (0-7 only). Optional 
 * '+' or '-' signs are accepted before the number. Leading and trailing whitespace 
 * is ignored. A single '0' is considered a valid octal number.
 *
 * @param stringText The string to validate as an octal number.
 * @return bool True if the string represents a valid octal number, false otherwise.
 * 
 * @code
 * bool result1 = is_octal("0123");   // true
 * bool result2 = is_octal("0");      // true
 * bool result3 = is_octal("-0567");  // true
 * bool result4 = is_octal("0189");   // false (contains '8' and '9')
 * bool result5 = is_octal("123");    // false (no leading '0')
 * @endcode
 */
bool is_octal(const std::string_view& stringText) noexcept 
{
    // ## Handle empty string
    if(stringText.empty()) { return false; }
    
    // ## Trim leading and trailing whitespace
    size_t uStart = 0;
    size_t uEnd = stringText.length();
    
    while(uStart < uEnd && (stringText[uStart] == ' ' || stringText[uStart] == '\t')) { uStart++; }
    while(uEnd > uStart && (stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t')) { uEnd--; }
    
    if(uStart >= uEnd) { return false; }
    
    size_t uIndex = uStart;
    
    // ## Check for optional sign
    if(stringText[uIndex] == '+' || stringText[uIndex] == '-') 
    {
        uIndex++;
        if(uIndex >= uEnd) { return false; } // Sign without content
    }
    
    // ## Must start with '0'
    if(stringText[uIndex] != '0') { return false; }
    
    // ## If only "0", it's valid
    if(uIndex + 1 >= uEnd) { return true; }
    
    uIndex++;
    
    // ## Validate remaining characters are octal digits
    bool bFoundDigit = true; // We already found '0'
    while(uIndex < uEnd) 
    {
        char iChar = stringText[uIndex];
        if(iChar < '0' || iChar > '7') { return false; }
        uIndex++;
    }
    
    return bFoundDigit;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string contains only alphabetic characters.
 *
 * This function validates whether the string contains only alphabetic characters
 * (a-z, A-Z). It does not accept numbers, special characters, or whitespace.
 * Leading and trailing whitespace is ignored, but internal whitespace will
 * cause the function to return false. Empty strings return false.
 *
 * @param stringText The string to validate as alphabetic.
 * @return bool True if the string contains only alphabetic characters, false otherwise.
 * 
 * @code
 * bool result1 = is_alpha("Hello");     // true
 * bool result2 = is_alpha("abc");       // true
 * bool result3 = is_alpha("Hello123");  // false
 * bool result4 = is_alpha("Hello World"); // false (contains space)
 * bool result5 = is_alpha("");          // false
 * @endcode
 */
bool is_alpha(const std::string_view& stringText) noexcept 
{
    // ## Handle empty string
    if(stringText.empty()) { return false; }
    
    // ## Trim leading and trailing whitespace
    size_t uStart = 0;
    size_t uEnd = stringText.length();
    
    while(uStart < uEnd && (stringText[uStart] == ' ' || stringText[uStart] == '\t')) { uStart++; }
    while(uEnd > uStart && (stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t')) { uEnd--; }
    
    if(uStart >= uEnd) { return false; }
    
    // ## Validate all characters are alphabetic
    bool bFoundAlpha = false;
    for(size_t uIndex = uStart; uIndex < uEnd; uIndex++) 
    {
        char iChar = stringText[uIndex];
        if( !((iChar >= 'a' && iChar <= 'z') || (iChar >= 'A' && iChar <= 'Z')) ) 
        {
            return false;
        }
        bFoundAlpha = true;
    }
    
    return bFoundAlpha;
}

_GD_MATH_TYPE_END