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


_GD_EXPRESSION_END