/**
 * \file gd_math.h
 * 
 * \brief Pack primitive value and some common derived values in byte buffer. 
 * 
 */



#pragma once
#include <cassert>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <type_traits>

#ifndef _GD_MATH_BEGIN
   #define _GD_MATH_BEGIN namespace gd { namespace math {
   #define _GD_MATH_END } }

   #define _GD_GROUP_ALGEBRA_BEGIN namespace algebra {
   #define _GD_GROUP_ALGEBRA_END }
#endif

_GD_MATH_BEGIN

template <typename TYPE, typename... ARGUMENTS>
void increase(TYPE increase_with_, ARGUMENTS&... values_)
{
   ((values_ += increase_with_), ...);
}

_GD_GROUP_ALGEBRA_BEGIN

// -------------------------------------------------------------------- ALGEBRA

/** ---------------------------------------------------------------------------
 * @brief Split primitive value into pair and the number of bits decide how to split them
 * @param uFrom primitive value that is split
 * @param uBitCount bits taken from lower bits in `uFrom`
 * @return std::pair<TYPE, TYPE> pair with two values that has been split from primitive
 */
template <typename TYPE>
std::pair<TYPE, TYPE> split_to_pair(TYPE uFrom, unsigned uBitCount)
{                                                                                                  assert( uBitCount < (sizeof(TYPE) * 8) );
   // ## Create masks
   TYPE uLowMask = (1ULL << uBitCount) - 1;
   TYPE uHighMask = ~uLowMask;

   TYPE uLow = uFrom & uLowMask;                                               // Low value part
   TYPE uHigh = (uFrom & uHighMask) >> uBitCount;                              // High value part

   return { uHigh, uLow };
}

/// splits primitive value in halft into pair
template <typename TYPE>
std::pair<TYPE, TYPE> split_to_pair(TYPE uFrom)
{
   return split_to_pair( uFrom, (sizeof( TYPE ) * 8) / 2 );
}


/** ---------------------------------------------------------------------------
 * @brief Join into primitive value from pair with two values
 * @param pair_ pari with two primitive values that are joined
 * @param uBitCount number of bits that decide how to combine pair value into single primitive value
 * @return TYPE joined primitive value
 */
template <typename TYPE>
TYPE join_from_pair(const std::pair<TYPE, TYPE>& pair_, unsigned uBitCount)
{                                                                                                  assert( uBitCount < (sizeof(TYPE) * 8) );
   // ## Create masks
   unsigned uBitCountSecond = (sizeof(TYPE) * 8)  - uBitCount;
   TYPE uHighMask = (1ULL << uBitCount) - 1;
   TYPE uLowMask = (1ULL << uBitCountSecond) - 1;

   TYPE uHigh = (pair_.first & uHighMask) << uBitCountSecond;
   TYPE uLow = pair_.second & uLowMask;

   TYPE uValue = uHigh | uLow;

   return uValue;
}

template <typename TYPE>
TYPE join_from_pair(const std::pair<TYPE, TYPE>& pair_)
{
   return join_from_pair( pair_, (sizeof( TYPE ) * 8) / 2 );
}





_GD_GROUP_ALGEBRA_END


_GD_MATH_END