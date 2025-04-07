#include "gd_expression_runtime.h"

_GD_EXPRESSION_BEGIN

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