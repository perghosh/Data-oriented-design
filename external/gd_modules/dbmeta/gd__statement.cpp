#include "gd/gd_binary.h"

#include "gd__statement.h"

_GD_MODULES_DBMETA_BEGIN


/// Initializes the `statement` object -------------------------------------- initialize
void statement::initialize()
{                                                                                                  assert( m_ptableStatement == nullptr );
   m_ptableStatement = std::make_unique<gd::table::arguments::table>();
   create_statement_s( *m_ptableStatement );
}

std::pair<bool, std::string> statement::add( std::string_view stringName, std::string_view stringStatement, std::string_view stingFormat, std::string_view stringType, std::string_view stringRule )
{                                                                                                  assert( stringName.empty() == false ); assert( stringStatement.empty() == false );
   enumFormat eFormat = to_format_s( stingFormat );
   uint32_t uType = (uint32_t)to_type_s( stringType );
   uint32_t uRule = 0;
   return add( stringName, stringStatement, eFormat, uType, uRule );
}


/** ------------------------------------------------------------------------ add
 * @brief Adds a new statement to the statement table.
 * @param stringName The name of the statement.
 * @param stringStatement The SQL statement text.
 * @param eFormat The format type of the statement.
 * @param uType The type identifier for the statement.
 * @param uRule The rule flags for the statement.
 * @return A pair containing success status (true) and the generated UUID of the new statement.
 */
std::pair<bool, std::string> statement::add( std::string_view stringName, std::string_view stringStatement, enumFormat eFormat, uint32_t uType, uint32_t uRule )
{                                                                                                  assert( m_ptableStatement != nullptr );
   auto uRow = m_ptableStatement->row_add_one();
   uint32_t uKey = (uint32_t)uRow;
   m_ptableStatement->cell_set( uRow, eColumnKey, (uint32_t)uKey );
   m_ptableStatement->cell_set( uRow, eColumnUuid, gd::types::uuid_generate_g() );
   m_ptableStatement->cell_set( uRow, eColumnName, stringName );             // name of the statement, this can be used to identify statement
   m_ptableStatement->cell_set( uRow, eColumnType, (uint32_t)uType );        // type of query statement to know what type of operation to perform.
   m_ptableStatement->cell_set( uRow, eColumnFormat, (uint32_t)eFormat );    // format is used to know how to parse statement 
   m_ptableStatement->cell_set( uRow, eColumnRule, uRule );                  // rule is optional, if not provided set to 0, this can be used for future extensions or flags related to statement execution, rule is for now just a placeholder for future use
   m_ptableStatement->cell_set( uRow, eColumnStatement, stringStatement );

   std::string stringUuid = m_ptableStatement->cell_get_variant_view( uRow, eColumnUuid ).as_string();

   return { true, stringUuid }; 
}

/** ------------------------------------------------------------------------ add
 * @brief Adds a new statement row to the statement table with values from the provided arguments.
 * 
 * @code
 * gd::argument::arguments argumentsStatement;
 * argumentsStatement.append( "uuid", "550e8400-e29b-41d4-a716-446655440000" );
 * argumentsStatement.append( "name", "SampleStatement" );
 * argumentsStatement.append( "format", "xml" );
 * argumentsStatement.append( "type", "select" );
 * argumentsStatement.append( "rule", "default" );
 * argumentsStatement.append( "statement", "SELECT 1" );
 * argumentsStatement.append( "description", "Example statement row" );
 * 
 * auto pairResult = statementInstance.add( argumentsStatement );
 * if( pairResult.first == false )
 * {
 *    // handle error in `pairResult.second`
 * }
 * @endcode
 * 
 * @param argumentsStatement Arguments containing statement properties (uuid, name, format, type, rule, statement, description). Missing or invalid values trigger defaults or auto-generation.
 * @return Pair containing success boolean and error message string (empty on success).
 */
std::pair<bool, std::string> statement::add( const gd::argument::arguments& argumentsStatement )
{                                                                                                  assert( m_ptableStatement != nullptr );
   std::pair<bool, std::string> result_;
   auto uRow = m_ptableStatement->row_add_one();
   uint32_t uKey = (uint32_t)uRow;
   m_ptableStatement->cell_set( uRow, eColumnKey, (uint32_t)uKey );

   // ## Set conlumv values for added row, if value is not provided or invalid set default values

   // ### UUID value, if not provided or invalid generate a new one

   auto uuid_ = argumentsStatement["uuid"];
   if( uuid_.is_string() == true && uuid_.is_true() )
   {
      // ## Try to parse the UUID from the string, if it fails generate a new one
      gd::types::uuid uuidValue;
      auto stringUuid = uuid_.as_string_view();
      if( stringUuid.length() == 32 || stringUuid.length() == 36 )            // if uuid length
      {

         if( stringUuid.length() == 32 ) { result_ = gd::binary_validate_hex_g( stringUuid ); }
         else if( stringUuid.length() == 36 ) { result_ = gd::binary_validate_uuid_g( stringUuid ); }
         if( result_.first == false ) { return { false, "Invalid UUID format: " + result_.second }; }


         uuidValue = gd::uuid( stringUuid.data(), stringUuid.data() + stringUuid.length() );
         m_ptableStatement->cell_set( uRow, eColumnUuid, uuidValue );
      }
   }
   else if( uuid_.is_binary() == true )
   {
      auto binary_ = uuid_.as_binary_view();
      if( binary_.size() == 16 )
      {
         gd::types::uuid uuidValue( binary_ );
         m_ptableStatement->cell_set( uRow, eColumnUuid, uuidValue );
      }
      else { return { false, "Invalid binary UUID length: " + std::to_string( binary_.size() ) }; }
   }
   else
   {
      m_ptableStatement->cell_set( uRow, eColumnUuid, gd::types::uuid_generate_g() ); // generate new UUID if not provided or invalid
   }

   // ### Name value, if not provided set empty string

   auto name_ = argumentsStatement["name"];
   if( name_.is_string() == true ) m_ptableStatement->cell_set( uRow, eColumnName, name_.as_string_view() );

   // ### Format value, if not provided set to eFormatRaw

   auto format_ = argumentsStatement["format"];
   uint32_t uFormat = (uint32_t)eFormatRaw;
   if( format_.is_string() == true ) uFormat = to_format_s( format_.as_string_view() );
   else if( format_.is_number() == true ) uFormat = format_.as_uint();
   if( uFormat == 0 || uFormat >= (uint32_t)eFormatMAX ) { return { false, "Invalid format value: " + std::to_string( uFormat ) }; }
   m_ptableStatement->cell_set( uRow, eColumnFormat, uFormat );

   // ### Type value, if not provided set to eTypeUnknown

   auto type_ = argumentsStatement["type"];
   uint32_t uType = (uint32_t)eTypeUnknown;
   if( type_.is_string() == true ) uType = to_format_s( type_.as_string_view() );
   else if( type_.is_number() == true ) uType = type_.as_uint();
   if( uType == 0 || uType >= (uint32_t)eTypeMAX ) { return { false, "Invalid type value: " + std::to_string( uType ) }; }
   m_ptableStatement->cell_set( uRow, eColumnType, uType );

   // ### Rule value, if not provided set to 0

   auto rule_ = argumentsStatement["rule"];
   if( rule_.is_number() == true ) m_ptableStatement->cell_set( uRow, eColumnRule, rule_.as_uint() );

   // ### Statement value, if not provided set to empty string

   auto statement_ = argumentsStatement["statement"];
   if( statement_.is_string() == true ) m_ptableStatement->cell_set( uRow, eColumnStatement, statement_.as_string_view() );

   auto description_ = argumentsStatement["description"];
   if( description_.is_string() == true && description_.is_true() ) m_ptableStatement->cell_set( uRow, eColumnDescription, description_.as_string_view() );

   return { true, "" };
}

std::pair<bool, std::string> statement::add( const gd::argument::arguments& argumentsStatement, const std::initializer_list<std::string_view>& listArgument )
{
   auto result_ = add( argumentsStatement );

   for( auto stringArgument : listArgument )
   {
      auto argument_ = argumentsStatement[stringArgument];
      if( argument_.is_string() == true && argument_.is_true() ) m_ptableStatement->cell_set( result_.first, stringArgument, argument_.as_string_view() );
   }

   return { true, "" };
}


/// @brief Retrieves the UUID for a given row index. ------------------------- get_id
gd::types::uuid statement::get_id( uint64_t uRow ) const
{                                                                                                  assert( m_ptableStatement != nullptr ); assert( uRow < m_ptableStatement->size() );
   gd::types::uuid uuid_ = m_ptableStatement->cell_get_variant_view( uRow, eColumnUuid ).as_binary_view();
   return uuid_; 
}


/// @brief Retrieves the SQL statement for a given row index. ---------------- get_statement
std::string_view statement::get_statement( uint64_t uRow ) const
{                                                                                                  assert( m_ptableStatement != nullptr ); assert( uRow < m_ptableStatement->size() );
   std::string_view stringStatement = m_ptableStatement->cell_get_variant_view( uRow, eColumnStatement ).as_string_view();
   return stringStatement; 
}


/** ------------------------------------------------------------------------- find
 * @brief Finds a UUID in the statement.
 * @param puuid Pointer to the UUID to search for.
 * @return The index of the UUID if found, or -1 if not found.
 */
int64_t statement::find( const gd::types::uuid* puuid ) const
{                                                                                                  assert( m_ptableStatement != nullptr ); assert( puuid != nullptr );
   int64_t iRow = m_ptableStatement->find( eColumnUuid, *puuid );
   return iRow;
}

/** ------------------------------------------------------------------------- find
 * @brief Finds a statement by its name.
 * @param stringName The name of the statement to search for.
 * @return The index of the statement if found, or -1 if not found.
 */
int64_t statement::find( std::string_view stringName ) const
{
   int64_t iRow = m_ptableStatement->find( eColumnName, stringName );
   return iRow;
}

/// @brief Counts the number of occurrences of a UUID in the statement. ----- count
size_t statement::count( const gd::types::uuid& uuidKey ) const
{
   size_t uCount = 0;
   for( const auto& row : m_ptableStatement->rows() )
   {
      auto uuidValue = row.cell_get_variant_view( eColumnUuid ).as_binary_view();
      if( uuidKey == uuidValue ) uCount++;
   }
   
   return uCount;
}

void statement::erase( uint64_t uRow )
{
   m_ptableStatement->erase( uRow );
}

void statement::create_statement_s( gd::table::arguments::table& tableStatement )
{
   // ## Create table with meta information about statements

   tableStatement.set_flags( gd::table::tag_meta{} );   
   tableStatement.column_prepare();
   tableStatement.column_add( {
      { "uint32",   0, "key"         }, // key (these keys do also represent row number to be fast)
      { "uuid",     0, "uuid"        }, // unique identifier for statement
      { "string",  32, "name"        }, // statement name, this is used to identify statement and also used in query templates to refer to statement
      { "uint32",   0, "type"        }, // type of statement, describes what kind of statement this is, like select, insert, update, delete, or other types of statements if needed
      { "uint32",   0, "format"      }, // statement format, this is used to determine how to generate sql statement, and how to parse arguments for statement
      { "uint32",   0, "rule"        }, // statement rule, this is used to determine how to use statement, and what is allowed for statement
      { "rstring",  0, "statement"   }, // raw data for statement, this is used to store the actual sql statement template, this can also be used to store other types of statements if needed
      { "string",   0, "description" }, // optional description for statement, this can be used to provide additional information about statement, like what it does, or how to use it
   }, gd::table::tag_type_name{});
   tableStatement.prepare();
}


_GD_MODULES_DBMETA_END