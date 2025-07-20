/**
* \brief Code rules on how to parse key-value formatted text
*
* The `code` struct defines parsing rules for key-value formatted text in the format:
* [key1: `this is text in backtics`] [key2: "quoted text"] [key3: unquoted text]
* 
* It allows customization of delimiters, quote characters, and parsing behavior.
*
* ## Usage
* - Configure delimiters and quote characters via constructors.
* - Use `next_pair` to move to the next key-value pair.
* - Use with parsing functions to extract key-value pairs from formatted text.
*
* ## Members
* - `m_uOpenBracket`: Opening bracket character (default: '[').
* - `m_uCloseBracket`: Closing bracket character (default: ']').
* - `m_uKeySeparator`: Key-value separator (default: ':').
* - `m_uQuotes`: Supported quote characters (default: "\"'`").
* - `m_uOptions`: Option flags for parsing behavior.
*
* ## Methods
* - `get_open_bracket()`: Returns the opening bracket character.
* - `get_close_bracket()`: Returns the closing bracket character.
* - `get_key_separator()`: Returns the key-value separator character.
* - `is_quote(uint8_t)`: Checks if the given character is a supported quote.
* - `next_pair(const uint8_t*, const uint8_t*)`: Returns pointer to next key-value pair.
* - `extract_key(const uint8_t*, const uint8_t*, std::string&)`: Extracts key from current position.
* - `extract_value(const uint8_t*, const uint8_t*, std::string&)`: Extracts value from current position.
*
* ## Example
* @code
* gd::parse::code codeRules; // Use default brackets and quotes
* std::string key, value;
* const char* pos = text.data();
* while ((pos = codeRules.next_pair(pos, text.data() + text.size())) != nullptr) {
*     pos = codeRules.extract_key(pos, text.data() + text.size(), key);
*     pos = codeRules.extract_value(pos, text.data() + text.size(), value);
*     // Process key-value pair
* }
* @endcode
*/
struct code
{
   enum enumOptions
   {
      eTrimWhitespace = 1 << 0,     ///< Trim whitespace from keys and values
      eAllowUnquoted = 1 << 1,      ///< Allow unquoted values
      eStrictQuoting = 1 << 2,      ///< Require matching quote types
      eSkipEmpty = 1 << 3,          ///< Skip empty key-value pairs
      eOptionsMAX = 1 << 4,
   };

   // ## construction -------------------------------------------------------------
   code() : 
      m_uOpenBracket('['), 
      m_uCloseBracket(']'), 
      m_uKeySeparator(':'), 
      m_uQuotes("\"'`"),
      m_uOptions(eTrimWhitespace | eAllowUnquoted) 
   {}

   code(uint8_t uOpenBracket, uint8_t uCloseBracket) : 
      m_uOpenBracket(uOpenBracket), 
      m_uCloseBracket(uCloseBracket), 
      m_uKeySeparator(':'), 
      m_uQuotes("\"'`"),
      m_uOptions(eTrimWhitespace | eAllowUnquoted) 
   {}

   code(uint8_t uOpenBracket, uint8_t uCloseBracket, uint8_t uKeySeparator) : 
      m_uOpenBracket(uOpenBracket), 
      m_uCloseBracket(uCloseBracket), 
      m_uKeySeparator(uKeySeparator), 
      m_uQuotes("\"'`"),
      m_uOptions(eTrimWhitespace | eAllowUnquoted) 
   {}

   code(const std::string_view& stringQuotes) : 
      m_uOpenBracket('['), 
      m_uCloseBracket(']'), 
      m_uKeySeparator(':'), 
      m_uQuotes(stringQuotes),
      m_uOptions(eTrimWhitespace | eAllowUnquoted) 
   {}

   ~code() {}

   // ## accessors ----------------------------------------------------------------
   uint8_t get_open_bracket() const { return m_uOpenBracket; }
   uint8_t get_close_bracket() const { return m_uCloseBracket; }
   uint8_t get_key_separator() const { return m_uKeySeparator; }
   const std::string_view& get_quotes() const { return m_uQuotes; }
   bool has_option(enumOptions option) const { return (m_uOptions & option) != 0; }

   bool is_quote(uint8_t uChar) const { 
      return m_uQuotes.find(uChar) != std::string_view::npos; 
   }

   bool is_whitespace(uint8_t uChar) const {
      return uChar == ' ' || uChar == '\t' || uChar == '\n' || uChar == '\r';
   }

   // ## parsing methods ----------------------------------------------------------

   /// Skip whitespace characters
   const uint8_t* skip_whitespace(const uint8_t* puPosition, const uint8_t* puEnd) const {
      while (puPosition < puEnd && is_whitespace(*puPosition)) {
         ++puPosition;
      }
      return puPosition;
   }

   /// Skip quoted section, returning pointer past closing quote
   const uint8_t* skip_quoted(const uint8_t* puPosition, const uint8_t* puEnd) const {
      if (puPosition >= puEnd || !is_quote(*puPosition)) {
         return puPosition;
      }

      const uint8_t uQuote = *puPosition;
      ++puPosition; // Skip opening quote

      while (puPosition < puEnd) {
         if (*puPosition == uQuote) {
            // Check for escaped quote (double quote)
            if (puPosition + 1 < puEnd && *(puPosition + 1) == uQuote) {
               puPosition += 2; // Skip both quotes
            } else {
               return puPosition + 1; // Return pointer past closing quote
            }
         } else {
            ++puPosition;
         }
      }

      return puPosition; // Unterminated quote
   }

   /// Find next occurrence of character, skipping quoted sections
   const uint8_t* find_char_skip_quotes(const uint8_t* puPosition, const uint8_t* puEnd, uint8_t uTarget) const {
      while (puPosition < puEnd) {
         if (*puPosition == uTarget) {
            return puPosition;
         } else if (is_quote(*puPosition)) {
            puPosition = skip_quoted(puPosition, puEnd);
         } else {
            ++puPosition;
         }
      }
      return nullptr;
   }

   /// Find the next key-value pair (opening bracket)
   const uint8_t* next_pair(const uint8_t* puPosition, const uint8_t* puEnd) const {
      while (puPosition < puEnd) {
         if (*puPosition == m_uOpenBracket) {
            return puPosition + 1; // Return position after opening bracket
         }
         ++puPosition;
      }
      return nullptr;
   }

   /// Extract key from current position (after opening bracket, before separator)
   const uint8_t* extract_key(const uint8_t* puPosition, const uint8_t* puEnd, std::string& key) const {
      key.clear();

      if (has_option(eTrimWhitespace)) {
         puPosition = skip_whitespace(puPosition, puEnd);
      }

      const uint8_t* puKeyStart = puPosition;

      // Find key separator, skipping quoted sections
      const uint8_t* puSeparator = find_char_skip_quotes(puPosition, puEnd, m_uKeySeparator);
      if (!puSeparator) return nullptr;

      const uint8_t* puKeyEnd = puSeparator;

      // Trim trailing whitespace from key if option is set
      if (has_option(eTrimWhitespace)) {
         while (puKeyEnd > puKeyStart && is_whitespace(*(puKeyEnd - 1))) {
            --puKeyEnd;
         }
      }

      key.assign(reinterpret_cast<const char*>(puKeyStart), puKeyEnd - puKeyStart);
      return puSeparator + 1; // Return position after separator
   }

   /// Extract value from current position (after separator, before closing bracket)
   const uint8_t* extract_value(const uint8_t* puPosition, const uint8_t* puEnd, std::string& value) const {
      value.clear();

      if (has_option(eTrimWhitespace)) {
         puPosition = skip_whitespace(puPosition, puEnd);
      }

      // Find closing bracket for this pair
      const uint8_t* puCloseBracket = find_char_skip_quotes(puPosition, puEnd, m_uCloseBracket);
      if (!puCloseBracket) return nullptr;

      if (puPosition < puEnd && is_quote(*puPosition)) {
         // Quoted value
         const uint8_t uQuote = *puPosition;
         const uint8_t* puValueStart = puPosition + 1;
         const uint8_t* puQuoteEnd = skip_quoted(puPosition, puEnd);
         const uint8_t* puValueEnd = (puQuoteEnd > puPosition + 1) ? puQuoteEnd - 1 : puQuoteEnd;

         // Handle escaped quotes in the extracted value
         for (const uint8_t* p = puValueStart; p < puValueEnd; ++p) {
            if (*p == uQuote && p + 1 < puValueEnd && *(p + 1) == uQuote) {
               value += static_cast<char>(uQuote);
               ++p; // Skip the second quote
            } else {
               value += static_cast<char>(*p);
            }
         }

         return puCloseBracket + 1; // Return position after closing bracket
      } else if (has_option(eAllowUnquoted)) {
         // Unquoted value
         const uint8_t* puValueStart = puPosition;
         const uint8_t* puValueEnd = puCloseBracket;

         // Trim trailing whitespace if option is set
         if (has_option(eTrimWhitespace)) {
            while (puValueEnd > puValueStart && is_whitespace(*(puValueEnd - 1))) {
               --puValueEnd;
            }
         }

         value.assign(reinterpret_cast<const char*>(puValueStart), puValueEnd - puValueStart);
         return puCloseBracket + 1; // Return position after closing bracket
      }

      return nullptr; // No valid value found
   }

   // Convenience methods for char pointers
   const char* next_pair(const char* pbszPosition, const char* pbszEnd) const {
      return reinterpret_cast<const char*>(next_pair(
         reinterpret_cast<const uint8_t*>(pbszPosition),
         reinterpret_cast<const uint8_t*>(pbszEnd)
      ));
   }

   const char* extract_key(const char* pbszPosition, const char* pbszEnd, std::string& key) const {
      return reinterpret_cast<const char*>(extract_key(
         reinterpret_cast<const uint8_t*>(pbszPosition),
         reinterpret_cast<const uint8_t*>(pbszEnd),
         key
      ));
   }

   const char* extract_value(const char* pbszPosition, const char* pbszEnd, std::string& value) const {
      return reinterpret_cast<const char*>(extract_value(
         reinterpret_cast<const uint8_t*>(pbszPosition),
         reinterpret_cast<const uint8_t*>(pbszEnd),
         value
      ));
   }

   // ## attributes ---------------------------------------------------------------
   unsigned m_uOptions;           ///< flag options for parsing behavior
   uint8_t m_uOpenBracket;        ///< opening bracket character
   uint8_t m_uCloseBracket;       ///< closing bracket character
   uint8_t m_uKeySeparator;       ///< key-value separator character
   std::string_view m_uQuotes;    ///< supported quote characters
};

// ## Usage example and helper functions ------------------------------------------

/// High-level parser class using the code rules
class CodeParser {
private:
   code m_rules;
   std::string m_text;

public:
   explicit CodeParser(const std::string& text, const code& rules = code{}) 
      : m_rules(rules), m_text(text) {}

   /// Parse all key-value pairs into a map
   std::unordered_map<std::string, std::string> parse_all() const {
      std::unordered_map<std::string, std::string> result;

      const char* pos = m_text.data();
      const char* end = m_text.data() + m_text.size();
      std::string key, value;

      while ((pos = m_rules.next_pair(pos, end)) != nullptr) {
         const char* after_key = m_rules.extract_key(pos, end, key);
         if (!after_key || key.empty()) continue;

         const char* after_value = m_rules.extract_value(after_key, end, value);
         if (!after_value) continue;

         if (!m_rules.has_option(code::eSkipEmpty) || !key.empty()) {
            result[key] = value;
         }

         pos = after_value;
      }

      return result;
   }

   /// Get value for specific key
   std::optional<std::string> get_value(const std::string& target_key) const {
      const char* pos = m_text.data();
      const char* end = m_text.data() + m_text.size();
      std::string key, value;

      while ((pos = m_rules.next_pair(pos, end)) != nullptr) {
         const char* after_key = m_rules.extract_key(pos, end, key);
         if (!after_key || key != target_key) {
            // Skip to next pair
            pos = m_rules.find_char_skip_quotes(
               reinterpret_cast<const uint8_t*>(pos),
               reinterpret_cast<const uint8_t*>(end),
               m_rules.get_close_bracket()
            );
            if (pos) pos = reinterpret_cast<const char*>(pos) + 1;
            continue;
         }

         const char* after_value = m_rules.extract_value(after_key, end, value);
         if (after_value) {
            return value;
         }
      }

      return std::nullopt;
   }
};

// ## Test function ------------------------------------------------------------
#include <iostream>

void test_code_parser() {
   std::string input = R"([key1: `this is text in backticks`] [key2: "quoted text with ""escaped"" quotes"] [key3: unquoted text] [empty: ])";

   // Test with default rules
   code defaultRules;
   CodeParser parser(input, defaultRules);

   std::cout << "=== Testing Code Parser ===\n";

   // Test individual key access
   if (auto val1 = parser.get_value("key1")) {
      std::cout << "key1: '" << *val1 << "'\n";
   }

   if (auto val2 = parser.get_value("key2")) {
      std::cout << "key2: '" << *val2 << "'\n";
   }

   if (auto val3 = parser.get_value("key3")) {
      std::cout << "key3: '" << *val3 << "'\n";
   }

   if (auto empty = parser.get_value("empty")) {
      std::cout << "empty: '" << *empty << "'\n";
   }

   // Test parsing all pairs
   std::cout << "\nAll pairs:\n";
   auto all_pairs = parser.parse_all();
   for (const auto& [key, value] : all_pairs) {
      std::cout << "  " << key << " = '" << value << "'\n";
   }

   // Test with custom rules (different brackets)
   std::cout << "\n=== Testing Custom Rules ===\n";
   code customRules('{', '}', '=');
   std::string customInput = "{name=`John Doe`} {age=30} {city=\"New York\"}";
   CodeParser customParser(customInput, customRules);

   auto custom_pairs = customParser.parse_all();
   for (const auto& [key, value] : custom_pairs) {
      std::cout << "  " << key << " = '" << value << "'\n";
   }
}



/** ---------------------------------------------------------------------------
* @brief find character similar to c-method `strchr` except here we are using code parsing rules
* @param pbszText text to search within
* @param chFind character to find
* @param code code object with rules on how to move in text (handles [], quotes, etc.)
* @param puCharacterClass ascii text block (256 bytes) with character classes or null to use default
* @return pointer to found character or nullptr if not found
*/
const char* strchr(const char* pbszText, char chFind, const code& codeRules, const uint8_t* puCharacterClass)
{
   if (puCharacterClass == nullptr) puCharacterClass = pCharacterClass_s;
   const char* pbszPosition = pbszText;

   while (*pbszPosition != '\0' && *pbszPosition != chFind)
   {
      if (!(puCharacterClass[*pbszPosition] & ASCII_TYPE_QUOTE))
      {
         pbszPosition++;
         continue;
      }
      else
      {
         if (codeRules.is_quote(*pbszPosition) == true)
         {
            // ## found quote, text within quote is skipped using code rules
            pbszPosition = reinterpret_cast<const char*>(
               codeRules.skip_quoted(
                  reinterpret_cast<const uint8_t*>(pbszPosition),
                  reinterpret_cast<const uint8_t*>(pbszPosition + strlen(pbszPosition))
               )
               );
         }
         else
         {
            pbszPosition++;
            continue;
         }
      }
   }

   if (*pbszPosition == chFind) return pbszPosition;
   return nullptr;
}

/** ---------------------------------------------------------------------------
* @brief Enhanced version that also handles code-specific delimiters (brackets, separators)
* @param pbszText text to search within
* @param chFind character to find
* @param code code object with rules on how to move in text
* @param puCharacterClass ascii text block (256 bytes) with character classes or null to use default
* @return pointer to found character or nullptr if not found
*/
const char* strchr_code(const char* pbszText, char chFind, const code& codeRules, const uint8_t* puCharacterClass)
{
   if (puCharacterClass == nullptr) puCharacterClass = pCharacterClass_s;
   const char* pbszPosition = pbszText;
   const char* pbszEnd = pbszPosition + strlen(pbszPosition);

   while (*pbszPosition != '\0' && *pbszPosition != chFind)
   {
      // Handle quotes using code rules
      if (codeRules.is_quote(*pbszPosition))
      {
         // Skip quoted section using code rules (handles escaping)
         pbszPosition = reinterpret_cast<const char*>(
            codeRules.skip_quoted(
               reinterpret_cast<const uint8_t*>(pbszPosition),
               reinterpret_cast<const uint8_t*>(pbszEnd)
            )
            );
      }
      // Handle brackets - skip entire bracketed sections if looking for something outside
      else if (*pbszPosition == codeRules.get_open_bracket() && chFind != codeRules.get_open_bracket())
      {
         // Skip to matching close bracket
         pbszPosition = reinterpret_cast<const char*>(
            codeRules.find_char_skip_quotes(
               reinterpret_cast<const uint8_t*>(pbszPosition + 1),
               reinterpret_cast<const uint8_t*>(pbszEnd),
               codeRules.get_close_bracket()
            )
            );
         if (pbszPosition) pbszPosition++; // Move past close bracket
         else break; // Unmatched bracket
      }
      else
      {
         pbszPosition++;
      }
   }

   if (*pbszPosition == chFind) return pbszPosition;
   return nullptr;
}

/** ---------------------------------------------------------------------------
* @brief Find character within a specific key-value pair context
* @param pbszText text to search within (should start inside a [key:value] pair)
* @param chFind character to find
* @param code code object with parsing rules
* @param bSearchInValue true to search in value part, false to search in key part only
* @return pointer to found character or nullptr if not found
*/
const char* strchr_keyvalue(const char* pbszText, char chFind, const code& codeRules, bool bSearchInValue = true)
{
   const char* pbszPosition = pbszText;
   const char* pbszEnd = pbszPosition + strlen(pbszPosition);

   // Find the key separator first
   const char* pbszSeparator = reinterpret_cast<const char*>(
      codeRules.find_char_skip_quotes(
         reinterpret_cast<const uint8_t*>(pbszPosition),
         reinterpret_cast<const uint8_t*>(pbszEnd),
         codeRules.get_key_separator()
      )
      );

   // Determine search boundaries
   const char* pbszSearchStart = pbszPosition;
   const char* pbszSearchEnd;

   if (!bSearchInValue) {
      // Search only in key part
      pbszSearchEnd = pbszSeparator ? pbszSeparator : pbszEnd;
   } else {
      // Search in value part (after separator)
      if (pbszSeparator) {
         pbszSearchStart = pbszSeparator + 1;
         // Find closing bracket
         pbszSearchEnd = reinterpret_cast<const char*>(
            codeRules.find_char_skip_quotes(
               reinterpret_cast<const uint8_t*>(pbszSearchStart),
               reinterpret_cast<const uint8_t*>(pbszEnd),
               codeRules.get_close_bracket()
            )
            );
         if (!pbszSearchEnd) pbszSearchEnd = pbszEnd;
      } else {
         return nullptr; // No separator found
      }
   }

   // Search within the determined boundaries
   pbszPosition = pbszSearchStart;
   while (pbszPosition < pbszSearchEnd && *pbszPosition != '\0' && *pbszPosition != chFind)
   {
      if (codeRules.is_quote(*pbszPosition))
      {
         pbszPosition = reinterpret_cast<const char*>(
            codeRules.skip_quoted(
               reinterpret_cast<const uint8_t*>(pbszPosition),
               reinterpret_cast<const uint8_t*>(pbszSearchEnd)
            )
            );
      }
      else
      {
         pbszPosition++;
      }
   }

   if (pbszPosition < pbszSearchEnd && *pbszPosition == chFind) return pbszPosition;
   return nullptr;
}

// ## Usage examples -----------------------------------------------------------

void demonstrate_strchr_with_code()
{
   // Test data
   std::string text = R"([key1: `this contains a : colon`] [key2: "another : here"] [key3: normal])";
   code codeRules; // Default rules

   std::cout << "=== Testing strchr with code rules ===\n";
   std::cout << "Text: " << text << "\n\n";

   // Find colons - should skip those inside quotes
   const char* pos = text.c_str();
   int colonCount = 0;

   while ((pos = strchr_code(pos, ':', codeRules, nullptr)) != nullptr) {
      colonCount++;
      std::cout << "Found colon #" << colonCount << " at position: " << (pos - text.c_str()) << "\n";
      std::cout << "Context: \"" << std::string(std::max(pos - 10, text.c_str()), 
         std::min(pos + 10, text.c_str() + text.length())) << "\"\n";
      pos++; // Move past this colon
   }

   std::cout << "\nTotal colons found (outside quotes): " << colonCount << "\n";

   // Test finding brackets
   std::cout << "\n=== Finding opening brackets ===\n";
   pos = text.c_str();
   int bracketCount = 0;

   while ((pos = strchr_code(pos, '[', codeRules, nullptr)) != nullptr) {
      bracketCount++;
      std::cout << "Found bracket #" << bracketCount << " at position: " << (pos - text.c_str()) << "\n";
      pos++; // Move past this bracket
   }

   // Test searching within specific key-value context
   std::cout << "\n=== Searching within key-value pairs ===\n";
   const char* pairStart = text.c_str();

   while ((pairStart = strchr_code(pairStart, '[', codeRules, nullptr)) != nullptr) {
      pairStart++; // Move past '['

      // Search for colon in key part only
      const char* keyColon = strchr_keyvalue(pairStart, ':', codeRules, false);
      if (keyColon) {
         std::cout << "Found colon in key part at position: " << (keyColon - text.c_str()) << "\n";
      }

      // Search for colon in value part
      const char* valueColon = strchr_keyvalue(pairStart, ':', codeRules, true);
      if (valueColon) {
         std::cout << "Found colon in value part at position: " << (valueColon - text.c_str()) << "\n";
      }

      // Move to next pair
      const char* closeBracket = strchr_code(pairStart, ']', codeRules, nullptr);
      if (closeBracket) {
         pairStart = closeBracket + 1;
      } else {
         break;
      }
   }
}