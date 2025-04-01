#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include  "gd/gd_arguments_shared.h"


/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CCommand
{
   // ## construction -------------------------------------------------------------
public:
   CCommand() {}
   CCommand(const std::string_view& stringName) : m_stringName(stringName) {}
   CCommand(const std::string_view& stringName, const gd::argument::shared::arguments& arguments) : m_stringName(stringName), m_arguments(arguments) {}
   // copy
   CCommand(const CCommand& o) { common_construct(o); }
   CCommand(CCommand&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CCommand& operator=(const CCommand& o) { common_construct(o); return *this; }
   CCommand& operator=(CCommand&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CCommand() {}
private:
   // common copy
   void common_construct(const CCommand& o);
   void common_construct(CCommand&& o) noexcept;

// ## operator -----------------------------------------------------------------
public:
   bool operator==(const CCommand& o) const { return m_stringName == o.m_stringName; }
   bool operator!=(const CCommand& o) const { return m_stringName != o.m_stringName; }
   bool operator<(const CCommand& o) const { return m_stringName < o.m_stringName; }
   bool operator==(const std::string_view& stringName) const { return m_stringName == stringName; }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   std::string_view GetName() const { return m_stringName; }
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
   std::string m_stringName;
   gd::argument::shared::arguments m_arguments;


// ## free functions ------------------------------------------------------------
public:
};


/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CCommands
{
   // ## construction -------------------------------------------------------------
public:
   CCommands() {}
   // copy
   CCommands(const CCommands& o) { common_construct(o); }
   CCommands(CCommands&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CCommands& operator=(const CCommands& o) { common_construct(o); return *this; }
   CCommands& operator=(CCommands&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CCommands() {}
private:
   // common copy
   void common_construct(const CCommands& o);
   void common_construct(CCommands&& o) noexcept;

   // ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{
   void Add( const CCommand& command_ ) { m_vectorCommand.push_back(command_); }
   void Add( CCommand&& command_ ) { m_vectorCommand.push_back(std::move(command_)); }

   size_t Size() const { return m_vectorCommand.size(); }
   bool Empty() const { return m_vectorCommand.empty(); }
   void Clear() { m_vectorCommand.clear(); m_stringName.clear(); }


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
   std::string m_stringName;
   std::vector<CCommand> m_vectorCommand;


// ## free functions ------------------------------------------------------------
public:



};