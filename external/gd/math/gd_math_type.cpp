// @FILE [tag: math, type] [description: Type checking functions] [type: source]

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
   bool is_number( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim leading and trailing whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   std::string_view trimmed = stringText.substr( uStart, uEnd - uStart );

   // ## Check for different number formats
   return is_integer( trimmed ) || is_decimal( trimmed ) || is_hex( trimmed ) || is_binary( trimmed ) || is_octal( trimmed );
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
bool is_integer( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim leading and trailing whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   size_t uIndex = uStart;

   // ## Check for optional sign
   if( stringText[uIndex] == '+' || stringText[uIndex] == '-' )
   {
      uIndex++;
      if( uIndex >= uEnd ) { return false; } // Sign without digits
   }

   // ## Validate remaining characters are digits
   bool bFoundDigit = false;
   while( uIndex < uEnd )
   {
      char iChar = stringText[uIndex];
      if( iChar < '0' || iChar > '9' ) { return false; }
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
bool is_unsigned( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim leading and trailing whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   // ## Reject strings with signs
   if( stringText[uStart] == '+' || stringText[uStart] == '-' ) { return false; }

   // ## Validate all characters are digits
   bool bFoundDigit = false;
   for( size_t uIndex = uStart; uIndex < uEnd; uIndex++ )
   {
      char iChar = stringText[uIndex];
      if( iChar < '0' || iChar > '9' ) { return false; }
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
bool is_decimal( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim leading and trailing whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   size_t uIndex = uStart;
   bool bFoundDecimalPoint = false;
   bool bFoundDigit = false;

   // ## Check for optional sign
   if( stringText[uIndex] == '+' || stringText[uIndex] == '-' )
   {
      uIndex++;
      if( uIndex >= uEnd ) { return false; } // Sign without content
   }

   // ## Validate characters
   while( uIndex < uEnd )
   {
      char iChar = stringText[uIndex];

      if( iChar == '.' )
      {
         if( bFoundDecimalPoint ) { return false; } // Multiple decimal points
         bFoundDecimalPoint = true;
      }
      else if( iChar >= '0' && iChar <= '9' )
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
bool is_hex( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim leading and trailing whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   size_t uIndex = uStart;

   // ## Check for optional sign
   if( stringText[uIndex] == '+' || stringText[uIndex] == '-' )
   {
      uIndex++;
      if( uIndex >= uEnd ) { return false; } // Sign without content
   }

   // ## Check for "0x" or "0X" prefix
   if( uIndex + 1 >= uEnd || stringText[uIndex] != '0' ) { return false; }
   uIndex++;

   if( stringText[uIndex] != 'x' && stringText[uIndex] != 'X' ) { return false; }
   uIndex++;

   if( uIndex >= uEnd ) { return false; } // No digits after prefix

   // ## Validate remaining characters are hex digits
   bool bFoundDigit = false;
   while( uIndex < uEnd )
   {
      char iChar = stringText[uIndex];
      if( !( ( iChar >= '0' && iChar <= '9' ) || ( iChar >= 'a' && iChar <= 'f' ) || ( iChar >= 'A' && iChar <= 'F' ) ) )
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
bool is_binary( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim leading and trailing whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   size_t uIndex = uStart;

   // ## Check for optional sign
   if( stringText[uIndex] == '+' || stringText[uIndex] == '-' )
   {
      uIndex++;
      if( uIndex >= uEnd ) { return false; } // Sign without content
   }

   // ## Check for "0b" or "0B" prefix
   if( uIndex + 1 >= uEnd || stringText[uIndex] != '0' ) { return false; }
   uIndex++;

   if( stringText[uIndex] != 'b' && stringText[uIndex] != 'B' ) { return false; }
   uIndex++;

   if( uIndex >= uEnd ) { return false; } // No digits after prefix

   // ## Validate remaining characters are binary digits
   bool bFoundDigit = false;
   while( uIndex < uEnd )
   {
      char iChar = stringText[uIndex];
      if( iChar != '0' && iChar != '1' ) { return false; }
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
bool is_octal( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim leading and trailing whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   size_t uIndex = uStart;

   // ## Check for optional sign
   if( stringText[uIndex] == '+' || stringText[uIndex] == '-' )
   {
      uIndex++;
      if( uIndex >= uEnd ) { return false; } // Sign without content
   }

   // ## Must start with '0'
   if( stringText[uIndex] != '0' ) { return false; }

   // ## If only "0", it's valid
   if( uIndex + 1 >= uEnd ) { return true; }

   uIndex++;

   // ## Validate remaining characters are octal digits
   bool bFoundDigit = true; // We already found '0'
   while( uIndex < uEnd )
   {
      char iChar = stringText[uIndex];
      if( iChar < '0' || iChar > '7' ) { return false; }
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
bool is_alpha( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim leading and trailing whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   // ## Validate all characters are alphabetic
   bool bFoundAlpha = false;
   for( size_t uIndex = uStart; uIndex < uEnd; uIndex++ )
   {
      char iChar = stringText[uIndex];
      if( !( ( iChar >= 'a' && iChar <= 'z' ) || ( iChar >= 'A' && iChar <= 'Z' ) ) )
      {
         return false;
      }
      bFoundAlpha = true;
   }

   return bFoundAlpha;
}


/** ---------------------------------------------------------------------------
 * @brief Checks if the given string contains only alphanumeric characters.
 *
 * This function validates whether the string contains only alphanumeric characters
 * (a-z, A-Z, 0-9). It does not accept special characters or whitespace.
 * Leading and trailing whitespace is ignored, but internal whitespace will
 * cause the function to return false. Empty strings return false.
 *
 * @param stringText The string to validate as alphanumeric.
 * @return bool True if the string contains only alphanumeric characters, false otherwise.
 *
 * @code
 * bool result1 = is_alphanumeric("Hello123");     // true
 * bool result2 = is_alphanumeric("abc");          // true
 * bool result3 = is_alphanumeric("123");          // true
 * bool result4 = is_alphanumeric("Hello_World");  // false (contains underscore)
 * bool result5 = is_alphanumeric("");             // false
 * @endcode
 */
bool is_alphanumeric( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim leading and trailing whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   // ## Validate all characters are alphanumeric
   bool bFoundChar = false;
   for( size_t uIndex = uStart; uIndex < uEnd; uIndex++ )
   {
      char iChar = stringText[uIndex];
      if( !( ( iChar >= 'a' && iChar <= 'z' ) || ( iChar >= 'A' && iChar <= 'Z' ) || ( iChar >= '0' && iChar <= '9' ) ) )
      {
         return false;
      }
      bFoundChar = true;
   }

   return bFoundChar;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string contains only lowercase alphabetic characters.
 *
 * This function validates whether the string contains only lowercase alphabetic
 * characters (a-z). It does not accept uppercase letters, numbers, special
 * characters, or whitespace. Leading and trailing whitespace is ignored, but
 * internal whitespace will cause the function to return false. Empty strings
 * return false.
 *
 * @param stringText The string to validate as lowercase.
 * @return bool True if the string contains only lowercase characters, false otherwise.
 *
 * @code
 * bool result1 = is_lowercase("hello");      // true
 * bool result2 = is_lowercase("abc");        // true
 * bool result3 = is_lowercase("Hello");      // false (contains uppercase)
 * bool result4 = is_lowercase("hello123");   // false (contains numbers)
 * bool result5 = is_lowercase("");           // false
 * @endcode
 */
bool is_lowercase( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim leading and trailing whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   // ## Validate all characters are lowercase
   bool bFoundLower = false;
   for( size_t uIndex = uStart; uIndex < uEnd; uIndex++ )
   {
      char iChar = stringText[uIndex];
      if( iChar < 'a' || iChar > 'z' )
      {
         return false;
      }
      bFoundLower = true;
   }

   return bFoundLower;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string contains only uppercase alphabetic characters.
 *
 * This function validates whether the string contains only uppercase alphabetic
 * characters (A-Z). It does not accept lowercase letters, numbers, special
 * characters, or whitespace. Leading and trailing whitespace is ignored, but
 * internal whitespace will cause the function to return false. Empty strings
 * return false.
 *
 * @param stringText The string to validate as uppercase.
 * @return bool True if the string contains only uppercase characters, false otherwise.
 *
 * @code
 * bool result1 = is_uppercase("HELLO");      // true
 * bool result2 = is_uppercase("ABC");        // true
 * bool result3 = is_uppercase("Hello");      // false (contains lowercase)
 * bool result4 = is_uppercase("HELLO123");   // false (contains numbers)
 * bool result5 = is_uppercase("");           // false
 * @endcode
 */
bool is_uppercase( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim leading and trailing whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   // ## Validate all characters are uppercase
   bool bFoundUpper = false;
   for( size_t uIndex = uStart; uIndex < uEnd; uIndex++ )
   {
      char iChar = stringText[uIndex];
      if( iChar < 'A' || iChar > 'Z' )
      {
         return false;
      }
      bFoundUpper = true;
   }

   return bFoundUpper;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string contains only whitespace characters.
 *
 * This function validates whether the string contains only whitespace characters
 * (space, tab, newline, carriage return, form feed, vertical tab). Empty strings
 * return false as they contain no characters at all.
 *
 * @param stringText The string to validate as whitespace.
 * @return bool True if the string contains only whitespace characters, false otherwise.
 *
 * @code
 * bool result1 = is_whitespace("   ");       // true
 * bool result2 = is_whitespace("\t\n");      // true
 * bool result3 = is_whitespace(" a ");       // false (contains 'a')
 * bool result4 = is_whitespace("");          // false (empty)
 * bool result5 = is_whitespace("  \t  ");    // true
 * @endcode
 */
bool is_whitespace( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Validate all characters are whitespace
   for( size_t uIndex = 0; uIndex < stringText.length(); uIndex++ )
   {
      char iChar = stringText[uIndex];
      if( iChar != ' ' && iChar != '\t' && iChar != '\n' &&  iChar != '\r' && iChar != '\f' && iChar != '\v' )
      {
         return false;
      }
   }

   return true;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string contains only printable ASCII characters.
 *
 * This function validates whether the string contains only printable ASCII
 * characters (ASCII values 32-126, which includes space through tilde).
 * Control characters, extended ASCII, and non-ASCII characters will cause
 * the function to return false. Empty strings return false.
 *
 * @param stringText The string to validate as printable.
 * @return bool True if the string contains only printable characters, false otherwise.
 *
 * @code
 * bool result1 = is_printable("Hello World"); // true
 * bool result2 = is_printable("123!@#");      // true
 * bool result3 = is_printable("Hello\n");     // false (contains newline)
 * bool result4 = is_printable("Hello\t");     // false (contains tab)
 * bool result5 = is_printable("");            // false
 * @endcode
 */
bool is_printable( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Validate all characters are printable (ASCII 32-126)
   for( size_t uIndex = 0; uIndex < stringText.length(); uIndex++ )
   {
      unsigned char uChar = static_cast<unsigned char>( stringText[uIndex] );
      if( uChar < 32 || uChar > 126 )
      {
         return false;
      }
   }

   return true;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string contains only ASCII characters.
 *
 * This function validates whether the string contains only valid ASCII characters
 * (values 0-127). This includes both printable and control characters. Extended
 * ASCII (128-255) and multi-byte UTF-8 characters will cause the function to
 * return false. Empty strings return true as they contain no non-ASCII characters.
 *
 * @param stringText The string to validate as ASCII.
 * @return bool True if the string contains only ASCII characters, false otherwise.
 *
 * @code
 * bool result1 = is_ascii("Hello World");  // true
 * bool result2 = is_ascii("123!@#");       // true
 * bool result3 = is_ascii("Hello\n");      // true (newline is ASCII)
 * bool result4 = is_ascii("Café");         // false (é is not ASCII)
 * bool result5 = is_ascii("");             // true
 * @endcode
 */
bool is_ascii( std::string_view stringText ) noexcept
{
   // ## Empty string is valid ASCII
   if( stringText.empty() ) { return true; }

   // ## Validate all characters are ASCII (0-127)
   for( size_t uIndex = 0; uIndex < stringText.length(); uIndex++ )
   {
      unsigned char uChar = static_cast<unsigned char>( stringText[uIndex] );
      if( uChar > 127 )
      {
         return false;
      }
   }

   return true;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string is valid UTF-8 encoding.
 *
 * This function validates whether the string is properly encoded UTF-8. It checks
 * for valid byte sequences according to the UTF-8 specification. Single-byte
 * ASCII characters (0-127) are valid UTF-8. Multi-byte sequences must follow
 * the proper continuation byte pattern. Empty strings return true as they are
 * valid (empty) UTF-8.
 *
 * @param stringText The string to validate as UTF-8.
 * @return bool True if the string is valid UTF-8, false otherwise.
 *
 * @code
 * bool result1 = is_utf8("Hello World");   // true
 * bool result2 = is_utf8("Café");          // true
 * bool result3 = is_utf8("你好");           // true
 * bool result4 = is_utf8("\xFF\xFE");      // false (invalid UTF-8)
 * bool result5 = is_utf8("");              // true
 * @endcode
 */
bool is_utf8( std::string_view stringText ) noexcept
{
   // ## Empty string is valid UTF-8
   if( stringText.empty() ) { return true; }

   size_t uIndex = 0;
   size_t uLength = stringText.length();

   while( uIndex < uLength )
   {
      unsigned char uChar = static_cast<unsigned char>( stringText[uIndex] );

      // ## Single-byte character (0xxxxxxx)
      if( uChar <= 0x7F )
      {
         uIndex++;
         continue;
      }

      // ## Determine number of bytes in this character
      size_t uNumberOfBytes = 0;
      if( ( uChar & 0xE0 ) == 0xC0 ) { uNumberOfBytes = 2; } // 110xxxxx
      else if( ( uChar & 0xF0 ) == 0xE0 ) { uNumberOfBytes = 3; } // 1110xxxx
      else if( ( uChar & 0xF8 ) == 0xF0 ) { uNumberOfBytes = 4; } // 11110xxx
      else { return false; } // Invalid start byte

      // ## Check if we have enough bytes remaining
      if( uIndex + uNumberOfBytes > uLength ) { return false; }

      // ## Validate continuation bytes (10xxxxxx)
      for( size_t i = 1; i < uNumberOfBytes; i++ )
      {
         unsigned char uByte = static_cast<unsigned char>( stringText[uIndex + i] );
         if( ( uByte & 0xC0 ) != 0x80 )
         {
            return false;
         }
      }

      uIndex += uNumberOfBytes;
   }

   return true;
}


/** ---------------------------------------------------------------------------
 * @brief Checks if the given string represents a complex number.
 *
 * This function validates whether the string represents a complex number in the
 * form "a+bi" or "a-bi" where 'a' is the real part and 'b' is the imaginary part.
 * Both parts can be integers or decimals. The imaginary unit can be 'i' or 'j'.
 * Spaces are allowed around operators. Valid formats include: "3+4i", "5-2j",
 * "-3+4i", "3.5+2.1i", "4i" (pure imaginary), "5" (pure real).
 *
 * @param stringText The string to validate as a complex number.
 * @return bool True if the string represents a complex number, false otherwise.
 *
 * @code
 * bool result1 = is_complex("3+4i");      // true
 * bool result2 = is_complex("5-2j");      // true
 * bool result3 = is_complex("-3+4i");     // true
 * bool result4 = is_complex("4i");        // true
 * bool result5 = is_complex("3.5+2.1i");  // true
 * bool result6 = is_complex("abc");       // false
 * @endcode
 */
bool is_complex( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   // ## Check for imaginary unit (i or j) at the end
   bool bHasImaginary = false;
   if( stringText[uEnd - 1] == 'i' || stringText[uEnd - 1] == 'j' )
   {
      bHasImaginary = true;
      uEnd--;
      while( uEnd > uStart && stringText[uEnd - 1] == ' ' ) { uEnd--; }
   }

   // ## Find operator position (+ or -) that's not at the start
   int iOperatorPos = -1;
   for( size_t uIndex = uStart + 1; uIndex < uEnd; uIndex++ )
   {
      if( stringText[uIndex] == '+' || stringText[uIndex] == '-' )
      {
         iOperatorPos = static_cast<int>( uIndex );
         break;
      }
   }

   // ## If no operator found, check if it's a pure real or pure imaginary number
   if( iOperatorPos == -1 )
   {
      if( bHasImaginary )
      {
         // Pure imaginary: check if valid number before 'i'/'j'
         if( uEnd == uStart ) { return true; } // Just "i" or "j"

         for( size_t uIndex = uStart; uIndex < uEnd; uIndex++ )
         {
            char iChar = stringText[uIndex];
            if( iChar == '-' || iChar == '+' )
            {
               if( uIndex != uStart ) { return false; }
            }
            else if( iChar != '.' && ( iChar < '0' || iChar > '9' ) )
            {
               return false;
            }
         }
         return true;
      }
      else
      {
         // Pure real number
         bool bHasDot = false;
         for( size_t uIndex = uStart; uIndex < uEnd; uIndex++ )
         {
            char iChar = stringText[uIndex];
            if( iChar == '-' || iChar == '+' )
            {
               if( uIndex != uStart ) { return false; }
            }
            else if( iChar == '.' )
            {
               if( bHasDot ) { return false; }
               bHasDot = true;
            }
            else if( iChar < '0' || iChar > '9' )
            {
               return false;
            }
         }
         return true;
      }
   }

   // ## Validate real part (before operator)
   size_t uRealStart = uStart;
   size_t uRealEnd = iOperatorPos;
   while( uRealEnd > uRealStart && stringText[uRealEnd - 1] == ' ' ) { uRealEnd--; }

   if( uRealStart < uRealEnd )
   {
      bool bHasDot = false;
      for( size_t uIndex = uRealStart; uIndex < uRealEnd; uIndex++ )
      {
         char iChar = stringText[uIndex];
         if( iChar == '-' || iChar == '+' )
         {
            if( uIndex != uRealStart ) { return false; }
         }
         else if( iChar == '.' )
         {
            if( bHasDot ) { return false; }
            bHasDot = true;
         }
         else if( iChar < '0' || iChar > '9' )
         {
            return false;
         }
      }
   }

   // ## Validate imaginary part (after operator)
   size_t uImagStart = iOperatorPos + 1;
   while( uImagStart < uEnd && stringText[uImagStart] == ' ' ) { uImagStart++; }

   if( !bHasImaginary ) { return false; } // Must have 'i' or 'j' if there's an operator

   if( uImagStart < uEnd )
   {
      bool bHasDot = false;
      for( size_t uIndex = uImagStart; uIndex < uEnd; uIndex++ )
      {
         char iChar = stringText[uIndex];
         if( iChar == '.' )
         {
            if( bHasDot ) { return false; }
            bHasDot = true;
         }
         else if( iChar < '0' || iChar > '9' )
         {
            return false;
         }
      }
   }

   return true;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string represents a boolean value.
 *
 * This function validates whether the string represents a boolean value.
 * Accepted values are: "true", "false", "1", "0", "yes", "no", "on", "off"
 * (case-insensitive). Leading and trailing whitespace is ignored.
 *
 * @param stringText The string to validate as a boolean.
 * @return bool True if the string represents a boolean value, false otherwise.
 *
 * @code
 * bool result1 = is_boolean("true");   // true
 * bool result2 = is_boolean("FALSE");  // true
 * bool result3 = is_boolean("1");      // true
 * bool result4 = is_boolean("0");      // true
 * bool result5 = is_boolean("yes");    // true
 * bool result6 = is_boolean("no");     // true
 * bool result7 = is_boolean("on");     // true
 * bool result8 = is_boolean("off");    // true
 * bool result9 = is_boolean("maybe");  // false
 * @endcode
 */
bool is_boolean( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   // ## Create lowercase version for comparison
   std::string stringLower;
   stringLower.reserve( uEnd - uStart );
   for( size_t uIndex = uStart; uIndex < uEnd; uIndex++ )
   {
      char iChar = stringText[uIndex];
      if( iChar >= 'A' && iChar <= 'Z' )
      {
         stringLower += static_cast<char>( iChar + 32 );
      }
      else
      {
         stringLower += iChar;
      }
   }

   // ## Check against valid boolean strings
   return stringLower == "true" || stringLower == "false" ||
      stringLower == "1" || stringLower == "0" ||
      stringLower == "yes" || stringLower == "no" ||
      stringLower == "on" || stringLower == "off";
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string represents a valid number in the specified base.
 *
 * This function validates whether the string represents a valid number in the
 * given base (2-36). For bases 2-10, only digits 0-(base-1) are valid. For
 * bases 11-36, letters A-Z (case-insensitive) represent values 10-35.
 * Leading '+' or '-' signs are allowed. Leading/trailing whitespace is ignored.
 *
 * @param stringText The string to validate as a base-n number.
 * @param iBase The numeric base (2-36) to validate against.
 * @return bool True if the string is a valid base-n number, false otherwise.
 *
 * @code
 * bool result1 = is_base_n("1010", 2);     // true (binary)
 * bool result2 = is_base_n("777", 8);      // true (octal)
 * bool result3 = is_base_n("123", 10);     // true (decimal)
 * bool result4 = is_base_n("1A3F", 16);    // true (hexadecimal)
 * bool result5 = is_base_n("ZZZ", 36);     // true (base-36)
 * bool result6 = is_base_n("128", 8);      // false (8 not valid in octal)
 * @endcode
 */
bool is_base_n( const std::string_view& stringText, int iBase ) noexcept
{
   // ## Validate base range
   if( iBase < 2 || iBase > 36 ) { return false; }

   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Trim whitespace
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   while( uStart < uEnd && ( stringText[uStart] == ' ' || stringText[uStart] == '\t' ) ) { uStart++; }
   while( uEnd > uStart && ( stringText[uEnd - 1] == ' ' || stringText[uEnd - 1] == '\t' ) ) { uEnd--; }

   if( uStart >= uEnd ) { return false; }

   // ## Handle optional sign
   if( stringText[uStart] == '+' || stringText[uStart] == '-' )
   {
      uStart++;
      if( uStart >= uEnd ) { return false; }
   }

   // ## Validate each character
   bool bFoundDigit = false;
   for( size_t uIndex = uStart; uIndex < uEnd; uIndex++ )
   {
      char iChar = stringText[uIndex];
      int iDigitValue = -1;

      // Convert character to digit value
      if( iChar >= '0' && iChar <= '9' )
      {
         iDigitValue = iChar - '0';
      }
      else if( iChar >= 'A' && iChar <= 'Z' )
      {
         iDigitValue = 10 + ( iChar - 'A' );
      }
      else if( iChar >= 'a' && iChar <= 'z' )
      {
         iDigitValue = 10 + ( iChar - 'a' );
      }

      // Check if digit is valid for this base
      if( iDigitValue < 0 || iDigitValue >= iBase )
      {
         return false;
      }

      bFoundDigit = true;
   }

   return bFoundDigit;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given string contains mathematical operators.
 *
 * This function validates whether the string contains any mathematical operators
 * (+, -, *, /, ^, %). This is a simple check for the presence of operators,
 * not a validation of mathematical expression correctness. The function returns
 * true if at least one operator is found.
 *
 * @param stringText The string to check for mathematical operators.
 * @return bool True if the string contains operators, false otherwise.
 *
 * @code
 * bool result1 = is_expression("3+4");      // true
 * bool result2 = is_expression("x*y");      // true
 * bool result3 = is_expression("2^3");      // true
 * bool result4 = is_expression("123");      // false
 * bool result5 = is_expression("a-b+c");    // true
 * @endcode
 */
bool is_expression( std::string_view stringText ) noexcept
{
   // ## Handle empty string
   if( stringText.empty() ) { return false; }

   // ## Check for mathematical operators
   for( size_t uIndex = 0; uIndex < stringText.length(); uIndex++ )
   {
      char iChar = stringText[uIndex];

      // Skip if it's a sign at the beginning
      if( ( iChar == '+' || iChar == '-' ) && uIndex == 0 )
      {
         continue;
      }

      // Check for operators
      if( iChar == '+' || iChar == '-' || iChar == '*' ||
          iChar == '/' || iChar == '^' || iChar == '%' )
      {
         return true;
      }
   }

   return false;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the parentheses in the string are balanced.
 *
 * This function validates whether all opening parentheses '(', brackets '[',
 * and braces '{' have corresponding closing counterparts ')', ']', '}' in
 * the correct order. The function checks that each closing bracket matches
 * the most recent unmatched opening bracket.
 *
 * @param stringText The string to check for balanced parentheses.
 * @return bool True if all parentheses are balanced, false otherwise.
 *
 * @code
 * bool result1 = is_balanced_parentheses("(a+b)");        // true
 * bool result2 = is_balanced_parentheses("[(a+b)*c]");    // true
 * bool result3 = is_balanced_parentheses("{[()]}");       // true
 * bool result4 = is_balanced_parentheses("(a+b");         // false
 * bool result5 = is_balanced_parentheses("(a+b]");        // false
 * bool result6 = is_balanced_parentheses("((a+b)");       // false
 * @endcode
 */
bool is_balanced_parentheses( std::string_view stringText ) noexcept
{
   // ## Empty string is balanced
   if( stringText.empty() ) { return true; }

   // ## Use a simple stack approach with fixed size buffer
   char stackBuffer[256];
   size_t uStackTop = 0;

   for( size_t uIndex = 0; uIndex < stringText.length(); uIndex++ )
   {
      char iChar = stringText[uIndex];

      // ## Push opening brackets onto stack
      if( iChar == '(' || iChar == '[' || iChar == '{' )
      {
         if( uStackTop >= 256 ) { return false; } // Stack overflow
         stackBuffer[uStackTop++] = iChar;
      }
      // ## Check closing brackets against stack
      else if( iChar == ')' || iChar == ']' || iChar == '}' )
      {
         if( uStackTop == 0 ) { return false; } // No matching opening bracket

         char iOpening = stackBuffer[--uStackTop];

         // Verify correct bracket type
         if( ( iChar == ')' && iOpening != '(' ) ||
             ( iChar == ']' && iOpening != '[' ) ||
             ( iChar == '}' && iOpening != '{' ) )
         {
            return false;
         }
      }
   }

   // ## All brackets should be matched (stack empty)
   return uStackTop == 0;
}

_GD_MATH_TYPE_END