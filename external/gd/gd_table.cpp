#include <stdexcept>
#include <charconv> 


#include "gd_table_column-buffer.h"

#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wunused-variable"
   #pragma clang diagnostic ignored "-Wunused-but-set-variable"
#elif defined( __GNUC__ )
#elif defined( _MSC_VER )
#endif


_GD_TABLE_BEGIN

// ============================================================================
// ====================================================================== names
// ============================================================================


template<int iGrowBy>
std::pair<uint8_t*, unsigned> reserve_g( uint8_t* pbBuffer, unsigned uSize, unsigned uMaxSize )
{
   if(uSize > uMaxSize)
   {
      unsigned uNewSize = uSize + (iGrowBy - uSize % iGrowBy);                 // new size + exgtra for avoiding to many allocations

      // ## create new buffer, copy from old buffer and update members
      uint8_t* pbNewBuffer = new uint8_t[uNewSize];
      if( uMaxSize > 0 )                                                       // do we need to copy existing data
      {                                                                        assert( pbBuffer != nullptr );
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
uint16_t names::add(const std::string_view& stringName)
{                                                                              assert( stringName.length() < 5000 ); // realistic
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


references::references( const references& o )
{
   for( auto it = std::begin( o.m_vectorReference ), itEnd = std::end(o. m_vectorReference ); it != itEnd; it++ )
   {
      const reference* preferenceFrom = (const reference*)it->get();
      reference* preference = allocate( *preferenceFrom );
      copy_data_s( preference, preferenceFrom->data(), preferenceFrom->size() );
   }
}

references& references::operator=( const references& o )
{
   for( auto it = std::begin( o.m_vectorReference ), itEnd = std::end(o. m_vectorReference ); it != itEnd; it++ )
   {
      const reference* preferenceFrom = (const reference*)it->get();
      reference* preference = allocate( *preferenceFrom );
      copy_data_s( preference, preferenceFrom->data(), preferenceFrom->size() );
   }
   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Adds a new reference to the internal collection using the provided variant view and returns its index.
 * @param v_ A constant reference to a gd::variant_view object containing the value to be referenced.
 * @return The index of the newly added reference in the internal vector, as a uint64_t.
 */
uint64_t references::add( const gd::variant_view& v_ )
{
   unsigned uSize = gd::types::value_size_g( v_.type(), v_.length()  );        // Get needed size to store value in bytes
   reference reference_( v_.type(), v_.length(), uSize );                                          assert( v_.length() <= uSize );

   reference* preference = allocate( reference_ );
   copy_data_s( preference, v_.get_value_buffer(), uSize );
#if DEBUG_RELEASE > 0
   //copy_data_s( preference, v_.get_value_buffer(), uSize );                                        
                                                                                                   DEBUG_RELEASE_EXECUTE( preference->assert_valid_d() );
#endif
   return m_vectorReference.size() - 1;
}

std::byte* references::add( uint64_t uSize, tag_buffer )
{                                                                                                  assert( uSize > sizeof(reference) );
   auto uValueSize = uSize - sizeof(reference);                                // total size needed (add reference object size) 
   reference reference_(gd::variant_type::eTypeUnknown, (unsigned)uValueSize, (unsigned)uValueSize);  assert(uSize <= 0x0fffffff); // realistic
   reference* preference = allocate( reference_ );

   return (std::byte*)preference;
}


void references::set( uint64_t uIndex, const uint8_t* puData, unsigned uSize )
{                                                                                                  assert( uIndex < m_vectorReference.capacity() );
   auto& preference = m_vectorReference[uIndex]; 
   copy_data_s( (reference*)preference.get(), puData, uSize );
}

int64_t references::find( const gd::variant_view& variantviewFindValue ) const noexcept
{
   for( auto it = std::begin( m_vectorReference ), itEnd = std::end( m_vectorReference ); it != itEnd; it++ )
   {
      const reference* preference = (const reference*)it->get();
      if( preference->length() == variantviewFindValue.length() )
      {
         if( memcmp( preference->data(), variantviewFindValue.get_value_buffer(), preference->size() ) == 0 ) // compare data
         {
            return std::distance( std::begin( m_vectorReference ), it );       // return index to data in list
         }
      }
   }
   return -1;                                                                  // no match, return -1 meaning that index is not found
}

/** ---------------------------------------------------------------------------
 * @brief allocate reference object and the amount of data that reference describes
 * Reference is used to store blob data in tables. It works like a pointer
 * and can store up to max value in `unsigned` (32 bit value). Values in cells will
 * store the index to the reference, references are stored separately and accessed
 * using indexes.
 * @param referenceCopy reference object describing data block where value is stored
 * @return pointer to reference
*/
reference* references::allocate( const reference& referenceToCopy )
{                                                                                                  assert( referenceToCopy.reference_count() == 1 );
#if DEBUG_RELEASE > 0
   auto uSize_d = sizeof(reference) + referenceToCopy.capacity();
#endif // DEBUG_RELEASE
   // To avoid errors using sanitizers a custom deleter is used that deletes the memory block allocated
   // with similar technique as it was allocated.
   unsigned uTotalSize = sizeof(reference) + referenceToCopy.capacity(); // Total size = reference object size and data


   //std::unique_ptr<reference, decltype(&reference::delete_reference_s) > preference = std::unique_ptr<reference, decltype(&reference::delete_reference_s) >( (reference*) new uint8_t[ uTotalSize ], reference::delete_reference_s );
   
#if DEBUG_RELEASE > 0
   // increase allocated size with 2 bytes to add debug markers, this may be deleted when everything is tested and works
   std::unique_ptr<uint8_t[]> pReference = std::make_unique<uint8_t[]>( uTotalSize + 2 ); 
#else
   std::unique_ptr<uint8_t[]> pReference = std::make_unique<uint8_t[]>( uTotalSize );
#endif // DEBUG_RELEASE
   
   reference* preferenceRaw = (reference*)pReference.get();
   
   memcpy(static_cast<void*>(preferenceRaw), &referenceToCopy, sizeof(reference) );
#if DEBUG_RELEASE > 0
   *preferenceRaw->data_end( 1 ) = uTailetextMarker_d;
   *preferenceRaw->data_end( 2 ) = uTailetextMarker_d;
   preferenceRaw->m_uAllocated_d = uTotalSize;
   preferenceRaw->m_puClone_d = nullptr;                                       // this need to be copied as soon as reference value is set
#endif // DEBUG_RELEASE


   m_vectorReference.push_back( std::move( pReference ) );

   return preferenceRaw;
}


void references::copy_data_s( reference* preference, const uint8_t* puData, unsigned uSize )
{                                                                                                  assert( preference->capacity() >= uSize );
   if( uSize > preference->capacity() )
   {
      std::string stringError( (const char*)puData, uSize - 1 );
      stringError += " [text do not fit within buffer] [size to copy: ";
      stringError += std::to_string( uSize );
      stringError += " size in buffer:  ";
      stringError += std::to_string( preference->capacity() );
      stringError += "]";
      throw std::runtime_error( stringError );
   }

   memcpy( preference->data(), puData, uSize );
#if DEBUG_RELEASE > 0
                                                                                                   DEBUG_RELEASE_EXECUTE( preference->clone_d() ); DEBUG_RELEASE_EXECUTE( preference->assert_valid_d() );
#endif // DEBUG_RELEASE
}

#if DEBUG_RELEASE > 0

void reference::assert_valid_d() const
{
   if( *data_end( 1 ) != uTailetextMarker_d || *data_end( 2 ) != uTailetextMarker_d || compare_d() == false )
   {
      std::string stringError( "Value = \"" );
      stringError.append( (const char*)data(), length() );
      stringError += "\" should be = \"";
      stringError.append( (const char*)data_clone_d(), length() );
      stringError += "\" [text do not fit within buffer or is different] [size to copy: ";
      stringError += std::to_string( length() );
      stringError += " size in buffer:  ";
      stringError += std::to_string( capacity() );
      stringError += "]";
      throw std::runtime_error( stringError );
   }
}

std::string reference::dump_d() const
{
   std::string stringDump;
   stringDump.append( "value = \"");
   stringDump.append( (const char*)data(), length() );
   stringDump.append( "\",\n");
   stringDump += "length = " + std::to_string( length() ) + "\n";
   bool bMarker = *data_end( 1 ) == uTailetextMarker_d;
   if( bMarker == true ) bMarker = *data_end( 2 ) == uTailetextMarker_d;
   if( bMarker == true ) stringDump.append( "tail marker is ok\n");
   else                  stringDump.append( "tail marker failed, someting has overwritten value in buffer\n");

   if( bMarker == false )
   {
      stringDump += "marker 1 is = " + std::to_string( (unsigned)*data_end( 1 ) ) + " should be " + std::to_string( uTailetextMarker_d ) + "\n";
      stringDump += "marker 2 is = " + std::to_string( (unsigned)*data_end( 2 ) ) + " should be " + std::to_string( uTailetextMarker_d ) + "\n";
   }
   
   return stringDump;
}

void reference::clone_d() {
   if( m_puClone_d == nullptr ) { m_puClone_d = new uint8_t[m_uAllocated_d]; }

   memcpy( m_puClone_d, this, m_uAllocated_d );
}

#endif // DEBUG_RELEASE


/** ---------------------------------------------------------------------------
 * @brief assigns value to argument column based on position and how many values it is in vector
 * Note that this method do not allocate and copy text, it just sets positions 
 * so you cant use it more than temporarily.
 * @param columnAssignTo reference to argument column getting values
 * @param vectorColumnData  vector with string_view items assigned to column
 * @return true if ok, false and error information if failed
 */
std::pair<bool, std::string> assign_to_column_g( argument::column& columnAssignTo, const std::vector<std::string_view>& vectorColumnData )
{
   using namespace gd::types;

   auto uArgumentCount = vectorColumnData.size();
   if( uArgumentCount == 1 )
   {                                                                                               assert( columnAssignTo.type() != 0 );
      columnAssignTo.name( vectorColumnData.at(0) );                           // only one argument means that it is the name
   }
   else                                                                        // more than one value then first is value type
   {
      columnAssignTo.type( gd::types::type_g( vectorColumnData.at( 0 ) ) );
      if( columnAssignTo.type() == 0 )
      {
         std::string stringError("type error: ");
         stringError += vectorColumnData.at( 0 );
         return { false, stringError }; 
      }

      if(uArgumentCount == 2)
      {
         columnAssignTo.name( vectorColumnData.at( 1 ) );                      // two arguments and second is always name
      }
      else
      {                                      
         // ## read "type,length,name,alias" or "type,name,alias"

         unsigned uIndex = 1;

         std::string_view string_ = vectorColumnData.at( uIndex );
         if(is_ctype_g( string_[0], "digit"_ctype ) == true)                   // if digit this is max value size for column
         {
            unsigned uSize;
            auto result_ = std::from_chars(string_.data(), string_.data() + string_.size(), uSize);
            if( result_.ec != std::errc() ) { return { false, string_.data() }; }

            columnAssignTo.size( uSize );

            uIndex++;
            string_ = vectorColumnData.at( uIndex );
         }

         columnAssignTo.name( string_ );
         uIndex++;

         if(uArgumentCount > uIndex)                                          // do we have a alias?
         {
            columnAssignTo.alias( vectorColumnData.at( uIndex ) );
            uIndex++;
         }
      }
   }

   return { true, "" };
}



_GD_TABLE_END

