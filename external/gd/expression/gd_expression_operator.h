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


/*

Eventuellt OT men m�jligen intressant problem och hur samma typ av l�sning hade sett ut i olika spr�k.

H�ller p� att skriva ihop en skript hantering. Efter en del letande tror jag enklaste �r att g�ra en egen variant f�r det enkla som beh�vs. Men f�r att dra nytta av koden i varianter av skript (om man beh�ver flera �n en) f�rs�ker jag skriva metoder som kan anv�ndas i olika l�sningar.

D� �r ett "skal" nedan, metoden �r en metod som k�rs n�r tv� v�rden plussas ihop "1 + 1". Det kommer finnas en hel h�g med s�dana h�r enklare metoder. F�r skriptet jag g�r nu s� kan varje v�rde (VALUE) sk�ta sig sj�lv. Men det kommer troligen en annan variant d�r v�rdet inte sk�ter sig sj�lv utan blir mer av en view d�r data finns i "runtime". Skrivs ett skriptspr�k d�r det skall g� och g�ra st�rre skript och de beh�ver vara snabba, d� finns mer m�jligheter om v�rden ligger kopplade till n�got slags container objekt, �ven f�r tr�dar och annat. 

Nedan �r d� ett exempel p� metod

[code]
template<typename VALUE,typename RUNTIME>
VALUEadd( const VALUE& l_, const VALUE& r_, RUNTIME* pr_ )
{
   l_.synchronize(r_, pr_);

   if( l_.is_integer() ) return value(l_.get_integer() + r_.get_integer());
   if( l_.is_double() ) return value(l_.get_double() + r_.get_double());
   if( l_.is_string() ) return value(l_.get_string() + r_.get_string());

   if( pr_ != nullptr ) pr_.add("[add] - Invalid addition operation", tag_error{});

   return VALUE();
}
[/code]

*/

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
 * @param pr_ Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of addition, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE add(VALUE& l_, VALUE& r_, RUNTIME* pr_)
{
   bool bOk = l_.synchronize(r_, pr_);
   if(bOk == true) 
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() + r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() + r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() + r_.get_string());
   }

   if(pr_ != nullptr) pr_->add("[add] - Invalid addition operation", tag_error{});

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
 * @param pr_ Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of subtraction, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE subtract(VALUE& l_, VALUE& r_, RUNTIME* pr_)
{
   bool bOk = l_.synchronize(r_, pr_);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() - r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() - r_.get_double());
   }
   if(pr_ != nullptr) pr_->add("[subtract] - Invalid subtract operation", tag_error{});
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
 * @param pr_ Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of multiplication, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE multiply(VALUE& l_, VALUE& r_, RUNTIME* pr_)
{
   bool bOk = l_.synchronize(r_, pr_);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() * r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() * r_.get_double());
   }
   if(pr_ != nullptr) pr_->add("[multiply] - Invalid multiply operation", tag_error{});
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
 * @param pr_ Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of division, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE divide(VALUE& l_, VALUE& r_, RUNTIME* pr_)
{
   bool bOk = l_.synchronize(r_, pr_);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() / r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() / r_.get_double());
   }
   if(pr_ != nullptr) pr_->add("[divide] - Invalid divide operation", tag_error{});
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
 * @param pr_ Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Result of modulo operation, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE modulo(VALUE& l_, VALUE& r_, RUNTIME* pr_)
{
   bool bOk = l_.synchronize(r_, pr_);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() % r_.get_integer());
   }
   if(pr_ != nullptr) pr_->add("[modulo] - Invalid modulo operation", tag_error{});
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
 * @param pr_ Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Boolean result of comparison, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE greater(VALUE& l_, VALUE& r_, RUNTIME* pr_)
{
   bool bOk = l_.synchronize(r_, pr_);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() > r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() > r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() > r_.get_string());
   }
   if(pr_ != nullptr) pr_->add("[greater] - Invalid greater operation", tag_error{});
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
 * @param pr_ Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Boolean result of comparison, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE less(VALUE& l_, VALUE& r_, RUNTIME* pr_)
{
   bool bOk = l_.synchronize(r_, pr_);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() < r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() < r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() < r_.get_string());
   }
   if(pr_ != nullptr) pr_->add("[less] - Invalid less operation", tag_error{});
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
 * @param pr_ Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Boolean result of comparison, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE greater_equal(VALUE& l_, VALUE& r_, RUNTIME* pr_)
{
   bool bOk = l_.synchronize(r_, pr_);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() >= r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() >= r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() >= r_.get_string());
   }
   if(pr_ != nullptr) pr_->add("[greater_equal] - Invalid greater_equal operation", tag_error{});
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
 * @param pr_ Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Boolean result of comparison, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE less_equal(VALUE& l_, VALUE& r_, RUNTIME* pr_)
{
   bool bOk = l_.synchronize(r_, pr_);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() <= r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() <= r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() <= r_.get_string());
   }
   if(pr_ != nullptr) pr_->add("[less_equal] - Invalid less_equal operation", tag_error{});
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
 * @param pr_ Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Boolean result of comparison, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE equal(VALUE& l_, VALUE& r_, RUNTIME* pr_)
{
   bool bOk = l_.synchronize(r_, pr_);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() == r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() == r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() == r_.get_string());
   }
   if(pr_ != nullptr) pr_->add("[equal] - Invalid equal operation", tag_error{});
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
 * @param pr_ Pointer to runtime environment for error reporting (can be nullptr)
 * @return VALUE Boolean result of comparison, or empty VALUE on error
 */
template<typename VALUE, typename RUNTIME>
VALUE not_equal(VALUE& l_, VALUE& r_, RUNTIME* pr_)
{
   bool bOk = l_.synchronize(r_, pr_);
   if(bOk == true)
   {
      if( l_.is_integer() == true ) return VALUE(l_.get_integer() != r_.get_integer());
      if( l_.is_double() == true ) return VALUE(l_.get_double() != r_.get_double());
      if( l_.is_string() == true ) return VALUE(l_.get_string() != r_.get_string());
   }
   if(pr_ != nullptr) pr_->add("[equal] - Invalid equal operation", tag_error{});
   return VALUE();
}


_GD_EXPRESSION_END