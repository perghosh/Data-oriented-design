// @FILE [tag: table, aggregate] [description: Calculate aggregate values for tables] [type: header] [name: gd_table_aggregate.h]

/**
 * \file gd_table_aggregate.h
 * 
 * \brief Aggregate logic for tables
 * 
 * 
 * 
 * 
 * 
 */

#pragma once

// ## undef min and/or max if these macros are defined (never use min or max macros)
#if defined(min) || defined(max)
# undef min
# undef max
#endif



#include <cassert>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <unordered_set>
#include <algorithm>

#include "gd/gd_table.h"

#ifndef _GD_TABLE_BEGIN
#  define _GD_TABLE_BEGIN namespace gd { namespace table { 
#  define _GD_TABLE_END } }
#endif

_GD_TABLE_BEGIN

 /**
  * \brief aggregate is used to aggregate data from table classes
  *
  * Methods similar to aggregate functions in sql databases are found here.  
  * sum, max, min, average etc.
  *
  \code
  \endcode
  */
template <typename TABLE>
class aggregate
{
   // ## construction -------------------------------------------------------------
public:
   aggregate() : m_ptable{ nullptr } {}
   aggregate( const TABLE* ptable ) : m_ptable{ ptable } {}
   // copy
   aggregate( const aggregate& o ) { common_construct( o ); }
   aggregate( aggregate&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   aggregate& operator=( const aggregate& o ) { common_construct( o ); return *this; }
   aggregate& operator=( aggregate&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~aggregate() {}
private:
   // common copy
   void common_construct( const aggregate& o ) {}
   void common_construct( aggregate&& o ) noexcept {}

   // ## operator -----------------------------------------------------------------
public:


   // ## methods ------------------------------------------------------------------
public:
   /** \name GET/SET
   *///@{

   //@}

   /** \name OPERATION
   *///@{

   // ## min operation - find minimum values

   template<typename TYPE>
   TYPE min( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const;
   template<typename TYPE>
   TYPE min( unsigned uColumn ) const { return min<TYPE>( uColumn, 0, m_ptable->get_row_count() ); }
   template<typename TYPE>
   TYPE min( const std::string_view& stringName ) const { return min<TYPE>( m_ptable->column_get_index( stringName ), 0, m_ptable->get_row_count() ); }
   template<typename TYPE>
   TYPE min( const std::string_view& stringName, uint64_t uBeginRow, uint64_t uCount ) const { return min<TYPE>( m_ptable->column_get_index( stringName ), uBeginRow, uCount ); }


   // ## max operation calculating size in bytes each value needs related to read it as text

   unsigned max( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount, tag_length ) const;
   unsigned max( unsigned uColumn, tag_length ) const { return max( uColumn, 0, m_ptable->get_row_count(), tag_length{}); }
   unsigned max( const std::string_view& stringName, tag_length ) const { return max( m_ptable->column_get_index( stringName ), 0, m_ptable->get_row_count(), tag_length{}); }
   unsigned max( const std::string_view& stringName, uint64_t uBeginRow, uint64_t uCount, tag_length ) const { return max( m_ptable->column_get_index( stringName ), uBeginRow, uCount, tag_length{}); }

   void max( std::vector<unsigned>& vectorLength, uint64_t uBeginRow, uint64_t uCount, tag_length );
   void max( std::vector<unsigned>& vectorLength, tag_length ) { max( vectorLength, 0, m_ptable->get_row_count(), tag_length{} ); }
   void max( std::vector<unsigned>& vectorLength, uint64_t uBeginRow, uint64_t uCount, const std::vector<unsigned>& vectorColumn, tag_length );

   void max( std::vector<unsigned>& vectorLength, tag_length, tag_text) { max(vectorLength, 0, m_ptable->get_row_count(), tag_length{}, tag_text{}); }
   void max( std::vector<unsigned>& vectorLength, uint64_t uBeginRow, uint64_t uCount, tag_length, tag_text );

   template<typename TYPE>
   TYPE sum( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const;
   template<typename TYPE>
   TYPE sum( unsigned uColumn ) const { return sum<TYPE>( uColumn, 0, m_ptable->get_row_count() ); }

   // ## average operation
   template<typename TYPE>
   double average( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const;
   template<typename TYPE>
   double average( unsigned uColumn ) const { return average<TYPE>( uColumn, 0, m_ptable->get_row_count() ); }
   template<typename TYPE>
   double average( const std::string_view& stringName ) const { return average<TYPE>( m_ptable->column_get_index( stringName ), 0, m_ptable->get_row_count() ); }
   template<typename TYPE>
   double average( const std::string_view& stringName, uint64_t uBeginRow, uint64_t uCount ) const { return average<TYPE>( m_ptable->column_get_index( stringName ), uBeginRow, uCount ); }

   // ## count operations
   uint64_t count( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const;
   uint64_t count( unsigned uColumn ) const { return count( uColumn, 0, m_ptable->get_row_count() ); }
   uint64_t count( const std::string_view& stringName ) const { return count( m_ptable->column_get_index( stringName ), 0, m_ptable->get_row_count() ); }
   uint64_t count( const std::string_view& stringName, uint64_t uBeginRow, uint64_t uCount ) const { return count( m_ptable->column_get_index( stringName ), uBeginRow, uCount ); }

   uint64_t count_not_null( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const;
   uint64_t count_not_null( unsigned uColumn ) const { return count_not_null( uColumn, 0, m_ptable->get_row_count() ); }
   uint64_t count_not_null( const std::string_view& stringName ) const { return count_not_null( m_ptable->column_get_index( stringName ), 0, m_ptable->get_row_count() ); }
   uint64_t count_not_null( const std::string_view& stringName, uint64_t uBeginRow, uint64_t uCount ) const { return count_not_null( m_ptable->column_get_index( stringName ), uBeginRow, uCount ); }

   uint64_t count_null( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const;
   uint64_t count_null( unsigned uColumn ) const { return count_null( uColumn, 0, m_ptable->get_row_count() ); }
   uint64_t count_null( const std::string_view& stringName ) const { return count_null( m_ptable->column_get_index( stringName ), 0, m_ptable->get_row_count() ); }
   uint64_t count_null( const std::string_view& stringName, uint64_t uBeginRow, uint64_t uCount ) const { return count_null( m_ptable->column_get_index( stringName ), uBeginRow, uCount ); }

   // ## unique count operation
   uint64_t count_unique( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const;
   uint64_t count_unique( unsigned uColumn ) const { return count_unique( uColumn, 0, m_ptable->get_row_count() ); }
   uint64_t count_unique( const std::string_view& stringName ) const { return count_unique( m_ptable->column_get_index( stringName ), 0, m_ptable->get_row_count() ); }
   uint64_t count_unique( const std::string_view& stringName, uint64_t uBeginRow, uint64_t uCount ) const { return count_unique( m_ptable->column_get_index( stringName ), uBeginRow, uCount ); }

   // ## variance and standard deviation operations
   template<typename TYPE>
   double variance( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const;
   template<typename TYPE>
   double variance( unsigned uColumn ) const { return variance<TYPE>( uColumn, 0, m_ptable->get_row_count() ); }
   template<typename TYPE>
   double variance( const std::string_view& stringName ) const { return variance<TYPE>( m_ptable->column_get_index( stringName ), 0, m_ptable->get_row_count() ); }
   template<typename TYPE>
   double variance( const std::string_view& stringName, uint64_t uBeginRow, uint64_t uCount ) const { return variance<TYPE>( m_ptable->column_get_index( stringName ), uBeginRow, uCount ); }

   template<typename TYPE>
   double std_deviation( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const;
   template<typename TYPE>
   double std_deviation( unsigned uColumn ) const { return std_deviation<TYPE>( uColumn, 0, m_ptable->get_row_count() ); }
   template<typename TYPE>
   double std_deviation( const std::string_view& stringName ) const { return std_deviation<TYPE>( m_ptable->column_get_index( stringName ), 0, m_ptable->get_row_count() ); }
   template<typename TYPE>
   double std_deviation( const std::string_view& stringName, uint64_t uBeginRow, uint64_t uCount ) const { return std_deviation<TYPE>( m_ptable->column_get_index( stringName ), uBeginRow, uCount ); }

   // ## median and percentile operations
   template<typename TYPE>
   TYPE median( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const;
   template<typename TYPE>
   TYPE median( unsigned uColumn ) const { return median<TYPE>( uColumn, 0, m_ptable->get_row_count() ); }
   template<typename TYPE>
   TYPE median( const std::string_view& stringName ) const { return median<TYPE>( m_ptable->column_get_index( stringName ), 0, m_ptable->get_row_count() ); }
   template<typename TYPE>
   TYPE median( const std::string_view& stringName, uint64_t uBeginRow, uint64_t uCount ) const { return median<TYPE>( m_ptable->column_get_index( stringName ), uBeginRow, uCount ); }

   template<typename TYPE>
   TYPE percentile( unsigned uColumn, double dPercentile, uint64_t uBeginRow, uint64_t uCount ) const;
   template<typename TYPE>
   TYPE percentile( unsigned uColumn, double dPercentile ) const { return percentile<TYPE>( uColumn, dPercentile, 0, m_ptable->get_row_count() ); }
   template<typename TYPE>
   TYPE percentile( const std::string_view& stringName, double dPercentile ) const { return percentile<TYPE>( m_ptable->column_get_index( stringName ), dPercentile, 0, m_ptable->get_row_count() ); }
   template<typename TYPE>
   TYPE percentile( const std::string_view& stringName, double dPercentile, uint64_t uBeginRow, uint64_t uCount ) const { return percentile<TYPE>( m_ptable->column_get_index( stringName ), dPercentile, uBeginRow, uCount ); }

   // ## string-specific operations
   unsigned count_contains( unsigned uColumn, const std::string_view& stringPattern, uint64_t uBeginRow, uint64_t uCount ) const;
   unsigned count_contains( unsigned uColumn, const std::string_view& stringPattern ) const { return count_contains( uColumn, stringPattern, 0, m_ptable->get_row_count() ); }
   unsigned count_contains( const std::string_view& stringName, const std::string_view& stringPattern ) const { return count_contains( m_ptable->column_get_index( stringName ), stringPattern, 0, m_ptable->get_row_count() ); }
   unsigned count_contains( const std::string_view& stringName, const std::string_view& stringPattern, uint64_t uBeginRow, uint64_t uCount ) const { return count_contains( m_ptable->column_get_index( stringName ), stringPattern, uBeginRow, uCount ); }

   
   std::vector<gd::variant_view> unique( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const;
   std::vector<gd::variant_view> unique( unsigned uColumn ) const { return unique(uColumn, 0, m_ptable->get_row_count()); }
   std::vector<gd::variant_view> unique( const std::string_view& stringName, uint64_t uBeginRow, uint64_t uCount ) const { return unique(m_ptable->column_get_index(stringName), uBeginRow, uCount); }
   std::vector<gd::variant_view> unique( const std::string_view& stringName ) const { return unique(m_ptable->column_get_index(stringName), 0, m_ptable->get_row_count()); }


   // ## fix operations performs specific task to handle edge cases

   void fix( std::vector<unsigned>& vectorLength, tag_text );

   //@}

protected:
   /** \name INTERNAL
   *///@{

   //@}

public:
   /** \name DEBUG
   *///@{

   //@}


   // ## attributes ----------------------------------------------------------------
public:
   const TABLE* m_ptable;

   // ## free functions ------------------------------------------------------------
public:



};

template<typename TABLE>
unsigned max( const TABLE& t_, unsigned uColumn, tag_length ) { return aggregate( &t_ ).max( uColumn, tag_length{}); }
template<typename TABLE>
unsigned max( const TABLE& t_, unsigned uColumn, uint64_t uBeginRow, uint64_t uCount, tag_length ) { return aggregate( &t_ ).max( uColumn, uBeginRow, uCount, tag_length{}); }
template<typename TABLE>
unsigned max( const TABLE& t_, const std::string_view& stringName, tag_length ) { return aggregate( &t_ ).max( stringName, tag_length{}); }
template<typename TABLE>
unsigned max( const TABLE& t_, const std::string_view& stringName, uint64_t uBeginRow, uint64_t uCount, tag_length ) { return aggregate( &t_ ).max( stringName, uBeginRow, uCount, tag_length{}); }


template<typename TYPE, typename TABLE>
TYPE sum( const TABLE& t_, unsigned uColumn ) { return aggregate( &t_ ).template sum<TYPE>( uColumn ); }
template<typename TYPE, typename TABLE>
TYPE sum( const TABLE& t_, unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) { return aggregate( &t_ ).template sum<TYPE>( uColumn, uBeginRow, uCount ); }


/** ---------------------------------------------------------------------------
 * @brief Find minimum value in specified column range
 * @param uColumn index to column to find minimum value in
 * @param uBeginRow start row to check values from
 * @param uCount number of rows from start row
 * @return minimum value found in column within specified range
 */
template<typename TABLE>
template<typename TYPE>
TYPE aggregate<TABLE>::min( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const {        assert( m_ptable != nullptr );
   uint64_t uEndRow = uBeginRow + uCount;
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count(); }

   auto eType = gd::types::type_g<TYPE>( gd::types::tag_ask_compiler{});
   auto uColumnType = m_ptable->column_get_ctype( uColumn );
   bool bHasNull = m_ptable->is_null();
   bool bInitialized = false;
   TYPE min_{};

   if( (( unsigned )eType & 0xff) == (uColumnType & 0xff) ) {
      for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) {
         if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue;
         TYPE value = (TYPE)m_ptable->cell_get_variant_view( uRow, uColumn );
         if( bInitialized == false || value < min_ ) {
            min_ = value;
            bInitialized = true;
         }
      }
   }
   else {
      for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) {
         if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue;
         gd::variant variantConvertTo;
         auto variantviewValue = m_ptable->cell_get_variant_view( uRow, uColumn );
         bool bOk = variantviewValue.convert_to( eType, variantConvertTo );
         if( bOk == true ) {
            TYPE value = (TYPE)variantConvertTo;
            if( bInitialized == false || value < min_ ) {
               min_ = value;
               bInitialized = true;
            }
         }
      }
   }
   return min_;
}

/** ---------------------------------------------------------------------------
 * @brief count max number of ascii characters needed for column
 * @param uColumn index to column max number of ascii characters value needs
 * @param uBeginRow start row to check values on
 * @param uCount number of rows from start row
 * @return unsigned count in ascii characters for value in column that needs most ascii characters
*/
template <typename TABLE>
unsigned aggregate<TABLE>::max( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount, tag_length ) const { assert( m_ptable != nullptr );
   unsigned uLength = 0;                  // max value length in bytes for max value found in column
   uint64_t uEndRow = uBeginRow + uCount; // last row where value is checked

   // ## iterate rows to check max length for values found in column

   bool bHasNull = m_ptable->is_null(); // get if table has null values
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count(); }
   for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) {
      if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue; // skip null values

      unsigned uValueLength = m_ptable->cell_get_length( uRow, uColumn );
      if( uValueLength <= uLength ) continue;
      uLength = uValueLength;
   }
   return uLength;
}


/** ---------------------------------------------------------------------------
 * @brief Calculates the maximum length of values in all columns for table or valid entries in vector.
 * 
 * This method iterates through the specified range of rows and columns in the table,
 * determining the maximum length of the values found. It updates the provided
 * vectorLength with the maximum lengths for each column.
 * 
 * @tparam TABLE The table type being aggregated.
 * @param vectorLength A reference to a vector that will hold the maximum lengths for each column.
 *                     If empty, it will be resized to match the number of columns in the table.
 * @param uBeginRow The starting row index for the range to check.
 * @param uCount The number of rows to include in the range.
 * @param tag_length A tag indicating that this is a length operation.
 * 
 * @note If the range exceeds the number of rows in the table, it is truncated
 *       to the valid range. Null values are ignored during length calculation.
 */
template <typename TABLE>
void aggregate<TABLE>::max( std::vector<unsigned>& vectorLength, uint64_t uBeginRow, uint64_t uCount, tag_length ) { assert( m_ptable != nullptr );
   if( vectorLength.empty() == true ) vectorLength.resize( m_ptable->get_column_count(), 0 );

   uint64_t uEndRow = uBeginRow + uCount; // last row where value is checked
   // ## iterate rows to check max length for values found in column
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count(); }

   bool bHasNull = m_ptable->is_null(); // get if table has null values
   unsigned uColumnCount = (unsigned)vectorLength.size() < m_ptable->get_column_count() ? (unsigned)vectorLength.size() : m_ptable->get_column_count(); // number of columns to check
   for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) {
      for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ ) {
         if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue; // skip null values

         unsigned uValueLength = m_ptable->cell_get_length( uRow, uColumn );
         unsigned uLength = vectorLength[uColumn];
         if( uLength < uValueLength ) vectorLength[uColumn] = uValueLength;
      }
   }
}

/** ---------------------------------------------------------------------------
* @brief Calculates the maximum length of values in specified columns.
* 
* This method iterates through the specified range of rows and columns in the table,
* determining the maximum length of the values found in the specified columns.
* It updates the provided vectorLength with the maximum lengths for each specified column.
* @param vectorLength A reference to a vector that will hold the maximum lengths for each specified column.
*                    If empty, it will be resized to match the number of specified columns.
* @param uBeginRow The starting row index for the range to check.
* @param uCount The number of rows to include in the range.
* @param vectorColumn A vector containing the indices of the columns to check.
* @param tag_length A tag indicating that this is a length operation.
* @note If the range exceeds the number of rows in the table, it is truncated
*      to the valid range. Null values are ignored during length calculation.
*/
template <typename TABLE>
void aggregate<TABLE>::max( std::vector<unsigned>& vectorLength, uint64_t uBeginRow, uint64_t uCount, const std::vector<unsigned>& vectorColumn, tag_length ) { assert( m_ptable != nullptr );
   if( vectorLength.empty() == true ) vectorLength.resize( vectorColumn.size(), 0);

   uint64_t uEndRow = uBeginRow + uCount; // last row where value is checked
   // ## iterate rows to check max length for values found in column
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count(); }
   bool bHasNull = m_ptable->is_null(); // get if table has null values

   //unsigned uColumnCount = vectorLength.size() < m_ptable->get_column_count() ?  vectorLength.size() : m_ptable->get_column_count(); // number of columns to check
   for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) {
      for( auto itColumn = std::begin( vectorColumn ), itEnd = std::end( vectorColumn ); itColumn != itEnd; itColumn++ ) {
         unsigned uColumn = *itColumn;
         if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue; // skip null values
         unsigned uValueLength = m_ptable->cell_get_length( uRow, uColumn );
         unsigned uLength = vectorLength[std::distance( vectorColumn.begin(), itColumn )];
         if( uLength < uValueLength ) vectorLength[ std::distance( vectorColumn.begin(), itColumn )] = uValueLength;
      }
   }
}

/** ---------------------------------------------------------------------------
* @brief Calculates the maximum length of values in all columns and adapts for text format (newline etc.).
* 
* This method iterates through the specified range of rows and columns in the table,
* determining the maximum length of the values found and adjusting for text format.
* This means that text-specific considerations (like newline characters) are taken into account.
* vectorLength with the maximum lengths for each column.
* 
* @tparam TABLE The table type being aggregated.
* @param vectorLength A reference to a vector that will hold the maximum lengths for each column.
*                     If empty, it will be resized to match the number of columns in the table.
* @param uBeginRow The starting row index for the range to check.
* @param uCount The number of rows to include in the range.
* 
* @note If the range exceeds the number of rows in the table, it is truncated
*       to the valid range. Null values are ignored during length calculation.
*/
template <typename TABLE>
void aggregate<TABLE>::max(std::vector<unsigned>& vectorLength, uint64_t uBeginRow, uint64_t uCount, tag_length, tag_text) {
   assert(m_ptable != nullptr);
   if( vectorLength.empty() == true ) vectorLength.resize(m_ptable->get_column_count(), 0);

   uint64_t uEndRow = uBeginRow + uCount; // last row where value is checked
   // ## iterate rows to check max length for values found in column
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count(); }

   bool bHasNull = m_ptable->is_null(); // get if table has null values
   unsigned uColumnCount = (unsigned)vectorLength.size() < m_ptable->get_column_count() ? (unsigned)vectorLength.size() : m_ptable->get_column_count(); // number of columns to check
   for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) {
      for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ ) {
         if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue; // skip null values

         bool bText = gd::types::detail::is_string( m_ptable->column_get_type( uColumn ) );

         if( bText == true ) 
         {
            // ### text column - need to check for newline characters etc and not just take length of string
            auto stringText = m_ptable->cell_get_variant_view( uRow, uColumn ).as_string_view();

            // ### calculate length for longest row
            unsigned uMaxLength = 0;
            unsigned uLineLength = 0;
            for( const char& c : stringText ) {
               if( c == '\n' ) 
               {
                  if( uLineLength > uMaxLength ) uMaxLength = uLineLength;
                  uLineLength = 0;
               }
               uLineLength++;
            }

            if( uLineLength > uMaxLength ) uMaxLength = uLineLength; // last line

            unsigned uValueLength = uMaxLength;
            unsigned uLength = vectorLength[uColumn];
            if( uLength < uValueLength ) vectorLength[uColumn] = uValueLength;
            continue;
         }

         unsigned uValueLength = m_ptable->cell_get_length(uRow, uColumn);
         unsigned uLength = vectorLength[uColumn];
         if( uLength < uValueLength ) vectorLength[uColumn] = uValueLength;
      }
   }
}



template<typename TABLE>
template<typename TYPE>
TYPE aggregate<TABLE>::sum( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const {
   uint64_t uEndRow = uBeginRow + uCount; // last row to sum value from
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count(); }

   auto eType = gd::types::type_g<TYPE>( gd::types::tag_ask_compiler{});
   auto uColumnType = m_ptable->column_get_ctype( uColumn );
   bool bHasNull = m_ptable->is_null(); // get if table has null values
   TYPE sum_{};

   if( (( unsigned )eType & 0xff) == (uColumnType & 0xff) ) {
      for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) {
         if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue; // skip null values
         sum_ += (TYPE)m_ptable->cell_get_variant_view( uRow, uColumn );
      }
   }
   else {
      for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) {
         if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue; // skip null values
         gd::variant variantConvertTo;
         auto variantviewValue = m_ptable->cell_get_variant_view( uRow, uColumn );
         bool bOk = variantviewValue.convert_to( eType,  variantConvertTo );
         if( bOk == true )
         {
            sum_ += (TYPE)variantConvertTo;
         }
      }
   }

   return sum_;
}

/** ---------------------------------------------------------------------------
 * @brief Count total number of values (including nulls) in specified column range
 * @param uColumn index to column to count values in
 * @param uBeginRow start row to count from
 * @param uCount number of rows from start row
 * @return total number of values in column within specified range
*/
template <typename TABLE>
uint64_t aggregate<TABLE>::count( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const { assert( m_ptable != nullptr );
   uint64_t uEndRow = uBeginRow + uCount;
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count(); }

   return uEndRow - uBeginRow;
}



/** ---------------------------------------------------------------------------
 * @brief Count non-null values in specified column range
 * @param uColumn index to column to count values in
 * @param uBeginRow start row to count from
 * @param uCount number of rows from start row
 * @return number of non-null values found in column within specified range
 */
template <typename TABLE>
uint64_t aggregate<TABLE>::count_not_null( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const { assert( m_ptable != nullptr );
   uint64_t uEndRow = uBeginRow + uCount;
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count(); }

   uint64_t uCount_ = 0;
   bool bHasNull = m_ptable->is_null();

   for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) {
      if( bHasNull == false || m_ptable->cell_is_null(uRow, uColumn) == false ) {
         uCount_++;
      }
   }
   return uCount_;
}

/** ---------------------------------------------------------------------------
 * @brief Count null values in specified column range
 * @param uColumn index to column to count null values in
 * @param uBeginRow start row to count from
 * @param uCount number of rows from start row
 * @return number of null values found in column within specified range
*/
template <typename TABLE>
uint64_t aggregate<TABLE>::count_null( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const { assert( m_ptable != nullptr );
   uint64_t uEndRow = uBeginRow + uCount;
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count(); }

   uint64_t uNullCount = 0;
   bool bHasNull = m_ptable->is_null();

   if( bHasNull == true ) {
      for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) {
         if( m_ptable->cell_is_null(uRow, uColumn) == true ) {
            uNullCount++;
         }
      }
   }
   return uNullCount;
}


/** ---------------------------------------------------------------------------
 * @brief Count unique values in specified column range
 * @param uColumn index to column to count unique values in
 * @param uBeginRow start row to count from
 * @param uCount number of rows from start row
 * @return number of unique values found in column within specified range
 */
template <typename TABLE>
uint64_t aggregate<TABLE>::count_unique( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const { assert( m_ptable != nullptr ); assert( uColumn < m_ptable->get_column_count() );
   std::unordered_set<std::string> unorderedset_;
   uint64_t uEndRow = uBeginRow + uCount;
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count(); }

   bool bHasNull = m_ptable->is_null();

   for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) 
   {
      if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue;

      gd::variant_view value_ = m_ptable->cell_get_variant_view(uRow, uColumn);
      std::string stringValue = value_.as_string();
      unorderedset_.insert(stringValue);
   }

   return unorderedset_.size();
}


/** ---------------------------------------------------------------------------
 * @brief Calculate variance of values in specified column range (non-template version)
 * @param uColumn index to column to calculate variance for
 * @param uBeginRow start row to calculate from
 * @param uCount number of rows from start row
 * @return variance of values found in column within specified range
 */
template<typename TABLE>
template<typename TYPE>
double aggregate<TABLE>::variance( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const { assert( m_ptable != nullptr );
   // Use double as default type for variance calculation
   auto eType = m_ptable->column_get_ctype( uColumn );
   
   // ## Dispatch to appropriate type based on column type

   if( gd::types::detail::is_integer( eType ) ) 
   {
      return variance<int64_t>( uColumn, uBeginRow, uCount );
   }
   else if( gd::types::detail::is_decimal( eType ) ) 
   {
      return variance<double>( uColumn, uBeginRow, uCount );
   }
   else 
   {
      return variance<double>( uColumn, uBeginRow, uCount );                  // For other types, try to convert to double
   }
}

/** ---------------------------------------------------------------------------
 * @brief Calculate standard deviation of values in specified column range
 * @param uColumn index to column to calculate standard deviation for
 * @param uBeginRow start row to calculate from
 * @param uCount number of rows from start row
 * @return standard deviation of values found in column within specified range
*/
template<typename TABLE>
template<typename TYPE>
double aggregate<TABLE>::std_deviation( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const {
   assert( m_ptable != nullptr );
   return std::sqrt( variance( uColumn, uBeginRow, uCount ) );
}


/** ---------------------------------------------------------------------------
 * @brief Find median value in specified column range
 * @param uColumn index to column to find median value in
 * @param uBeginRow start row to check values from
 * @param uCount number of rows from start row
 * @return median value found in column within specified range
*/
template<typename TABLE>
template<typename TYPE>
TYPE aggregate<TABLE>::median( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const { assert( m_ptable != nullptr );
   uint64_t uEndRow = uBeginRow + uCount;
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count(); }

   auto eType = gd::types::type_g<TYPE>( gd::types::tag_ask_compiler{});
   auto uColumnType = m_ptable->column_get_ctype( uColumn );
   bool bHasNull = m_ptable->is_null();
   std::vector<TYPE> vectorValue;

   if( (( unsigned )eType & 0xff) == (uColumnType & 0xff) ) 
   {
      for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) 
      {
         if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue;
         TYPE value = (TYPE)m_ptable->cell_get_variant_view( uRow, uColumn );
         vectorValue.push_back( value );
      }
   }
   else 
   {
      for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) 
      {
         if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue;
         gd::variant variantConvertTo;
         auto variantviewValue = m_ptable->cell_get_variant_view( uRow, uColumn );
         bool bOk = variantviewValue.convert_to( eType, variantConvertTo );
         if( bOk == true ) 
         {
            TYPE value = (TYPE)variantConvertTo;
            vectorValue.push_back( value );
         }
      }
   }

   if( vectorValue.empty() ) return TYPE{};

   std::sort(vectorValue.begin(), vectorValue.end());                         // Sort values to find median
   size_t uSize = vectorValue.size();

   // ## Calculate median value

   if( uSize % 2 == 0 ) 
   {
      
      if constexpr( std::is_integral_v<TYPE> ) 
      {
         return (vectorValue[uSize/2 - 1] + vectorValue[uSize/2]) / 2;        // Even number of elements - return average of middle two (no single value in the middle)
      } 
      else 
      {
         return (vectorValue[uSize/2 - 1] + vectorValue[uSize/2]) / static_cast<TYPE>(2.0); // Floating point - return precise average
      }
   } 
   else 
   {
      return vectorValue[uSize / 2];                                          // Odd number of elements - return middle element
   }
}

/** ---------------------------------------------------------------------------
 * @brief Find percentile value in specified column range
 * @param uColumn index to column to find percentile value in
 * @param dPercentile percentile to find (0.0 to 100.0)
 * @param uBeginRow start row to check values from
 * @param uCount number of rows from start row
 * @return percentile value found in column within specified range
*/
template<typename TABLE>
template<typename TYPE>
TYPE aggregate<TABLE>::percentile( unsigned uColumn, double dPercentile, uint64_t uBeginRow, uint64_t uCount ) const { assert( m_ptable != nullptr ); assert( dPercentile >= 0.0 && dPercentile <= 100.0 );
   
   uint64_t uEndRow = uBeginRow + uCount;
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count(); }

   auto eType = gd::types::type_g<TYPE>( gd::types::tag_ask_compiler{});
   auto uColumnType = m_ptable->column_get_ctype( uColumn );
   bool bHasNull = m_ptable->is_null();
   std::vector<TYPE> vectorValue;

   if( type_number_g( eType ) == type_number_g( uColumnType ) ) 
   {
      for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) 
      {
         if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue;
         TYPE value_ = (TYPE)m_ptable->cell_get_variant_view( uRow, uColumn );
         vectorValue.push_back( value_ );
      }
   }
   else {
      for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) {
         if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue;
         gd::variant variantConvertTo;
         auto variantviewValue = m_ptable->cell_get_variant_view( uRow, uColumn );
         bool bOk = variantviewValue.convert_to( eType, variantConvertTo );
         if( bOk == true ) {
            TYPE value_ = (TYPE)variantConvertTo;
            vectorValue.push_back( value_ );
         }
      }
   }

   if( vectorValue.empty() ) return TYPE{};

   std::sort( vectorValue.begin(), vectorValue.end() );

   if( dPercentile == 0.0 ) return vectorValue.front();
   if( dPercentile == 100.0 ) return vectorValue.back();

   double dIndex = (dPercentile / 100.0) * (vectorValue.size() - 1);
   size_t uLowerIndex = static_cast<size_t>( std::floor( dIndex ) );
   size_t uUpperIndex = static_cast<size_t>( std::ceil( dIndex ) );

   if( uLowerIndex == uUpperIndex ) {
      return vectorValue[uLowerIndex];
   }

   // Linear interpolation between the two values
   double dFraction = dIndex - std::floor( dIndex );
   if constexpr( std::is_integral_v<TYPE> ) 
   {
      return static_cast<TYPE>( vectorValue[uLowerIndex] + dFraction * (vectorValue[uUpperIndex] - vectorValue[uLowerIndex]) );
   } 
   else 
   {
      return vectorValue[uLowerIndex] + static_cast<TYPE>( dFraction ) * (vectorValue[uUpperIndex] - vectorValue[uLowerIndex]);
   }
}

/** ---------------------------------------------------------------------------
 * @brief Count values in column that contain the specified pattern
 * @param uColumn index to column to search in
 * @param stringPattern pattern to search for within cell values
 * @param uBeginRow start row to search from
 * @param uCount number of rows from start row
 * @return number of values in column that contain the pattern within specified range
 */
template <typename TABLE>
unsigned aggregate<TABLE>::count_contains( unsigned uColumn, const std::string_view& stringPattern, uint64_t uBeginRow, uint64_t uCount ) const { assert( m_ptable != nullptr );
   assert( uColumn < m_ptable->get_column_count() );
   
   uint64_t uEndRow = uBeginRow + uCount;
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count(); }

   unsigned uMatchCount = 0;
   bool bHasNull = m_ptable->is_null();

   for( uint64_t uRow = uBeginRow; uRow < uEndRow; uRow++ ) {
      if( bHasNull == true && m_ptable->cell_is_null(uRow, uColumn) == true ) continue; // skip null values

      gd::variant_view value_ = m_ptable->cell_get_variant_view(uRow, uColumn);
      std::string stringValue = value_.as_string();
      
      // Check if the pattern is found in the string value
      if( stringValue.find( stringPattern ) != std::string::npos ) { uMatchCount++; }
   }

   return uMatchCount;
}

/**
 * @brief Retrieves unique values from a specified column within a range of rows.
 * 
 * This method iterates over the specified range of rows in the given column
 * and collects unique values, skipping null values. The uniqueness is determined
 * by comparing the string representation of the values.
 * 
 * @tparam TABLE The table type being aggregated.
 * @param uColumn The index of the column to retrieve unique values from.
 * @param uBeginRow The starting row index for the range.
 * @param uCount The number of rows to include in the range.
 * @return A vector of unique values as `gd::variant_view` objects.
 * 
 * @note If the range exceeds the number of rows in the table, it is truncated
 *       to the valid range. Null values are ignored during the uniqueness check.
 */
template <typename TABLE>
std::vector<gd::variant_view> aggregate<TABLE>::unique(unsigned uColumn, uint64_t uBeginRow, uint64_t uCount) const {  assert(m_ptable != nullptr); assert( uColumn < m_ptable->get_column_count() );

   std::vector<gd::variant_view> vectorUnique; // To store unique values
   std::unordered_set<std::string> setSeen; // To track already seen values

   uint64_t uEndRow = uBeginRow + uCount; // Calculate the end row
   if( uEndRow > m_ptable->get_row_count() ) { uEndRow = m_ptable->get_row_count();  }

   bool bHasNull = m_ptable->is_null(); // Check if the table has null values

   for(uint64_t uRow = uBeginRow; uRow < uEndRow; ++uRow)
   {
      if(bHasNull && m_ptable->cell_is_null(uRow, uColumn)) continue;          // Skip null values

      
      gd::variant_view value_ = m_ptable->cell_get_variant_view(uRow, uColumn); // Get the value as a variant_view

      
      std::string stringValue = value_.as_string();                            // Convert the value to a string for comparison

      // ## Check if the value is already seen
      if(setSeen.find(stringValue) == setSeen.end()) 
      {
         setSeen.insert(stringValue);                                          // Mark as seen
         vectorUnique.push_back(value_);                                       // Add to unique values
      }
   }

   return vectorUnique;
}

/**
 * @brief Adjusts the length of binary columns in the vectorLength vector.
 *
 * This method iterates through the vectorLength vector and doubles the length
 * for binary columns. It is used to ensure that the length of binary data is
 * correctly represented.
 *
 * @tparam TABLE The table type being aggregated.
 * @param vectorLength A reference to a vector of unsigned integers representing lengths.
 * @param tag_text A tag indicating that this is a text operation.
 */
template <typename TABLE>
void aggregate<TABLE>::fix( std::vector<unsigned>& vectorLength, tag_text ) { assert( m_ptable != nullptr ); assert( vectorLength.empty() == false );
   unsigned uColumnCount = (unsigned)vectorLength.size() < m_ptable->get_column_count() ? (unsigned)vectorLength.size() : m_ptable->get_column_count(); // number of columns to check
   for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ ) {
      auto uType = m_ptable->column_get_type( uColumn );
      if(gd::types::detail::is_binary(uType) == true) {
         auto uLength = vectorLength[uColumn];
         vectorLength[uColumn] = (uLength * 2);
      }
   }
}



_GD_TABLE_END
