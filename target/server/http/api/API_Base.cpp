// @FILE [tag: api, base] [summary: Implementation of base class for API commands] [type: source] [name: API_Base.cpp]

#include "../Application.h"

#include "API_Base.h"

// ## implementation -------------------------------------------------------------

/** -------------------------------------------------------------------------- CAPI_Base::common_construct
 * @brief Copy constructor implementation for CAPI_Base
 * 
 * @param o Source object to copy from
 */
 /*
void CAPI_Base::common_construct( const CAPI_Base& o )
{
    m_vectorCommand = o.m_vectorCommand;
    m_uCommandIndex = o.m_uCommandIndex;
    m_argumentsParameter = o.m_argumentsParameter;
    m_objects = o.m_objects;
    m_stringLastError = o.m_stringLastError;
    m_papplication = o.m_papplication;
}
*/

/** -------------------------------------------------------------------------- CAPI_Base::common_construct
 * @brief Move constructor implementation for CAPI_Base
 * 
 * @param o Source object to move from
 */
void CAPI_Base::common_construct( CAPI_Base&& o ) noexcept
{
    m_vectorCommand = std::move( o.m_vectorCommand );
    m_uCommandIndex = o.m_uCommandIndex;
    m_argumentsParameter = std::move( o.m_argumentsParameter );
    m_objects = std::move( o.m_objects );
    m_stringLastError = std::move( o.m_stringLastError );
    m_papplication = o.m_papplication;
    o.m_papplication = nullptr;
    o.m_uCommandIndex = 0;
}

/** -------------------------------------------------------------------------- CAPI_Base::GetDocument
 * @brief Retrieves the document associated with the current API instance.
 * 
 * @return Pointer to the CDocument object.
 * @note Note that document returned need a name for document or it will return the "default" document.
 */
CDocument* CAPI_Base::GetDocument()
{                                                                                                  assert( m_papplication != nullptr );
   if( m_pdocument != nullptr ) return m_pdocument;

   std::string stringDocument = m_argumentsParameter[{ {"document"}, {"doc"} }].as_string();
   if( stringDocument.empty() == true ) stringDocument = "default";

   CDocument* pdocument = m_papplication->DOCUMENT_Get( stringDocument );

   if( pdocument == nullptr ) { m_stringLastError = "document not found: " + stringDocument; } // generate error if document do not exists

   m_pdocument = pdocument;

   return pdocument;
}

/** -------------------------------------------------------------------------- CAPI_Base::GetArgumentIndex
 * @brief Returns the number of times a given argument name appears in the command list.
 * 
 * Note that it is possible to have multiple occurrences of the same command and in order to match
 * arguments correctly this method counts how many times the specified argument name appears from
 * the active command index.
 * 
 * @param stringName The name of the argument to search for.
 * @return The number of occurrences of the specified argument name in the command list.
 */
size_t CAPI_Base::GetArgumentIndex( const std::string_view& stringName ) const 
{                                                                                                  assert( m_vectorCommand.empty() == false && "No commands");
   size_t uCount = 0;
   for( unsigned uIndex = 0; uIndex < m_uCommandIndex; ++uIndex )
   {
      std::string_view stringCommand = m_vectorCommand[uIndex];
      if( stringCommand == stringName ) { ++uCount; }
   }
   return uCount;
}

/** --------------------------------------------------------------------------  CAPI_Base::GetArgumentIndex
 * @brief Returns the number of times a given argument name appears in the command list.
 * 
 * Note that it is possible to have multiple occurrences of the same command and in order to match
 * arguments correctly this method counts how many times the specified argument name appears from
 * the active command index.
 * 
 * @param stringFirst The first name of the argument to search for.
 * @param stringSecond The second name of the argument to search for.
 * @return The number of occurrences of the specified argument name in the command list.
 */
size_t CAPI_Base::GetArgumentIndex( const std::string_view& stringFirst, const std::string_view& stringSecond ) const 
{                                                                                                  assert( m_vectorCommand.empty() == false && "No commands");
   size_t uCount = 0;
   for( unsigned uIndex = 0; (uIndex + 1) < m_uCommandIndex; ++uIndex )
   {
      std::string_view stringCommandFirst = m_vectorCommand[uIndex];
      std::string_view stringCommandSecond = m_vectorCommand[uIndex + 1];
      
      if( stringCommandFirst == stringFirst || stringCommandSecond == stringSecond ) { ++uCount; }
   }
   return uCount;
}

/// @brief Check argument name exists
bool CAPI_Base::Exists(const std::string_view& stringName) const
{
   return m_argumentsParameter.exists(stringName);
}
