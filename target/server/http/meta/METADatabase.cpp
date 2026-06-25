#include <array>
#include <filesystem>

#include "pugixml/pugixml.hpp"


#include "../convert/CONVERTCore.h"

#include "METADatabase.h"

NAMESPACE_META_BEGIN

/** ---------------------------------------------------------------------------
 * Initialize metadata objects used in META::CDatabase.
 */
std::pair<bool, std::string> CDatabase::Initialize()
{
   try
   {
      // ## Create the necessary tables

      m_ptableTable = std::make_unique<gd::table::arguments::table>();
      CreateTable_s(*m_ptableTable);
      
      m_ptableColumn = std::make_unique<gd::table::arguments::table>();
      CreateColumn_s(*m_ptableColumn);

      
      m_ptableJoin = std::make_unique<gd::table::arguments::table>();
      CreateJoin_s(*m_ptableJoin);
      
      m_ptableComputed = std::make_unique<gd::table::arguments::table>();
      CreateComputed_s(*m_ptableComputed);

      m_pdatabase = std::make_unique<gd::modules::dbmeta::database>();
      m_pdatabase->initialize();

      m_pexpression = std::make_unique<gd::modules::dbmeta::expression>();
      m_pexpression->initialize();
   }
   catch (const std::exception& e)
   {
      return { false, e.what() };
   }
   
   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * Add information for tables in database.
 */
std::pair<bool, std::string> CDatabase::Add( gd::table::dto::table& tableTable, gd::types::tag_table )
{
   std::array<uint8_t, 512> buffer_;
   gd::argument::arguments argumentsRow( buffer_ );

   for( auto itRow = tableTable.row_begin(); itRow != tableTable.row_end(); itRow++ )
   {
      argumentsRow.clear();
      itRow.get_arguments( argumentsRow );

      argumentsRow.append( "key", (uint32_t)itRow.get_row() );
      m_ptableTable->row_add( argumentsRow, gd::table::tag_arguments{}, gd::table::tag_convert{});

      m_pdatabase->add(argumentsRow, gd::types::tag_table{});
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * Add information for columns in database. 
 */
std::pair<bool, std::string> CDatabase::Add( gd::table::dto::table& tableColumn, gd::types::tag_column )
{
   std::array<uint8_t, 512> buffer_;
   gd::argument::arguments argumentsRow( buffer_ );
   std::array<uint8_t, 512> add_;
   gd::argument::arguments argumentsAdd( add_ );

   int iColumnType = -1;
   int iColumnPK = -1;
   int iColumnFK = -1;
   int iColumnNotNull = -1;
   auto vectorColumn = tableColumn.column_get_name();
   unsigned uColumnIndex = 0;
   for( const auto& column_ : vectorColumn )
   {
      uColumnIndex++;
      if( column_.find( "type" ) != std::string_view::npos ) { iColumnType = uColumnIndex -1; continue; }
      if( column_.find( "pk" ) != std::string_view::npos ) { iColumnPK = uColumnIndex - 1; continue; }
      if( column_.find( "fk" ) != std::string_view::npos ) { iColumnFK = uColumnIndex - 1; continue; }
      if( column_.find( "null" ) != std::string_view::npos ) { iColumnNotNull = uColumnIndex - 1; continue; }
   }

   for( auto itRow = tableColumn.row_begin(); itRow != tableColumn.row_end(); itRow++ )
   {
      argumentsRow.clear();
      itRow.get_arguments( argumentsRow );
      argumentsAdd.clear();
      argumentsAdd.append( argumentsRow, {"table", "column", "ordinal"});

      if( iColumnType != -1 )
      {
         std::string stringType = itRow.cell_get_variant_view( (unsigned)iColumnType ).as_string();
         if( stringType.empty() == false )
         {
            uint32_t uType = CONVERT::DatabaseTypeToGdType( stringType );
            argumentsRow.append( "type", uType );
         }
      }

      uint32_t uFlags = 0;

      if( iColumnPK != -1 )
      {
         bool bPK = itRow.cell_get_variant_view( (unsigned)iColumnPK ).as_bool();
         uFlags |= bPK ? eColumnFlagKey : 0;
      }

      if( iColumnFK != -1 )
      {
         bool bFK = itRow.cell_get_variant_view( (unsigned)iColumnFK ).as_bool();
         uFlags |= bFK ? eColumnFlagFKey : 0;
      }

      if( iColumnNotNull != -1 )
      {
         bool bNotNull = itRow.cell_get_variant_view( (unsigned)iColumnNotNull ).as_bool();
         uFlags |= bNotNull ? eColumnFlagNotNull : 0;
      }

      argumentsRow.append( "flags", uFlags );
      argumentsRow.append( "key", (uint32_t)itRow.get_row() );
      m_ptableColumn->row_add( argumentsRow, gd::table::tag_arguments{}, gd::table::tag_convert{});
   }

   // ## Generate index for table name to be able to find table by name ......
   // get column for table name
   auto uColumnSchema = m_ptableColumn->column_find_index("schema");                                assert(uColumnSchema != (unsigned)-1 && "Column 'schema' not found in table metadata");
   auto uColumnTable = m_ptableColumn->column_find_index("table");                                  assert(uColumnTable != (unsigned)-1 && "Column 'table' not found in table metadata");
   m_indexstringTable = gd::table::create_index_g<gd::table::index_string_string>(*m_ptableColumn, uColumnSchema, uColumnTable);
   m_indexstringTable.compact();                                              // compact index to remove duplicates, only first row with lowest index is kept  


   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Find key for table by table name
 * @param stringTable name of table to find key for, can be in format "schema.table"
 * @param puKey pointer to variable where key is placed if found, can be nullptr if key is not needed
 * @return true if table is found and key is placed in puKey, false if table is not found
 */
bool CDatabase::Table_FindKey(std::string_view stringTable, uint32_t* puKey) const noexcept
{
   std::string_view stringSchema;
   // check for schema by finding . in table name, if found split schema and table name
   auto position_ = stringTable.find('.');
   if(position_ != std::string_view::npos)
   {
      stringSchema = stringTable.substr(0, position_);
      stringTable = stringTable.substr(position_ + 1);
   }

   auto iRow = m_pdatabase->find(stringSchema, stringTable, gd::types::tag_table{});
   if(iRow != -1)
   {
      uint32_t uKey = m_pdatabase->table_key(iRow);
      if(puKey != nullptr) { *puKey = uKey; }
      return true;
   }
   return false;
}


bool CDatabase::IsReadyToLinkTables() const
{
   bool bOk = (m_ptableTable && m_ptableTable->size() > 0);
   if( bOk == true ) 
   { 
      bool bColumnOk = (m_ptableColumn && m_ptableColumn->size() > 0);
      bool bJoinOk = (m_ptableJoin && m_ptableJoin->size() > 0);
      bool bComputedOk = (m_ptableComputed && m_ptableComputed->size() > 0);

      bOk = ( bColumnOk || bJoinOk || bComputedOk );
   }

   return bOk;
}

std::pair<bool, std::string> CDatabase::LinkTablesTables()
{                                                                                                  assert( m_ptableTable && m_ptableTable->size() > 0 );
   std::array<uint8_t, 256> buffer_;
   gd::argument::arguments argumentsFind( buffer_ );
   
   if( m_ptableColumn->size() != 0 )
   {
      for( auto itRow = m_ptableColumn->row_begin(); itRow != m_ptableColumn->row_end(); itRow++ )
      {
         argumentsFind.clear();
         auto vSchema_ = itRow.cell_get_variant_view( "schema" );
         auto vTable_ = itRow.cell_get_variant_view( "table" );
         if( vSchema_.is_string() == true ) { argumentsFind["schema"] = vSchema_.as_string_view(); }
         if( vTable_.is_string() == true ) { argumentsFind["table"] = vTable_.as_string_view(); }

         std::vector< std::pair< std::string_view, gd::variant_view> > vectorFind;
         for( auto [key_, value_] : argumentsFind.named() ) { vectorFind.push_back({ key_, value_ }); }

         int64_t iFindRow = m_ptableTable->find( vectorFind );                                     assert( iFindRow != -1 );
         if( iFindRow != -1 )
         {
#ifndef NDEBUG
            auto stringTableName_d = m_ptableTable->cell_get_variant_view( iFindRow, "table" ).as_string();
#endif // NDEBUG
            auto v_ = m_ptableTable->cell_get_variant_view( iFindRow, 0u );
            m_ptableColumn->cell_set( iFindRow, "table-key", v_);
         }
      }
   }

   return { true, "" };
}

std::pair<bool, std::string> CDatabase::ComputeTextLength( std::string_view stringTable, std::vector<std::string_view> vectorField, uint64_t* puMaxLength ) const
{                                                                                                  assert( m_ptableColumn && m_ptableColumn->size() > 0 );
   uint64_t uMaxNameLength = 0;

   // ## First calculate max length for text names in column
   for( const auto& stringMatchColumn : vectorField ) { uMaxNameLength = std::max( uMaxNameLength, (uint64_t)stringMatchColumn.length() ); }


   for( auto itRow = m_ptableColumn->row_begin(); itRow != m_ptableColumn->row_end(); itRow++ )
   {
      auto table_ = itRow.cell_get_variant_view( "table" );
      auto stringMatchTable = table_.as_string_view();
      if( stringMatchTable != stringTable ) { continue; }                              // Skip other tables

      // ## Note that all columns in table are grouped so when the table is found 
      //    we can skip the rest of the table and match columns that belongs to found table

      for( ;itRow != m_ptableColumn->row_end() && stringTable == stringMatchTable; itRow++ )
      { 
         auto column_ = itRow.cell_get_variant_view( "column" );
         auto stringColumn = column_.as_string_view();
         for( const auto& stringMatchColumn : vectorField )
         {
            if( stringColumn == stringMatchColumn )
            {
               // ## Compute max length for text names in column
               auto alias_ = itRow.cell_get_variant_view( "alias" );
               if( alias_.is_string() == true ) uMaxNameLength = std::max( uMaxNameLength, (uint64_t)alias_.as_string_view().length() );
               break;
            }
         }
      }

      break;
   }

   if( puMaxLength != nullptr ) { *puMaxLength = uMaxNameLength; }
   return { true, "" };
}

/** --------------------------------------------------------------------------
 * @brief Read selected column metadata into table
 * @param stringTable name of table to filter columns on
 * @param vectorField fields in table to read information about
 * @param ptableColumn pointer to table where column information are placed 
 * @return success if ok, false and error message if not ok
 */
std::pair<bool, std::string> CDatabase::ReadColumnMetadata( std::string_view stringTable, std::vector<std::string_view> vectorField, gd::table::dto::table* ptableColumn ) const
{
   std::array<uint8_t, 256> buffer_;
   auto vectorColumnNames = ptableColumn->column_get_name();
   auto vectorColumnIndices = m_ptableColumn->column_get_index( vectorColumnNames );
   
   // ## Find first row for table name in internal table holding column information
   
   for( auto itRow = m_ptableColumn->row_begin(); itRow != m_ptableColumn->row_end(); itRow++ )
   {
      auto table_ = itRow.cell_get_variant_view( "table" );
      auto stringMatchTable = table_.as_string_view();
      if( stringMatchTable != stringTable ) { continue; }                              // Skip other tables
      
      for( ;itRow != m_ptableColumn->row_end() && stringTable == stringMatchTable; itRow++ )
      { 
         auto column_ = itRow.cell_get_variant_view( "column" );
         auto stringColumn = column_.as_string_view();
         for( const auto& stringMatchColumn : vectorField )
         {
            if( stringColumn == stringMatchColumn )
            {
               gd::argument::arguments arguments_( buffer_ );
               m_ptableColumn->cell_get( itRow.get_row(), vectorColumnIndices, arguments_);
               ptableColumn->row_add( arguments_, gd::table::tag_arguments{});
               break;
            }
         }
      }
      break;
   }      
   
   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Find a row in the column metadata table by hierarchical search criteria
 * 
 * Performs a hierarchical search through the column metadata table using schema,
 * table, and column names as filters. The search progressively narrows results:
 * first by schema (if provided), then by table name (if provided), and finally
 * by column name (if provided).
 * 
 * @param argumentsFind Arguments containing search criteria with keys:
 *                      - "schema": Schema name (optional)
 *                      - "table": Table name (optional)
 *                      - "column": Column name (optional)
 * @return Row index (0-based) if found, -1 if not found
 * 
 * @note The method uses progressive filtering where each level depends on the
 *       success of the previous level. If a schema search fails, table and column
 *       searches are not performed.
 */
int64_t CDatabase::Column_FindRow( const gd::argument::arguments& argumentsFind ) const noexcept
{
#ifndef NDEBUG
   // ## Validate keys
   auto vectorKey = argumentsFind.get_keys();
   for( const auto& key_ : vectorKey )
   {
      if( key_ != "schema" && key_ != "table" && key_ != "column" )
      {                                                                                            assert( false && "Invalid key in argumentsFind, expected keys are 'schema', 'table', 'column'" );
         return -1; // Invalid key found
      }
   };
#endif // NDEBUG

   int64_t iRow = 0;
   std::string_view stringSchema = argumentsFind["schema"].as_string_view();  // Get schema name from arguments
   std::string_view stringTable = argumentsFind["table"].as_string_view();    // Get table name from arguments
   std::string_view stringColumn = argumentsFind["column"].as_string_view();  // Get column name from arguments
                                                                                                   assert( stringColumn.empty() == false && "Column name is required to find column metadata" );

   if( stringSchema.empty() == false || stringTable.empty() == false )
   {

      const auto [bFound, iIndexRow] = m_indexstringTable.find(stringSchema, stringTable);
      assert(bFound == true && iIndexRow == iRow && "Index for table name does not match found row in column metadata table");
      iRow = iIndexRow;

#ifndef NDEBUG
      unsigned uColumnSchema = m_ptableColumn->column_get_index("schema");
      unsigned uColumnTable = m_ptableColumn->column_get_index("table");

      int64_t iRow_d = 0;

      if(stringSchema.empty() == false)
      {
         iRow_d = m_ptableColumn->find(uColumnSchema, stringSchema);
      }

      if(iRow != -1 && stringTable.empty() == false)
      {
         iRow_d = m_ptableColumn->find(uColumnTable, (uint64_t)iRow_d, stringTable);
      }
      else
      {
         iRow_d = m_ptableColumn->find(uColumnTable, stringTable);
      }

      assert(iIndexRow == iRow_d && "Index for table name does not match found row in column metadata table");
#endif // NDEBUG
   }
   /*
   else

   if( stringSchema.empty() == false )
   {
      unsigned uColumn = m_ptableColumn->column_get_index( "schema" );
      iRow = m_ptableColumn->find( uColumn, stringSchema, stringSchema );
   }

   if( iRow != -1 && stringTable.empty() == false )
   {
      unsigned uColumn = m_ptableColumn->column_get_index( "table" );
      iRow = m_ptableColumn->find( uColumn, (uint64_t)iRow, stringTable );
#ifndef NDEBUG
      const auto [bFound_d, iIndexRow_d] = m_indexstringTable.find(stringTable);
      assert(bFound_d == true && iIndexRow_d == iRow && "Index for table name does not match found row in column metadata table");
#endif // NDEBUG
   }
   */

   if( iRow != -1 && stringColumn.empty() == false )
   {
      unsigned uColumn = m_ptableColumn->column_get_index( "column" );
      iRow = m_ptableColumn->find( uColumn, (uint64_t)iRow, stringColumn );
   }
    
   return iRow;
}

/// @brief Find a row in the expression table by search criteria
int64_t CDatabase::Expression_FindRow( const gd::argument::arguments& argumentsFind ) const noexcept
{
#ifndef NDEBUG
   std::string stringFind_d = gd::argument::debug::print(argumentsFind);
#endif // NDEBUG
   if(argumentsFind.empty() == true) { return -1; }                             // No search criteria provided
   std::array<uint8_t, 256> buffer_;
   gd::argument::arguments argumentsExpression(buffer_);

   int64_t iRow = -1;

   if(argumentsFind.exists("table") == true)                              // If "table" then get key to table
   {
      std::string_view stringTable = argumentsFind["table"].as_string_view();
      std::string_view stringSchema;
      auto position_ = stringTable.find('.');
      if(position_ != std::string_view::npos)
      {
         stringSchema = stringTable.substr(0, position_);
         stringTable = stringTable.substr(position_ + 1);
      }  

      auto iRow = m_pdatabase->find(stringSchema, stringTable, gd::types::tag_table{});
      if(iRow != -1) { argumentsExpression["table-key"] = m_pdatabase->table_key(iRow); } // If table is found, add key to table to search criteria
   }
   
   argumentsExpression.append(argumentsFind, { "id", "column", "group", "type" });

   iRow = m_pexpression->find(argumentsExpression);

   return iRow;
}

/// @brief Get the type of an expression from the expression table
uint32_t CDatabase::Expression_GetType(uint64_t uRow) const noexcept
{                                                                                                  assert( uRow < m_pexpression->size() && "Row index out of range" );
   if(uRow >= m_pexpression->size()) { return 0; }                            // Out of range check
   uint32_t uType = m_pexpression->get_type(uRow);
   return uType;
}

std::string_view CDatabase::Expression_GetExpression(uint64_t uRow) const noexcept
{                                                                                                  assert( uRow < m_pexpression->size() && "Row index out of range" );
   if(uRow >= m_pexpression->size()) { return ""; }                            // Out of range check
   std::string_view stringExpression = m_pexpression->get_expression(uRow);
   return stringExpression;
}

/// @brief Get the key of a table from the database by table name
int32_t CDatabase::Table_GetKey(std::string_view stringTable) const noexcept
{
   int32_t iKey = -1;
   auto iRow = m_pdatabase->find(stringTable, gd::types::tag_table{});
   if(iRow != -1) { iKey = (int32_t)m_pdatabase->table_key(iRow); }
   return iKey;   
}

/// @brief Get the type of a column from the column metadata table
uint32_t CDatabase::Column_GetType( uint64_t uRow ) const noexcept
{                                                                                                  assert( uRow < m_ptableColumn->get_row_count() && "Row index out of range" );
   uint32_t uType = 0;
   unsigned uColumn = m_ptableColumn->column_get_index( "type" );
   gd::variant_view vType = m_ptableColumn->cell_get_variant_view( uRow, uColumn );
   if( vType.is_uint32() == true ) { uType = vType.as_uint32(); }
   return uType;
}

/// @brief Get the maximum size of a column from the column metadata table
uint32_t CDatabase::Column_GetMaxSize( uint64_t iRow ) const noexcept
{                                                                                                  assert( iRow < m_ptableColumn->get_row_count() && "Row index out of range" );
   uint32_t uSize = 0;
   unsigned uColumn = m_ptableColumn->column_get_index( "size" );
   gd::variant_view vSize = m_ptableColumn->cell_get_variant_view( iRow, uColumn );
   if( vSize.is_uint32() == true ) { uSize = vSize.as_uint32(); }
   return uSize;
}

// @TODO: Implement a more efficient index for column metadata, and use that finding column information
void CDatabase::Column_CreateIndex() 
{
   // ## Create index for column metadata table, this is used to quickly find column information by schema, table and column name
   //    The index is created by concatenating schema, table and column name into a single string and using that as key for the index.
   //    This allows for fast lookup of column information based on the combination of schema, table and column name.
   // ## Note that this is a simple implementation of an index, in a real implementation you would want to use a more efficient data structure for the index, like a hash map or a trie.
}

/** ---------------------------------------------------------------------------
 * @brief Loads SQL expressions from an XML file into a table structure.
 *
 * <expressions table="owner table">
 *    <expression> expression logic </expression>
 *    <expression name="expression name" column="column name" type="data type"> expression logic </expression>
 *    <expression name="expression name" column="table.field.type;table.field.type;"> expression logic </expression>
 * </expressions>
 *
 * @param stringFilename The path to the XML file containing expressions to load.
 * @param ptableExpression Pointer to the table that will receive the loaded expression data.
 * @param  Tag dispatch parameter to specify XML format loading.
 * @return A pair containing a boolean success flag and an error message string (empty on success).
 */
std::pair<bool, std::string> CDatabase::LoadExpressions(std::string_view stringFilename, gd::table::dto::table* ptableExpression, gd::types::tag_xml)
{
   using namespace gd::modules::dbmeta;

   if(std::filesystem::exists(stringFilename) == false) { return { false, "File not found: " + std::string(stringFilename) }; }

   gd::table::dto::table tableExpression = gd::modules::dbmeta::expression::create_expression_s();

   gd::argument::arguments argumentsExpression;
   argumentsExpression.reserve(256);

   // ## Initialize pugixml document ..........................................
   pugi::xml_document xmldocument;

   // Load the XML file
   pugi::xml_parse_result xmlparseresult = xmldocument.load_file(stringFilename.data());
   if(false == xmlparseresult ) { return { false, "XML parsing error: " + std::string(xmlparseresult.description()) }; }

   // ## Prepare error logic ..................................................
   struct error_ { std::string stringError; std::string stringNode; };
   std::vector<error_> vectorError; // Vector to hold errors encountered during loading
   auto add_error_ = [&vectorError](std::string_view stringError, pugi::xml_node xmlnode) {
      std::string stringNode;
      if(xmlnode) { stringNode = std::string(xmlnode.name()) + " with value '" + xmlnode.child_value() + "'"; }
      vectorError.push_back({ std::string(stringError), stringNode });
   };

   auto expressions_ = xmldocument.document_element().children("expressions"); // find all expression nodes
   for(auto expressionsActive : expressions_)
   {
      std::string_view stringGroup = expressionsActive.attribute("group").value(); // optional group for expressions
      if(stringGroup.empty() == false) { m_pexpression->add_group(stringGroup); } // create group for expressions if group attribute is found

      for(auto expression_ : expressionsActive.children("expression"))
      {
         argumentsExpression.clear();

         if(stringGroup.empty() == false) { argumentsExpression["group"] = stringGroup; }

         // ## id information .................................................
         std::string_view stringId = expression_.attribute("id").value();
         if(stringId.empty() == true) { add_error_("id is missing", expression_); continue; }
         argumentsExpression["id"] = stringId;

         // ## table information ...............................................
         std::string_view stringTable = expression_.attribute("table").value();
         if(stringTable.empty() == true) { stringTable = expressionsActive.attribute("table").value(); } // get table from expressions if not found in expression
         if(stringTable.empty() == true) { add_error_("Table is missing", expression_); continue; }
         argumentsExpression["table"] = stringTable;

         // ## Find table key for expression to connect table .................
         uint32_t uTableKey = 0;
         if(Table_FindKey(stringTable, &uTableKey) == false) { add_error_("Failed to find table key", expression_); continue; }
         argumentsExpression["table-key"] = uTableKey;                         // Add table key to arguments for expression, this is used to quickly find owner table for expression

         // ## column information .................................................
         std::string_view stringColumn = expression_.attribute("column").value();
         if(stringColumn.empty() == true) { add_error_("Column is missing", expression_); continue; }
         argumentsExpression["column"] = stringColumn;

         std::string_view stringType = expression_.attribute("type").value();
         if(stringType.empty() == false) 
         {
            uint32_t uType = gd::types::type_g(stringType);
            argumentsExpression["type"] = uType;
         }

         std::string_view stringExpression = expression_.attribute("expression").value();
         if(stringExpression.empty() == true) { stringExpression = expression_.child_value(); }
         if(stringExpression.empty() == true) { add_error_("Expression is missing", expression_); continue; }
         argumentsExpression["expression"] = stringExpression;

         tableExpression.row_add(argumentsExpression, gd::table::tag_arguments{});
      }
   }

   // ## Check for errors and return if any ...................................
   if(vectorError.empty() == false)
   {
      std::string stringError = "Errors encountered while loading expressions:\n";
      for(const auto& error : vectorError)
      {
         stringError += "- " + error.stringError;
         if(error.stringNode.empty() == false) { stringError += " in node: " + error.stringNode; }
         stringError += "\n";
      }
      return { false, stringError };
   }

   // ## add loaded expressions to database ...................................
   auto result_ = m_pexpression->add(tableExpression);
   if(result_.first == false) { return { false, "Failed to add expressions to database: " + result_.second }; }

   // ## Update table keys for expression table, this is used to quickly find owner table

   return { true, "" };
}



void CDatabase::CreateTable_s( gd::table::arguments::table& tableTable )
{
   // ## Create table with meta information physical tables, this also works as a type of index for database table

   tableTable.set_flags( gd::table::tag_meta{} );   
   tableTable.column_prepare();
   tableTable.column_add( {
      { "uint32",  0, "key"         }, // key (these keys do also represent row number to be fast)
      { "uint32",  0, "first-key"   }, // first row (key of first row) in database table
      { "rstring", 0, "schema"      }, // table schema
      { "rstring", 0, "table"       }, // table name
      { "rstring", 0, "alias"       }, // alias
      { "rutf8",   0, "description" }  // column description (custom value)
   }, gd::table::tag_type_name{});
   tableTable.prepare();
   
}

void CDatabase::CreateColumn_s( gd::table::arguments::table& tableColumn )
{                                                                                                  assert( tableColumn.empty() == true );
   // ## Create table with meta information for physical tables and columns
   tableColumn.set_flags( gd::table::tag_meta{} );
   tableColumn.column_prepare();
   tableColumn.column_add( {
      { "uint32",  0, "key"         }, // key (these keys do also represent row number to be fast)
      // Link
      { "uint32",  0, "table-key"    }, // key to the parent table in `m_ptableTable` (defines which table this column belongs to)

      // Name for table and column      
      { "rstring", 0, "schema"      }, // table schema
      { "rstring", 0, "table"       }, // table name
      { "rstring", 0, "column"      }, // column name
      
      // Column properties
      { "uint32",  0, "ordinal"     }, // column positions in table
      { "uint32",  0, "type"        }, // column type, what type of data that column holds
      { "uint32",  0, "size"        }, // column size, this is the maximum size of data for the column if column holds non primitive type
      { "uint32",  0, "flags"       }, // column flags (specifics like code, key, indexed, unique, required etc.)
      { "rstring", 0, "alias"       }, // column alias (custom value)
      { "rstring", 0, "default"     }, // column default value (custom value)
      { "rutf8",   0, "description" }  // column description (custom value)
   }, gd::table::tag_type_name{});
   tableColumn.prepare();
}

void CDatabase::CreateJoin_s( gd::table::arguments::table& tableJoin )
{                                                                                                  assert( tableJoin.empty() == true );
   // ## Create table with meta information to join tables

   tableJoin.set_flags( gd::table::tag_meta{} );
   tableJoin.column_prepare();
   tableJoin.column_add( {
      { "uint32",  0, "key"           }, // key
      // Parent Side (Where we start)
      { "uint32",  0, "parent-key"    }, // parent key to table in database description table 
      { "rstring", 0, "parent-alias"  }, // alias for parent table
      { "uint32",  0, "parent-suffix" }, // if number is used to make the name unique

      // Child Side (Where we go)
      { "uint32",  0, "child-key"     }, // child key to table in database description table 
      { "rstring", 0, "child-alias"   }, // alias for child table
      { "uint32",  0, "child-suffix"  }, // if number is used to make the name unique

      // Join Logic
      { "rstring", 0, "join-on"       }, // The logic: string to generate in sql "parent.id = child.parent_id" or "{=parent}.id = {=child}.parent_id"
      { "uint32",  0, "join-type"     }, // Flags to configure what is allowed and default in joinging, like Inner, Left, Right, etc.
      { "uint32",  0, "cardinality"   }, // Enum: 1:1, 1:N, N:M

      // Metadata
      { "rutf8",   0, "description"   } 
   }, gd::table::tag_type_name{});
   tableJoin.prepare();
}

void CDatabase::CreateComputed_s( gd::table::arguments::table& tableComputed )
{
   // ## Create table with meta information for calculated/virtual columns
   // These columns are generated by SQL logic (expressions) rather than being physical fields.

   tableComputed.set_flags( gd::table::tag_meta{} );
   tableComputed.column_prepare();
   tableComputed.column_add( {
      { "uint32",  0, "key"          }, // key (unique ID)
      
      // Link
      { "uint32",  0, "table-key"    }, // key to the parent table in tableTable (defines which table this column belongs to)

      // Definition
      { "rstring", 0, "name"         }, // The name/alias for the resulting column (e.g., "total_price")
      { "uint32",  0, "type"         }, // The data type of the result (important for type checking and casting)
      { "rutf8",   0, "expression"   }, // The SQL logic (e.g., "price * quantity" or "CONCAT(first, ' ', last)")
      
      // Configuration
      { "uint32",  0, "flags"        }, // Flags for specific logic (e.g., IsAggregate, IsDistinct, RequiresGroupBy)
      
      // Metadata
      { "rutf8",   0, "description"  }  // Description of the calculated logic
   }, gd::table::tag_type_name{});
   tableComputed.prepare();
}


/*
   table_.column_add( { 
      {"rstring", 0, "table_name"}, 
      {"rstring", 0, "column_name"}, 
      {"uint32", 0, "type"}, 
      {"uint32", 0, "size"}, 
      {"uint32", 0, "flags"}, 
      {"uint32", 0, "types"}, 
      {"uint32", 0, "properties"}, 
      {"rstring", 0, "alias"}, 
      {"rstring", 0, "default"}, 
      {"rutf8", 0, "description" } }, 
      gd::table::tag_type_name{}
   );


*/



NAMESPACE_META_END
