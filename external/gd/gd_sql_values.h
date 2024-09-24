#pragma once

#include <cassert>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "gd_arguments.h"
#include "gd_variant.h"
#include "gd_variant_view.h"


#ifndef _GD_SQL_VALUES_BEGIN
#define _GD_SQL_VALUES_BEGIN namespace gd::sql {
#define _GD_SQL_VALUES_END }
_GD_SQL_VALUES_BEGIN
#else
_GD_SQL_VALUES_BEGIN
#endif

/**
 * \brief 
 *
 *
 *
 \code
 \endcode
 */
class values 
{
// ## construction -------------------------------------------------------------
public:
   values() {}
   // copy
   values( const values& o ) { common_construct( o ); }
   values( values&& o ) noexcept { common_construct( o ); }
   // assign
   values& operator=( const values& o ) { common_construct( o ); return *this; }
   values& operator=( values&& o ) noexcept { common_construct( o ); return *this; }
   
	~values() {}
private:
   // common copy
   void common_construct( const values& o ) {}
   void common_construct( values&& o ) noexcept {}

// ## operator -----------------------------------------------------------------
public:
   

// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   
//@}

/** \name OPERATION
*///@{
   
//@}

protected:
/** \name INTERNAL
*///@{
   
//@}

public:
/** \name DEBUG
*///@{
   
//@}


// ## attributes ----------------------------------------------------------------
public:
   
   
// ## free functions ------------------------------------------------------------
public:
   


};







_GD_SQL_VALUES_END