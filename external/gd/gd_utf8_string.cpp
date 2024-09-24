/*
## Overview
| Name | Description |
| - | - |
|  |  |


*/


#include <stdexcept>
#include <vector>
#include <algorithm>

#include "gd_utf8_string.h"

#pragma warning( push )
#pragma warning( disable : 4244 4267 )

namespace gd { namespace utf8 { 

string::string( gd::utf8::buffer bufferStack ) {                              assert( bufferStack.m_uSize > sizeof( string::buffer ) );
   m_pbuffer = reinterpret_cast<string::buffer*>( bufferStack.m_pBuffer );
   m_pbuffer->capacity( bufferStack.m_uSize - sizeof( string::buffer ) );
   m_pbuffer->flags( eBufferStorageStack | eBufferStorageSingle );
   m_pbuffer->size( 0 );
   m_pbuffer->count( 0 );
   m_pbuffer->set_reference( 1 );
}

string::string( std::string_view stringText )
   : m_pbuffer(string::m_pbuffer_empty_reference)
{ 
   assign( stringText.data(), stringText.length() ); 
}

/**
 * @brief == operator comparing utf8 string with string_view
 * @param stringEqualWith string to compare with
 * @param stringEqualTo string to compare to
 * @return 
*/
bool operator==( const string& stringEqualWith, std::string_view stringEqualTo )
{
   auto uCount = count( stringEqualTo ).first;
   if( stringEqualWith.count() == uCount && uCount )
   {
      return stringEqualWith.compare( reinterpret_cast<const string::value_type*>(stringEqualTo.data()) );
   }

   return stringEqualWith.count() == uCount; // returns true if 0 length
}

std::ostream& operator<<( std::ostream& ostreamTo, const string& s)
{
   ostreamTo << s.c_str();
   return ostreamTo;
}


/**
 * @brief copy string object
 * @param o string object that is copied
*/
void string::copy(const string& o)
{
   m_pbuffer = o.m_pbuffer;

   if( o.m_pbuffer->is_common_empty() == false )
   {
      if(o.m_pbuffer->is_refcount() == true)
      {
         m_pbuffer->add_reference();
#        ifdef DEBUG
         m_psz = reinterpret_cast<const char*>( m_pbuffer->c_buffer() );
#        endif

      }
      else
      {
         _clone( o );
      }
   }
}


string& string::assign( const char* pbszText, uint32_t uLength )
{
   uint32_t uSize = gd::utf8::size( pbszText, uLength );
   if( string::is_empty( m_pbuffer ) == true ) allocate_exact( uSize );
   else if( uSize > m_pbuffer->capacity() ) allocate( uSize - m_pbuffer->capacity() );

   convert_ascii( pbszText, m_pbuffer->c_buffer() );

   m_pbuffer->size( m_pbuffer->size() + uSize );
   m_pbuffer->count( m_pbuffer->count() + uLength );

   return *this;
}

/**
 * @brief assign text to string
 * @param pbszText pointer to utf8 formated text assigned to string
 * @param uSize size in bytes
 * @param uCount number of utf8 characters
 * @return string reference
*/
string& string::assign( const value_type* pbszText, uint32_t uSize, uint32_t uCount )
{
   m_pbuffer = string::safe_to_modify(m_pbuffer);

   if( uSize > m_pbuffer->capacity() ) allocate_exact( uSize );

   memcpy( c_buffer(), pbszText, uSize );
   m_pbuffer->size( uSize );
   m_pbuffer->count( uCount );
   m_pbuffer->c_buffer_end()[0] = '\0';

   return *this;
}

void string::push_back( uint8_t ch )
{
   allocate( SIZE8_MAX_UTF_SIZE );

   auto pbszEnd = c_buffer_end();

   auto uSize = convert( ch, pbszEnd );
   pbszEnd[uSize] = '\0';

   m_pbuffer->size( m_pbuffer->size() + uSize );
   m_pbuffer->count( m_pbuffer->count() + 1 );
}



void string::push_back( uint16_t ch )
{
   allocate( SIZE16_MAX_UTF_SIZE );

   auto pbszEnd = c_buffer_end();
   auto uSize = convert( ch, pbszEnd );

   pbszEnd[uSize] = '\0';

   m_pbuffer->size( m_pbuffer->size() + uSize );
   m_pbuffer->count( m_pbuffer->count() + 1 );
}


/**
 * @brief append character store in 32 bit value
 * @param ch character added to string
 */
void string::push_back( uint32_t ch )
{
   allocate( SIZE32_MAX_UTF_SIZE );                                                     // add four bytes, max size for utf32_t character

   auto pbszEnd = c_buffer_end();
   auto uSize = convert( ch, pbszEnd );

   pbszEnd[uSize] = '\0';

   m_pbuffer->size( m_pbuffer->size() + uSize );
   m_pbuffer->count( m_pbuffer->count() + 1 );
}

string& string::append( const char* pbszText, uint32_t uLength )
{
   uint32_t uSize = gd::utf8::size( pbszText, uLength );
   allocate( uSize );

   auto pbszEnd = c_buffer_end();
   convert_ascii( pbszText, pbszEnd );

   m_pbuffer->size( m_pbuffer->size() + uSize );
   m_pbuffer->count( m_pbuffer->count() + uLength );

   return *this;
}

string& string::append( const value_type* puText, uint32_t uSize )
{
   return append( puText, uSize, gd::utf8::count( puText ).first );
}

string& string::append( const value_type* puText, uint32_t uSize, uint32_t uCount )
{
   allocate( uSize );

   auto puEnd = c_buffer_end();
   memcpy( puEnd, puText, uSize );

   m_pbuffer->size( m_pbuffer->size() + uSize );
   m_pbuffer->count( m_pbuffer->count() + uCount );
   m_pbuffer->c_buffer_end()[0] = '\0';

   return *this;
}


string& string::replace( const_iterator itFrom, const_iterator itTo, const_pointer pbszText, uint32_t uLength )
{                                                                             assert( uLength < 0x01000000 ); // realistic
                                                                              assert( string::verify_iterator( *this, itFrom ) == true );
                                                                              assert( string::verify_iterator( *this, itTo ) == true );

   std::size_t uSizeInString = itTo.get() - itFrom.get();// size in string that is replaced
   const_pointer pInsert = itFrom.get(); // reinterpret_cast<pointer>( itFrom.get() );

   // count number of characters that is removed
   auto uReplaceCount = gd::utf8::count( itFrom, itTo ).first;
   auto uInsertCount = gd::utf8::count( pbszText, pbszText + uLength ).first;

   if( uLength > uSizeInString )
   {                                                                          assert( uLength - uSizeInString < 0x01000000 ); // realistic
      pInsert = expand( pInsert, static_cast<uint32_t>(uLength - uSizeInString) );
   }
   else if( uLength < uSizeInString )
   { 
      contract( itTo.get(), static_cast<uint32_t>( uSizeInString - uLength) );
   }

   memcpy( (void*)pInsert, pbszText, uLength );
   m_pbuffer->size( size() + (uLength - uSizeInString) );
   m_pbuffer->count( count() + (uInsertCount - uReplaceCount) );

   return *this;
}

/**
 * @brief insert character X number of times in section between iterators
 * @param itFrom where to insert
 * @param itTo end of section that is replaced
 * @param uSize number of characters inserted
 * @param uCharacter character to insert
 * @return reference to string
*/
string& string::insert( iterator itFrom, iterator itTo, uint32_t uSize, uint32_t uCharacter )
{                                                                             assert( uCharacter < 0x01000000 ); // realistic
                                                                              assert( string::verify_iterator( *this, itFrom ) == true );
                                                                              assert( string::verify_iterator( *this, itTo ) == true );
   value_type pChar[4];                            // buffer storing character
   auto uCharLength = gd::utf8::convert( uCharacter, pChar );// number of character values needed for character
   std::size_t uSizeNeeded = uCharLength * uSize;  // size needed in string to store character
   std::size_t uSizeInString = itTo.get() - itFrom.get();// size in string that is replaced
   pointer pInsert = itFrom.get(); // reinterpret_cast<pointer>( itFrom.get() );

   if( uSizeNeeded > uSizeInString )
   {                                                                          assert( uSizeNeeded - uSizeInString < 0x01000000 ); // realistic
      pInsert = (pointer)expand( pInsert, static_cast<uint32_t>(uSizeNeeded - uSizeInString) );
   }
   else if( uSizeNeeded < uSizeInString )
   { 
      contract( itTo.get(), static_cast<uint32_t>( uSizeInString - uSizeNeeded) );
   }

   while( uSize-- )
   {
      switch( uCharLength )
      {
         case 0: throw std::runtime_error("invalid UTF-8 (operation = next)");
         case 1: *pInsert = *pChar; break;
         case 2: pInsert[0] = pChar[0]; pInsert[1] = pChar[1]; break;
         case 3: pInsert[0] = pChar[0]; pInsert[1] = pChar[1]; pInsert[2] = pChar[2]; break;
         case 4: pInsert[0] = pChar[0]; pInsert[1] = pChar[1]; pInsert[2] = pChar[2]; pInsert[3] = pChar[3]; break;
      }
      
      pInsert += uCharLength;                                                 assert( verify_iterator( *this, pInsert ) );
   }

   return *this;
}


/**
 * @brief squeezes string (compress) where ch parameter defines what character that will striped in the squeeze process
 * @param itFrom position from where to squeeze
 * @param itEnd position to end squeezing
 * @param ch character to identify what parts to remove
 * @return new size
*/
std::size_t string::squeeze( iterator itFrom, iterator itEnd, uint32_t ch )
{
   uint8_t pCharacter[4];
   auto uCharSize = convert( ch, pCharacter );

   iterator itInsert = itFrom;

   auto it = itFrom;
   for( ; it != itEnd; it.next() )
   {
      if( it.value32() == ch ) continue;

      if( itInsert != it ) copy_character( itInsert.get(), it.get() );

      itInsert++;
   }

   if( itEnd != cend() )
   {
      auto uSize = cend().get() - itEnd.get();
      memmove( itInsert.get(), itEnd.get(), uSize );
   }
   
   *itInsert.get() = '\0';
   m_pbuffer->size( itInsert.get() - begin().get() );
   m_pbuffer->count( gd::utf8::count( m_pbuffer->c_buffer() ).first );

   return 0;
}


/**
 * @brief find character in string
 * @param ch character to find
 * @return iterator to position in string where character is found, if iterator is equal to end then no character is found
*/
string::const_iterator string::find( value_type ch ) const
{
   auto p = gd::utf8::move::find( c_buffer(), ch );
   if( p != nullptr ) return const_iterator( p );

   return end();
}

/*
string::const_iterator string::find( std::string_view stringFind ) const
{
   if( stringFind.length() > size() ) return cend();

   auto uCount = gd::utf8::count( stringFind.data() ).first;
   if( uCount == stringFind.length() )
   {                                                                          // no need to allocate
      return find( reinterpret_cast<const_pointer>(stringFind.data()), stringFind.length() );
   }

   string stringFindCopy( stringFind );

   return find( stringFind );
}
*/
/**
 * @brief find character in string
 * @param ch character to find
 * @param itFrom position to start search from
 * @return iterator to position in string where character is found, if iterator is equal to end then no character is found
*/
string::const_iterator string::find( value_type ch, const_iterator itFrom ) const
{                                                                             assert( string::verify_iterator( *this, itFrom ) == true );
   auto p = gd::utf8::move::find( itFrom, ch );
   if( p != nullptr ) return const_iterator( p );

   return end();
}

string::const_iterator string::find( const_pointer pbszText, uint32_t uLength ) const
{
   auto p = gd::utf8::move::find( c_buffer(), c_end(), pbszText, uLength );
   if( p != nullptr ) return const_iterator( p );

   return end();
}

/**
 * @brief Find text in string
 * @param itFrom start position in string (offset)
 * @param pbszFind text to find
 * @param uLength length of text
 * @return iterator to position in string where text is found, if iterator is equal to end then no character is found
*/
string::const_iterator string::find( const_iterator itFrom, const_pointer pbszFind, uint32_t uLength ) const
{                                                                             assert( string::verify_iterator( *this, itFrom ) == true );
   auto p = gd::utf8::move::find( itFrom.get(), c_end(), pbszFind, uLength );
   if( p != nullptr ) return const_iterator( p );

   return end();
}

/**
 * @brief Return string object with text between iterators
 * @param itFrom start position
 * @param itTo end position
 * @return string
*/
string string::substr( const_iterator itFrom, const_iterator itTo )
{
   string s(itFrom, itTo);
   return std::move( s );
   //return string( itFrom, itTo );
}

/**
 * @brief swap characters in internal buffer
 * @param it1 iterator to first character that is to be swapped
 * @param it2 iterator to second character that is to be swapped
*/
void string::swap( iterator it1, iterator it2 )
{                                                                             assert( string::verify_iterator( *this, it1 ) == true );
                                                                              assert( string::verify_iterator( *this, it2 ) == true );
   auto itBegin = begin();
   auto pbuffer = string::safe_to_modify( m_pbuffer );
   if( pbuffer != m_pbuffer )
   {
      m_pbuffer = pbuffer;
      auto it = begin();
      it1 = it.get() + (it1.get() - itBegin.get());
      it2 = it.get() + (it2.get() - itBegin.get());
   }

   auto uCharacterSize1 = gd::utf8::get_character_size( it1.get() );
   auto uCharacterSize2 = gd::utf8::get_character_size( it2.get() );
   if( uCharacterSize1 == uCharacterSize2 )
   {
      auto _temp = *it1;
      gd::utf8::convert( *it2, it1.get() );
      gd::utf8::convert( _temp, it2.get() );
   }
   else
   {

   }
}



/*
void string::sort( std::function<bool( const value_type v1, const value_type v2 )> compare )
{

}
*/

/*
template <class _BidIt, class _Pr>
_CONSTEXPR20 _BidIt _Insertion_sort_unchecked(const _BidIt _First, const _BidIt _Last, _Pr _Pred) {
    // insertion sort [_First, _Last)
    if (_First != _Last) {
        for (_BidIt _Mid = _First; ++_Mid != _Last;) { // order next element
            _BidIt _Hole               = _Mid;
            _Iter_value_t<_BidIt> _Val = _STD move(*_Mid);

            if (_DEBUG_LT_PRED(_Pred, _Val, *_First)) { // found new earliest element, move to front
                _Move_backward_unchecked(_First, _Mid, ++_Hole);
                *_First = _STD move(_Val);
            } else { // look for insertion point after first
                for (_BidIt _Prev = _Hole; _DEBUG_LT_PRED(_Pred, _Val, *--_Prev); _Hole = _Prev) {
                    *_Hole = _STD move(*_Prev); // move hole down
                }

                *_Hole = _STD move(_Val); // insert element in hole
            }
        }
    }

    return _Last;
}
*/

void string::sort( iterator itFirst, iterator itLast, std::function<bool( value_type v1, value_type v2 )> compare )
{                                                                              assert( itLast.get() - itFirst.get() < 0x00100000 );
                                                                               assert( string::verify_iterator( *this, itFirst ) == true );
                                                                               assert( string::verify_iterator( *this, itLast ) == true );
   auto pbuffer = string::safe_to_modify(m_pbuffer);
   if( pbuffer != m_pbuffer )
   {
      itFirst = m_pbuffer->move_to( pbuffer, itFirst.get() );
      itLast = m_pbuffer->move_to( pbuffer, itLast.get() );
      m_pbuffer = pbuffer;                                                     DEBUG_ONLY( m_psz = m_pbuffer->c_str() );
   }
   std::vector<uint32_t> vectorValue;
   for( auto it = itFirst; it != itLast; it++ ) vectorValue.push_back( *it );
   std::sort( vectorValue.begin(), vectorValue.end(), compare );
   for( auto it : vectorValue )
   {
      gd::utf8::convert( it, itFirst.get() );
      itFirst++;
   }

/*
   TODO: sort without heap allocation is hard because of different sizes for each character.
   if( itFirst < itLast )
   {
      for( auto itMiddle = itFirst; ++itMiddle != itLast;)
      {                                                                        // order next element
         auto itHole = itMiddle;                                               // position for value that is checked
         auto uValue = *itMiddle;
         if( compare( uValue, *itFirst ) == true )
         {
            auto it = itHole + 1;                                              // position after hole to get move section
            auto uSize = itMiddle.get() - itFirst.get();                       // memory that is moved
#ifdef DEBUG
            intptr_t iFirst = (intptr_t)itFirst.get() % 1000;
            intptr_t iMiddle = (intptr_t)itMiddle.get() % 1000;
            intptr_t iDifference = iMiddle - iFirst;
            intptr_t iIt = (intptr_t)it.get() % 1000;
            intptr_t iDifferenceIt = iIt - iFirst;
#endif // DEBUG

            if( gd::utf8::get_character_size( itMiddle.get() ) != gd::utf8::get_character_size( itFirst.get() ) )
            {
               // reposition iterator in buffer because character sizes do not match for the moved
               if( gd::utf8::get_character_size( itMiddle.get() ) < gd::utf8::get_character_size( itFirst.get() ) ) itMiddle = itMiddle.get() - 1;
               else                                                                                                 itMiddle = itMiddle.get() + 1;
            }


            
            //if( gd::utf8::get_character_size( itMiddle.get() ) == gd::utf8::get_character_size( itFirst.get() ) )
            {                                                                  assert( it.get() <= (c_buffer() + size()) );
               memmove( it.get() - uSize, itFirst.get(), uSize );              // move memory
            }
            else
            {                                                                  assert( it.get() <= (c_buffer() + size()) );
               memmove( it.get() - uSize, itFirst.get(), uSize );              // move memory
            }
            gd::utf8::convert( uValue, itFirst.get() );                        // insert value to first position
         }
         else
         {
            // ## check if value is to be inserted  after first
            auto it = itHole;
            for( ; compare( uValue, *--it ); itHole = it )
            {
               gd::utf8::convert( *it, itHole.get() );
            }
            if( itHole != itMiddle ) gd::utf8::convert(uValue, itHole.get());
         }
      }
   }
   */
}

/**
 * @brief Erase characters from string
 * @param itFirst position from characters are to be erased
 * @param itLast last position for characters that is to be removed
 * @param bCount if count in string is to be updated (this will increase time and may not be needed if you are doing multiple erase and just want to update count in the end)
 * @return 
*/
string::iterator string::erase( iterator itFirst, iterator itLast, bool bCount )
{                                                                             assert( itLast.get() > itFirst.get() ); assert( itLast.get() <= end().get() );

   if( bCount == true ) m_pbuffer->count( gd::utf8::count( m_pbuffer->c_buffer() ).first );

   uint32_t uMoveSize{0};
   uint32_t uRemoveSize = static_cast<uint32_t>(itLast.get() - itFirst.get());

   if( itLast != end() ) { uMoveSize = static_cast<uint32_t>(end().get() - itLast.get()); }

   if( uMoveSize > 0 )
   {
      std::memcpy( itFirst.get(), itLast.get(), uMoveSize );                   // close gap
   }
   
   m_pbuffer->size( m_pbuffer->size() - uRemoveSize );
   m_pbuffer->c_buffer_end()[0] = '\0';

   if( bCount == true ) m_pbuffer->count( gd::utf8::count( m_pbuffer->c_buffer() ).first );

   return itFirst;
}

/**
 * @brief Expand string and make space at position
 * @param pPosition pointer to position in string where we need to make space
 * @param uSize 
 * @return pointer to same position in string, if new buffer pointer is placed at same position in new buffer
*/
string::const_pointer string::expand( const_pointer pPosition, uint32_t uSize )
{
   auto uOffset = pPosition - c_buffer();                                     // offset to position in string that is expanded

   allocate( uSize );                                                         // allocate if needed
   pPosition = c_buffer() + uOffset;                                          // reposition position if allocation was done
   auto uMoveSize = size() - uOffset;                                         // calculate size to move
   // move data from position to end to new position and make a uSize gap
   std::memmove( (uint8_t*)pPosition + uSize, pPosition, uMoveSize + 1 );               // move data to make gap, add one will add the ending \0 character.

   m_pbuffer->size( m_pbuffer->size() + uSize );
   return c_buffer() + uOffset;
}

/**
 * @brief Contracts string. Moves section between position forward in string uSize bytes
 * @param pvPosition moves all characters forward after this position in string 
 * @param uSize number of bytes to move character
 * @return new position in string where the previous position was
*/
string::const_pointer string::contract( const_pointer pvPosition, uint32_t uSize )
{
   auto uMoveSize = c_buffer_end() - pvPosition;
   std::memmove( (uint8_t*)pvPosition - uSize, pvPosition, uMoveSize );
   m_pbuffer->size( m_pbuffer->size() - uSize );
   m_pbuffer->null_terminate();
   return pvPosition - uSize;
}


/**
 * @brief allocate new buffer for string if needed
 * @param uSize added size string needs to have
*/
void string::allocate(uint32_t uSize)
{                                                                              assert( m_pbuffer->check_flags() );
   //uSize += sizeof(string::buffer);
   if( uSize + m_pbuffer->size() >= m_pbuffer->capacity() )
   {
      uint32_t uFlags = m_pbuffer->is_type_reference() ? eBufferStorageReferenceCount : eBufferStorageSingle;
      auto _size_old = m_pbuffer->size(); // + sizeof(string::buffer);
      uint32_t uSizeAll = uSize + m_pbuffer->size() + sizeof(string::buffer);

      if( uSizeAll < 64 ) uSizeAll = 64;
      else if( uSizeAll < 1024 ) uSizeAll = 1024;
      else
      {
         uint32_t uAdd = uSizeAll % 4096;
         uSizeAll += uAdd;

         if( uAdd < 64 ) uSizeAll += 4096;
      }

      uint8_t* puNew = new uint8_t[uSizeAll];
      memcpy( puNew, m_pbuffer, _size_old + sizeof(string::buffer) );          assert( _size_old == 0 || m_pbuffer->c_buffer_end()[0] == '\0' );
      string::release( m_pbuffer );
      m_pbuffer = reinterpret_cast<string::buffer*>( puNew );
      m_pbuffer->set_reference( 1 );
      m_pbuffer->flags(uFlags);
      m_pbuffer->capacity( uSizeAll - sizeof(string::buffer) );              // new capacity after increased size
      m_pbuffer->null_terminate();
#  ifdef DEBUG
      m_psz = reinterpret_cast<const char*>( m_pbuffer->c_buffer() );
#  endif
   }
}

/**
 * @brief Allocate buffer and specify the exact buffer size
 * @param uSize size buffer gets
*/
void string::allocate_exact(uint32_t uSize)
{                                                                               assert( uSize < 0x01000000 ); // realistic !!
                                                                                assert( m_pbuffer->check_flags() );

/*
   buffer* pbufferOld = m_pbuffer;

   // ## allocate buffer
   auto _size = uSize + sizeof(string::buffer);
   uint8_t* puNew = new uint8_t[_size + 1];                                     // exact size + zero ending
   m_pbuffer = reinterpret_cast<string::buffer*>( puNew );

   if( string::is_empty( pbufferOld ) == false )                               // if old buffer has a valid string, copy
   {
      m_pbuffer->capacity( uSize );
      if( pbufferOld->size() > 0 )
      {
         if( pbufferOld->size() > uSize )                                       // if string is larger then we need to walk backwards to find where to cut, utf8 remember... a character may be stored in multiple bytes
         {
            auto uCount = pbufferOld->count();
            auto pubszEnd = pbufferOld->c_buffer_end();
            while( uCount > 0 && pubszEnd - pbufferOld->c_buffer() > uSize )
            {
               pubszEnd = move::previous( pubszEnd );
               uCount--;
            }

            uSize = static_cast<uint32_t>( pubszEnd - pbufferOld->c_buffer() );
            m_pbuffer->count( uCount );
            m_pbuffer->size( uSize );
         }
                                                                               assert( pbufferOld->c_buffer_end()[0] == '\0' );
         memcpy( m_pbuffer->c_buffer(), pbufferOld->c_buffer(), uSize );      
         m_pbuffer->c_buffer()[uSize] = '\0';
#        ifdef DEBUG
         m_psz = reinterpret_cast<const char*>( m_pbuffer->c_buffer() );
#        endif
         string::release( pbufferOld );
      }
   }
   else
   {
      m_pbuffer->reset( uSize );
#     ifdef DEBUG
      m_psz = reinterpret_cast<const char*>( m_pbuffer->c_buffer() );
#     endif
   }
   */

   allocate_exact(*this, uSize);
#  ifdef DEBUG
   m_psz = reinterpret_cast<const char*>(m_pbuffer->c_buffer());
#  endif

}

void string::allocate_exact(string& stringObject, uint32_t uSize)
{                                                                                assert(uSize < 0x01000000); // realistic !!
   buffer* pbufferOld = stringObject.m_pbuffer;
   auto pbuffer = stringObject.m_pbuffer;

   // ## allocate buffer
   auto _size = uSize + sizeof(string::buffer);
   uint8_t* puNew = new uint8_t[_size + 1];                                     // exact size + zero ending
   pbuffer = reinterpret_cast<string::buffer*>(puNew);
   uint32_t uFlags = pbufferOld->is_type_reference() ? eBufferStorageReferenceCount : eBufferStorageSingle;

   if(string::is_empty(pbufferOld) == false)                                    // if old buffer has a valid string, copy
   {
      pbuffer->capacity(uSize);                                                 // set buffer size
      if(pbufferOld->size() > 0)
      {
         if(pbufferOld->size() > uSize)                                         // if string is larger then we need to walk backwards to find where to cut, utf8 remember... a character may be stored in multiple bytes
         {
            auto uCount = pbufferOld->count();
            auto pubszEnd = pbufferOld->c_buffer_end();
            while(uCount > 0 && pubszEnd - pbufferOld->c_buffer() > uSize)
            {
               pubszEnd = move::previous(pubszEnd);
               uCount--;
            }

            uSize = static_cast<uint32_t>(pubszEnd - pbufferOld->c_buffer());
            pbuffer->count(uCount);
            pbuffer->size(uSize);
         }
         assert(pbufferOld->c_buffer_end()[0] == '\0');
         memcpy(pbuffer->c_buffer(), pbufferOld->c_buffer(), uSize);
         pbuffer->c_buffer()[uSize] = '\0';
         string::release(pbufferOld);
      }
   }
   else
   {
      pbuffer->reset(uSize);
   }
   pbuffer->flags(uFlags);                                                       // set flags

   stringObject.m_pbuffer = pbuffer;

#  ifdef DEBUG
   stringObject.m_psz = reinterpret_cast<const char*>(pbuffer->c_buffer());
#  endif

}


bool string::verify_iterator( const string& stringObject, const_pointer p )
{
   auto itBegin = std::begin( stringObject );
   auto itEnd = std::end( stringObject );

   if( p >= itBegin && p <= itEnd ) return true;
   return false;
}


/**
 * @brief clone string
 * This will always create a new copy of string, no reference copy
 * @param o string that is copied
*/
void string::_clone(const string& o)
{
   if( o.m_pbuffer->is_refcount() )
   {
      m_pbuffer = o.m_pbuffer;
      m_pbuffer->add_reference();
   }
   else if( o.m_pbuffer->is_common_empty() == false )
   {
      allocate_exact(o.size());
      m_pbuffer->flags( eBufferStorageSingle );                               // string type is referenced counted
      m_pbuffer->size(o.size());
      m_pbuffer->count(o.count());
      m_pbuffer->set_reference(1);
      memcpy(c_buffer(), o.c_buffer(), size());
   }

   /*
   if(m_pbuffer != string::m_pbuffer_empty)
   {                                                                           assert( o.m_pbuffer->is_refcount() == false ); // only clone ref counted
                                                                               assert( o.m_pbuffer->get_reference() > 1 );    // no need to clone if single
      m_pbuffer->release();                                                   // release referenced string before create new copy
      allocate_exact(o.size());
      m_pbuffer->flags( eBufferStorageReferenceCount );                       // string type is referenced counted
      m_pbuffer->size(o.size());
      m_pbuffer->count(o.count());
      m_pbuffer->set_reference(1);
      memcpy(c_buffer(), o.c_buffer(), size());
   }
   */

#  ifdef DEBUG
   m_psz = reinterpret_cast<const char*>( m_pbuffer->c_buffer() );
#  endif
}

string::buffer* string::buffer::clone()
{
   auto _size = size() + sizeof(string::buffer);
   uint8_t* puNew = new uint8_t[_size + 1];                                    // exact size + zero ending
   auto pbuffer = reinterpret_cast<string::buffer*>(puNew);
   memcpy( puNew, this, _size ); 
   memcpy(pbuffer->c_buffer(), c_buffer(), size() + 1 );                       // add null terminator
   uint32_t uFlags = is_type_reference() ? eBufferStorageReferenceCount : eBufferStorageSingle;
   pbuffer->flags(uFlags);                                                     // set flags
   pbuffer->capacity( size() );
   return pbuffer;
}


} }

#pragma warning( pop )