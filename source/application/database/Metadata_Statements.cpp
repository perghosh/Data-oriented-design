/**
 * \file Metadata_Statements.cpp
 * 
 * \brief Store database statement information, this could be any statement that is executable in database.
 * 
 */




#include "gd/gd_arguments.h"
#include "gd/gd_parse.h"
#include "gd/gd_sql_value.h"
#include "gd/gd_types.h"
#include "gd/gd_utf8.hpp"

#include "Metadata_Statements.h"

APPLICATION_DATABASE_METADATA_BEGIN


CStatement* CStatements::Find( const std::string_view& stringName, tag_nolock )
{
   for( auto it = std::begin( m_vectorStatement ), itEnd = std::end( m_vectorStatement ); it != itEnd; it++ )
   {
      if( it->GetName().compare( stringName, gd::variant_type::tag_explicit{} ) == true ) return &(*it);
   }

   return nullptr;
}

CStatement* CStatements::Find( const std::string_view& stringName )
{
   std::lock_guard<std::shared_mutex> lockguard( m_sharedmutex );
   return Find( stringName, tag_nolock{} );
}


const CStatement* CStatements::Find( const std::string_view& stringName, tag_nolock ) const noexcept
{
   std::lock_guard<std::shared_mutex> lockguard( m_sharedmutex );

   for( auto it = std::begin( m_vectorStatement ), itEnd = std::end( m_vectorStatement ); it != itEnd; it++ )
   {
      if( it->GetName().compare( stringName, gd::variant_type::tag_explicit{} ) == true ) return &(*it);
   }

   return nullptr;
}

const CStatement* CStatements::Find( const std::string_view& stringName ) const noexcept
{
   std::lock_guard<std::shared_mutex> lockguard( m_sharedmutex );
   return Find( stringName, tag_nolock{} );
}

const CStatement* CStatements::Find( const std::string_view& stringType, const std::string_view& stringName ) const noexcept
{
   std::lock_guard<std::shared_mutex> lockguard(m_sharedmutex);

   for( auto it = std::begin(m_vectorStatement), itEnd = std::end(m_vectorStatement); it != itEnd; it++ )
   {
      if( it->GetName().compare(stringName, gd::variant_type::tag_explicit{}) == true && 
          it->GetType().compare(stringType, gd::variant_type::tag_explicit{}) == true ) return &( *it );
   }
   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Appends a new SQL statement to the collection.
 *
 * This method extracts the necessary information from the provided arguments
 * and creates a new `CStatement` object. The new statement is then appended
 * to the internal vector of statements.
 *
 * @param argumentsStatement The arguments containing the details of the statement.
 *                           Expected keys in the arguments:
 *                           - "type": The type of the statement (string).
 *                           - "name": The name of the statement (string).
 *                           - "sql": The SQL query string (string).
 *                           - "flags": The flags for the statement (unsigned).
 *                           - "repeat-parse": Optional flag to indicate repeat parse (boolean).
 *                           - "ignore-parse": Optional flag to indicate ignore parse (boolean).
 *                           - "cache": Optional flag to indicate caching (boolean).
 */
void CStatements::Append(const gd::argument::arguments& argumentsStatement) {
   // Extract the necessary information from the arguments
   std::string stringType = argumentsStatement["type"].as_string();
   std::string stringName = argumentsStatement["name"].as_string();
   std::string stringSql = argumentsStatement["sql"].as_string();
   unsigned uFlags = argumentsStatement["flags"].as_uint();
   if( uFlags == 0 )                                                           // if flags not set then check for individual flags
   {
      if( argumentsStatement["repeat-parse"].is_true() == true ) uFlags |= CStatement::eFlagRepeatParse;
      if( argumentsStatement["ignore-parse"].is_true() == true ) uFlags |= CStatement::eFlagIgnoreParse;
      if( argumentsStatement["cache"].is_true() == true ) uFlags |= CStatement::eFlagCache;
   }

   if( stringType.empty() == true ) { stringType = "select"; }

   // Create a new CStatement object using the extracted information
   CStatement statement(stringType, stringName, stringSql, uFlags);

   // Append the new statement to the vector
   {
      std::unique_lock lock(m_sharedmutex);
      m_vectorStatement.push_back(std::move(statement));
   }
}


void CStatements::Append( CStatement&& statement )
{
   std::unique_lock<std::shared_mutex> lockguard( m_sharedmutex );

   m_vectorStatement.push_back( std::move( statement ) );
}

void CStatements::Append( CStatement&& statement, tag_nolock )
{
   m_vectorStatement.push_back( std::move( statement ) );
}

void CStatements::Append( CStatement&& statement, std::function<void ( CStatement* )> callback_ )
{
   std::unique_lock<std::shared_mutex> lockguard( m_sharedmutex );

   m_vectorStatement.push_back( std::move( statement ) );
   auto* pstatement = &m_vectorStatement.back();                               // get ponter to added statement

   if( callback_ )
   {                                                                           // call callback for more modification
      callback_( pstatement );
   }
}

/** ---------------------------------------------------------------------------
 * @brief Remove statement from internal list of statements
 * Be careful removing statements, it could be that there are other threads that
 * are working with statements within list and they will be affected if this is
 * modified.
 * @param stringName name for statement to remove.
 */
void CStatements::Remove( const std::string_view& stringName )
{
   std::unique_lock<std::shared_mutex> lockguard( m_sharedmutex );

   for( auto it = std::begin( m_vectorStatement ), itEnd = std::end( m_vectorStatement ); it != itEnd; it++ )
   {
      if( it->GetName().as_string_view() == stringName )
      {
         m_vectorStatement.erase( it );
         return;
      }
   }
}

/// Clear all statements
void CStatements::Clear()
{
   std::unique_lock<std::shared_mutex> lockguard( m_sharedmutex );
   m_vectorStatement.clear();
}

/** ---------------------------------------------------------------------------
 * @brief set table or tables. if multiple tables then each table is separated with ','
 * @param stringTable table set
*/
void CStatement::TABLE_Set( const std::string_view& stringTable )
{
   m_arguments.set("table", stringTable);
}

/** ---------------------------------------------------------------------------
 * @brief add table name to statement
 * This is used to inform about existing tables in statement to know how to fix rules for it
 * @param stringTable table added to statement
*/
void CStatement::TABLE_Add( const std::string_view& stringTable )
{
   auto argument_ = m_arguments["table"];
   if( argument_.empty() == true )
   {
      m_arguments.append( "table", stringTable );
   }
   else
   {
      std::string string_ = argument_;
      string_ += ',';
      string_ += stringTable;
      m_arguments.set("table", string_.c_str());
   }
}

/** ---------------------------------------------------------------------------
 * @brief Get number of tables used in query
 * @return unsigned number of tables used in query
*/
unsigned CStatement::TABLE_GetCount() const
{
   unsigned uCount = 0;
   auto argument_ = m_arguments["table"];

   if( argument_.empty() == false )
   {
      uCount = 1;                                                                                  
      auto stringTable = argument_.get_variant_view().get_string_view();
      const char* pbsz = stringTable.data();                                                       assert( *pbsz != ',');
      while( *pbsz != '\0' )
      {
         if( *pbsz == ',' ) uCount++;
         pbsz++;
      }
   }

   return uCount;
}

std::string CStatement::TABLE_Get( unsigned uTableIndex ) const
{
   std::string stringTable;
   auto argument_ = m_arguments["table"];
   if( argument_.empty() == false )
   {
      auto stringTableData = argument_.get_variant_view().get_string_view();
      const char* pbszEnd = stringTableData.data();
      bool bFound = gd::parse::moveto_character_g( pbszEnd, ',', uTableIndex, gd::parse::tag_zero_end{} );
      if( bFound == true )
      {
         const char* pbszBegin = stringTableData.data();

         // if not first index then move back to start of table by finding table separators
         if( uTableIndex > 0 ) pbszBegin = gd::parse::next_character_g( pbszEnd - 1, ',', gd::parse::tag_reverse{} ) + 1;

         stringTable.assign( pbszBegin, pbszEnd - pbszBegin );
         return stringTable;
      }
   }

   return stringTable;
}


/** ---------------------------------------------------------------------------
 * @brief Get hardcoded text from statement
 * @return std::pair<bool, std::string> true and statement text if ok, false and error on fail
*/
std::pair<bool, std::string> CStatement::GetCompiledText() const
{
   std::string stringText;
   auto argument_ = m_arguments["sql"];
   if( argument_.empty() == false )
   {
      stringText = argument_.as_string();
   }

   return { true, stringText };
}

/** ---------------------------------------------------------------------------
 * @brief Format sql string, replace arguments in string with the wild card pattern '{*}'
 * 
 * *Sample on how to replace 'tags' in text by adding variables for each tag*
@code
	using namespace gd::parse; using namespace application::database;

   std::string_view stringTemplate = "test {=test} {=test} {=test} {=missing}";
	gd::argument::arguments argumentsValue = { {"test", 1 } };
   CStatement statement;
   statement.Append( { "name", stringTemplate});
	auto [bOk, stringSql ] = statement.GetCompiledText( argumentsValue );
   assert( stringSql == "test 1 1 1 NULL" );
@endcode

 * *Sample on how to replace 'tags' in text by adding variables for each tag. In this sample variant_view is used adding variable*
@code
	std::string stringSql;
	gd::argument::arguments argumentsValue;
	argumentsValue = { {"test", 1 } };
	argumentsValue.append_argument( { {"text", "1234" } }, gd::argument::arguments::tag_view{} );
	std::string_view stringTemplate = "test {=test} {} {=test} '{==text}' {=missing}";
   CStatement statement;
   statement.Append( { "name", stringTemplate});
	auto [bOk, stringSql ] = statement.GetCompiledText( argumentsValue );
   assert( stringSql == "test 1 {} 1 1234 NULL" );
@endcode

 * @param argumentsValue values replacing names within brackets
 * @return true and compiled text if ok, false and error information on error
*/
std::pair<bool, std::string> CStatement::GetCompiledText( const gd::argument::arguments& argumentsValue ) const
{
   using namespace gd::parse;                                                  // need some parse methods in here
   std::string stringText;
   std::string stringSql;
   auto argument_ = m_arguments["sql"];
   if( argument_.empty() == false )
   {
      stringText = argument_.as_string();
   }

   const char* pbszTemplateEnd = stringText.c_str() + stringText.length();
   strchr_for_each( stringText, '{', [pbszTemplateEnd,&argumentsValue, &stringSql](const auto& stringPart, auto iIndex) -> const char* {
		stringSql += stringPart;
		if( iIndex != -1 )
		{
         // ## find name in string, name is placed between {  } characters and the starting equal sign
			auto pbszNameBegin = stringPart.data() + stringPart.length();
			auto pbszNameEnd = skip_wildcard_g( pbszNameBegin, pbszTemplateEnd, "{*}" );			      assert( pbszNameBegin < pbszNameEnd );
			pbszNameBegin++;
			pbszNameEnd--;

			if( *pbszNameBegin == '=' )
			{
            bool bRaw = false;
            pbszNameBegin++;
            if( *pbszNameBegin == '=' )                                        // Two = (==) inserts value as it is without andy fixing to be compatible in sql queries
            {
               bRaw = true;
               pbszNameBegin++;
            }

            const std::string_view stringName( pbszNameBegin, pbszNameEnd - pbszNameBegin );// argument name
            auto variantviewValue = argumentsValue[stringName].get_variant_view(); // get named value in arguments

            if( bRaw == false ) gd::sql::append_g( variantviewValue, stringSql );
            else                gd::sql::append_g( variantviewValue, stringSql, gd::sql::tag_raw{} );
			}
         else
         {                                                                     // No "command" character found, insert '{' and skip
            stringSql += *(pbszNameBegin - 1);                                 // add start character '{'
            return pbszNameBegin;
         }
         return pbszNameEnd + 1;
		}

      return nullptr;
	}, tag_sql{} );

   return { true, stringSql };
}



APPLICATION_DATABASE_METADATA_END

