/**
 * \file gd_math.h
 * 
 * \brief Pack primitive value and some common derived values in byte buffer. 
 * 
 */

#pragma once

#include <array>
#include <cassert>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <type_traits>

#include "gd_compiler.h"

#ifndef _GD_MATH_BEGIN
   #define _GD_MATH_BEGIN namespace gd { namespace math {
   #define _GD_MATH_END } }

   #define _GD_GROUP_ALGEBRA_BEGIN namespace algebra {
   #define _GD_GROUP_ALGEBRA_END }

   #define _GD_GROUP_AREA_BEGIN namespace area {
   #define _GD_GROUP_AREA_END }
#endif

_GD_MATH_BEGIN

template <typename TYPE, typename... ARGUMENTS>
void increase(TYPE increase_with_, ARGUMENTS&... values_)
{
   ((values_ += increase_with_), ...);
}

template <typename TYPE1, typename TYPE2>
std::pair<TYPE2, TYPE2> increase_pair(TYPE1 increase_with_, const std::pair<TYPE2, TYPE2>& pair_)
{
   return std::pair<TYPE2, TYPE2>( pair_.first + increase_with_, pair_.second + increase_with_ );
}


_GD_GROUP_ALGEBRA_BEGIN

// -------------------------------------------------------------------- ALGEBRA

#ifdef GD_COMPILER_HAS_CPP20_SUPPORT

template <typename TYPE>
struct point
{
   constexpr point() noexcept : m_x(TYPE{}), m_y(TYPE{}) {}
   constexpr point(TYPE x_, TYPE y_) noexcept : m_x(x_), m_y(y_) {}
   constexpr point(const std::pair<TYPE, TYPE>& pair_) noexcept : m_x(pair_.first), m_y(pair_.second) {}

   operator std::pair<TYPE, TYPE>() const { return {m_x, m_y}; }

   constexpr bool operator==(const point<TYPE>& other_) const { return m_x == other_.m_x && m_y == other_.m_y; }
   constexpr bool operator!=(const point<TYPE>& other_) const { return !(*this == other_); }
   constexpr bool operator<(const point<TYPE>& other_) const { return std::tie(m_x, m_y) < std::tie(other_.m_x, other_.m_y); }

   // operator addition
   point<TYPE> operator+(const TYPE& value_) const { return point<TYPE>(m_x + value_, m_y + value_); }
   point<TYPE> operator+(const point<TYPE>& other_) const { return point<TYPE>(m_x + other_.m_x, m_y + other_.m_y); }

   // operator subtraction
   point<TYPE> operator-(const TYPE& value_) const { return point<TYPE>(m_x - value_, m_y - value_); }
   point<TYPE> operator-(const point<TYPE>& other_) const { return point<TYPE>(m_x - other_.m_x, m_y - other_.m_y); }

   // add equal operator
   point<TYPE>& operator+=(const TYPE& value_) { m_x += value_; m_y += value_; return *this; }
   point<TYPE>& operator+=(const point<TYPE>& other_) { m_x += other_.m_x; m_y += other_.m_y; return *this; }

   // subtract equal operator
   point<TYPE>& operator-=(const TYPE& value_) { m_x -= value_; m_y -= value_; return *this; }
   point<TYPE>& operator-=(const point<TYPE>& other_) { m_x -= other_.m_x; m_y -= other_.m_y; return *this; }
   

   constexpr TYPE x() const { return m_x; }
   constexpr TYPE y() const { return m_y; }

   constexpr TYPE distance_squared(const point<TYPE>& other_) const noexcept {
      TYPE dx = m_x - other_.m_x;
      TYPE dy = m_y - other_.m_y;
      return dx * dx + dy * dy;
   }

   /// Calculate distance to other point
   constexpr double distance(const point<TYPE>& other_) const noexcept {
      return std::sqrt(static_cast<double>(distance_squared(other_)));
   }

   TYPE m_x = TYPE{};
   TYPE m_y = TYPE{};
};

// Deduction guide
template<typename TYPE> point(std::pair<TYPE, TYPE>) -> point<TYPE>;

template<typename T> concept Numeric = std::is_arithmetic_v<T>;

template<Numeric TYPE>
struct line {
   // Constructors
   constexpr line() noexcept : m_pointStart(point<TYPE>{}), m_pointEnd(point<TYPE>{}) {}
   constexpr line(const point<TYPE>& start, const point<TYPE>& end) noexcept : m_pointStart(start), m_pointEnd(end) {}
   constexpr line(const std::pair<TYPE, TYPE>& start, const std::pair<TYPE, TYPE>& end) noexcept
      : m_pointStart(start), m_pointEnd(end) {}

   // Conversion to pair of points
   constexpr operator std::pair<point<TYPE>, point<TYPE>>() const noexcept { return {m_pointStart, m_pointEnd}; }

   // Comparison operators
   constexpr bool operator==(const line& o) const noexcept { return m_pointStart == o.m_pointStart && m_pointEnd == o.m_pointEnd; }
   constexpr bool operator!=(const line& o) const noexcept { return !(*this == o); }
   constexpr bool operator<(const line& o) const noexcept {
      return std::tie(m_pointStart, m_pointEnd) < std::tie(o.m_pointStart, o.m_pointEnd);
   }

   // Arithmetic operators (translation)
   constexpr line operator+(const TYPE& value_) const noexcept {
      return line(m_pointStart + value_, m_pointEnd + value_);
   }
   constexpr line operator+(const point<TYPE>& offset_) const noexcept {
      return line(m_pointStart + offset_, m_pointEnd + offset_);
   }
   constexpr line operator-(const TYPE& value_) const noexcept {
      return line(m_pointStart - value_, m_pointEnd - value_);
   }
   constexpr line operator-(const point<TYPE>& offset_) const noexcept {
      return line(m_pointStart - offset_, m_pointEnd - offset_);
   }

   // Compound assignment
   constexpr line& operator+=(const TYPE& value_) noexcept {
      m_pointStart += value_;
      m_pointEnd += value_;
      return *this;
   }
   constexpr line& operator+=(const point<TYPE>& offset_) noexcept {
      m_pointStart += offset_;
      m_pointEnd += offset_;
      return *this;
   }
   constexpr line& operator-=(const TYPE& value_) noexcept {
      m_pointStart -= value_;
      m_pointEnd -= value_;
      return *this;
   }
   constexpr line& operator-=(const point<TYPE>& offset_) noexcept {
      m_pointStart -= offset_;
      m_pointEnd -= offset_;
      return *this;
   }

   // Accessors
   constexpr point<TYPE> start() const noexcept { return m_pointStart; }
   constexpr point<TYPE> end() const noexcept { return m_pointEnd; }

   // Utility functions
   constexpr TYPE length_squared() const noexcept { return m_pointStart.distance_squared(m_pointEnd); }
   constexpr double length() const noexcept { return m_pointStart.distance(m_pointEnd); }
   constexpr point<TYPE> midpoint() const noexcept { return point<TYPE>(( m_pointStart.x() + m_pointEnd.x() ) / TYPE{ 2 }, ( m_pointStart.y() + m_pointEnd.y() ) / TYPE{ 2 }); }

private:
   point<TYPE> m_pointStart;
   point<TYPE> m_pointEnd;
};

// Deduction guide
template<typename T>
line(std::pair<T, T>, std::pair<T, T>) -> line<T>;

#endif // GD_COMPILER_HAS_CPP20_SUPPORT

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

_GD_GROUP_AREA_BEGIN

// --------------------------------------------------------------------___ AREA


/// ---------------------------------------------------------------------------
/// check if point is within box
template <typename TYPE>
bool is_inside_box( TYPE px_, TYPE py_, TYPE x_, TYPE y_, TYPE width_, TYPE height_ )
{
   bool bInside = (px_ >= x_ && px_ <= (x_ + width_)) && (py_ >= y_ && py_ <= (y_ + height_) );
   return bInside;
}
// overload `is_inside_box`
template <typename TYPE>
bool is_inside_box(const std::pair<TYPE, TYPE>& pairPoint, TYPE x_, TYPE y_, TYPE width_, TYPE height_ )
{
   bool bInside = is_inside_box( pairPoint.first, pairPoint.second, x_, y_, width_, height_ );
   return bInside;
}
// overload `is_inside_box`
template <typename TYPE>
bool is_inside_box(const std::pair<TYPE, TYPE>& pairPoint, const std::array<TYPE, 4>& array_ )
{
   bool bInside = is_inside_box( pairPoint.first, pairPoint.second, array_[0], array_[1], array_[2], array_[3]);
   return bInside;
}
// overload `is_inside_box`
template <typename TYPE>
bool is_inside_box(TYPE px_, TYPE py_, TYPE width_, TYPE height_ )
{
   bool bInside = is_inside_box( px_, py_, TYPE{}, TYPE{}, width_, height_);
   return bInside;
}

// overload `is_inside_box`
template <typename TYPE>
bool is_inside_box(const std::pair<TYPE, TYPE>& pairPoint, const std::pair<TYPE, TYPE>& pairXY, const std::pair<TYPE, TYPE>& pairSize )
{
   bool bInside = is_inside_box( pairPoint.first, pairPoint.second, pairXY.first, pairXY.second, pairSize.first, pairSize.second);
   return bInside;
}

// overload `is_inside_box`
template <typename TYPE>
bool is_inside_box(const std::pair<TYPE, TYPE>& pairPoint, const std::pair<TYPE, TYPE>& pairSize )
{
   bool bInside = is_inside_box( pairPoint.first, pairPoint.second, TYPE{}, TYPE{}, pairSize.first, pairSize.second);
   return bInside;
}



_GD_GROUP_AREA_END


_GD_MATH_END