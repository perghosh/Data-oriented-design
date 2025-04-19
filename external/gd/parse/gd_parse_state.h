/**
 * @file gd_parse_state.h
 */

#pragma once


#include <cassert>


#include "gd/gd_types.h"
#include "gd/gd_strings.h"


#ifndef _GD_PARSE_BEGIN
#  define _GD_PARSE_STATE_BEGIN namespace gd { namespace parse { namespace state {
#  define _GD_PARSE_STATE_END } } }
#endif

_GD_PARSE_STATE_BEGIN

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class code
{
public:
   /**
    * \brief
    *
    *
    */
   struct state
   {
      // ## construction ------------------------------------------------------------
      state() {}
      // copy
      state(const state& o) { common_construct(o); }
      state(state&& o) noexcept { common_construct(std::move(o)); }
      // assign
      state& operator=(const state& o) { common_construct(o); return *this; }
      state& operator=(state&& o) noexcept { common_construct(std::move(o)); return *this; }

      ~state() {}
      // common copy
      void common_construct(const state& o) {}
      void common_construct(state&& o) noexcept {}

   // ## methods -----------------------------------------------------------------

   /** \name DEBUG
   *///@{

   //@}

   // ## attributes --------------------------------------------------------------
      std::string_view m_stringName;

   // ## free functions ----------------------------------------------------------

   };


// ## construction -------------------------------------------------------------
public:
   code() {}
   // copy
   code(const code& o) { common_construct(o); }
   code(code&& o) noexcept { common_construct(std::move(o)); }
   // assign
   code& operator=(const code& o) { common_construct(o); return *this; }
   code& operator=(code&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~code() {}
private:
// common copy
void common_construct(const code& o) {}
void common_construct(code&& o) noexcept {}

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
   std::string_view m_stringState; ///< name of active state
   uint8_t m_puMatchBuffer[0x100] = {}; ///< buffer for code
   gd::strings32 m_strings32Pool; ///< pool of strings


// ## free functions ------------------------------------------------------------
public:



};

_GD_PARSE_STATE_END
