/*
## Overview
| Name | Description |
| - | - |
| character | get one character from buffer as uint32_t |
| convert* | convert character into bytes and place it in specified buffer |
| count | count utf8 characters in buffer |
| find | finds sequence within text |
| next* | moves text pointer to new position |
| previous | moves text pointer to new position |
| normalize | convert to normal type from utf8 |
| print | "prints" utf8 characters into some string object |
| size | size needed in bytes to store characters |
| split | split text into multiple text parts |
| trim | remove space characters from start and end |
| is | is methods checks if text is something or not |

*/

#pragma once

#include <cassert>
#include <stdint.h>
#include <cstring>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>
#include <type_traits>


#ifndef _GD_UTF8_BEGIN
#  define _GD_UTF8_BEGIN namespace gd { namespace utf8 {
#  define _GD_UTF8_END } }
#endif



namespace gd { 
   namespace utf8 {
      // tag dispatcher for utf8 if no support for char8_t
      struct tag_utf8{};
      /// tag dispatcher for wildcard matching
      struct tag_wildcard{};
      /// tag dispatcher when there is a find operations
      struct tag_find{};
      /// tag dispatcher for analyze operations
      struct tag_analyze {};

      /// tag dispatcher for stl string
      struct tag_string{};
      /// tag dispatcher for stl string_view
      struct tag_string_view{};

      constexpr uint8_t UTF8_MAX_ASCII = 0x7F;
      constexpr uint8_t UTF8_MIN_ENCODE = 0x80;
      constexpr uint8_t UTF8_VALIDATE_TAIL_MASK = 0xC0;
      constexpr uint32_t SIZE8_MAX_UTF_SIZE = 2;
      constexpr uint32_t SIZE16_MAX_UTF_SIZE = 3;
      constexpr uint32_t SIZE32_MAX_UTF_SIZE = 6;


      ///@{ 
      void normalize( uint32_t uCharacter, char& value );
      uint32_t character(const uint8_t* pubszText); // -------------------------------------------- character
      template <typename UTF8_TYPE>
      uint32_t character(const UTF8_TYPE* pbszText) { // ------------------------------------------ character
         static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
         return character(reinterpret_cast<const uint8_t*>(pbszText));
      };
      ///@}

      // ## is* methods

      /// check if size value is any `npos` value returned from string search methods that returns position or `npos` if not found
      template<typename POSITION>
      bool is_npos(const POSITION uPosition) {
         if constexpr( sizeof(POSITION) == sizeof(uint32_t) ) return static_cast<int32_t>(uPosition) < 0;
         else if constexpr( sizeof(POSITION) == sizeof(uint64_t) ) return static_cast<int64_t>(uPosition) < 0;
         //else static_assert(false, "invalid size type compare to npos");
         assert(false);
         return false;
      }

      template<typename POSITION>
      bool is_found(const POSITION uPosition) {
         return !is_npos(uPosition);
      }

      // ## `count` methods is used to count number of characters (`strlen` are wrappers)

      /**
       * Count number of characters in utf8 buffer
       */
      ///@{ 
      std::pair<uint32_t, const uint8_t*> count(const uint8_t* pubszText);                         // count utf8 characters
      inline std::pair<uint32_t, const uint8_t*> count(const std::string_view stringText) { 
         if( stringText.empty() == false ) return count( reinterpret_cast<const uint8_t*>(stringText.data()) );
         return { 0, nullptr };
      };
      template <typename UTF8_TYPE>
      std::pair<uint32_t, const uint8_t*> count(const UTF8_TYPE* pbszText) {                       // count utf8 characters
         static_assert( sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
         return count( reinterpret_cast<const uint8_t*>(pbszText) ); 
      };
      std::pair<uint32_t, const uint8_t*> count( const uint8_t* pubszText, const uint8_t* pubszEnd );
      template <typename UTF8_TYPE>
      std::pair<uint32_t, const uint8_t*> count(const UTF8_TYPE* pbszText, const UTF8_TYPE* pbszEnd) { // count utf8 characters
         static_assert( sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
         return count( reinterpret_cast<const uint8_t*>(pbszText), reinterpret_cast<const uint8_t*>(pbszEnd) ); 
      };
      inline uint32_t strlen(const uint8_t* pubszText) { return count(pubszText).first; };         // count utf8 characters
      inline uint32_t strlen(const std::string_view stringText) { return count( reinterpret_cast<const uint8_t*>(stringText.data()) ).first; };
      template <typename UTF8_TYPE>
      uint32_t strlen(const UTF8_TYPE* pbszText) {                                                 // count utf8 characters
         static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
         return count(reinterpret_cast<const uint8_t*>(pbszText)).first;
      };

      /// ## `size` methods is used calculate needed size to store character as utf8 value

      uint32_t size( uint8_t ch ); // ------------------------------------------------------------- size
      uint32_t size( uint16_t ch ); // ------------------------------------------------------------ size
      uint32_t size( uint32_t ch ); // ------------------------------------------------------------ size
      uint32_t size( wchar_t ch ); // ------------------------------------------------------------- size
      /// count needed size to store char string as utf8 string
      inline uint32_t size( const char* pbsz ) { // ----------------------------------------------- size
         uint32_t uSize = 0;
         for( ; *pbsz != 0; pbsz++ ) uSize += size( static_cast<uint8_t>(*pbsz) );
         return uSize;
      }
      inline uint32_t size(const std::string_view& stringCountSize) { // -------------------------- size
         uint32_t uSize = 0;
         for( auto it : stringCountSize  ) uSize += size(static_cast<uint8_t>(it));
         return uSize;
      }
      inline uint32_t size(const wchar_t* pwsz) { // ---------------------------------------------- size
         uint32_t uSize = 0;
         for( ; *pwsz != 0; pwsz++ ) uSize += size(static_cast<wchar_t>(*pwsz));
         return uSize;
      }
      inline uint32_t size(const wchar_t* pwsz, const wchar_t* pwszEnd) { // ---------------------- size
         uint32_t uSize = 0;
         for( ; pwsz != pwszEnd; pwsz++ ) uSize += size(static_cast<wchar_t>(*pwsz));
         return uSize;
      }

      /// count needed size to store list of char values as utf8 string
      inline uint32_t size( std::initializer_list<char> listString ) { // ------------------------- size
         uint32_t uSize = 0;
         for( auto it : listString ) uSize += size( static_cast<uint8_t>(it) );
         return uSize;
      }

      ///@}

      ///@{ 
      /// Count number of utf8 bytes needed to store char text
      uint32_t size( const char* pbszText, uint32_t uLength ); // --------------------------------- size
      ///@}

      /// calculate size in bytes needed to store character
      uint32_t get_character_size( const uint8_t* pubszText ); // --------------------------------- get_character_size

      /// validate utf8 text, returns true if valid utf8 characters, false if not
      std::pair<bool, const uint8_t*> validate( const uint8_t* pubBegin, const uint8_t* pubEnd );
      inline std::pair<bool, const uint8_t*> validate( const std::string_view& stringText ) { return validate( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length() ); }

      std::pair<bool, const uint8_t*> validate_hex( const uint8_t* pubBegin, const uint8_t* pubEnd );
      inline std::pair<bool, const uint8_t*> validate_hex( const std::string_view& stringText ) { return validate_hex( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length() ); }


      // ## `convert*` methods
      // A lot of convert methods and what the mostly do is to convert from something
      // to utf8. If there is only one name of a string format than that format is
      // converted to utf8.
      // There are also some methods that convert between two different formats
      // that is not utf8. Then both format names are in the method name.
      // Same when text is converted from utf8 to ano format, then 
      // both names are in method name.
      // sample: 
      // - `convert_ascii` converts from ascii to utf8
      // - `convert_utf8_to_ascii` converts from utf8 to ascii

      ///@{
      uint32_t convert(uint8_t uCharacter, uint8_t* pbszTo); // ----------------------------------- convert
      uint32_t convert(uint16_t uCharacter, uint8_t* pbszTo); // ---------------------------------- convert
      uint32_t convert(uint32_t uCharacter, uint8_t* pbszTo); // ---------------------------------- convert
      uint32_t convert(uint32_t uCharacter, std::string& stringTo ); // --------------------------- convert

#if defined(__cpp_char8_t)
      inline uint32_t convert(uint32_t uCharacter, char8_t* pbszTo) { return convert(uCharacter, reinterpret_cast<uint8_t*>(pbszTo)); }

      /// convert from utf16 to utf8. make sure that buffer is large enough to hold text
      std::tuple<bool, const char16_t*, char8_t*> convert_utf16_to_uft8(const char16_t* pwszUtf16, char8_t* pbszUtf8 );// convert
      template <typename UTF16_TYPE, typename UTF8_TYPE>
      std::tuple<bool, const UTF16_TYPE*, UTF8_TYPE*> convert_utf16_to_uft8(const UTF16_TYPE* pwszUtf16, UTF8_TYPE* pbszUtf8) {
         static_assert(sizeof(UTF16_TYPE) == 2, "Value isn't compatible with char16_t");
         static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with char8_t");
         auto _result = convert_utf16_to_uft8( reinterpret_cast<const char16_t*>(pwszUtf16), reinterpret_cast<char8_t*>(pbszUtf8) );
         return std::tuple<bool, const UTF16_TYPE*, UTF8_TYPE*>(
            std::get<0>(_result), 
            reinterpret_cast<const UTF16_TYPE*>(std::get<1>(_result)), 
            reinterpret_cast<UTF8_TYPE*>(std::get<2>(_result))
         );
      }
#endif
      std::tuple<bool, const wchar_t*, char*> convert_utf16_to_uft8(const wchar_t* pwszUtf16, char* pbszUtf8, tag_utf8 );// convert

      /// Convert Unicode buffer to stl string where text is stored in utf8 format
      std::tuple<bool, const uint16_t*> convert_utf16_to_uft8(const uint16_t* pwszUtf16, std::string& stringUtf8);

      /// convert utf8 buffer to stl wstring
      std::tuple<bool, const uint8_t*> convert_utf8_to_uft16(const uint8_t* pbszUtf8, std::wstring& stringUtf16);


      /// convert ascii text to utf8
      std::pair<bool, const uint8_t*> convert_ascii(const uint8_t* pbszFrom, uint8_t* pbszTo); // - convert_ascii
      template <typename UTF8_TYPE, typename TYPE>
      std::pair<bool, const uint8_t*> convert_ascii(const UTF8_TYPE* pbszFrom, TYPE* pbszTo) {
         return convert_ascii(reinterpret_cast<const uint8_t*>(pbszFrom), reinterpret_cast<uint8_t*>(pbszTo));
      }
      std::pair<bool, const uint8_t*> convert_ascii(const uint8_t* pbszFrom, uint8_t* pbszTo, const uint8_t* pbszEnd);
      template <typename UTF8_TYPE, typename TYPE>
      std::pair<bool, const uint8_t*> convert_ascii(const UTF8_TYPE* pbszFrom, TYPE* pbszTo, const TYPE* pbszEnd) {
         return convert_ascii(reinterpret_cast<const uint8_t*>(pbszFrom), reinterpret_cast<uint8_t*>(pbszTo), reinterpret_cast<const uint8_t*>(pbszEnd));
      }

      void convert_ascii( const char* pbsBegin, const char* pbsEnd, std::string& stringTo );
      inline void convert_ascii( const std::string_view& stringFrom, std::string& stringTo ) { convert_ascii( stringFrom.data(), stringFrom.data() + stringFrom.length(), stringTo ); }

      void convert_ascii( const char* pbsBegin, const char* pbsEnd, uint8_t* puTo, const uint8_t* puEnd );

      /// convert utf8 text to ascii
      std::pair<bool, const uint8_t*> convert_utf8_to_ascii( const uint8_t* puBegin, const uint8_t* puEnd, char* pbszTo, const char* pbszEnd );
      std::string convert_utf8_to_ascii( const std::string_view& stringUtf8 );


      /// convert unicode text to utf8
      std::pair<bool, const uint16_t*> convert_unicode(const uint16_t* pbszFrom, uint8_t* pbszTo, const uint8_t* pbszEnd);
      template <typename UNICODE_TYPE, typename UTF8_TYPE>
      std::pair<bool, const uint16_t*> convert_unicode(const UNICODE_TYPE* pwszFrom, UTF8_TYPE* pbszTo, UTF8_TYPE* pbszEnd) {
#ifdef _MSC_VER         
         static_assert(sizeof(UNICODE_TYPE) == 2, "Value isn't compatible with uint16_t");
#endif         
         static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
#ifndef NDEBUG
         [[maybe_unused]] const wchar_t* pwszFrom_d = (const wchar_t*)reinterpret_cast<const uint16_t*>(pwszFrom);
#endif // !NDEBUG        
         return convert_unicode(reinterpret_cast<const uint16_t*>(pwszFrom), reinterpret_cast<uint8_t*>(pbszTo), reinterpret_cast<const uint8_t*>(pbszEnd));
      }
      ///@}

      /// Converts unicode text to ascii, characters that can't be copied (above ascii codes) are skipped
      std::pair<const wchar_t*, const char*> convert_unicode_to_ascii(const wchar_t* pwszFrom, char* pbszTo, const char* pbszEnd);
      std::string convert_unicode_to_ascii( std::wstring_view stringUnicode );

      /// Converts ascii text to unicode
      std::pair<const char*, const wchar_t*> convert_ascii_to_unicode(const char* pszFrom, wchar_t* pwszTo, const wchar_t* pwszEnd);
      std::wstring convert_ascii_to_unicode(std::string_view stringAscii);

      /// Converts json formated escaped text to utf8
      std::pair<bool, const uint8_t*> convert_json(const uint8_t* puFrom, const uint8_t* puEnd, uint8_t* puTo);
      std::string convert_json( const std::string_view& stringJson );

      // ## convert text to number

      uint32_t atou(const uint8_t* puNumber);
      inline uint32_t atou(const char* pbszNumber) { return atou( (const uint8_t*)pbszNumber ); }
      uint32_t atou(const uint8_t* puBegin, const uint8_t* puEnd);
      inline uint32_t atou(const char* pbszBegin, const char* pbszEnd) { return atou( (const uint8_t*)pbszBegin, (const uint8_t*)pbszEnd ); }
      inline uint32_t atou( const std::string_view& stringNumber ) { return atou( (const uint8_t*)stringNumber.data(), (const uint8_t*)(stringNumber.data() + stringNumber.length()) ); }

      uint64_t atou64(const uint8_t* puNumber);
      inline uint64_t atou64(const char* pbszNumber) { return atou64( (const uint8_t*)pbszNumber ); }
      uint64_t atou64(const uint8_t* puBegin, const uint8_t* puEnd);
      inline uint64_t atou64(const char* pbszBegin, const char* pbszEnd) { return atou64( (const uint8_t*)pbszBegin, (const uint8_t*)pbszEnd ); }
      inline uint64_t atou64( const std::string_view& stringNumber ) { return atou64( (const uint8_t*)stringNumber.data(), (const uint8_t*)(stringNumber.data() + stringNumber.length()) ); }

      uint32_t atou(const uint8_t* puNumber, tag_find);
      inline uint32_t atou(const char* pbszNumber, tag_find) { return atou( (const uint8_t*)pbszNumber, tag_find{}); }
      uint32_t atou(const uint8_t* puBegin, const uint8_t* puEnd, tag_find);

      // ## convert number to text

      uint8_t* itoa( int32_t iNumber, uint8_t* pbszTo );
      inline char* itoa( int32_t iNumber, char* pbszTo ) { return (char*)itoa( iNumber, (uint8_t*)pbszTo ); }
      uint8_t* utoa( uint32_t uNumber, uint8_t* pbszTo );
      inline char* utoa( uint32_t uNumber, char* pbszTo ) { return (char*)utoa( uNumber, (uint8_t*)pbszTo ); }
      uint8_t* itoa( int64_t iNumber, uint8_t* pbszTo );
      inline char* itoa( int64_t iNumber, char* pbszTo ) { return (char*)itoa( iNumber, (uint8_t*)pbszTo ); }
      uint8_t* utoa( uint64_t uNumber, uint8_t* pbszTo );
      inline char* utoa( uint64_t uNumber, char* pbszTo ) { return (char*)utoa( uNumber, (uint8_t*)pbszTo ); }
      
      [[nodiscard]] inline std::string itoa_to_string(int32_t iNumber) { uint8_t p[12]; itoa(iNumber, p); return std::string((char*)p); }
      [[nodiscard]] inline std::string utoa_to_string(uint32_t uNumber) { uint8_t p[12]; utoa(uNumber, p); return std::string((char*)p); }
      [[nodiscard]] inline std::string itoa_to_string(int64_t iNumber) { uint8_t p[23]; itoa(iNumber, p); return std::string((char*)p); }
      [[nodiscard]] inline std::string utoa_to_string(uint64_t uNumber) { uint8_t p[23]; utoa(uNumber, p); return std::string((char*)p); }

      // ## convert hex text to binary

      std::pair<bool, uint8_t*> convert_hex_to_binary( const uint8_t* pbszFrom, const uint8_t* pbszTo, uint8_t* puBinary );
      inline std::pair<bool, uint8_t*> convert_hex_to_binary( const std::string_view& stringText, uint8_t* puBinary ) {
         return convert_hex_to_binary( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length(), puBinary );
      }


      std::intptr_t distance(const uint8_t* p1, const uint8_t* p2);


      ///@{
      uint8_t* copy_character( uint8_t* puCopyTo, const uint8_t* puCopyFrom );
      ///@}


      /**
       * @brief move operation for pointer in utf-8 buffer
      */
      namespace move {

         /** \name NEXT OPERATIONS
         moves to next...
*move operations*
| name | description |
| - | - |
| `advance` | advance specified number of characters (if negative move backwards |
| `end` | moves to end of text |
| `find` | tries to find character within the selected range of characters |
| `find_character` | tries to find utf8 character within the selected range of characters |
| `next` | moves to next character or to specified amount of characters|
| `next_space` | moves to first space character (space = space, tab or carriage return) |
| `next_non_space` | moves to next character |
| `previous` | move backwards |
         *///@{ 

         // move to next utf8 character in buffer pointer points at
         const uint8_t* next(const uint8_t* pubszPosition); // ------------------------------------ next                                       
         // move to next utf8 character in buffer pointer points at
         inline uint8_t* next(uint8_t* pubszPosition) { return const_cast<uint8_t*>( next( static_cast<const uint8_t*>(pubszPosition) ) ); } // next
         // move to next utf8 character in buffer pointer points at
         template <typename UTF8_TYPE>
         const UTF8_TYPE* next(const UTF8_TYPE* pubszPosition) { // ------------------------------- next
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            return reinterpret_cast<const UTF8_TYPE*>( next( reinterpret_cast<const uint8_t*>(pubszPosition) ) );
         };
         // move to next utf8 character in buffer pointer points at
         template <typename UTF8_TYPE>
         UTF8_TYPE* next(UTF8_TYPE* pubszPosition) { // ------------------------------------------- next
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            return reinterpret_cast<UTF8_TYPE*>( next( reinterpret_cast<uint8_t*>(pubszPosition) ) );
         };

         // move specified count in buffer pointer points at
         const uint8_t* next(const uint8_t* pubszPosition, uint32_t uCount ); // ------------------ next
         inline uint8_t* next(uint8_t* pubszPosition, uint32_t uCount ) { return const_cast<uint8_t*>( next( static_cast<const uint8_t*>(pubszPosition), uCount ) ); } // move specified count in buffer pointer points at
         template <typename UTF8_TYPE>
         const UTF8_TYPE* next(const UTF8_TYPE* pubszPosition, uint32_t uCount ) {                 // move specified count in buffer pointer points at
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            return reinterpret_cast<const UTF8_TYPE*>( next(reinterpret_cast<const uint8_t*>(pubszPosition), uCount) );
         };

         // move pointer to next utf8 character in buffer pointer to pointer points at, return true or false if it succeeded
         inline bool next(const uint8_t** ppubszPosition) { // ------------------------------------ next
            const uint8_t* p = next(*ppubszPosition);
            if(p != *ppubszPosition) { ppubszPosition = &p; return true; }
            return false;
         }
         // move pointer to next utf8 character in buffer pointer to pointer points at, return true or false if it succeeded
         template <typename UTF8_TYPE>
         bool next(const UTF8_TYPE** ppubszPosition) { // ----------------------------------------- next
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            return reinterpret_cast<const UTF8_TYPE*>(next(reinterpret_cast<const uint8_t**>(ppubszPosition)));
         };
         bool next(const uint8_t** ppubszPosition, uint32_t uCount); // --------------------------- next
         template <typename UTF8_TYPE>
         bool next(const UTF8_TYPE** ppubszPosition, uint32_t uCount) { // ------------------------ next
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            return reinterpret_cast<const UTF8_TYPE*>(next(reinterpret_cast<const uint8_t**>(ppubszPosition), uCount));
         };

         // move pointer to next space character (space, carriage return, tab or new line)
         const uint8_t* next_space(const uint8_t* pubszPosition); // ------------------------------ next_space
         template <typename UTF8_TYPE>
         const UTF8_TYPE* next_space(const UTF8_TYPE* pubszPosition) { // ------------------------- next_space
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            return reinterpret_cast<const UTF8_TYPE*>(next_space(reinterpret_cast<const uint8_t*>(pubszPosition)));
         };

         // move pointer to next non space character
         const uint8_t* next_non_space(const uint8_t* pubszPosition); // -------------------------- next_non_space
         template <typename UTF8_TYPE>
         const UTF8_TYPE* next_non_space(const UTF8_TYPE* pubszPosition) { // --------------------- next_non_space
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            return reinterpret_cast<const UTF8_TYPE*>(next_non_space(reinterpret_cast<const uint8_t*>(pubszPosition)));
         };

         ///@}



         ///@{ 
         // move to previous utf8 character in buffer pointer points at
         const uint8_t* previous(const uint8_t* pubszPosition); // -------------------------------- previous
         inline uint8_t* previous(uint8_t* pubszPosition) { return const_cast<uint8_t*>( previous( static_cast<const uint8_t*>(pubszPosition) ) ); }// move to previous utf8 character in buffer pointer points at
         template <typename UTF8_TYPE>
         const UTF8_TYPE* previous(const UTF8_TYPE* pubszPosition) {                               // move to previous utf8 character in buffer pointer points at
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            return reinterpret_cast<const UTF8_TYPE*>( previous( reinterpret_cast<const uint8_t*>(pubszPosition) ) );
         };
         // move to previous utf8 character in buffer pointer points at
         template <typename UTF8_TYPE>
         UTF8_TYPE* previous(UTF8_TYPE* pubszPosition) { // --------------------------------------- previous
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            return reinterpret_cast<UTF8_TYPE*>( previous( reinterpret_cast<uint8_t*>(pubszPosition) ) );
         };

         const uint8_t* previous(const uint8_t* pubszPosition, uint32_t uCount);                   // move backwards specified count in utf8 character buffer pointer points at
         inline uint8_t* previous(uint8_t* pubszPosition, uint32_t uCount ) { return const_cast<uint8_t*>( previous( static_cast<const uint8_t*>(pubszPosition), uCount ) ); } // // move backwards specified count in utf8 character buffer pointer points at
         // move backwards specified count in utf8 character buffer pointer points at
         template <typename UTF8_TYPE>
         const UTF8_TYPE* previous( const UTF8_TYPE* pubszPosition, uint32_t uCount ) { // -------- previous     
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            return reinterpret_cast<const UTF8_TYPE*>( previous( reinterpret_cast<const uint8_t*>(pubszPosition), uCount ) );
         };
         ///@}

         ///@{
         /// advance specified number of characters (if negative move backwards)
         inline const uint8_t* advance(const uint8_t* pubszPosition, int iOffset) {
            if(iOffset > 0) return next(pubszPosition, iOffset);
            else if(iOffset < 0) return previous(pubszPosition, -iOffset);
            return pubszPosition;
         }
         /// advance specified number of characters (if negative move backwards)
         inline uint8_t* advance(uint8_t* pubszPosition, int iOffset) {
            if(iOffset > 0) return next(pubszPosition, iOffset);
            else if(iOffset < 0) return previous(pubszPosition, -iOffset);
            return pubszPosition;
         }
         ///@}

         // move to end of string (finds zero character and stop)
         const uint8_t* end(const uint8_t* pubszPosition); // ------------------------------------- end 
         inline uint8_t* end(uint8_t* pubszPosition) { return const_cast<uint8_t*>( end( static_cast<const uint8_t*>(pubszPosition) ) ); }// move to end of string (finds zero character and stop)
         template <typename UTF8_TYPE>
         const UTF8_TYPE* end(const UTF8_TYPE* pubszPosition) { // -------------------------------- end
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            return reinterpret_cast<const UTF8_TYPE*>(end(reinterpret_cast<const uint8_t*>(pubszPosition)));
         };


         const uint8_t* find( const uint8_t* pubszPosition, uint32_t uCharacter ); // ------------- find
         template <typename UTF8_TYPE, typename CHARACTER>
         const UTF8_TYPE* find( const UTF8_TYPE* pubszPosition, CHARACTER Character ) { // -------- find
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            uint32_t uCharacter;
            if constexpr( sizeof(Character) == sizeof(uint8_t) ) uCharacter = static_cast<uint8_t>(Character);       // 1 byte
            else if constexpr( sizeof(Character) == sizeof(uint16_t) ) uCharacter = static_cast<uint16_t>(Character);// 2 byte value
            else uCharacter = static_cast<uint32_t>(Character);                                                      // 4 byte or over
            return reinterpret_cast<const UTF8_TYPE*>( find( reinterpret_cast<const uint8_t*>(pubszPosition), uCharacter ) );
         }
         const uint8_t* find( const uint8_t* pubszPosition, const uint8_t* pubszEnd, uint32_t uCharacter ); // find
         template <typename UTF8_TYPE, typename CHARACTER>
         const UTF8_TYPE* find( const UTF8_TYPE* pubszPosition, const UTF8_TYPE* pubszEnd, CHARACTER Character ) { // find
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            uint32_t uCharacter;
            if constexpr( sizeof(Character) == sizeof(uint8_t) ) uCharacter = static_cast<uint8_t>(Character);       // 1 byte
            else if constexpr( sizeof(Character) == sizeof(uint16_t) ) uCharacter = static_cast<uint16_t>(Character);// 2 byte value
            else uCharacter = static_cast<uint32_t>(Character);                                                      // 4 byte or over
            return reinterpret_cast<const UTF8_TYPE*>( find( reinterpret_cast<const uint8_t*>(pubszPosition), reinterpret_cast<const uint8_t*>(pubszEnd), uCharacter ) );
         }
         const uint8_t* find_character( const uint8_t* pubszPosition, const uint8_t* pubszCharacter, uint32_t ulength );
         const uint8_t* find_character( const uint8_t* pubszPosition, const uint8_t* pubszEnd, const uint8_t* pubszCharacter, uint32_t ulength );
         inline const uint8_t* find_character( const uint8_t* pubszPosition, const uint8_t* pubszCharacter ) { return find_character( pubszPosition, pubszCharacter, static_cast<uint32_t>(std::strlen( reinterpret_cast<const char*>(pubszCharacter)) ) ); }

         const uint8_t* find( const uint8_t* pubszPosition, const uint8_t* pubszEnd, const uint8_t* pubszFind, uint32_t uSize ); // find
         inline const uint8_t* find( const uint8_t* pubszText, const uint8_t* pubszFind ) { return find( pubszText, pubszText + std::strlen( reinterpret_cast<const char*>(pubszText) ), pubszFind, static_cast<uint32_t>( std::strlen( reinterpret_cast<const char*>(pubszFind) ) ) ); }
         inline const uint8_t* find( const uint8_t* pubszText, const uint8_t* pubszEnd, const uint8_t* pubszFind ) { return find( pubszText, pubszEnd, pubszFind, static_cast<uint32_t>( std::strlen( reinterpret_cast<const char*>(pubszFind) ) ) ); }

         /// Finds the specified UTF-8 character in a given string view and returns new string view starting from that character.
         std::string_view find(const std::string_view& stringText, uint32_t uCharacter);


         /// Finds the nth occurrence of a specific character in a byte array.
         const uint8_t* find_nth(const uint8_t* pubszPosition, size_t uNth, uint32_t uCharacter);
         template <typename UTF8_TYPE, typename CHARACTER>
         const UTF8_TYPE* find_nth( const UTF8_TYPE* pubszPosition, size_t uNth, CHARACTER Character ) { // -------- find
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            uint32_t uCharacter;
            if constexpr( sizeof(Character) == sizeof(uint8_t) ) uCharacter = static_cast<uint8_t>(Character);       // 1 byte
            else if constexpr( sizeof(Character) == sizeof(uint16_t) ) uCharacter = static_cast<uint16_t>(Character);// 2 byte value
            else uCharacter = static_cast<uint32_t>(Character);                                                      // 4 byte or over
            return reinterpret_cast<const UTF8_TYPE*>( find_nth( reinterpret_cast<const uint8_t*>(pubszPosition), uNth, uCharacter ) );
         }

         /// Searches for the nth occurrence of a specified UTF-8 character within a given range of bytes and returns a pointer to its position or nullptr if not found.
         const uint8_t* find_nth( const uint8_t* pubszPosition, const uint8_t* pubszEnd, size_t uNth, uint32_t uCharacter ); // find
         template <typename UTF8_TYPE, typename CHARACTER>
         const UTF8_TYPE* find_nth( const UTF8_TYPE* pubszPosition, const UTF8_TYPE* pubszEnd, size_t uNth, CHARACTER Character ) { // find
            static_assert(sizeof(UTF8_TYPE) == 1, "Value isn't compatible with uint8_t");
            uint32_t uCharacter;
            if constexpr( sizeof(Character) == sizeof(uint8_t) ) uCharacter = static_cast<uint8_t>(Character);       // 1 byte
            else if constexpr( sizeof(Character) == sizeof(uint16_t) ) uCharacter = static_cast<uint16_t>(Character);// 2 byte value
            else uCharacter = static_cast<uint32_t>(Character);                                                      // 4 byte or over
            return reinterpret_cast<const UTF8_TYPE*>( find_nth( reinterpret_cast<const uint8_t*>(pubszPosition), reinterpret_cast<const uint8_t*>(pubszEnd), uNth, uCharacter ) );
         }

         /// Finds the nth occurrence of a specified UTF-8 character in a given string view and returns new string view starting from that character.
         std::string_view find_nth(const std::string_view& stringText, size_t uNth, uint32_t uCharacter);
      }

      /**
       * @brief json formated text encoding
       */
      namespace json {
         // ## Validation methods
         bool is_encoded( uint8_t uChar );
         std::pair<bool, const uint8_t*> validate( const uint8_t* pubBegin, const uint8_t* pubEnd );

         /// get character number
         uint32_t character(const uint8_t* pubszCharacter, bool* pbUnicode );
         /// get character number
         uint32_t character( const uint8_t* pubszCharacter );
         template <typename UTF8_TYPE>
         uint32_t character( const UTF8_TYPE* pCharacter, bool* pbUnicode ) { return character( reinterpret_cast<const uint8_t*>( pCharacter ), pbUnicode ); }
         template <typename UTF8_TYPE>
         uint32_t character(const UTF8_TYPE* pCharacter) { return character( reinterpret_cast<const uint8_t*>( pCharacter ) ); }

         /// count number of characters
         uint32_t count( const uint8_t* pubszText );
         /// count number of characters
         uint32_t count( const uint8_t* pubszBegin, const uint8_t* pubszEnd );
         template <typename UTF8_TYPE>
         uint32_t count(const UTF8_TYPE* pText) { return count( reinterpret_cast<const uint8_t*>( pText ) ); }
         template <typename UTF8_TYPE>
         uint32_t count(const UTF8_TYPE* pBegin, const UTF8_TYPE* pEnd) { return count( reinterpret_cast<const uint8_t*>( pBegin ), reinterpret_cast<const uint8_t*>( pEnd ) ); }

         /// Needed size for to store text as utf8
         uint32_t size( const uint8_t* pubszText );
         /// Needed size for to store text as utf8
         uint32_t size( const uint8_t* pubszBegin, const uint8_t* pubszEnd );
         template <typename UTF8_TYPE>
         uint32_t size(const UTF8_TYPE* pText) { return size( reinterpret_cast<const uint8_t*>( pText ) ); }
         template <typename UTF8_TYPE>
         uint32_t size(const UTF8_TYPE* pBegin, const UTF8_TYPE* pEnd) { return size( reinterpret_cast<const uint8_t*>( pBegin ), reinterpret_cast<const uint8_t*>( pEnd ) ); }

         /// move to next character
         const uint8_t* next( const uint8_t* pubszPosition );
         template <typename UTF8_TYPE>
         const UTF8_TYPE* next(const UTF8_TYPE* pubszPosition) {
            return reinterpret_cast<const UTF8_TYPE*>( next( reinterpret_cast<const uint8_t*>( pubszPosition ) ) );
         }


         /// find character that need to be escape to be allowerd in json
         const uint8_t* find_character_to_escape( const uint8_t* pubszBegin, const uint8_t* pubszEnd );
         inline const uint8_t* find_character_to_escape( const uint8_t* pubszText ) { return find_character_to_escape( pubszText, pubszText + strlen( pubszText ) ); }
         inline const char* find_character_to_escape( const char* pubszBegin, const char* pubszEnd ) { return (const char*)find_character_to_escape( (const uint8_t*)pubszBegin, (const uint8_t*)pubszEnd ); }
         inline const char* find_character_to_escape( const std::string_view& stringText ) { return find_character_to_escape( &(*stringText.begin()), stringText.data() + stringText.length() ); }

         std::pair<bool, const uint8_t*> convert_utf8_to_json( const uint8_t* pubszText, const uint8_t* pubszEnd, uint8_t* pbszTo );
         inline std::pair<bool, const uint8_t*> convert_utf8_to_json( const uint8_t* pubszText, uint8_t* pbszTo ) { return convert_utf8_to_json( pubszText, pubszText + strlen(pubszText), pbszTo); };
         inline std::pair<bool, const uint8_t*> convert_utf8_to_json( const std::string_view& stringText, uint8_t* pbszTo ) { 
            return convert_utf8_to_json( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length(), pbszTo); 
         };

         bool convert_utf8_to_json(const uint8_t* pubszText, const uint8_t* pubszEnd, std::string& stringTo );
         inline bool convert_utf8_to_json( const char* pbszText, std::string& stringTo ) {
            return convert_utf8_to_json( reinterpret_cast<const uint8_t*>(pbszText), reinterpret_cast<const uint8_t*>(pbszText) + strlen(pbszText), stringTo); 
         };
         inline bool convert_utf8_to_json( const std::string_view& stringText, std::string& stringTo ) { 
            return convert_utf8_to_json( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length(), stringTo); 
         };
         inline std::string convert_utf8_to_json( const std::string_view& stringText ) { 
            std::string stringTo;
            convert_utf8_to_json( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length(), stringTo); 
            return stringTo;
         };

      }

      /**
       * @brief operation used to convert between format used in uri/url text
       * |name|description|
       * |-|-|
       * |character|Convert uri character sequence to single 32 bit number|
       * |size|Calculate number of bytes that is needed to stor uri formated character|
       * |count|Count number of characters in buffer|
       * |get_character_size|Compute number of bytes needed to store all uri formated text as utf-8|
       * |next|Moves to next character in uri formated text|
       * |next_sequence|finds next special uri formated sequence (percent and hex number like "%5A")|
       * |convert_uri_to_uf8|Convert uri formated text to utf-8|
      */
      namespace uri {
         // ## convert uri formated character to utf8 character
         uint32_t character(const uint8_t* pubszCharacter);
         inline uint32_t character(const char* pbszCharacter) { return character( reinterpret_cast<const uint8_t*>( pbszCharacter ) ); }

         // ## calculate needed size to stor uri formated character as utf8 character
         uint32_t size( const uint8_t* pubszCharacter );
         inline uint32_t size(const char* pbszCharacter) { return size( reinterpret_cast<const uint8_t*>( pbszCharacter ) ); }

         // ## calculate number of utf8 characters found in text
         std::pair<uint32_t, const uint8_t*> count(const uint8_t* pubszText, const uint8_t* pubszEnd);
         inline std::pair<uint32_t, const char*> count(const std::string_view& stringText) { 
            auto result_ = uri::count( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length()); 
            return { result_.first, reinterpret_cast<const char*>( result_.second ) };
         }

         // ## Validation methods
         bool is_encoded( uint8_t uChar );
         std::pair<bool, const uint8_t*> validate( const uint8_t* pubBegin, const uint8_t* pubEnd );

         // ## Calculate needed buffer size to store uri text as utf8 text 
         uint32_t get_character_size( const uint8_t* pubszText, const uint8_t* pubszEnd );
         inline uint32_t get_character_size( const std::string_view& stringText ) {
            return uri::get_character_size( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length() );
         }

         // ## move position within uri formated text
         const uint8_t* next( const uint8_t* pubszPosition );
         const uint8_t* next_sequence( const uint8_t* pubszPosition, const uint8_t* pubszEnd  );
         inline const uint8_t* next_sequence( const std::string_view& stringUriText ) { return next_sequence( reinterpret_cast<const uint8_t*>(stringUriText.data()), reinterpret_cast<const uint8_t*>(stringUriText.data()) + stringUriText.length() ); }

         // ## convert uri formated text to utf7
         std::pair<bool, const uint8_t*> convert_uri_to_uf8( const uint8_t* pubszText, const uint8_t* pubszEnd, uint8_t* pbszTo );
         inline std::pair<bool, const uint8_t*> convert_uri_to_uf8( const uint8_t* pubszText, uint8_t* pbszTo ) { return convert_uri_to_uf8( pubszText, pubszText + strlen(pubszText), pbszTo); };
         inline std::pair<bool, const uint8_t*> convert_uri_to_uf8( const std::string_view& stringText, uint8_t* pbszTo ) { 
            return convert_uri_to_uf8( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length(), pbszTo); 
         };
         std::pair<bool, const uint8_t*> convert_uri_to_uf8( const std::string_view& stringUri, std::string& stringUtf8 );

         // ## convert utf8 text to uri escaped text
         std::pair<bool, const uint8_t*> convert_utf8_to_uri( const uint8_t* pubszText, const uint8_t* pubszEnd, uint8_t* pbszTo );
         inline std::pair<bool, const uint8_t*> convert_utf8_to_uri( const uint8_t* pubszText, uint8_t* pbszTo ) { return convert_utf8_to_uri( pubszText, pubszText + strlen(pubszText), pbszTo); };
         inline std::pair<bool, const uint8_t*> convert_utf8_to_uri( const std::string_view& stringText, uint8_t* pbszTo ) { 
            return convert_utf8_to_uri( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length(), pbszTo); 
         };
         bool convert_utf8_to_uri( const uint8_t* pubszText, const uint8_t* pubszEnd, std::string& stringTo );
         inline bool convert_utf8_to_uri( const char* pbszText, std::string& stringTo ) {
            return convert_utf8_to_uri( reinterpret_cast<const uint8_t*>(pbszText), reinterpret_cast<const uint8_t*>(pbszText) + strlen(pbszText), stringTo); 
         };
         inline bool convert_utf8_to_uri( const uint8_t* pubszText, std::string& stringTo ) { return convert_utf8_to_uri( pubszText, pubszText + strlen(pubszText), stringTo); };
         inline bool convert_utf8_to_uri( const std::string_view& stringText, std::string& stringTo ) { 
            return convert_utf8_to_uri( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length(), stringTo); 
         };



         // TODO: validate uri text
         // TODO: copy uri text to utf8 buffer

      }

      namespace xml {
         uint32_t size( const uint8_t* pubszCharacter );
         bool is_encoded( uint8_t uChar );

         // ## convert utf8 text to xml escaped text
         std::pair<bool, const uint8_t*> convert_utf8_to_xml( const uint8_t* pubszText, const uint8_t* pubszEnd, uint8_t* pbszTo );
         inline std::pair<bool, const uint8_t*> convert_utf8_to_xml( const uint8_t* pubszText, uint8_t* pbszTo ) { return convert_utf8_to_xml( pubszText, pubszText + strlen(pubszText), pbszTo); };
         inline std::pair<bool, const uint8_t*> convert_utf8_to_xml( const std::string_view& stringText, uint8_t* pbszTo ) { 
            return convert_utf8_to_xml( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length(), pbszTo); 
         };

         bool convert_utf8_to_xml( const uint8_t* pubszText, const uint8_t* pubszEnd, std::string& stringTo );
         inline bool convert_utf8_to_xml( const char* pbszText, std::string& stringTo ) {
            return convert_utf8_to_xml( reinterpret_cast<const uint8_t*>(pbszText), reinterpret_cast<const uint8_t*>(pbszText) + strlen(pbszText), stringTo); 
         };
         inline bool convert_utf8_to_xml( const std::string_view& stringText, std::string& stringTo ) { 
            return convert_utf8_to_xml( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length(), stringTo); 
         };
         inline std::string convert_utf8_to_xml( const std::string_view& stringText ) { 
            std::string stringTo;
            convert_utf8_to_xml( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data()) + stringText.length(), stringTo); 
            return stringTo;
         };

      }
   } 

   namespace utf16 {

      uint32_t character(const uint16_t* pubszText); // ------------------------------------------- character
      template <typename UTF16_TYPE>
      uint32_t character(const UTF16_TYPE* pbszText) { // ----------------------------------------- character
         //static_assert(sizeof(UTF16_TYPE) == 2, "Value isn't compatible with uint8_t");          // #GCC generates error here, try to fix this 2022-10-10
         return character(reinterpret_cast<const uint16_t*>(pbszText));
      };

      uint32_t size(uint16_t ch);
   }

} // gd


namespace gd {
   namespace ascii {
      /**
       * compare two strings
       */
      ///@{ 
      int strcmp( const char* pbsz1, const char* pbsz2, utf8::tag_wildcard );
      bool strcmp( const char* piText, size_t uTextLength, const char* piMatch, size_t uMatchLength, utf8::tag_wildcard );
      inline bool strcmp(const std::string_view& stringText, const char* piPattern, size_t uPatternLength, utf8::tag_wildcard) {
         return strcmp(stringText.data(), stringText.length(), piPattern, uPatternLength, utf8::tag_wildcard{});
      }
      inline bool strcmp(const std::string_view& stringText, const std::string_view& stringPattern, utf8::tag_wildcard) {
         return strcmp(stringText.data(), stringText.length(), stringPattern.data(), stringPattern.length(), utf8::tag_wildcard{} );
      }

      int stricmp(std::string_view string1, std::string_view string2);

      ///@}
   }
} // gd



namespace gd {
   namespace utf8 {

      // ## Trim methods used to remove spaces

      /**
       * @brief find positions in text where characters start and ends
       * @param puText pointer to text scanned for first and last character
       * @param puEnd end of text
       * @return std::pair<const uint8_t*,const uint8_t*> first and last position in string where first non space is found
       */
      inline std::pair<const uint8_t*,const uint8_t*> trim(const uint8_t* puText, const uint8_t* puEnd)
      {                                                                       assert( puText < puEnd ); assert( (puEnd - puText) < 0x20000 ); // check for proper order and realistic value
         auto puFirst = puText;
         while( puFirst != puEnd )
         {
            puFirst++;
            if( *puFirst > ' ' ) break;
         }

         auto* puLast = puEnd;
         while( puLast != puFirst )
         {
            puLast--;
            if( *puLast > ' ' ) break;
         }

         return { puFirst, puLast + 1 };
      }

      /// find positions in text where characters start
      inline std::pair<const uint8_t*, const uint8_t*> trim(const uint8_t* puText) { return trim(puText, puText + std::strlen((const char*)puText)); }
      inline std::pair<const uint8_t*, const uint8_t*> trim(const std::string_view& stringText) { return trim((const uint8_t*)stringText.data(), (const uint8_t*)stringText.data() + stringText.length()); }
      /**
       * @brief find positions in text where characters start
       * @param puText pointer to text scanned for first and last character
       * @param puEnd end of text
       * @return std::pair<const uint8_t*,const uint8_t*> first and last position in string where first non space is found
       */
      inline std::pair<const uint8_t*,const uint8_t*> trim_left(const uint8_t* puText, const uint8_t* puEnd)
      {                                                                       assert( puText < puEnd ); assert( (puEnd - puText) < 0x20000 ); // check for proper order and realistic value
         auto puFirst = puText;
         while( puFirst != puEnd )
         {
            puFirst++;
            if( *puFirst > ' ' ) break;
         }

         return { puFirst, puEnd };
      }

      /// find positions in text where characters start
      inline std::pair<const uint8_t*, const uint8_t*> trim_left(const uint8_t* puText) { return trim(puText, puText + std::strlen((const char*)puText)); }
      inline std::pair<const uint8_t*, const uint8_t*> trim_left(const std::string_view& stringText) { return trim_left((const uint8_t*)stringText.data(), (const uint8_t*)stringText.data() + stringText.length()); }


      /**
       * @brief find positions in text where characters ends
       * @param puText pointer to text scanned for first and last character
       * @param puEnd end of text
       * @return std::pair<const uint8_t*,const uint8_t*> first and last position in string where first non space is found
       */
      inline std::pair<const uint8_t*,const uint8_t*> trim_right(const uint8_t* puText, const uint8_t* puEnd)
      {                                                                       assert( puText < puEnd ); assert( (puEnd - puText) < 0x20000 ); // check for proper order and realistic value
         auto* puLast = puEnd;
         while( puLast != puText )
         {
            puLast--;
            if( *puLast > ' ' ) break;
         }

         return { puText, puLast + 1 };
      }

      /// find positions in text where characters ends
      inline std::pair<const uint8_t*, const uint8_t*> trim_right(const uint8_t* puText) { return trim(puText, puText + std::strlen((const char*)puText)); }
      inline std::pair<const uint8_t*, const uint8_t*> trim_right(const std::string_view& stringText) { return trim_right((const uint8_t*)stringText.data(), (const uint8_t*)stringText.data() + stringText.length()); }

      inline std::string trim_to_string(const uint8_t* puText, const uint8_t* puEnd)
      {
         auto [first_, last_] = trim( puText, puEnd );
         return std::string(first_, last_);
      }
      inline std::string trim_to_string(const uint8_t* puText) { return trim_to_string(puText, puText + std::strlen((const char*)puText)); }
      inline std::string trim_to_string(const std::string_view& stringText) { return trim_to_string((const uint8_t*)stringText.data(), (const uint8_t*)stringText.data() + stringText.length()); }

      inline std::string trim_left_to_string(const uint8_t* puText, const uint8_t* puEnd)
      {
         auto [first_, last_] = trim(puText, puEnd);
         return std::string(first_, last_);
      }
      inline std::string trim_left_to_string(const uint8_t* puText) { return trim_left_to_string(puText, puText + std::strlen((const char*)puText)); }
      inline std::string trim_left_to_string(const std::string_view& stringText) { return trim_left_to_string((const uint8_t*)stringText.data(), (const uint8_t*)stringText.data() + stringText.length()); }

      inline std::string trim_right_to_string(const uint8_t* puText, const uint8_t* puEnd)
      {
         auto [first_, last_] = trim(puText, puEnd);
         return std::string(first_, last_);
      }
      inline std::string trim_right_to_string(const uint8_t* puText) { return trim_right_to_string(puText, puText + std::strlen((const char*)puText)); }
      inline std::string trim_right_to_string(const std::string_view& stringText) { return trim_right_to_string((const uint8_t*)stringText.data(), (const uint8_t*)stringText.data() + stringText.length()); }

      // ## Quote string value
      std::string quoted( const std::string_view& stringToQuote );
      std::string quoted_if_text( const std::string_view& stringToQuote );

      
      // ## Split methods, split string into parts

      void split( const char* pbBegin, const char* pbEnd, char chSplit, std::vector<std::string_view>& vectorPart );
      void split( const std::string_view& stringText, char chSplitWith, std::vector<std::string_view>& vectorPart );
      inline std::vector<std::string_view> split( const std::string_view& stringText, char chSplitWith ) { std::vector<std::string_view> v_; split( stringText, chSplitWith, v_ ); return v_; }
      void split( const std::string_view& stringText, char chSplitWith, std::vector<std::string>& vectorPart );
      inline std::vector<std::string> split( const std::string_view& stringText, char chSplitWith, tag_string ) { std::vector<std::string> v_; split( stringText, chSplitWith, v_ ); return v_; }
      void split( const std::string_view& stringText, const std::string_view& stringSplit, std::vector<std::string>& vectorPart );
      void split( const std::string_view& stringText, uint32_t uSplitWith, std::vector<std::string>& vectorPart );
      inline std::vector<std::string> split(const std::string_view& stringText, uint32_t uSplitWith, tag_string) {
         std::vector<std::string> vectorPart;
         split( stringText, uSplitWith, vectorPart );
         return vectorPart;
      }
      inline std::vector<std::string_view> split(const std::string_view& stringText, uint32_t uSplitWith, tag_string_view) {
         std::vector<std::string_view> vectorPart;
         split( stringText, uSplitWith, vectorPart );
         return vectorPart;
      }

      void split( const char* pbBegin, const char* pbEnd, char chSplitWith, std::vector< std::variant< unsigned, std::string > >& vectorPart );
      inline void split(const std::string_view& stringText, char chSplitWith, std::vector< std::variant< unsigned, std::string > >& vectorPart) {
         split( stringText.data(), stringText.data() + stringText.length(), chSplitWith, vectorPart );
      }

      // ### split into vector with `string_view`

      void split( const char* pbBegin, const char* pbEnd, const std::vector<std::size_t>& vectorSplit, std::vector<std::string_view>& vectorPart );
      inline void split(const std::string_view& stringText, const std::vector<std::size_t>& vectorSplit, std::vector<std::string_view>& vectorPart) {
         split( &(*stringText.begin()), stringText.data() + stringText.length(), vectorSplit, vectorPart );
      }
      inline std::vector<std::string_view> split(const std::string_view& stringText, const std::vector<std::size_t>& vectorSplit, tag_string_view) {
         std::vector<std::string_view> vectorPart;
         split( &(*stringText.begin()), stringText.data() + stringText.length(), vectorSplit, vectorPart );
         return vectorPart;
      }

      // ### split into vector with `string`

      void split( const char* pbBegin, const char* pbEnd, const std::vector<std::size_t>& vectorSplit, std::vector<std::string>& vectorPart );
      inline void split(const std::string_view& stringText, const std::vector<std::size_t>& vectorSplit, std::vector<std::string>& vectorPart) {
         split( &(*stringText.begin()), stringText.data() + stringText.length(), vectorSplit, vectorPart );
      }
      inline std::vector<std::string> split(const std::string_view& stringText, const std::vector<std::size_t>& vectorSplit, tag_string) {
         std::vector<std::string> vectorPart;
         split( &(*stringText.begin()), stringText.data() + stringText.length(), vectorSplit, vectorPart );
         return vectorPart;
      }

      const char* split( const char* pbBegin, const char* pbEnd, char chSplit, char chStop, std::vector<std::string_view>& vectorPart );
      std::string_view split( const std::string_view& stringText, char chSplit, char chStop, std::vector<std::string_view>& vectorPart );

      /// Splits string into pair of strings, one character is used to split the pair and ano to split pairs
      void split_pair( const char* pbBegin, const char* pbEnd, char chSplit, char chSplitPair, std::vector< std::pair<std::string_view,std::string_view>>& vectorPair );
      inline void split_pair( const std::string_view& stringText, char chSplit, char chSplitPair, std::vector< std::pair<std::string_view,std::string_view> >& vectorPair ) {
         split_pair( stringText.data(), stringText.data() + stringText.length(), chSplit, chSplitPair, vectorPair );
      }
      inline std::vector< std::pair<std::string_view,std::string_view> > split_pair( const char* pbBegin, const char* pbEnd, char chSplit, char chSplitPair, tag_string_view ) {
         std::vector< std::pair<std::string_view,std::string_view> > vector_;
         split_pair( pbBegin, pbEnd, chSplit, chSplitPair, vector_ );
         return vector_;
      }
      inline std::vector< std::pair<std::string_view,std::string_view> > split_pair( const std::string_view& stringText, char chSplit, char chSplitPair, tag_string_view ) {
         std::vector< std::pair<std::string_view,std::string_view> > vector_;
         split_pair( stringText, chSplit, chSplitPair, vector_ );
         return vector_;
      }

      void split_pair( const char* pbBegin, const char* pbEnd, char chSplit, char chSplitPair, std::vector< std::pair<std::string,std::string> >& vectorPair );
      inline void split_pair( const std::string_view& stringText, char chSplit, char chSplitPair, std::vector< std::pair<std::string,std::string> >& vectorPair ) {
         split_pair( stringText.data(), stringText.data() + stringText.length(), chSplit, chSplitPair, vectorPair );
      }
      inline std::vector< std::pair<std::string,std::string> > split_pair( const char* pbBegin, const char* pbEnd, char chSplit, char chSplitPair, tag_string ) {
         std::vector< std::pair<std::string,std::string> > vector_;
         split_pair( pbBegin, pbEnd, chSplit, chSplitPair, vector_ );
         return vector_;
      }
      inline std::vector< std::pair<std::string,std::string> > split_pair( const std::string_view& stringText, char chSplit, char chSplitPair, tag_string ) {
         std::vector< std::pair<std::string,std::string> > vector_;
         split_pair( stringText, chSplit, chSplitPair, vector_ );
         return vector_;
      }



      // ## mid methods, select text between something

      /// get mid section and if pair pointer is sent the mark where mid section is extracted from
      std::string mid( const uint8_t* puBegin, const uint8_t* puEnd, uint8_t uStop, std::pair< const uint8_t*, const uint8_t* >* ppairPosition );
      inline std::string mid(const char* pbszBegin, const char* pbszEnd, char chStop, std::pair< const char*, const char* >* ppairPosition) {
         return mid( (const uint8_t*)pbszBegin, (const uint8_t*)pbszEnd, (uint8_t)chStop, (std::pair< const uint8_t*, const uint8_t* >*)ppairPosition );
      }
      inline std::string mid(const std::string_view& stringText, char chStop, std::pair< const char*, const char* >* ppairPosition) {
         return mid( &(*stringText.begin()), stringText.data() + stringText.length(), chStop, ppairPosition );
      }
      inline std::string mid(const std::string_view& stringText, char chStop, std::pair< std::string, std::string >& pairEndings) {
         const char* pbszBegin = stringText.data();
         std::pair< const char*, const char* > pair_;
         std::string stringMid = mid( pbszBegin, pbszBegin + stringText.length(), chStop, &pair_ );
         // ## set front and back string
         pairEndings.first = std::string_view( pbszBegin, pair_.first - pbszBegin );
         pairEndings.second = std::string_view( pair_.second, (pbszBegin + stringText.length()) - pair_.second );
         return stringMid;
      }

      // ## Offset methods, find in string and mark offset positions from start of string

      /// Find offset positions for character in text
      void offset( const char* pbBegin, const char* pbEnd, char chMarkOffset, std::vector<std::size_t>& vectorOffset );
      inline void offset( const std::string_view& stringText, char chMarkOffset, std::vector<std::size_t>& vectorOffset ) {
         offset( &(*stringText.begin()), stringText.data() + stringText.length(), chMarkOffset, vectorOffset);
      }
      inline std::vector<std::size_t> offset( const std::string_view& stringText, char chMarkOffset ) {
         std::vector<std::size_t> vectorOffset;
         offset( &(*stringText.begin()), stringText.data() + stringText.length(), chMarkOffset, vectorOffset);
         return vectorOffset;
      }


      // void split( const std::string_view& stringText, const std::string_view& stringSplit, std::vector<gd::argument::arguments>& vectorPart );

      // ## Print utf8 i various forms

      /// print utf8 buffer as hex to hex buffer
      uint8_t* print_hex( const uint8_t* puText, const uint8_t* puEnd, uint8_t* puHex );
      /// print to string as hex
      void print_hex( const uint8_t* puText, const uint8_t* puEnd, std::string& stsringHex );
   }
}

namespace gd {
   namespace utf8 {
      namespace format {
         void pad_right( std::string& stringText, std::size_t uLength, char chSpace );
         void pad_left( std::string& stringText, std::size_t uLength, char chSpace );

         void to_uuid( const uint8_t* puData, std::string& stringUuid );

         /// replace all find strings passed to method with replace strings and return string with new replaced parts
         std::string replace( const std::string_view& stringText, const std::string_view& stringFind, const std::string_view& stringReplace
         );
      }
   }
}

namespace gd {
   namespace utf8 {
      /// check if text is text and also inform if ascii or utf8 encoded
      bool is_text(const uint8_t* puText, size_t uLength, const char** ppiType = nullptr );
      /// check if text is text and also inform if ascii or utf8 encoded
      inline bool is_text(const char* pbszText, size_t uLength, const char** ppiType = nullptr) {
         return is_text(reinterpret_cast<const uint8_t*>( pbszText ), uLength, ppiType);
      }
      /// check if text is text and also inform if ascii or utf8 encoded
      inline bool is_text(const std::string_view& stringText, const char** ppiType = nullptr) {
         return is_text(reinterpret_cast<const uint8_t*>( stringText.data() ), stringText.length(), ppiType);
      }
   }
}




namespace gd {
   namespace utf8 {
      namespace debug {
         std::string print( const uint8_t* puText, const uint8_t* puEnd, const char* pbszSplit, unsigned uColumnCount );
         inline std::string print( std::string_view stringText ) { return print( reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data() + stringText.length()), " ", 8); }
         inline std::string print( const uint8_t* puText, const uint8_t* puEnd ) { return print( puText, puEnd, " ", 8 ); }
         inline std::string print( const uint8_t* puText, std::size_t uLength ) { return print( puText, puText + uLength, " ", 8 ); }
         std::string print( const void* puText, std::size_t uLength );
      }
   }
}

