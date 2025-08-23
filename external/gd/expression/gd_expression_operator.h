/**
* @file gd_expression_value.h
* 
* @brief 
* 
*/

#pragma once

#include <cassert>
#include <fstream>
#include <functional>
#include <cstdint>
#include <string>
#include <utility>
#include <variant>

#include "gd_expression.h"

#ifndef _GD_EXPRESSION_BEGIN
#define _GD_EXPRESSION_BEGIN namespace gd { namespace expression {
#define _GD_EXPRESSION_END } }
#endif

_GD_EXPRESSION_BEGIN 


/** ---------------------------------------------------------------------------
 * @brief Adds two VALUE objects together
 * 
 * This function handles addition of integers, floating point numbers, and strings.
 * Values are synchronized to ensure type compatibility before operation.
 * 
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand
 * @param r_ Right operand
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of addition, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE add(VALUE& l_, VALUE& r_, RUNTIME* pruntime)
{
   bool bOk = l_.synchronize(r_, pruntime);
   if(bOk == true) 
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() + r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() + r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() + r_.get_string());
   }

   if(pruntime != nullptr) pruntime->add("[add] - Invalid addition operation", tag_error{});

   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Subtracts one VALUE object from another
 * 
 * This function handles subtraction of integers and floating point numbers.
 * Values are synchronized to ensure type compatibility before operation.
 * 
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand (minuend)
 * @param r_ Right operand (subtrahend)
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of subtraction, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE subtract(VALUE& l_, VALUE& r_, RUNTIME* pruntime)
{
   bool bOk = l_.synchronize(r_, pruntime);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() - r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() - r_.get_double());
   }
   if(pruntime != nullptr) pruntime->add("[subtract] - Invalid subtract operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Multiplies two VALUE objects
 * 
 * This function handles multiplication of integers and floating point numbers.
 * Values are synchronized to ensure type compatibility before operation.
 * 
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand (multiplicand)
 * @param r_ Right operand (multiplier)
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of multiplication, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE multiply(VALUE& l_, VALUE& r_, RUNTIME* pruntime)
{
   bool bOk = l_.synchronize(r_, pruntime);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() * r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() * r_.get_double());
   }
   if(pruntime != nullptr) pruntime->add("[multiply] - Invalid multiply operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Divides one VALUE object by another
 * 
 * This function handles division of integers and floating point numbers.
 * Values are synchronized to ensure type compatibility before operation.
 * 
 * @warning No check for division by zero is performed
 * 
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand (dividend)
 * @param r_ Right operand (divisor)
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of division, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE divide(VALUE& l_, VALUE& r_, RUNTIME* pruntime)
{
   bool bOk = l_.synchronize(r_, pruntime);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() / r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() / r_.get_double());
   }
   if(pruntime != nullptr) pruntime->add("[divide] - Invalid divide operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Calculates modulo (remainder) of division between two VALUE objects
 * 
 * This function handles modulo operation for integers only.
 * Values are synchronized to ensure type compatibility before operation.
 * 
 * @warning No check for modulo by zero is performed
 * 
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand (dividend)
 * @param r_ Right operand (divisor)
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of modulo operation, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE modulo(VALUE& l_, VALUE& r_, RUNTIME* pruntime)
{
   bool bOk = l_.synchronize(r_, pruntime);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() % r_.get_integer());
   }
   if(pruntime != nullptr) pruntime->add("[modulo] - Invalid modulo operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Checks if one VALUE object is greater than another
 * 
 * This function compares integers, floating point numbers, and strings.
 * Values are synchronized to ensure type compatibility before operation.
 * 
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand
 * @param r_ Right operand
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Boolean result of comparison, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE greater(VALUE& l_, VALUE& r_, RUNTIME* pruntime)
{
   bool bOk = l_.synchronize(r_, pruntime);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() > r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() > r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() > r_.get_string());
   }
   if(pruntime != nullptr) pruntime->add("[greater] - Invalid greater operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Checks if one VALUE object is less than another
 * 
 * This function compares integers, floating point numbers, and strings.
 * Values are synchronized to ensure type compatibility before operation.
 * 
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand
 * @param r_ Right operand
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Boolean result of comparison, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE less(VALUE& l_, VALUE& r_, RUNTIME* pruntime)
{
   bool bOk = l_.synchronize(r_, pruntime);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() < r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() < r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() < r_.get_string());
   }
   if(pruntime != nullptr) pruntime->add("[less] - Invalid less operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Checks if one VALUE object is greater than or equal to another
 * 
 * This function compares integers, floating point numbers, and strings.
 * Values are synchronized to ensure type compatibility before operation.
 * 
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand
 * @param r_ Right operand
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Boolean result of comparison, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE greater_equal(VALUE& l_, VALUE& r_, RUNTIME* pruntime)
{
   bool bOk = l_.synchronize(r_, pruntime);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() >= r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() >= r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() >= r_.get_string());
   }
   if(pruntime != nullptr) pruntime->add("[greater_equal] - Invalid greater_equal operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Checks if one VALUE object is less than or equal to another
 * 
 * This function compares integers, floating point numbers, and strings.
 * Values are synchronized to ensure type compatibility before operation.
 * 
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand
 * @param r_ Right operand
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Boolean result of comparison, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE less_equal(VALUE& l_, VALUE& r_, RUNTIME* pruntime)
{
   bool bOk = l_.synchronize(r_, pruntime);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() <= r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() <= r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() <= r_.get_string());
   }
   if(pruntime != nullptr) pruntime->add("[less_equal] - Invalid less_equal operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Checks if two VALUE objects are equal
 * 
 * This function compares integers, floating point numbers, and strings.
 * Values are synchronized to ensure type compatibility before operation.
 * 
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand
 * @param r_ Right operand
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Boolean result of comparison, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE equal(VALUE& l_, VALUE& r_, RUNTIME* pruntime)
{
   bool bOk = l_.synchronize(r_, pruntime);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() == r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() == r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() == r_.get_string());
   }
   else
   {
      if( l_.is_null() == true && r_.is_null() == true ) return VALUE(false); // null are always false on equality
   }
   if(pruntime != nullptr) pruntime->add("[equal] - Invalid equal operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Checks if two VALUE objects are not equal
 * 
 * This function compares integers, floating point numbers, and strings.
 * Values are synchronized to ensure type compatibility before operation.
 * 
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand
 * @param r_ Right operand
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Boolean result of comparison, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE not_equal(VALUE& l_, VALUE& r_, RUNTIME* pruntime)
{
   bool bOk = l_.synchronize(r_, pruntime);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() != r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() != r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() != r_.get_string());
   }
   if(pruntime != nullptr) pruntime->add("[equal] - Invalid equal operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Performs bitwise AND operation on two VALUE objects
 *
 * Values are synchronized to ensure type compatibility before operation.
 *
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand
 * @param r_ Right operand
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of bitwise AND, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE bitwise_and( VALUE& l_, VALUE& r_, RUNTIME* pruntime )
{
   bool bOk = l_.synchronize(r_, pruntime);
   if( bOk == true )
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() & r_.get_integer());
   }
   if( pruntime != nullptr ) pruntime->add("[and] - Invalid and operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Performs logical AND operation on two VALUE objects
 *
 * Values are synchronized to ensure type compatibility before operation.
 *
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand
 * @param r_ Right operand
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of logical AND, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE logical_and( VALUE& l_, VALUE& r_, RUNTIME* pruntime )
{
   bool bOk = l_.synchronize(r_, pruntime);
   if( bOk == true )
   {
      if( l_.is_bool() == true ) return VALUE(l_.get_bool() && r_.get_bool());
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() && r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() && r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string().empty() == false && r_.get_string().empty() == false );
   }
   if( pruntime != nullptr ) pruntime->add("[and] - Invalid and operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Performs bitwise OR operation on two VALUE objects
 *
 * Values are synchronized to ensure type compatibility before operation.
 *
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand
 * @param r_ Right operand
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of bitwise OR, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE bitwise_or( VALUE& l_, VALUE& r_, RUNTIME* pruntime )
{
   bool bOk = l_.synchronize(r_, pruntime);
   if( bOk == true )
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() | r_.get_integer());
   }
   if( pruntime != nullptr ) pruntime->add("[and] - Invalid and operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Performs logical OR operation on two VALUE objects
 *
 * Values are synchronized to ensure type compatibility before operation.
 *
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand
 * @param r_ Right operand
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of logical OR, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE logical_or( VALUE& l_, VALUE& r_, RUNTIME* pruntime )
{
   bool bOk = l_.synchronize(r_, pruntime);
   if( bOk == true )
   {
      if( l_.is_bool() == true ) return VALUE(l_.get_bool() || r_.get_bool());
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() || r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() || r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string().empty() == false || r_.get_string().empty() == false );
   }
   if( pruntime != nullptr ) pruntime->add("[and] - Invalid and operation", tag_error{});
   return VALUE();
}

/** ---------------------------------------------------------------------------
 * @brief Performs bitwise XOR operation on two VALUE objects
 *
 * Values are synchronized to ensure type compatibility before operation.
 *
 * @tparam VALUE The value type being operated on
 * @tparam RUNTIME Runtime environment type for error reporting
 * @param l_ Left operand
 * @param r_ Right operand
 * @param pruntime Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of bitwise XOR, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE bitwise_xor( VALUE& l_, VALUE& r_, RUNTIME* pruntime )
{
   bool bOk = l_.synchronize(r_, pruntime);
   if( bOk == true )
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() ^ r_.get_integer());
   }
   if( pruntime != nullptr ) pruntime->add("[and] - Invalid and operation", tag_error{});
   return VALUE();
}



_GD_EXPRESSION_END