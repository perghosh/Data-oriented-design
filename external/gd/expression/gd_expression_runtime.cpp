#include <cstring>

#include "gd_expression_runtime.h"

_GD_EXPRESSION_BEGIN


/// @brief find method by name, returns pointer to method or nullptr if not found
const method* runtime::find_method(const std::string_view& stringName) const 
{
   // Define a lambda for comparing the method name with the search string
   auto compare_ = [](const method* pmethod, const std::string_view& stringName) -> bool {
      bool bCompare = pmethod->name()[0] == stringName[0]; // compare first character
      if( bCompare == false ) return false; // if first character is not equal, return false
      return std::string_view(pmethod->name()) < stringName;
   };

   const auto& tupleMethod_ = m_vectorMethod[0];
   const method* pmethodBegin = std::get<1>(tupleMethod_);
   const method* pmethodEnd = pmethodBegin + std::get<0>(tupleMethod_);
   const method* pmethodFind = std::lower_bound(pmethodBegin, pmethodEnd, stringName );
   if( pmethodFind != pmethodEnd )
   {                                                                                               assert( pmethodFind->name() == stringName );
      return pmethodFind;
   }

   return nullptr; // Return nullptr if no match is found
}

/** ---------------------------------------------------------------------------
 * @brief Finds a method by name within a specified namespace
 *
 * @param stringName The fully qualified method name, expected format "namespace::method"
 * @param tag_namespace Namespace tag parameter (unused in function body)
 * @return const method* Pointer to the found method, or nullptr if no match is found
 * 
 * @note The function searches through m_vectorMethod (starting from the second element).
 * When match against namespace it tries to find the method within that namespace. It 
 * uses std::lower_bound with a custom comparator for lookup. 
*/
const method* runtime::find_method(const std::string_view& stringName, tag_namespace) const 
{
   // Define a lambda for comparing the method name with the search string
   auto compare_ = [](const method* pmethod, const std::string_view& stringName) -> bool {
      bool bCompare = pmethod->name()[0] == stringName[0]; // compare first character
      if( bCompare == false ) return false; // if first character is not equal, return false
      return std::string_view(pmethod->name()) < stringName;
      };

   for( auto it = m_vectorMethod.cbegin() + 1; it != m_vectorMethod.cend(); ++it )
   {
      const auto& stringNamespace = std::get<2>(*it);

      if( std::memcmp(stringNamespace.data(), stringName.data(), stringNamespace.length()) != 0 ) continue; // skip if namespace does not match

      // Get the method name after the namespace, name is in format "namespace::method"
      std::string_view stringMethod = stringName.substr(stringNamespace.length() + 2); // get method name after namespace
      const method* pmethodBegin = std::get<1>(*it);
      const method* pmethodEnd = pmethodBegin + std::get<0>(*it);
      const method* pmethodFind = std::lower_bound(pmethodBegin, pmethodEnd, stringMethod );
      if( pmethodFind != pmethodEnd )
      {                                                                                            //assert( stringName.find( pmethodFind->name() ) != std::string::npos );
         if( stringMethod == pmethodFind->name() ) return pmethodFind;
         return nullptr;
      }
   }

   return nullptr; // Return nullptr if no match is found
}


/// @brief find variable by name, returns index or -1 if not found
int runtime::find_variable(const std::string_view& stringName) const
{
   for( size_t u = 0; u < m_vectorVariable.size(); ++u )
   {
      if( m_vectorVariable[u].first == stringName ) return static_cast<int>(u);
   }
   return -1;
}

/// @brief get variable value by index
const value::variant_t& runtime::get_variable(size_t uIndex) const
{                                                                                                  assert(uIndex < m_vectorVariable.size());
   return m_vectorVariable[uIndex].second;
}

bool runtime::find_value(const std::string_view& stringName, value::variant_t* pvariant_) 
{
   bool bFound = false; // if variable is found or not
   if( m_functionFind )                                                        // if function is set
   {
      bFound = m_functionFind(stringName, pvariant_);                          // call the function to get value externally
   }
   else
   {
      int iIndex = find_variable(stringName);                                   // find variable in vector, vector that stores variables in runtime
      if( iIndex >= 0 )
      {
         *pvariant_ = get_variable(iIndex);
         bFound = true;
      }
   }

   return bFound;
}

std::string runtime::dump() const
{
   std::string stringResult;
   for( const auto& pair : m_vectorVariable )
   {
      if( !stringResult.empty() ) stringResult += ", ";
      stringResult += pair.first + " = " + value( pair.second ).as_string();
   }

   stringResult += '\n';

   return stringResult;
}



_GD_EXPRESSION_END