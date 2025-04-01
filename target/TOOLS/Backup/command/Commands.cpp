#include "Commands.h"

void CCommand::common_construct(const CCommand& o)
{
   m_stringName = o.m_stringName;
   m_arguments = o.m_arguments;
}

void CCommand::common_construct(CCommand&& o) noexcept
{
   m_stringName = std::move(o.m_stringName);
   m_arguments = std::move(o.m_arguments);
}


// ----------------------------------------------------------------------------
// ------------------------------------------------------------------ CCommands
// ----------------------------------------------------------------------------

void CCommands::common_construct(const CCommands& o)
{
   m_stringName = o.m_stringName; // Copy the name
   m_vectorCommand = o.m_vectorCommand; // Copy the vector of commands
}

void CCommands::common_construct(CCommands&& o) noexcept
{
   m_stringName = std::move(o.m_stringName); // Move the name
   m_vectorCommand = std::move(o.m_vectorCommand); // Move the vector of commands
}