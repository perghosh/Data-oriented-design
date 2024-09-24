#include "gd_database_record.h"

_GD_DATABASE_BEGIN

template<int iGrowBy>
std::pair<uint8_t*, unsigned> reserve_g( uint8_t* pbBuffer, unsigned uSize, unsigned uMaxSize )
{
   if(uSize > uMaxSize)
   {
      unsigned uNewSize = uSize + (iGrowBy - uSize % iGrowBy);                 // new size + exgtra for avoiding to many allocations

      // ## create new buffer, copy from old buffer and update members
      uint8_t* pbNewBuffer = new uint8_t[uNewSize];
      if( uMaxSize > 0 )                                                       // do we need to copy existing data
      {                                                                                            assert( pbBuffer != nullptr );
         memcpy(pbNewBuffer, pbBuffer, uMaxSize);
         delete [] pbBuffer;
      }
      return { pbNewBuffer, uNewSize };
   }

   return { pbBuffer, uMaxSize };
}

template<typename BYTE, int iGrowBy>
std::pair<BYTE*, unsigned> reserve_g( BYTE* pbBuffer, unsigned uSize, unsigned uMaxSize )
{
   auto _result = reserve_g<iGrowBy>( (uint8_t*)pbBuffer, uSize, uMaxSize );
   return { (BYTE*)_result.first, _result.second };
}

/** ---------------------------------------------------------------------------
 * @brief Add name to internal buffer that stores name
 * `names` object stores names (strings) that do not change, each name is store
 * in one single buffer.
 * @param stringName name added to buffer
 * @return offset position in buffer for stored name
*/
uint16_t names::add(std::string_view stringName)
{                                                                                                  assert( stringName.length() < 1000 ); // realistic
   auto uNameLLengthAndExtra = stringName.length() + 1 + sizeof(uint16_t);     // add name lenght and extra bytes to store lenght and zero ending
   reserve( m_uSize + (unsigned)uNameLLengthAndExtra );

   unsigned uNameOffset = last_position();                                     // get end position where new name is added
   char* pbName = m_pbBufferNames + uNameOffset;                               // get pointer
   *(uint16_t*)pbName = (uint16_t)stringName.length();                         // set name lenght before name (lenght is stored as uint16_t)
   pbName += sizeof(uint16_t);                                                 // move past name lenght in buffer
   uNameOffset += sizeof( uint16_t );                                          // add two bytes sizeof(uint16_t) storing name length, after this add name characters
   memcpy( pbName, stringName.data(), uNameLLengthAndExtra - sizeof(uint16_t) );// copy name and zero termination
   m_uSize += decltype(m_uSize)(uNameLLengthAndExtra);                         // set to end of name (where next name starts)

   return uNameOffset;                                                         // return offset position to where name was inserted
}

/** ---------------------------------------------------------------------------
 * @brief maker sure that internal buffer is big enough to store size
 * @param uSize needed size in internal buffer
*/
void names::reserve(unsigned uSize)
{
   // `reserve_g` is used to resize buffer if needed
   std::tie( m_pbBufferNames, m_uMaxSize ) = reserve_g<char,m_uBufferGrowBy_s>( m_pbBufferNames, uSize, m_uMaxSize );
}

// ============================================================================
// ==================================================================== buffers
// ============================================================================


/** ---------------------------------------------------------------------------
 * @brief Add primitive value, primitive value is same as value that has a fixed max buffer size
 * To avoid to allocate to much memory there is one logic to add primitive (fixed max size values)
 * and another to logic used to manage values that can be "any" size and grow dynamically if needed.
 * @param uColumnType type of column
 * @param uSize max size buffer needs to be to store value
 * @return offset position in buffer storing fixed values
*/
uint32_t buffers::primitive_add( unsigned uColumnType, unsigned uSize )
{                                                                                                  assert( uSize != 0 ); // if 0 then value is "derived" (flexible size)
   uint8_t* pbBuffer = nullptr;
   unsigned uSizeOld = m_uSize;

   unsigned uSizeAndExtra = uSize + sizeof( uSize ) + sizeof( uColumnType );   // needed size in buffer for primitive/fixed type

   // ## allocate storage if needed
   std::tie( m_pbBufferPrimitve, m_uMaxSize ) = reserve_g<uint8_t,m_uBufferGrowBy_s>( m_pbBufferPrimitve, uSizeAndExtra + m_uSize, m_uMaxSize );
   
   uint8_t* pbValue = m_pbBufferPrimitve + uSizeOld;
   *(unsigned*)pbValue = uSize;                                                // buffer size
   pbValue += sizeof(unsigned);
   *(unsigned*)pbValue = uColumnType;                                          // value type

   m_uSize += uSizeAndExtra;                                                   // add current buffer size for fixed buffers

   return uSizeOld + sizeof( uColumnType ) + sizeof( uSize );                  // return offset position to location in buffer where value is stored
}

/** ---------------------------------------------------------------------------
 * @brief Add "primitive"derived" value, value that are able to store any number of bytes
 * Derived buffers are buffers used for columns that do not have any max value in them.
 * Before reading value from result into column there is a check to see if buffer is large
 * enought, if not relocation is done and this is only done for column buffer.
 * Fixed buffers are store in `m_pbBufferPrimitve` as one long serie of data (same memory block)
 * @param uColumnType type of column
 * @param uSize max size buffer needs to be to store value
 * @return offset position in buffer storing fixed values
*/
uint16_t buffers::derived_add(unsigned uColumnType, unsigned uSize)
{
   if( uSize == 0 ) uSize = m_uStartSizeForDerivedBuffer_s;                    // Set default start size if no size is specified

   unsigned uSizeAndExtra = uSize + sizeof( uSize ) + sizeof( uColumnType );

   std::unique_ptr<uint8_t> pBuffer( new uint8_t[uSizeAndExtra] );
   uint8_t* puBuffer = pBuffer.get();

   *( unsigned* )puBuffer = uSize;                                             // Set max size that can be store in buffer
   puBuffer += sizeof( unsigned );
   *(unsigned*)puBuffer = uColumnType;                                         // Set column type

   m_vectorBuffer.push_back( std::move( pBuffer ) );

   return (uint16_t)m_vectorBuffer.size() - 1;
}

/** ---------------------------------------------------------------------------
 * @brief Reize buffer holding fixed values, all fixed values are stored in one single buffer large enough to store all fixed values
 * @param uType number for value type
 * @param uSize max size for value 
 * @return previous size for buffer
*/
unsigned buffers::primitive_resize( unsigned uType, unsigned uSize )
{
   uint8_t* pbBuffer = nullptr;
   unsigned uSizeOld = m_uSize;

   if(uSize > m_uMaxSize)
   {
      unsigned uSizeAndExtra = uSize + sizeof( unsigned ) + sizeof( unsigned );
      std::tie( m_pbBufferPrimitve, m_uMaxSize ) = reserve_g<uint8_t,m_uBufferGrowBy_s>( m_pbBufferPrimitve, uSizeAndExtra, m_uMaxSize );
      m_uSize = uSizeAndExtra;
   }

   return uSizeOld;
}

/** ---------------------------------------------------------------------------
 * @brief Resize derived buffers, derived buffers are used for values that can have any buffer size
 * @param uIndex index to buffer that needs to be resized
 * @param uSize new size for buffer (has to be larger than previous buffer)
 * @return pointer to new buffer
*/
uint8_t* buffers::derived_resize( unsigned uIndex, unsigned uSize )
{                                                                              assert( uIndex < m_vectorBuffer.size() );
   const uint8_t* puBuffer = m_vectorBuffer[uIndex].get();                     assert( *( unsigned* )puBuffer < uSize );

   uSize = uSize + (m_uStartSizeForDerivedBuffer_s - (uSize % m_uStartSizeForDerivedBuffer_s));

   unsigned uOldSize = *( unsigned* )puBuffer;                                 assert( uOldSize < uSize );// if old size is bigger, something is wrong

   /// ## create new buffer with new size (remember that start of buffer has type and buffer length and value length)
   unsigned uTotalSize = uSize + sizeof(unsigned) + sizeof(unsigned);
   uint8_t* puBiggerBuffer = new uint8_t[uTotalSize];

   memcpy( puBiggerBuffer, puBuffer, uOldSize + sizeof( unsigned ) * 2 );      // copy old buffer information into new buffer
   *( unsigned* )puBiggerBuffer = uSize;
   m_vectorBuffer[uIndex].reset( puBiggerBuffer );                             // replace buffer
   return puBiggerBuffer;
}

/** ---------------------------------------------------------------------------
 * @brief add column to record
 * @param uColumnType Type of column @see enumColumnTypeComplete .
 * @param uSize size in bytes needed to store value for non primitive types
 * @param stringName field name
 * @param stringAlias alias name
 * @return record& reference to record to nest calls
*/
record& record::add( unsigned uColumnType, unsigned uSize, const std::string_view& stringName, const std::string_view& stringAlias )
{
   column columnAdd;
   unsigned uValueOffset{0};

   columnAdd.type( uColumnType );
   columnAdd.size( uSize );
   if( stringName.empty() == false ) columnAdd.name( m_namesColumn.add( stringName ) );
   if( stringAlias.empty() == false ) columnAdd.alias( m_namesColumn.add( stringAlias ) );

   columnAdd.index( (unsigned)m_vectorColumn.size() );

   if( uSize > 0 )                                                             // do we have a fixed value? (size is specified)
   {
      uValueOffset = m_buffersValue.primitive_add( uColumnType, uSize );       // add buffer for fixed size type
      columnAdd.value( uValueOffset );
      columnAdd.set_fixed( true );
      columnAdd.size_buffer( uSize );
   }
   else
   {
      uValueOffset = m_buffersValue.derived_add( uColumnType, uSize );         // add buffer for fixed size type
      columnAdd.value( uValueOffset );
   }

   m_vectorColumn.push_back( columnAdd );
   
   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Add column to record 
 * @param uType type for column
 * @param uCType type for column
 * @param uSizeFixed size if fixed
 * @param uStartBufferSize if size can grow this is the start buffer size
 * @param stringName column name
 * @param stringAlias column alias
 * @return reference to record if chain calls
*/
record& record::add( unsigned uType, unsigned uCType, unsigned uSizeFixed, unsigned uStartBufferSize, const std::string_view& stringName, const std::string_view& stringAlias, unsigned uState )
{
   column columnAdd;
   unsigned uValueOffset{0};

   columnAdd.type( uType );
   columnAdd.ctype( uCType );
   columnAdd.size( uSizeFixed );
   columnAdd.state( uState );
   if( stringName.empty() == false ) columnAdd.name( m_namesColumn.add( stringName ) );
   if( stringAlias.empty() == false ) columnAdd.alias( m_namesColumn.add( stringAlias ) );

   columnAdd.index( (unsigned)m_vectorColumn.size() );

   if( uSizeFixed > 0 )                                                        // do we have a fixed value? (size is specified)
   {
      uValueOffset = m_buffersValue.primitive_add( uType, uSizeFixed );  // add buffer for fixed size type
      columnAdd.value( uValueOffset );
      columnAdd.set_fixed( true );
      columnAdd.size_buffer( uSizeFixed );
   }
   else
   {
      uValueOffset = m_buffersValue.derived_add( uType, uStartBufferSize ); // add buffer for fixed size type
      columnAdd.value( uValueOffset );
      columnAdd.size_buffer( uStartBufferSize );
   }

   m_vectorColumn.push_back( columnAdd );
   
   return *this;
}


/** ---------------------------------------------------------------------------
 * @brief return buffer for value buffer
 * @param uIndex column index buffer is returned for
 * @return uint8_t* pointer to buffer storing column value data
*/
uint8_t* record::buffer_get( unsigned uIndex ) const
{                                                                                                  
   const record::column* pcolumn = get_column( uIndex );                                           assert( pcolumn->size_buffer() != 0 );
   if( pcolumn->size_buffer() != 0 )
   {
      if( pcolumn->is_fixed() == true )
      {
         return m_buffersValue.primitive_data_offset( pcolumn->value() );
      }
      else
      {
         unsigned uBufferIndex = pcolumn->value();                             // value holds index to buffer
         return m_buffersValue.derived_data_value( uBufferIndex );             // return pointer to data
      }
   }

   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Get alias for column if alias is set
 * @param uIndex index to column alias is returned for
 * @return alias name if found, empty if no alias
*/
std::string_view record::alias_get( unsigned uIndex ) const
{                                                                              assert( uIndex < size() );
   const record::column* pcolumn = get_column( uIndex );
   if( pcolumn->alias() > 0 ) return m_namesColumn.get( pcolumn->alias() );

   return std::string_view();
}

/** ---------------------------------------------------------------------------
 * @brief Return all names in vector
 * @return std::vector<std::string_view> vector with column names
*/
std::vector<std::string_view> record::alias_get() const
{
   std::vector<std::string_view> vectorAlias;
   for( unsigned u = 0, uTo = size(); u < uTo; u++ )
   {
      vectorAlias.push_back( alias_get( u ) );
   }

   return vectorAlias;
}

unsigned record::type_get( unsigned uIndex ) const noexcept
{                                                                              assert( uIndex < size() );
   const record::column* pcolumn = get_column( uIndex );
   return pcolumn->type();
}

std::vector<unsigned> record::type_get() const
{
   std::vector<unsigned> vectorType;
   for( unsigned u = 0, uTo = size(); u < uTo; u++ )
   {
      vectorType.push_back( type_get( u ) );
   }

   return vectorType;
}


/** ---------------------------------------------------------------------------
 * @brief Get name for column if name is set
 * @param uIndex index to column name is returned for 
 * @return column name if found, empty if no name
*/
std::string_view record::name_get( unsigned uIndex ) const noexcept
{                                                                              assert( uIndex < size() );
   const record::column* pcolumn = get_column( uIndex );
   if( pcolumn->name() ) return m_namesColumn.get( pcolumn->name() );

   return std::string_view();
}

/** ---------------------------------------------------------------------------
 * @brief Return all names in vector
 * @return std::vector<std::string_view> vector with column names
*/
std::vector<std::string_view> record::name_get() const
{
   std::vector<std::string_view> vectorName;
   for( unsigned u = 0, uTo = size(); u < uTo; u++ )
   {
      vectorName.push_back( name_get( u ) );
   }

   return vectorName;
}

void record::get_column( std::vector<std::tuple<unsigned, unsigned, std::string_view>>& vectorColumn ) const
{
   for( unsigned u = 0, uTo = size(); u < uTo; u++ )
   {
      const record::column* pcolumn = get_column( u );
      auto uType = pcolumn->type();
      unsigned uSize = gd::types::is_primitive_g( uType ) ? 0 : pcolumn->size();
      vectorColumn.push_back( { pcolumn->type(), uSize, m_namesColumn.get( pcolumn->name() ) });
   }
}


/** ---------------------------------------------------------------------------
 * @brief empty columns
*/
void record::clear()
{
   m_vectorColumn.clear();
}

/** -----------------------------------------------------------------------------------------------
 * @brief Get value in specified column
 * @param uColumnIndex Index to column where retured value i value
 * @return gd::variant value is placed and returned in variant
*/
gd::variant record::get_variant( unsigned uColumnIndex ) const
{                                                                             assert( uColumnIndex < get_column_count() );
   const gd::database::record::column* pcolumn = get_column( uColumnIndex );

   if (pcolumn->is_null() == false)
   {
      unsigned uType = pcolumn->type();
      uint8_t* pbBuffer = buffer_get( uColumnIndex );              // get buffer to column
      switch( uType )
      {
      case eColumnTypeCompleteInt16: return gd::variant( *(int16_t*)pbBuffer );
      case eColumnTypeCompleteInt32: return gd::variant( (int32_t)*(int64_t*)pbBuffer);
      case eColumnTypeCompleteInt64: return gd::variant( *(int64_t*)pbBuffer );
      case eColumnTypeCompleteCFloat: return gd::variant( *(float*)pbBuffer );
      case eColumnTypeCompleteCDouble: return gd::variant( *(double*)pbBuffer );
      case eColumnTypeCompleteDecimal: return gd::variant( (const char*)pbBuffer, (size_t)pcolumn->size() );
      case eColumnTypeCompleteString: return gd::variant( (const char*)pbBuffer, (size_t)pcolumn->size() );
      case eColumnTypeCompleteUtf8String: return gd::variant( (const char*)pbBuffer, (size_t)pcolumn->size() );
      case eColumnTypeCompleteBinary: return gd::variant( (unsigned char*)pbBuffer, (size_t)pcolumn->size() );
         
      default: assert(false);
      }
   }

   return gd::variant();
}

/** -----------------------------------------------------------------------------------------------
 * @brief Collects values from active row and place them in returned vector with `variant`
 * @return std::vector<gd::variant> get all values from current row
*/
std::vector<gd::variant> record::get_variant() const
{
   std::vector<gd::variant> vectorValue;
   for( auto u = 0u, uTo = get_column_count(); u < uTo; u++ )
   { 
      vectorValue.push_back( get_variant( u ) );
   }
   return vectorValue;
}

/** -----------------------------------------------------------------------------------------------
 * @brief Get value in specified column
 * @param uColumnIndex Index to column where returned value i value
 * @return gd::variant_view value is placed and returned in variant_view
*/
gd::variant_view record::get_variant_view( unsigned uColumnIndex ) const
{                                                                              assert( uColumnIndex < get_column_count() );
   const gd::database::record::column* pcolumn = get_column( uColumnIndex );

   if (pcolumn->is_null() == false)
   {
      unsigned uType = pcolumn->type();
      uint8_t* pbBuffer = buffer_get( uColumnIndex );                         // get buffer to column
      switch( uType )
      {
      case eColumnTypeCompleteUnknown: return gd::variant_view();
      case eColumnTypeCompleteInt16: return gd::variant_view( *(int16_t*)pbBuffer);
      case eColumnTypeCompleteInt32: return gd::variant_view( *(int32_t*)pbBuffer);
      case eColumnTypeCompleteInt64: return gd::variant_view( *(int64_t*)pbBuffer );
      case eColumnTypeCompleteCFloat: return gd::variant_view( *(float*)pbBuffer );
      case eColumnTypeCompleteCDouble: return gd::variant_view( *(double*)pbBuffer );
      case eColumnTypeCompleteDecimal: return gd::variant_view( (const char*)pbBuffer, (size_t)pcolumn->size() );
      case eColumnTypeCompleteString: return gd::variant_view( (const char*)pbBuffer, (size_t)pcolumn->size() );
      case eColumnTypeCompleteUtf8String: return gd::variant_view( gd::variant_type::utf8( (const char*)pbBuffer, (size_t)pcolumn->size()) );
      case eColumnTypeCompleteBinary: return gd::variant_view( (unsigned char*)pbBuffer, (size_t)pcolumn->size() );
         
      default: assert(false);
      }
   }

   return gd::variant_view();
}

/** -----------------------------------------------------------------------------------------------
 * @brief Get value in named column
 * @param stringName column name for returned value
 * @return gd::variant_view value is placed and returned in variant_view
*/
gd::variant_view record::get_variant_view( const std::string_view& stringName ) const
{
   int iColumnIndex = get_column_index_for_name( stringName );
   if( iColumnIndex != -1 ) return get_variant_view( iColumnIndex );

   return gd::variant_view();
}


/** -----------------------------------------------------------------------------------------------
 * @brief Collects values from record and place them in returned vector with `variant_view`
 * @return std::vector<gd::variant_view> get all values from current row
*/
std::vector<gd::variant_view> record::get_variant_view() const
{
   std::vector<gd::variant_view> vectorValue;
   vectorValue.reserve( get_column_count() );
   for( auto u = 0u, uTo = get_column_count(); u < uTo; u++ )
   { 
      vectorValue.push_back( get_variant_view( u ) );
   }
   return vectorValue;
}

/** -----------------------------------------------------------------------------------------------
 * @brief Collects record values from specified columns and place them in returned vector with `variant_view`
 * @param vectorIndex column index where values are read and inserted to vector
 * @return std::vector<gd::variant_view> vector with values
*/
std::vector<gd::variant_view> record::get_variant_view( const std::vector<unsigned>& vectorIndex ) const
{
   std::vector<gd::variant_view> vectorValue;
   vectorValue.reserve( vectorIndex.size() );
   for( auto it : vectorIndex )
   {                                                                                               assert( it < get_column_count() );
      vectorValue.push_back( get_variant_view( it ) );
   }
   return vectorValue;
}


/** -----------------------------------------------------------------------------------------------
 * @brief Return record values in arguments object
 * @return gd::argument::arguments arguments with values from current row
*/
gd::argument::arguments record::get_arguments() const
{
   gd::argument::arguments argumentsRow;

   for( unsigned uColumn = 0, uColumnCount = get_column_count(); uColumn < uColumnCount; uColumn++ )
   {
      auto stringName = name_get( uColumn );
      gd::variant_view variantviewValue = (*this)[uColumn];
      argumentsRow.append_argument( stringName, variantviewValue );
   }

   return argumentsRow;
}





/** ---------------------------------------------------------------------------
 * @brief return needed size to store value type, if 0 then value size is variable
 * @param eType type of value size is returned for
 * @return number of bytes needed to store value
*/
unsigned record::size_s( enumColumnTypeNumber eType )
{                                                                              assert( (unsigned)eType < 0xff );
   unsigned uSize = value_size_g( eType );

   return uSize;
}

namespace debug {

   std::string print( const record::column& column )
   {
      std::string stringPrint;

      stringPrint += "index: ";
      stringPrint += std::to_string( column.index() );
      stringPrint += "; type: ";
      stringPrint += gd::types::type_name_g( column.type() );
      stringPrint += "; size: ";
      stringPrint += std::to_string( column.size() );
      stringPrint += "; buffer size: ";
      stringPrint += std::to_string( column.size_buffer() );
      stringPrint += "; state: ";
      stringPrint += std::to_string( column.state() );
      stringPrint += "; ctype: ";
      stringPrint += std::to_string( (int)column.ctype() );

      return stringPrint;
   }

}


_GD_DATABASE_END


