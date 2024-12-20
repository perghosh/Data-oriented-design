/**
 * \file gd_parse.h
 * 
 * \brief Core parse logic
 
 * ## Methods
 * - `next_*` methods to move pointer to next section and returns position where section is found
 * - `moveto_*` moves pointer to next position in string, bool is returned if movement was done
 * - `read_*` read value from text into primitive value
 * - `skip_*` skip part in string
 * - `get_line*` read line from text
 * - `read_line*` read text to harvest words in some form from the text
 * - `strchr` find character in text
 * - `strstr` find text part in text
 

 * ## Sample parse methods moving and reading value
 @code
 // Parse methods used to to move and read values
 TEST_CASE( "next and read methods", "[parse]" ) {
   using namespace gd::parse;

   const char* pbszNumber = "0123456789 0123456789 0123456789 0123456789";
   const char* pbszDecimal = "-33.22";

   {
      auto p_ = next_space_g( pbszNumber );                                    REQUIRE( p_ != pbszNumber); REQUIRE( *p_ == ' ' );
      p_ = next_non_space_g( p_ );                                             REQUIRE( *p_ != ' ' );
      p_ = next_character_g( p_, '4' );                                        REQUIRE(*p_ == '4');
      p_ = next_non_integer_g( p_ );                                           REQUIRE(*p_ == ' ');

      int64_t iValue = 0;
      read_int64( pbszNumber, iValue );                                        REQUIRE( iValue == 123456789 );

      double dValue = 0;
      read_double( pbszDecimal, dValue );                                      REQUIRE( dValue == -33.22 );
   }
}
 @endcode

 * ## Read unquoted space separated values from file into table

 @code
 // Read file with two columns containing decimal values, columns are space separated
TEST_CASE( "read msd data", "[parse]" ) {
   using namespace gd::parse;
   using namespace gd::types;

   std::string stringFileName = GetApplicationFolder( "resource/ignore-data/msd-example.data" );
   std::string stringExampleData;
   gd::file::read_file_g( stringFileName, stringExampleData );

   std::vector<gd::variant_view> vectorValue;
   std::vector<unsigned> vectorType = { eTypeCDouble, eTypeCDouble };

   const char* pbszFileData = stringExampleData.c_str();
   const char* pbszFileDataEnd = stringExampleData.c_str() + stringExampleData.length();

   gd::table::table_column_buffer tableMsd( 100 );                             // pre allocate 100 rows
   tableMsd.column_add( { { "double", 0}, { "double", 0} }, gd::table::tag_type_name{} ); // add columns
   tableMsd.prepare();                                                         // prepare table for work

   gd::table::table_column_buffer tableInteger( 100 );                         // pre allocate 100 rows
   tableInteger.column_add( { { "int32", 0}, { "int32", 0} }, gd::table::tag_type_name{} ); // add columns
   tableInteger.prepare();                                                     // prepare table for work

   csv csvRules(" *");                                                         // csv rules, each column is divided with space characters

   auto pbszPosition = pbszFileData;                                           // position on data to be read
   while( pbszPosition < pbszFileDataEnd )                                     // more data to read?
   {
      auto pbszLineEnd = strchr( pbszPosition, '\n' );                         // find end of line
      if( pbszLineEnd != nullptr ) pbszLineEnd++;                              // move to first character on next line
      else { pbszLineEnd = pbszFileDataEnd;  }                                 // no more linefeed characters, last row

      read_line_g( pbszPosition, pbszLineEnd, vectorValue, vectorType, csvRules ); // read line data
      tableMsd.row_add( vectorValue );                                         // add to table where value need to match with type
      tableMsd.row_add( vectorValue, gd::table::tag_convert{});                // add to table where columns do not match with type, converts value before adding
      pbszPosition = pbszLineEnd;

      vectorValue.clear();
   }

   if( tableMsd.empty() == false )
   {
      double dSum0 = 0;
      double dSum1 = 0;
      tableMsd.row_for_each( [&dSum0, &dSum1]( auto& vectorRow, auto uIndex ) -> bool {
         dSum0 += (double)vectorRow[0];
         dSum1 += (double)vectorRow[1];
         return true;
      });

      double dAverage0 = dSum0 / tableMsd.get_row_count();
      double dAverage1 = dSum1 / tableMsd.get_row_count();
   }
}
 @endcode
 * 
 * ## Sample code on how to generate sql queries using table and parameterized sql string
 * 
 @code
// Read file with two columns containing decimal values, columns are space separated
TEST_CASE( "Generate sql", "[table]" ) {
   gd::table::table_column_buffer t( 1 );
   t.column_add( { { "string", 20}, { "int32", 0}, { "int64", 0}, { "int16", 0} }, gd::table::tag_type_name{} );
   t.prepare();
   t.row_add( { "one", 1, 2, 3 }, gd::table::tag_convert{} );
   t.row_add( { "two", 1, 2, 3 }, gd::table::tag_convert{} );

   // ** Write most of the work your self to prepare sql to get a feeling on how it is done **
   if( t.get_row_count() > 0 )
   {
      std::string stringInsertSqlFrame = "INSERT INTO TSample(FString, FNumber1, FNumber2, FNumber3) VALUES(?)"; // sql insert frame to build from
      t.row_for_each( [&stringInsertSqlFrame]( const auto& vectorValue, const auto& iRow ) -> bool { // iterate rows in table
         // ## build value part to insert query
         std::string stringValues;
         for( auto it : vectorValue )
         {
            if( stringValues.empty() == false ) stringValues += ", ";
            gd::sql::append_g( it, stringValues );
         }

         std::string stringInsert; // generated insert query
         const char* pbszCopy = stringInsertSqlFrame.c_str(); // get pointer to insert sql query to finalize       
         while( const char* pbszFind = gd::parse::strchr( pbszCopy, '?', gd::parse::tag_sql{} ) ) // find '?' to be replaced
         {
            stringInsert.append( pbszCopy, pbszFind );
            stringInsert.append( stringValues );
            pbszCopy = pbszFind + 1;                                           // move to character after '?' that was replaced
         }

         // ## append last part
         if( pbszCopy != stringInsertSqlFrame.c_str() )
         {
            stringInsert.append( pbszCopy );
         }

         std::cout << stringInsert << "\n";

         return true;
      });
   }

   // ** Generate sql for selected row **
   if( t.get_row_count() > 0 )
   {
      std::string stringInsert;
      std::string stringInsertSqlFrame = "INSERT INTO TSample(FString, FNumber1, FNumber2, FNumber3) VALUES(?,?,?,?)"; // sql insert frame to build from
      auto vectorValue = t.row_get_variant_view( 0 );                          // get values on row 0
      // ## iterate each '?' and process in callback
      gd::parse::strchr_for_each( stringInsertSqlFrame, [&stringInsert, &vectorValue]( const std::string_view& stringPart, int iIndex ) {
         stringInsert += stringPart;
         if( iIndex != -1 ) gd::sql::append_g( vectorValue[iIndex], stringInsert );
         }, gd::parse::tag_sql{} );

      std::cout << stringInsert << "\n";
   }

   // ** Generate sql for all rows in table **
   if( t.get_row_count() > 0 )
   {
      std::string stringInsertSqlFrame = "INSERT INTO TSample(FString, FNumber1, FNumber2, FNumber3) VALUES(?,?,?,?)"; // sql insert frame to build from
      // iterate each row in table
      t.row_for_each( [&stringInsertSqlFrame]( const auto& vectorValue, const auto& iRow ) -> bool {
         std::string stringInsert; // generated insert query
         // ## iterate each '?' and process in callback
         gd::parse::strchr_for_each( stringInsertSqlFrame, [&stringInsert, &vectorValue]( const std::string_view& stringPart, int iIndex ) {
            stringInsert += stringPart;
            if( iIndex != -1 ) gd::sql::append_g( vectorValue[iIndex], stringInsert );
         }, gd::parse::tag_sql{} );

         std::cout << stringInsert << "\n"; // print insert sql
         return true;
       });
   }
}
 @endcode
 * 
 */

#pragma once

#include <cassert>
#include <cstring>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>


#include "gd_types.h"
#include "gd_arguments.h"
#include "gd_variant_view.h"

#if defined( __clang__ )
   #pragma GCC diagnostic push
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma clang diagnostic ignored "-Wunused-value"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma GCC diagnostic ignored "-Wunused-value"
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning( disable : 4267 26495 26812 )
#endif

#ifndef _GD_PARSE_JSON_BEGIN
#define _GD_PARSE_JSON_BEGIN namespace gd { namespace parse { namespace json {
#define _GD_PARSE_JSON_END } } }
#endif


#ifndef _GD_PARSE_BEGIN
#define _GD_PARSE_BEGIN namespace gd { namespace parse {
#define _GD_PARSE_END } }
_GD_PARSE_BEGIN
#else
_GD_PARSE_BEGIN
#endif

/// tag dispatcher for csv specific overloads
struct tag_csv {};
/// tag dispatcher for sql specific overloads
struct tag_sql {};
/// tag dispatcher for json specific data
struct tag_json {};
/// tag dispatcher for uri formated data
struct tag_uri {};


/// tag dispatcher for methods including zero ending in string
struct tag_zero_end {};

/// find backwards, note that methods going backwards do not check for terminator
struct tag_reverse {};


/// tag dispatcher used to find value in current scope, what scope means depends on text format searched in
struct tag_scope {};


/// tag dispatcher used for wild card match
struct tag_wildcard {};

/// tag dispatcher to use avx256 instructions
struct tag_avx256 {};


// ----------------------------------------------------------------------------
// ---------------------------------------------------------------------- ascii
// ----------------------------------------------------------------------------



/**
 * \brief
 *
 *
 */
struct ascii
{
   enum enumType
   {
/*
      e1          = 0b0000'0001,
      e2          = 0b0000'0010,
      e3          = 0b0000'0100,
      e4          = 0b0000'1000,
      e5          = 0b0001'0000,
      e6          = 0b0010'0000,
      e7          = 0b0100'0000,
      e8          = 0b1000'0000,
      eBegin      = e1,
      eEnd        = e2,
      */
   };

   typedef uint8_t            value_type;
   typedef value_type*        iterator;
   typedef const value_type*  const_iterator;
   typedef value_type*        pointer;
   typedef const value_type*  const_pointer;
   typedef value_type&        reference;
   typedef const value_type&  const_reference;
   typedef size_t             size_type;
   typedef ptrdiff_t          difference_type;

   // ## construction -------------------------------------------------------------

   ascii() { memset( m_puBuffer, 0, sizeof( m_puBuffer ) ); }
   ~ascii() {}

   reference operator[]( int iIndex ) { assert( iIndex >= 0 && iIndex < 0x100 ); return m_puBuffer[iIndex]; }
   const_reference operator[]( int iIndex ) const { assert( iIndex >= 0 && iIndex < 0x100 ); return m_puBuffer[iIndex]; }
   reference operator[]( std::size_t uIndex ) { assert( uIndex < 0x100 ); return m_puBuffer[uIndex]; }
   const_reference operator[]( std::size_t uIndex ) const { assert( uIndex < 0x100 ); return m_puBuffer[uIndex]; }

   /// set ascii character flag
   void set_flag( uint8_t uFlag, value_type uValue ) { m_puBuffer[uValue] |= uFlag; }
   /// set flag for characters
   void set_flag( uint8_t uFlag, const uint8_t* puCaracter, unsigned uLength );
   void set_flag( uint8_t uFlag, const std::string_view& stringCaracter ) { set_flag( uFlag, (const uint8_t*)stringCaracter.data(), (unsigned)stringCaracter.length() ); }
   /// find if ascii character has a value
   bool find( value_type uValue ) const { return m_puBuffer[uValue] != 0; }
   /// reset internal flags, all ascii is set to 0
   void clear() { memset( m_puBuffer, 0, sizeof( m_puBuffer ) ); }

   // ## attributes
   uint8_t m_puBuffer[0x100];			   ///< buffer with ascii to match
   
};



// ----------------------------------------------------------------------------
// ------------------------------------------------------------------------ csv
// ----------------------------------------------------------------------------


/**
 * \brief csv rules on how to parse csv formated text
 *
 *
 */
struct csv
{
   enum enumDelimiter
   {
      eDelimiterMany = 1 << 8,
      eDelimiterMAX = 1 << 9,
   };

   // ## construction -------------------------------------------------------------

   csv(): m_uDelimiter(','), m_uLineEnd('\n'), m_uQuote('\"') {}
   csv( uint8_t uDelimiter ): m_uDelimiter( uDelimiter ), m_uLineEnd('\n'), m_uQuote('\"') {}
   csv( unsigned uDelimiter ): m_uDelimiter( uDelimiter ), m_uLineEnd('\n'), m_uQuote('\"') {}
   csv( const std::string_view& stringDelimiter );
   csv( uint8_t uDelimiter, uint8_t uLineEnd ): m_uDelimiter( uDelimiter ), m_uLineEnd( uLineEnd ), m_uQuote('\"') {}
   ~csv() {}

   uint8_t get_delimiter() const { return uint8_t(m_uDelimiter & 0x000000ff); }
   uint8_t get_lineend() const { return m_uLineEnd; }
   uint8_t get_quote() const { return m_uQuote; }

   bool is_delimiter_single_char() const { return (m_uDelimiter & 0xffffff00) == 0; }
   bool is_quote( uint8_t uQuote ) const { return m_uQuote == uQuote; }

   const uint8_t* next_value( const uint8_t* puPosition, const uint8_t* puEnd ) const;
   const char* next_value( const char* pbszPosition, const char* pbszEnd ) const { return (const char*)( next_value( (const uint8_t*)pbszPosition, (const uint8_t*)pbszEnd ) ); }

   // ## attributes
   unsigned m_uOptions = 0;      ///< flag options like if values should be trimmed
   unsigned m_uDelimiter;        ///< column delimiter
   uint8_t m_uLineEnd;           ///< line endings
   uint8_t m_uQuote;             ///< Quote character
};

// ----------------------------------------------------------------------------
// ------------------------------------------------------------------------ sql
// ----------------------------------------------------------------------------


/**
 * \brief csv rules on how to parse csv formated text
 *
 *
 */
struct sql
{
   enum enumDelimiter
   {
      eDelimiterMany = 1 << 8,
      eDelimiterMAX = 1 << 9,
   };

   // ## construction -------------------------------------------------------------

   sql() { m_puQuote[0] = ('\''); m_puQuote[1] = '\0'; }
   sql( const sql& o ) { m_uOptions = o.m_uOptions; *(uint16_t*)m_puQuote = *(uint16_t*)o.m_puQuote; }
   ~sql() {}

   sql& operator=( const sql& o ) { m_uOptions = o.m_uOptions; *(uint16_t*)m_puQuote = *(uint16_t*)o.m_puQuote; return *this; }

   bool is_quote( uint8_t uQuote ) const { return m_puQuote[0] == uQuote || m_puQuote[1] == uQuote; }

   // ## attributes
   unsigned m_uOptions = 0;      ///< flag options like if values should be trimmed
   uint8_t m_puQuote[2];         ///< Quote character
};

// ----------------------------------------------------------------------------
// ------------------------------------------------------------------------ sql
// ----------------------------------------------------------------------------


/**
 * \brief json rules on how to parse json formated text
 *
 *
 */
struct json_rule
{
   enum enumDelimiter
   {
      eDelimiterMany = 1 << 8,
      eDelimiterMAX = 1 << 9,
   };

   // ## construction -------------------------------------------------------------

   json_rule() { m_puQuote[0] = ('\''); m_puQuote[1] = '\"'; }
   json_rule( const json_rule& o ) { m_uOptions = o.m_uOptions; *(uint16_t*)m_puQuote = *(uint16_t*)o.m_puQuote; }
   ~json_rule() {}

   json_rule& operator=( const json_rule& o ) { m_uOptions = o.m_uOptions; *(uint16_t*)m_puQuote = *(uint16_t*)o.m_puQuote; return *this; }

   bool is_quote( uint8_t uQuote ) const { return m_puQuote[0] == uQuote || m_puQuote[1] == uQuote; }

   // ## attributes
   unsigned m_uOptions = 0;      ///< flag options like if values should be trimmed
   uint8_t m_puQuote[2];         ///< Quote character
};


// ----------------------------------------------------------------------------
// ---------------------------------------------------------------- querystring
// ----------------------------------------------------------------------------


/**
 * \brief querystring is the format of parameters sent in url
 *
 *
 */
struct querystring
{
   // ## construction -------------------------------------------------------------

   querystring() : m_uName{'&'}, m_uValue{'='} {}
   querystring( const querystring& o ) { m_uName = o.m_uName; m_uValue = o.m_uValue; }
   ~querystring() {}

   querystring& operator=( const querystring& o ) { m_uName = o.m_uName; m_uValue = o.m_uValue; return *this; }

   uint8_t get_name() const { return m_uName; }
   uint8_t get_value() const { return m_uValue; }

   bool is_name( char ch ) const { return uint8_t(ch) == m_uName; }
   bool is_value( char ch ) const { return uint8_t(ch) == m_uValue; }

   // ## attributes
   uint8_t m_uName;            ///< character marking start of name
   uint8_t m_uValue;           ///< character marking start of value
};




/**
 * \brief
 *
 *
 */
struct state
{

   /**
    * \brief
    *
    *
    */
   struct match
   {
      // ## construction -------------------------------------------------------------

      match() {}
      ~match() {}

      // ## attributes
      ascii::enumType m_uType;           ///< state type
      std::string m_stringMatch;				   ///< match string to change state
   };


   // ## construction -------------------------------------------------------------

   state() {}
   ~state() {}

   // ## attributes
   ascii m_character;
   std::vector< match > m_vectorMatch;
   
};

// ## next methods (returns if found otherwise same)

const char* next_character_g( const char* pbsz, char chFind );
const char* next_character_g( const char* pbsz, char chFind, tag_reverse );
const char* next_character_g( const char* pbsz, char chFind, tag_zero_end );
const char* next_character_g( const char* pbsz, const char* pbszEnd, char chFind );
inline const char* next_character_g( const char* pbsz, std::size_t uLength, char chFind ) { return next_character_g( pbsz, pbsz + uLength, chFind ); }
inline const char* next_character_g( const std::string_view& stringText, char chFind ) { return next_character_g( stringText.data(), stringText.data() + stringText.length(), chFind); }
const char* next_character_g( const char* pbsz, const char* pbszEnd, char chFind, tag_avx256 );
inline const char* next_character_g( const char* pbsz, std::size_t uLength, char chFind, tag_avx256 ) { return next_character_g( pbsz, pbsz + uLength, chFind, tag_avx256{}); }
inline const char* next_character_g( const std::string_view& stringText, char chFind, tag_avx256 ) { return next_character_g( stringText.data(), stringText.data() + stringText.length(), chFind, tag_avx256{}); }

const char* next_character_g( const char* pbsz, const ascii& characterFind );
const char* next_character_g( const char* pbsz, const char* pbszEnd, const ascii& characterFind );
inline const char* next_character_g( const char* pbsz, std::size_t uLength, const ascii& characterFind ) { return next_character_g( pbsz, pbsz + uLength, characterFind ); }

const char* next_character_or_end_g( const char* pbsz, const char* pbszEnd, char chFind );
inline const char* next_character_or_end_g( const char* pbsz, std::size_t uLength, char chFind ) { return next_character_or_end_g( pbsz, pbsz + uLength, chFind ); }
const char* next_character_or_end_g( const char* pbsz, char chFind, tag_avx256 );


const char* next_space_g( const char* pbsz );
const char* next_space_g( const char* pbsz, const char* pbszEnd );
inline const char* next_space_g( const char* pbsz, std::size_t uLength ) { return next_space_g( pbsz, pbsz + uLength ); }

const char* next_non_decimal_g( const char* pbsz );

const char* next_non_integer_g( const char* pbsz );

const char* next_non_space_g( const char* pbsz );
const char* next_non_space_g( const char* pbsz, const char* pbszEnd );
inline const char* next_non_space_g( const char* pbsz, std::size_t uLength ) { return next_non_space_g( pbsz, pbsz + uLength ); }
const char* next_non_space_or_end_g( const char* pbsz );

// ## align pointer

/// move to next 128 bit boundary
inline const char* next_align128_g( const char* pbsz ) {
   size_t uNext = 16 - (uintptr_t)pbsz % 16;
   if( uNext == 16 ) return pbsz;
   return pbsz + uNext;
}

/// move to next 256 bit boundary
inline const char* next_align256_g( const char* pbsz ) {
   size_t uNext = 32 - (uintptr_t)pbsz % 32;
   if( uNext == 32 ) return pbsz;
   return pbsz + uNext;
}


// const char* next_comment_g( const char* pbsz, const ascii& characterFind, const std::string_view& stringBegin, const std::string_view& stringEnd );

// ## moveto methods

/// move pointer to `find` character
inline bool moveto_character_g( const char*& pbsz, char chFind ) {
   const char* p_ = pbsz;  pbsz = next_character_g( p_, chFind ); return (pbsz != p_);
}
/// move pointer to nth `find` character
inline bool moveto_character_g( const char*& pbsz, char chFind, unsigned uCount ) {
   int iIndex = (int)uCount;
   while( iIndex >= 0 ) {
      const char* p_ = pbsz;  pbsz = next_character_g( p_, chFind ); 
      if( iIndex == 0 ) return (pbsz != p_);                                   // at the 0 nth character, return if character is found
      else if( *pbsz == '\0' ) return false;
      iIndex--;                                                                // decrease number for characters to find
      pbsz++;                                                                  // move past find character
   }
   return false;
}
/// move pointer to nth `find` character
inline bool moveto_character_g( const char*& pbsz, char chFind, unsigned uCount, tag_zero_end ) {
   int iIndex = (int)uCount;
   while( iIndex >= 0 ) {
      const char* p_ = pbsz;  pbsz = next_character_g( p_, chFind, tag_zero_end{});
      if( iIndex == 0 ) return (pbsz != p_ || *pbsz == '\0' );                 // at the 0 nth character, return if character is found
      else if( *pbsz == '\0' ) return false;
      iIndex--;                                                                // decrease number for characters to find
      pbsz++;                                                                  // move past find character
   }
   return false;
}

inline bool moveto_character_g( const char*& pbsz, std::size_t uLength, char chFind ) {
   const char* p_ = pbsz;  pbsz = next_character_g( p_, uLength, chFind ); return (pbsz != p_);
}
inline bool moveto_character_g( const char*& pbsz, const char* pbszEnd, char chFind ) {
   auto uLength = pbszEnd - pbsz;                                                                  assert( uLength < 0x0100'0000 ); // realistic
   const char* p_ = pbsz;  pbsz = next_character_g( p_, uLength, chFind ); return (pbsz != p_);
}
inline bool moveto_character_g( const char*& pbsz, const ascii& characterFind ) {
   const char* p_ = pbsz;  pbsz = next_character_g( p_, characterFind ); return (pbsz != p_);
}




// ## read methods - reads

const char* read_boolean_g( const char* pbsz, bool& bValue );
const char* read_boolean_g( const char* pbsz, const char* pbszEnd, bool& bValue );
const char* read_int32_g( const char* pbsz, int32_t& iValue );
const char* read_int64_g( const char* pbsz, int64_t& iValue );
const char* read_int64_g( const char* pbsz, const char* pbszEnd, int64_t& iValue );
const char* read_double_g( const char* pbsz, double& dValue );
const char* read_quoted_g( const char* pbsz, const char* pbszEnd, std::string_view& stringValue );

unsigned read_type_g( const char* pbsz, size_t uLength, const unsigned* puCheckType );
inline const unsigned read_type_g( const std::string_view& stringValue, unsigned* puCheckType ) { return read_type_g( stringValue.data(), stringValue.length(), puCheckType ); }
inline const unsigned read_type_g( const std::string_view& stringValue, std::vector<unsigned> vectorType ) { return read_type_g( stringValue.data(), stringValue.length(), vectorType.data() ); }

// ## skip methods - moves position over characters

const char* skip_alnum_g( const char* pbsz );
const char* skip_alnum_g( const char* pbsz, const char* pbszEnd );
const char* skip_decimal_g( const char* pbsz );
const char* skip_decimal_g( const char* pbsz, const char* pbszEnd );
const char* skip_integer_g( const char* pbsz );
const char* skip_integer_g( const char* pbsz, const char* pbszEnd );
const char* skip_space_g( const char* pbsz );
const char* skip_space_g( const char* pbsz, const char* pbszEnd );
const char* skip_character_g( const char* pbsz, const char* pbszEnd, char chSkip );

const char* skip_string_g( const char* pbsz );
inline const uint8_t* skip_string_g( const uint8_t* pu ) { return (const uint8_t*)skip_string_g( (const char*)pu ); }
const char* skip_string_g( const char* pbsz, tag_json );
const char* skip_string_g( const char* pbsz, const char* pbszEnd, tag_json );

const char* skip_quoted_g( const char* pbsz );
const char* skip_quoted_g( const char* pbsz, const char* pbszEnd );

const char* skip_escaped_g( const char* pbsz );
const char* skip_escaped_g( const char* pbsz, const char* pbszEnd );

const char* skip_wildcard_g( const char* pbsz, const char* pbszEnd, const char* pbszWildcard, unsigned uLength );
inline const char* skip_wildcard_g( const char* pbsz, const char* pbszEnd, const std::string_view& stringMatch ) { return skip_wildcard_g( pbsz, pbszEnd, stringMatch.data(), stringMatch.length() ); }

/// Reads text line and extracts all words (separated by spaces)
std::pair<bool, const char*> read_line_g( const char* pbsz, const char* pbszEnd, std::vector<std::string_view>& vectorWord );
inline std::pair<bool, const char*> read_line_g( const std::string_view& stringLine, std::vector<std::string_view>& vectorWord ) { return read_line_g( stringLine.data(), stringLine.data() + stringLine.length(), vectorWord ); }

std::pair<bool, const char*> read_line_g( const char* pbsz, const char* pbszEnd, std::vector<std::string>& vectorWord );
inline std::pair<bool, const char*> read_line_g( const std::string_view& stringLine, std::vector<std::string>& vectorWord ) { return read_line_g( stringLine.data(), stringLine.data() + stringLine.length(), vectorWord ); }

/// Reads text line and extracts all words enclosed in start and end characters specified
std::pair<bool, const char*> read_line_g( const char* pbsz, const char* pbszEnd, char chBegin, char chEnd, std::vector<std::string_view>& vectorWord );
inline std::pair<bool, const char*> read_line_g(const std::string_view& stringLine, char chBeginMarker, char chEndMarker, std::vector<std::string_view>& vectorWord) {
   return read_line_g( &(*stringLine.begin()), &(*stringLine.begin()) + stringLine.length(), chBeginMarker, chEndMarker, vectorWord );
}

/// Write text to line and replace all words enclosed in start and end characters specified
std::pair<bool, const char*> write_line_g( const char* pbsz, const char* pbszEnd, char chBegin, char chEnd, const argument::arguments& argumentsValue, std::string& stringTo );
inline std::pair<bool, const char*> write_line_g(const std::string_view& stringLine, char chBeginMarker, char chEndMarker, const gd::argument::arguments& argumentsValue, std::string& stringTo ) {
   return write_line_g( &(*stringLine.begin()), &(*stringLine.begin()) + stringLine.length(), chBeginMarker, chEndMarker, argumentsValue, stringTo);
}
/// write to string that is returned, note that this doesn't check for errors in release mode
inline std::string write_line_g(const std::string_view& stringLine, char chBeginMarker, char chEndMarker, const gd::argument::arguments& argumentsValue ) {
   std::string stringTo;
#ifndef NDEBUG
   auto result_ = write_line_g( &(*stringLine.begin()), &(*stringLine.end()), chBeginMarker, chEndMarker, argumentsValue, stringTo ); assert( result_.first );
#else
   write_line_g( &(*stringLine.begin()), &(*stringLine.end()), chBeginMarker, chEndMarker, argumentsValue, stringTo );
#endif
   return stringTo;
}




// ## CSV

/// read line values into vector with variant view objects
std::pair<bool, const char*> read_line_g( const char* pbsz, const char* pbszTextEnd, std::vector<gd::variant_view>& vectorValue, std::vector<unsigned> vectorType, const csv& csv, std::function<bool( gd::variant_view&, unsigned )> callback_ );
inline std::pair<bool, const char*> read_line_g( const char* pbsz, const char* pbszEnd, std::vector<gd::variant_view>& vectorValue, std::vector<unsigned> vectorType, const csv& csv ) {
   return read_line_g( pbsz, pbszEnd, vectorValue, vectorType, csv, nullptr );
}
/// Read csv line into vector of strings
std::pair<bool, const char*> read_line_g( const char* pbsz, const char* pbszEnd, std::vector<std::string>& vectorValue, const csv& csv );
inline std::pair<bool, const char*> read_line_g(const std::string_view& stringText, std::vector<std::string>& vectorValue, const csv& csv) {
   return read_line_g( stringText.data(), stringText.data() + stringText.length(), vectorValue, csv );
}

const char* get_line_g( const char* pbsz, const char* pbszEnd, std::string_view& stringLine, const char* pbszNewLine );
inline const char* get_line_g( const std::string_view& stringText, std::string_view& stringLine, const char* pbszNewLine ) { return get_line_g( stringText.data(), stringText.data() + stringText.length(), stringLine, pbszNewLine ); }
const char* get_line_g( const char* pbsz, const char* pbszEnd, std::string_view& stringLine, const char* pszbNewLine, const csv& csv );
inline const char* get_line_g(const std::string_view& stringText, std::string_view& stringLine, const char* pszbNewLine, const csv& csv) {
   return get_line_g( stringText.data(), stringText.data() + stringText.length(), stringLine, pszbNewLine, csv );
}

const char* get_line_g( const char* pbsz, const char* pbszEnd, std::string_view& stringLine, const csv& csv );
inline std::string_view get_line_g(const std::string_view& stringtext, const csv& csv) {
   std::string_view string_; 
   get_line_g( stringtext.data(), stringtext.data() + stringtext.length(), string_, csv);
   return string_;
}


// ## strchr methods mimics the c variant of strchr but has rules on how to find character
const char* strchr( const char* pbszText, char chFind );
inline const uint8_t* strchr( const uint8_t* puText, char chFind ) { return (const uint8_t*)strchr( (const char*)puText, chFind ); }
const char* strchr( const char* pbszText, const char* pbszEnd, char chFind );
const char* strchr( const char* pbszText, char chFind, const csv& csv, const uint8_t* puCharacterClass );
inline const char* strchr( const char* pbszText, char chFind, const csv& csv ) { return strchr( pbszText, chFind, csv, nullptr ); }
inline const char* strchr( const char* pbszText, char chFind, const uint8_t* puCharacterClass ) { return strchr( pbszText, chFind, csv(), puCharacterClass ); }
const char* strchr( const char* pbszBegin, const char* pbszEnd, char chFind, const csv& csv, const uint8_t* puCharacterClass );
//inline const char* strchr( const char* pbszBegin, const char* pbszEnd, char chFind ) { return strchr( pbszBegin, pbszEnd, chFind, csv(), nullptr ); }
inline const char* strchr( const char* pbszBegin, const char* pbszEnd, char chFind, const csv& csv ) { return strchr( pbszBegin, pbszEnd, chFind, csv, nullptr ); }
inline const char* strchr( const char* pbszBegin, const char* pbszEnd, char chFind, const uint8_t* puCharacterClass ) { return strchr( pbszBegin, pbszEnd, chFind, csv(), puCharacterClass ); }

const char* strchr( const char* pbszBegin, const char* pbszEnd, char chFind, char chScopeEnd, tag_scope );

// ## SQL

// ### Find character within sql formated string (without comments)

const char* strchr( const char* pbszText, char chFind, const sql& sql, const uint8_t* puCharacterClass );
inline const char* strchr( const char* pbszText, char chFind, tag_sql ) { return strchr( pbszText, chFind, sql(), nullptr ); }
inline const char* strchr( const char* pbszText, char chFind, const sql& sql ) { return strchr( pbszText, chFind, sql, nullptr ); }
inline const char* strchr( const char* pbszText, char chFind, const uint8_t* puCharacterClass, tag_sql ) { return strchr( pbszText, chFind, sql(), puCharacterClass ); }

const char* strchr( const char* pbszBegin, const char* pbszEnd, char chFind, const sql& sql, const uint8_t* puCharacterClass );
inline const char* strchr( const char* pbszBegin, const char* pbszEnd, char chFind, tag_sql ) { return strchr( pbszBegin, pbszEnd, chFind, sql(), nullptr ); }
inline const char* strchr( const char* pbszBegin, const char* pbszEnd, char chFind, const sql& sql ) { return strchr( pbszBegin, pbszEnd, chFind, sql, nullptr ); }
inline const char* strchr( const char* pbszBegin, const char* pbszEnd, char chFind, const uint8_t* puCharacterClass, tag_sql ) { return strchr( pbszBegin, pbszEnd, chFind, sql(), puCharacterClass ); }

// ## JSON

// ### Find character within json formated string (without comments)
const char* strchr( const char* pbszText, char chFind, const json_rule& json_rule, const uint8_t* puCharacterClass );
inline const char* strchr( const char* pbszText, char chFind, const json_rule& json_rule ) { return strchr( pbszText, chFind, json_rule, nullptr ); }

const char* strchr( const char* pbszBegin, const char* pbszEnd, char chFind, const json_rule& json_rule, const uint8_t* puCharacterClass );
inline const char* strchr( const std::string_view& stringJson, char chFind, const json_rule& json_rule, const uint8_t* puCharacterClass ) { return strchr( stringJson.data(), stringJson.data() + stringJson.length(), chFind, json_rule, puCharacterClass ); }
inline const char* strchr( const std::string_view& stringJson, char chFind, const json_rule& json_rule ) { return strchr( stringJson.data(), stringJson.data() + stringJson.length(), chFind, json_rule, nullptr ); }

// ## Iterator for string to replace or process multiple positions for selected character

void strchr_for_each( const std::string_view& stringSql, char chFind, const sql& sql, const uint8_t* puCharacterClass, std::function<const char* ( const std::string_view&, int )> callback_ );
inline void strchr_for_each( const std::string_view& stringSql, std::function<const char* ( const std::string_view&, int )> callback_, tag_sql ) { strchr_for_each( stringSql, '?', sql(), nullptr, callback_ ); }
inline void strchr_for_each( const std::string_view& stringSql, char chFind, std::function<const char* ( const std::string_view&, int )> callback_, tag_sql ) { strchr_for_each( stringSql, chFind, sql(), nullptr, callback_ ); }
inline void strchr_for_each( const std::string_view& stringSql, char chFind, const sql& sql, std::function<const char* ( const std::string_view&, int )> callback_ ) { strchr_for_each( stringSql, chFind, sql, nullptr, callback_ ); }



void strstr_for_each( const std::string_view& stringSql, const char* pbszFind, unsigned uLength, const sql& sql, const uint8_t* puCharacterClass, std::function<void ( const std::string_view&, int )> callback_ );
inline void strstr_for_each( const std::string_view& stringSql, const std::string_view& stringFind, std::function<void ( const std::string_view&, int )> callback_, tag_sql ) { strstr_for_each( stringSql, stringFind.data(), stringFind.length(), sql(), nullptr, callback_ ); }

// ## QueryString

/// read line values into vector with pair of string_view and variant_view for each value
std::pair<bool, const char*> read_line_g( const char* pbsz, const char* pbszEnd, std::vector<std::pair<std::string_view,std::string_view>>& vectorValue, const querystring& querystring );
inline std::pair<bool, const char*> read_line_g( const std::string_view& stringQueryString, std::vector<std::pair<std::string_view,std::string_view>>& vectorValue, const querystring& querystring ) { return read_line_g( stringQueryString.data(), stringQueryString.data() + stringQueryString.length(), vectorValue, querystring ); }
std::pair<bool, const char*> read_line_g( 
   const char* pbsz, 
   const char* pbszEnd, 
   std::vector<std::pair<std::string,std::string>>& vectorValue, 
   const querystring& querystring, 
   std::function< std::pair< std::string, std::string >( std::string_view, std::string_view )> format_ );
inline std::pair<bool, const char*> read_line_g( const std::string_view& stringQueryString, std::vector<std::pair<std::string,std::string>>& vectorValue, const querystring& querystring, std::function< std::pair< std::string, std::string >( std::string_view, std::string_view )> format_ ) { 
   return read_line_g( stringQueryString.data(), stringQueryString.data() + stringQueryString.length(), vectorValue, querystring, format_ ); 
}

inline std::pair<bool, const char*> read_line_g(const std::string_view& stringQueryString, std::vector<std::pair<std::string, std::string>>& vectorValue, const querystring& querystring) {
   return read_line_g(stringQueryString.data(), stringQueryString.data() + stringQueryString.length(), vectorValue, querystring, nullptr);
}

// ## strstr methods, similar to c `strstr` but ignores string parts in text

const char* strstr( const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const uint8_t* puCharacterClass );
const char* strstr( const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const csv& csv, const uint8_t* puCharacterClass );
const char* strstr( const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const sql& sql, const uint8_t* puCharacterClass );
const char* strstr( const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const sql& sql, const uint8_t* puCharacterClass, tag_wildcard );

const char* strstr( const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const json_rule& json_rule, const uint8_t* puCharacterClass );
inline const char* strstr(const char* pbszBegin, const char* pbszEnd, const std::string_view& stringFind, const json_rule& json_rule, const uint8_t* puCharacterClass) { return strstr( pbszBegin, pbszEnd, stringFind.data(), stringFind.size(), json_rule, puCharacterClass ); }
inline const char* strstr(const std::string_view& stringText, const std::string_view& stringFind, const json_rule& json_rule, const uint8_t* puCharacterClass) { return strstr( stringText.data(), stringText.data() + stringText.size(), stringFind.data(), stringFind.size(), json_rule, puCharacterClass); }
inline const char* strstr(const char* pbszBegin, const char* pbszEnd, const std::string_view& stringFind, const json_rule& json_rule) { return strstr( pbszBegin, pbszEnd, stringFind.data(), stringFind.size(), json_rule, nullptr ); }
inline const char* strstr(const std::string_view& stringText, const std::string_view& stringFind, const json_rule& json_rule) { return strstr( stringText.data(), stringText.data() + stringText.size(), stringFind.data(), stringFind.size(), json_rule, nullptr); }


// ## strcpy methods, similar to c `strcpy` with extra functionality

const char* strcpy( const char* pbszText, std::string& stringCopy, char chCopyTo );
const char* strcpy( const char* pbszText, std::string& stringCopy, const std::vector<char>& vectorCopyTo );

// ## split methods
void split_g( const std::string_view& stringText, const std::string_view& stringSplit, std::vector<std::string>& vectorPart, const csv& csv );


// ## Escape methods, escapes string characters for different formats

void escape_g( std::string& stringText, tag_csv );
void escape_g( std::string& stringText, tag_sql );

_GD_PARSE_END


// ----------------------------------------------------------------------------
// ------------------------------------------------------ JSON specific methods
// ----------------------------------------------------------------------------


_GD_PARSE_JSON_BEGIN

// ## move to next key in json

const char* next_key_g( const char* pbszJson, const char* pbszName, unsigned uNameLength, int* piLevel );
inline const char* next_key_g( const char* pbszJson, const char* pbszName, unsigned uNameLength ) { return next_key_g( pbszJson, pbszName, uNameLength, nullptr ); }
inline const char* next_key_g( const char* pbszJson, const std::string_view& stringKey ) { return next_key_g( pbszJson, stringKey.data(), stringKey.length() ); }
inline const char* next_key_g( const char* pbszJson, const std::string_view& stringKey, int* piLevel ) { return next_key_g( pbszJson, stringKey.data(), stringKey.length(), piLevel ); }

const char* next_key_g( const char* pbszJson, const char* pbszName, unsigned uNameLength, tag_scope );
inline const char* next_key_g( const char* pbszJson, const std::string_view& stringKey, tag_scope ) { return next_key_g( pbszJson, stringKey.data(), stringKey.length(), tag_scope{}); }



/// goto value for key, pointer need to be at key
const char* find_key_value_g( const char* pbszKey );

void read_value( const char* pbszJson, gd::variant_view& variantviewValue );
const char* read_key_value_g( const char* pbszKey, gd::variant_view& variantviewValue );
gd::variant_view read_key_value_g( const char* pbszKey );

_GD_PARSE_JSON_END


#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
