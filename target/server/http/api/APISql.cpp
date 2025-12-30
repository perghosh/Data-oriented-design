// @FILE [tag: api, sql] [summary: API SQL command class] [type: source] [name: APISql.cpp]

#include "gd/gd_arguments.h"
#include "gd/gd_binary.h"

#include "../Router.h"
#include "../Document.h"
#include "../Application.h"

#include "../meta/METAQueries.h"

#include "APISql.h"

using namespace META;

/** --------------------------------------------------------------------------
 * @brief Executes the SQL command based on the command vector and parameters.
 *
 * This method processes the SQL command stored in m_vectorCommand and uses
 * the parameters in m_argumentsParameter to perform the requested operation.
 *
 * @return std::pair<bool, std::string> A pair containing:
 *         - bool: Success status (true if operation succeeded, false otherwise)
 *         - std::string: Error message if operation failed, empty string if succeeded
 *
 * @note The command vector must not be empty. The method asserts this condition.
 */
std::pair<bool, std::string> CAPISql::Execute()
{                                                                                                  assert( m_vectorCommand.empty() == false && "No commands");
   // ## execute SQL command based on m_vectorCommand and m_argumentsParameter

   if( m_vectorCommand.empty() == true ) return { true, "No commands"};

   std::pair<bool, std::string> result_(true,"");

   CRouter::Encode_s( m_argumentsParameter, { "query" } );

   for( std::size_t uIndex = 0; uIndex < m_vectorCommand.size(); ++uIndex )
   {
      m_uCommandIndex = static_cast<unsigned>( uIndex );
      std::string_view stringCommand = m_vectorCommand[uIndex];

      if( stringCommand == "sql" ) continue;

      if( stringCommand == "add" )
      {
         result_ = Execute_Add();
      }
      /*
      else if( stringCommand == "insert" )
      {
         result_ = Execute_Insert();
      }
      else if( stringCommand == "update" )
      {
         result_ = Execute_Update();
      }
      else if( stringCommand == "delete" )
      {
         result_ = Execute_Delete();
      }
      else if( stringCommand == "procedure" || stringCommand == "proc" )
      {
         result_ = Execute_StoredProcedure();
      }
      */
      else
      {
         return { false, "unknown SQL command: " + std::string(stringCommand) };
      }

      if( result_.first == false ) { return result_; }
   }

   return { true, "" };
}

std::pair<bool, std::string> CAPISql::Execute_Add()
{
   // TODO: Implement SQL query execution logic
   
   // Get query from parameters
   std::string stringQuery = m_argumentsParameter["query"].as_string();

   CDocument* pdocument = GetDocument();

   CQueries* pqueries_ = pdocument->QUERIES_Get();                                                 assert( pqueries_ && "Document do not have queries");
   
   auto result_ = pqueries_->Add( stringQuery, CQueries::eFormatText, nullptr );
   if( result_.first == false ) return result_;

   std::string_view stringId = result_.second;
   gd::argument::arguments* parguments_ = new gd::argument::arguments( { { "Id", stringId } }, gd::types::tag_view{});
   m_objects.Add( parguments_ );
   
   return { true, "" };
}

/*
std::pair<bool, std::string> CAPISql::Execute_Insert()
{
   // TODO: Implement SQL insert execution logic
   
   // Get table and values from parameters
   std::string stringTable = m_argumentsParameter["table"].as_string();
   std::string stringValues = m_argumentsParameter["values"].as_string();
   
   // Validate parameters
   if( stringTable.empty() )
   {
      return { false, "Table name not provided" };
   }
   if( stringValues.empty() )
   {
      return { false, "Values not provided" };
   }
   
   // TODO: Execute insert and get result
   // For now, create a placeholder result
   gd::argument::arguments* parguments_ = new gd::argument::arguments( { 
      { "insert_id", 0 },
      { "rows_affected", 1 } 
   } );
   m_objects.Add( parguments_ );
   
   return { true, "" };
}

std::pair<bool, std::string> CAPISql::Execute_Update()
{
   // TODO: Implement SQL update execution logic
   
   // Get table, set clause, and where clause from parameters
   std::string stringTable = m_argumentsParameter["table"].as_string();
   std::string stringSet = m_argumentsParameter["set"].as_string();
   std::string stringWhere = m_argumentsParameter["where"].as_string();
   
   // Validate parameters
   if( stringTable.empty() )
   {
      return { false, "Table name not provided" };
   }
   if( stringSet.empty() )
   {
      return { false, "Set clause not provided" };
   }
   
   // TODO: Execute update and get result
   // For now, create a placeholder result
   gd::argument::arguments* parguments_ = new gd::argument::arguments( { 
      { "rows_affected", 1 } 
   } );
   m_objects.Add( parguments_ );
   
   return { true, "" };
}

std::pair<bool, std::string> CAPISql::Execute_Delete()
{
   // TODO: Implement SQL delete execution logic
   
   // Get table and where clause from parameters
   std::string stringTable = m_argumentsParameter["table"].as_string();
   std::string stringWhere = m_argumentsParameter["where"].as_string();
   
   // Validate parameters
   if( stringTable.empty() )
   {
      return { false, "Table name not provided" };
   }
   
   // TODO: Execute delete and get result
   // For now, create a placeholder result
   gd::argument::arguments* parguments_ = new gd::argument::arguments( { 
      { "rows_affected", 1 } 
   } );
   m_objects.Add( parguments_ );
   
   return { true, "" };
}

std::pair<bool, std::string> CAPISql::Execute_StoredProcedure()
{
   // TODO: Implement SQL stored procedure execution logic
   
   // Get procedure name and parameters
   std::string stringProcedure = m_argumentsParameter["procedure"].as_string();
   
   // Validate parameters
   if( stringProcedure.empty() )
   {
      return { false, "Procedure name not provided" };
   }
   
   // TODO: Execute stored procedure and get result
   // For now, create a placeholder result
   gd::argument::arguments* parguments_ = new gd::argument::arguments( { 
      { "result", "placeholder" },
      { "rows", 0 } 
   } );
   m_objects.Add( parguments_ );
   
   return { true, "" };
}
*/
