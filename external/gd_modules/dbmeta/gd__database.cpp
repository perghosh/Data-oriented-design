// @FILE [tag: database, meta] [description: Database description and meta information] [type: source] [name: gd__database.cpp]

#include "gd/gd_sql_types.h"

#include "gd__database.h"

_GD_MODULES_DBMETA_BEGIN


std::pair<bool, std::string> database::initialize()
{
   // ## Create the necessary tables

   m_ptableTable = std::make_unique<gd::table::arguments::table>();           // create table for table metadata
   create_table_s( *m_ptableTable );
   
   m_ptableColumn = std::make_unique<gd::table::arguments::table>();          // create table for column metadata
   create_column_s(*m_ptableColumn);
   
   return { true, "" };
}

// @TODO: general method to adding expression
std::pair<bool, std::string> database::add(const gd::argument::arguments& argumentsTable, gd::types::tag_table)
{                                                                                                  assert(m_ptableTable != nullptr); assert(m_ptableTable->empty() == false);
   std::array<uint8_t, 256> buffer_;
   gd::argument::arguments argumentsRow( buffer_ );

   uint32_t uKey = (uint32_t)m_ptableTable->size() + 1u;
   argumentsRow.append("key", uKey );
   argumentsRow.append(argumentsTable, { "schema", "table", "alias", "description" } );            assert(argumentsRow.exists("table" ) == true);

   m_ptableTable->row_add(argumentsRow, gd::table::tag_arguments{}, gd::table::tag_convert{});
   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * Add information for tables in database.
 * 
 * 
 */
std::pair<bool, std::string> database::add( gd::table::dto::table& tableTable, gd::types::tag_table )
{
   std::array<uint8_t, 512> buffer_;
   gd::argument::arguments argumentsRow( buffer_ );

   for( auto itRow = tableTable.row_begin(); itRow != tableTable.row_end(); itRow++ )
   {
      argumentsRow.clear();
      itRow.get_arguments( argumentsRow );

      argumentsRow.append( "key", (uint32_t)itRow.get_row() );
      m_ptableTable->row_add( argumentsRow, gd::table::tag_arguments{}, gd::table::tag_convert{});
   }

   return { true, "" };
}

std::pair<bool, std::string> database::add( gd::table::dto::table& tableColumn, gd::types::tag_column )
{
   using namespace gd::sql;

   enum enumColumnFlag
   {
      eKey = 0x01,     ///< column is a key column, used for where part of query
      eFkey = 0x02,    ///< column is a foreign key column, used for where part of query
      eNotnull = 0x04, ///< column is not null, used for where part
      eComputed = 0x08,///< column is computed, used for select part of query
   };

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
            uint32_t uType = sql_type_to_gd_type( stringType );
            argumentsRow.append( "type", uType );
         }
      }

      uint32_t uFlags = 0;

      if( iColumnPK != -1 )
      {
         bool bPK = itRow.cell_get_variant_view( (unsigned)iColumnPK ).as_bool();
         uFlags |= bPK ? eKey : 0;
      }

      if( iColumnFK != -1 )
      {
         bool bFK = itRow.cell_get_variant_view( (unsigned)iColumnFK ).as_bool();
         uFlags |= bFK ? eFkey : 0;
      }

      if( iColumnNotNull != -1 )
      {
         bool bNotNull = itRow.cell_get_variant_view( (unsigned)iColumnNotNull ).as_bool();
         uFlags |= bNotNull ? eNotnull  : 0;
      }

      argumentsRow.append( "flags", uFlags );
      argumentsRow.append( "key", (uint32_t)itRow.get_row() );
      m_ptableColumn->row_add( argumentsRow, gd::table::tag_arguments{}, gd::table::tag_convert{});
   }

   return { true, "" };
}

/// @brief Find table in database by table name, this is used to link database information
int64_t database::find(const std::string_view& stringTable, gd::types::tag_table) const noexcept
{
   return find(std::string_view{}, stringTable, gd::types::tag_table{});
}

/** --------------------------------------------------------------------------- find
 * Find table in database by schema and table name, this is used to link database information
 * that belongs to table together.
 *
 * @return key for table if found, otherwise -1
 */
int64_t database::find(const std::string_view& stringSchema, const std::string_view& stringTable, gd::types::tag_table) const noexcept
{                                                                                                  assert(m_ptableTable != nullptr); assert(m_ptableTable->empty() == false);
   for(auto itRow = m_ptableTable->row_begin(); itRow != m_ptableTable->row_end(); itRow++)
   {
      std::string_view stringRowSchema = itRow.cell_get_variant_view("schema").as_string_view();
      std::string_view stringRowTable = itRow.cell_get_variant_view("table").as_string_view();

      if(stringRowSchema == stringSchema && stringRowTable == stringTable)  return (int64_t)itRow.get_row();
   }
   return -1;
 }

void database::create_table_s( gd::table::arguments::table& tableTable )
{
   // ## Create table with meta information physical tables, this also works as a type of index for database table

   tableTable.set_flags( gd::table::tag_meta{} );   
   tableTable.column_prepare();
   tableTable.column_add( {
      { "uint32",  0, "key"         }, // key (these keys do also represent row number to be fast)
      { "uint32",  0, "first-key"   }, // first row (key of first row) in database table, for speed up searches
      { "rstring", 0, "schema"      }, // table schema
      { "rstring", 0, "table"       }, // table name
      { "rstring", 0, "alias"       }, // alias
      { "rutf8",   0, "description" }  // column description (custom value)
   }, gd::table::tag_type_name{});
   tableTable.prepare();
}


void database::create_column_s( gd::table::arguments::table& tableColumn )
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

_GD_MODULES_DBMETA_END
