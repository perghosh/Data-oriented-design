/**
 * @file gd_expression_value.h
 * 
 * @brief 
 * 
 */

#include "../gd_compiler.h"

#include "gd_expression_value.h"


_GD_EXPRESSION_BEGIN

// ----------------------------------------------------------------------------
// ---------------------------------------------------------------------- value
// ----------------------------------------------------------------------------

bool value::get_bool() const 
{
   if( is_bool() ) return std::get<bool>(m_value);
   if( is_integer() ) return std::get<int64_t>(m_value) != 0;
   if( is_double() ) return std::get<double>(m_value) != 0.0;
   return false;
}

/// @brief get integer value, returns 0 unable to convert
int64_t value::as_integer() const
{
   if( is_integer() == true ) return std::get<int64_t>(m_value);
   if( is_double() == true ) return static_cast<int64_t>( std::get<double>(m_value) );
   if( is_bool() == true ) return static_cast<int64_t>( std::get<bool>(m_value) );
   if( is_string() == true )
   {
#if GD_COMPILER_HAS_CPP20_SUPPORT
      try { return std::stoll(std::get<std::string>(m_value)); }
      catch( ... ) { return 0; }
#else
      try {
         long long int iTemp = std::stoll(std::get<std::string>(m_value));
         if( iTemp > std::numeric_limits<int64_t>::max() || iTemp < std::numeric_limits<int64_t>::min() )
         {
            throw std::out_of_range("Value out of range for int64_t");
         }
         return static_cast<int64_t>(iTemp);
      }
      catch( const std::invalid_argument& e ) { return 0; }                   // Invalid string format
      catch( const std::out_of_range& e ) { return 0; }                       // Value too large for long long int
#endif // GD_COMPILER_HAS_CPP20_SUPPORT
   }
   return 0;
}

/// @brief get double value, converts integer if needed, returns 0.0 if unable to convert
double value::as_double() const 
{
   if( is_double() == true ) { return std::get<double>(m_value); } 
   else if( is_integer() == true ) { return static_cast<double>(std::get<int64_t>(m_value)); } 
   else if( is_bool() == true ) { return static_cast<double>(std::get<bool>(m_value)); }
   else if( is_string() == true ) 
   {
      try { return std::stod(std::get<std::string>(m_value));} 
      catch (...) 
      {
         return 0.0;
      }
   }
   return 0.0;
}

/// @brief get string value, converts other types if possible 
std::string value::as_string() const
{
   if( is_string() == true ) return std::get<std::string>(m_value);
   if( is_integer() == true ) return std::to_string(std::get<int64_t>(m_value));
   if( is_double() == true ) return std::to_string(std::get<double>(m_value));
   if( is_bool() == true ) return std::get<bool>(m_value) ? "true" : "false";
   return "";
}

/// @brief get string value, converts other types if possible 
std::string_view value::as_string_view() const
{
   if( is_string() == true ) return std::get<std::string>(m_value);
   return "";
}


/// @brief get boolean value, converts other types if possible
bool value::as_bool() const
{
   if( is_bool() == true ) return std::get<bool>(m_value);
   if( is_integer() == true ) return std::get<int64_t>(m_value) != 0;
   if( is_double() == true ) return std::get<double>(m_value) != 0.0;
   if( is_string() == true )
   {
      const auto& string_ = std::get<std::string>(m_value);
      return !string_.empty() && (string_ != "0" && string_ != "false");
   }
   return false;
}

gd::expression::variant_t value::as_variant() const 
{
   if(std::holds_alternative<std::string>(m_value)) 
   {
      return std::get<std::string>(m_value); // Return std::string directly
   } 
   else if(std::holds_alternative<int64_t>(m_value)) 
   {
      return std::get<int64_t>(m_value);
   } 
   else if(std::holds_alternative<double>(m_value)) 
   {
      return std::get<double>(m_value);
   }
   else if(std::holds_alternative<bool>(m_value)) 
   {
      return std::get<bool>(m_value);
   }
   return {}; // Default case (should not occur)
}


/// @brief attempt to convert current value to integer
bool value::to_integer() 
{
   if( is_integer() ) return true;
   if( is_double() ) { m_value = static_cast<int64_t>(std::get<double>(m_value)); return true; }
   if( is_bool() ) { m_value = static_cast<int64_t>(std::get<bool>(m_value)); return true; }
   if( is_string() ) 
   {
#if GD_COMPILER_HAS_CPP20_SUPPORT
      try { m_value = std::stoll(std::get<std::string>(m_value)); return true; }
      catch( ... ) { return false; }
#else
      try {
         long long int iTemp = std::stoll(std::get<std::string>(m_value));
         if( iTemp > std::numeric_limits<int64_t>::max() || iTemp < std::numeric_limits<int64_t>::min() )
         {
            throw std::out_of_range("Value out of range for int64_t");
         }
         m_value = static_cast<int64_t>(iTemp);
         return true;
      }
      catch( const std::invalid_argument& e ) { return 0; }                   // Invalid string format
      catch( const std::out_of_range& e ) { return 0; }                       // Value too large for long long int
#endif // GD_COMPILER_HAS_CPP20_SUPPORT
   }
   return false;
}
/// @brief attempt to convert current value to double
bool value::to_double() 
{
   if( is_double() ) return true;
   if( is_integer() ) { m_value = static_cast<double>(std::get<int64_t>(m_value)); return true; }
   if( is_bool() ) { m_value = static_cast<double>(std::get<bool>(m_value)); return true; }
   if( is_string() ) 
   {
      try { m_value = std::stod(std::get<std::string>(m_value)); return true; }
      catch( ... ) { return false; }
   }
   return false;
}

/// @brief attempt to convert current value to string
bool value::to_string()
{
   if( is_string() ) return true;
   if( is_integer() ) { m_value = std::to_string(std::get<int64_t>(m_value)); return true; }
   if( is_double() ) { m_value = std::to_string(std::get<double>(m_value)); return true; }
   if( is_bool() ) { m_value = std::get<bool>(m_value) ? "true" : "false"; return true; }
   return false;
}

/// @brief attempt to convert current value to boolean
bool value::to_bool()
{
   if( is_bool() ) return true;
   if( is_integer() ) { m_value = std::get<int64_t>(m_value) != 0; return true; }
   if( is_double() ) { m_value = std::get<double>(m_value) != 0.0; return true; }
   if( is_string() )
   {
      const auto& string_ = std::get<std::string>(m_value);
      m_value = !string_.empty() && ( string_ != "0" && string_ != "false" );
      return true;
   }
   return false;
}

bool value::synchronize( value& value_, void* )
{
   if( m_value.index() == value_.index() ) return true;

   bool bOk;

   switch( m_value.index() )
   {
   case 0: // integer
      bOk = value_.to_integer();
      break;
   case 1: // double
      bOk = value_.to_double();
      break;
   case 2: // string
      bOk = value_.to_string();
      break;
   case 3: // bool
      bOk = value_.to_bool();
      break;
   default:
      assert(false);
      bOk = false;
   }

   return bOk;
}

_GD_EXPRESSION_END