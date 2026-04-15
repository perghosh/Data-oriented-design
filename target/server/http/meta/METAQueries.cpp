// @FILE [tag: query] [description: http session] [type: source] [name: METAQueries.cpp]

#include <format>

#include "gd/gd_binary.h"
#include "gd/gd_uuid.h"
#include "gd/gd_file.h"

#include "pugixml/pugixml.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

#include "METAQueries.h"


NAMESPACE_META_BEGIN

void CQueries::common_construct(const CQueries &o)
{
   m_argumentProperty = o.m_argumentProperty;
   //m_tableQuery = o.m_tableQuery;

   // @TODO: Complete this, with copying statement object, this is not implemented yet
}



std::pair<bool, std::string> CQueries::Initialize( const gd::argument::arguments& arguments_ )
{
   std::lock_guard<std::mutex> lock_(m_mutexStatement);

   if( m_statement.empty() == true ) 
   { 
      m_statement.initialize();
   }

   //CQueries::CreateTable_s( m_tableQuery );
   return { true, "" };
}

std::pair<bool, std::string> CQueries::Add( std::string_view stringQuery, enumFormat eFormat, const gd::argument::arguments* parguments_ )
{

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Adds a new query to the table with the specified ID, type, format, and query.
 *
 * @param stringName Name for query.
 * @param stringType The type of the query.
 * @param stringFormat The format of the query.
 * @param stringQuery The query itself.
 * @return std::pair<bool, std::string> A pair containing a boolean indicating success and the ID of the added query.
 */
std::pair<bool, std::string> CQueries::Add( std::string_view stringName, std::string_view stringType, std::string_view stringFormat, std::string_view stringQuery )
{
   if( stringType.empty() == true ) stringType = "select";
   if( stringQuery.empty() ) { return { false, "Invalid input" }; }

   std::lock_guard<std::mutex> lock_(m_mutexStatement);

   auto result_ = m_statement.add( stringName, stringQuery, stringFormat, stringType ); // TODO: Compmlete this, new logic
   if( result_.first == false ) { return { false, result_.second }; }

   return result_;
}

std::pair<bool, std::string> CQueries::Delete( const std::pair<std::string_view, std::string_view>& pair_ )
{
   std::lock_guard<std::mutex> lock_(m_mutexStatement);

   std::string_view stringName = pair_.first;
   std::string_view stringUuid = pair_.second;
   int64_t iRow = -1;
   if( stringName.empty() == false) { iRow = m_statement.find( stringName ); }
   if(iRow == -1)
   {
      gd::uuid uuid_( stringUuid );
      gd::types::uuid uuidFind( uuid_.data() );
      iRow = m_statement.find( uuidFind );
   }

   if( iRow == -1 ) return { false, std::format( "No row for {} or {}", stringName, stringUuid)};

   m_statement.erase( iRow );

   return { true, "" };
}

int64_t CQueries::Find( const gd::argument::arguments& arguments_ ) const
{
   if( arguments_.exists( "name" ) == true )
   {
      std::string_view stringName = arguments_["name"].get<std::string_view>();
      if( stringName.empty() == false )
      {
         int64_t iRow = m_statement.find( stringName );
         if( iRow != -1 ) return iRow;
      }
   }
   /*
   gd::uuid uuidFind;
   std::string_view stringUuid = arguments_.get<std::string_view>( "id" );
   if( stringUuid.empty() == false )
   {
      uuidFind = gd::uuid( stringUuid );
      gd::types::uuid uuidFindType( uuidFind.data() );
      int64_t iRow = m_tableQuery.find( eColumnId, uuidFindType );
      if( iRow != -1 ) return iRow;
   }
   */
   return -1;
}

/// @brief Retrieves the UUID of a query based on its row index.
gd::types::uuid CQueries::GetQueryId( uint64_t uRow )
{
   gd::types::uuid uuidId = m_statement.get_id( uRow );

   return uuidId;
}

std::pair<bool, std::string> CQueries::GetQuery( std::string_view stringName, std::string& stringQuery )
{
   auto iIndex = m_statement.find( stringName );
   if( iIndex != -1 )
   {
      stringQuery = m_statement.get_statement( iIndex );
      return { true, "" };
   }

   return { false, std::format( "No query found for name '{}'", stringName ) };
}

/** --------------------------------------------------------------------------
 * @brief Retrieves all values of a specific argument name from the arguments attached to a query at the given row index.
 * @param uRow The row index of the query.
 * @param stringName The name of the argument.
 * @return A vector of variant views representing the values of the specified argument.
 */
std::vector< gd::variant_view > CQueries::GetArgumentsValues( uint64_t uRow, std::string_view stringKey ) const
{
   const gd::argument::shared::arguments* parguments = GetQueryArguments( uRow );
   if( parguments == nullptr ) return {};

   return parguments->get_argument_all( stringKey, gd::types::tag_view{} );
}

std::pair<bool, std::string> CQueries::Load( std::string_view stringPath )
{
   gd::file::path path_( stringPath );

   // ## Determine file format based on extension
   std::string stringExtension = path_.extension().string();
   std::transform( stringExtension.begin(), stringExtension.end(), stringExtension.begin(), ::tolower );
   if( stringExtension == ".xml" ) { return Load_s( stringPath, m_statement, gd::types::tag_xml{} ); }
   else { return { false, "Unsupported file format: " + stringExtension }; }
    
   return { true, "" };
}

/** -------------------------------------------------------------------------- Load_s
 * @brief Loads SQL statements from an XML file into a statement collection.
 * @param stringFilename The path to the XML file containing statement definitions.
 * @param statement_ The statement collection to populate with loaded statements.
 * @param  Tag dispatch parameter to indicate XML format loading.
 * @return A pair containing a success flag and an error message (empty on success).
 */
std::pair<bool, std::string> CQueries::Load_s( std::string_view stringFilename, gd::modules::dbmeta::statement& statement_, gd::types::tag_xml )
{
   using namespace gd::modules::dbmeta;

   if( std::filesystem::exists( stringFilename ) == false ) { return { false, "File not found: " + std::string( stringFilename ) }; }

   gd::argument::arguments argumentsStatement;
   argumentsStatement.reserve( 256 );

   // ## Initialize pugixml document .........................................
   pugi::xml_document xmldocument;

   // Load the XML file
   pugi::xml_parse_result xmlparseresult = xmldocument.load_file( stringFilename.data() );
   if( !xmlparseresult ) { return { false, "XML parsing error: " + std::string( xmlparseresult.description() ) }; }

   // query statements nodes
   for( pugi::xml_node xmlnodeStatements : xmldocument.document_element().children( "statements" ) )
   {
      for( pugi::xml_node xmlnodeStatement : xmlnodeStatements.children( "statement" ) )
      {
         argumentsStatement.clear();

         // ## Attributes (always present)
         argumentsStatement["uuid"] = xmlnodeStatement.attribute( "id" ).value();
         argumentsStatement["name"] = xmlnodeStatement.attribute( "name" ).value();
         argumentsStatement["type"] = xmlnodeStatement.attribute( "type" ).value();
         argumentsStatement["format"] = xmlnodeStatement.attribute( "format" ).value();
         argumentsStatement["description"] = xmlnodeStatement.attribute( "description" ).value();

         // ## Handle 'ui' attribute or child element (optional)
         pugi::xml_attribute xmlattrbuteUi = xmlnodeStatement.attribute( "ui" );
         if( xmlattrbuteUi ) { argumentsStatement["ui"] = xmlattrbuteUi.value(); }
         else
         {  // ### fallback to <ui> child element
            pugi::xml_node xmlnodeUi = xmlnodeStatement.child( "ui" );
            if( xmlnodeUi ) { argumentsStatement["ui"] = xmlnodeUi.child_value(); }
         }

         // ## Handle 'code' attribute or child element (optional)
         pugi::xml_attribute xmlattrbuteCode = xmlnodeStatement.attribute( "code" );
         if( xmlattrbuteCode ) { argumentsStatement["code"] = xmlattrbuteCode.value(); }
         else
         {  // ### fallback to <code> child element
            pugi::xml_node xmlnodeCode = xmlnodeStatement.child( "code" );
            if( xmlnodeCode ) { argumentsStatement["code"] = xmlnodeCode.child_value(); }
         }

         std::string statementText = xmlnodeStatement.text().get();         // get element text, this will include CDATA content if present

         argumentsStatement["statement"] = statementText;

         auto result_ = statement_.add( argumentsStatement, { "ui", "code"});
         if( result_.first == false ) { return { false, "Error adding statement: " + result_.second }; }
      }
   }
   return { true, "" };
}


NAMESPACE_META_END
