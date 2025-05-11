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

   // ## max operation calculating size in bytes each value needs related to read it as text

   unsigned max( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount, tag_length ) const;
   unsigned max( unsigned uColumn, tag_length ) const { return max( uColumn, 0, m_ptable->get_row_count(), tag_length{}); }
   unsigned max( const std::string_view& stringName, tag_length ) const { return max( m_ptable->column_get_index( stringName ), 0, m_ptable->get_row_count(), tag_length{}); }
   unsigned max( const std::string_view& stringName, uint64_t uBeginRow, uint64_t uCount, tag_length ) const { return max( m_ptable->column_get_index( stringName ), uBeginRow, uCount, tag_length{}); }

   void max( std::vector<unsigned>& vectorLength, uint64_t uBeginRow, uint64_t uCount, tag_length );
   void max( std::vector<unsigned>& vectorLength, tag_length ) { max( vectorLength, 0, m_ptable->get_row_count(), tag_length{} ); }
   void max( std::vector<unsigned>& vectorLength, uint64_t uBeginRow, uint64_t uCount, const std::vector<unsigned>& vectorColumn, tag_length );

   template<typename TYPE>
   TYPE sum( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const;
   template<typename TYPE>
   TYPE sum( unsigned uColumn ) const { return sum<TYPE>( uColumn, 0, m_ptable->get_row_count() ); }

   
   std::vector<gd::variant_view> unique( unsigned uColumn, uint64_t uBeginRow, uint64_t uCount ) const;


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
   std::unordered_set<std::string> seenValues; // To track already seen values

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