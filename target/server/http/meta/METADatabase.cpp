#include <array>

#include "METADatabase.h"

NAMESPACE_META_BEGIN

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
   }
   catch (const std::exception& e)
   {
      return { false, e.what() };
   }
   
   return { true, "" };
}

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
   }

   return { true, "" };
}

std::pair<bool, std::string> CDatabase::Add( gd::table::dto::table& tableTable, gd::types::tag_column )
{
   std::array<uint8_t, 512> buffer_;
   gd::argument::arguments argumentsRow( buffer_ );

   for( auto itRow = tableTable.row_begin(); itRow != tableTable.row_end(); itRow++ )
   {
      argumentsRow.clear();
      itRow.get_arguments( argumentsRow );

      m_ptableColumn->row_add( argumentsRow, gd::table::tag_arguments{} );
      itRow.cell_set( "key", (uint32_t)itRow.get_row() );
   }

   return { true, "" };
}


void CDatabase::CreateTable_s( gd::table::arguments::table& tableTable )
{
   // ## Create table with meta information physical tables, this also works as a type of index for database table

   tableTable.set_flags( gd::table::tag_meta{} );   
   tableTable.column_prepare();
   tableTable.column_add( {
      { "uint32",  0, "key"         }, // key (these keys do also represent row number to be fast)
      { "uint32",  0, "first_key"   }, // first row (key of first row) in database table
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
      { "uint32",  0, "table_key"    }, // key to the parent table in `m_ptableTable` (defines which table this column belongs to)

      // Name for table and column      
      { "rstring", 0, "schema"      }, // table schema
      { "rstring", 0, "table"       }, // table name
      { "rstring", 0, "column"      }, // column name
      
      // Column properties
      { "uint32",  0, "type"        }, // column type
      { "uint32",  0, "size"        }, // column size
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
      { "uint32",  0, "parent_key"    }, // parent key to table in database description table 
      { "rstring", 0, "parent_alias"  }, // alias for parent table
      { "uint32",  0, "parent_suffix" }, // if number is used to make the name unique

      // Child Side (Where we go)
      { "uint32",  0, "child_key"     }, // child key to table in database description table 
      { "rstring", 0, "child_alias"   }, // alias for child table
      { "uint32",  0, "child_suffix"  }, // if number is used to make the name unique

      // Join Logic
      { "rstring", 0, "join_on"       }, // The logic: string to generate in sql "parent.id = child.parent_id" or "{=parent}.id = {=child}.parent_id"
      { "uint32",  0, "join_type"     }, // Flags to configure what is allowed and default in joinging, like Inner, Left, Right, etc.
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
      { "uint32",  0, "table_key"    }, // key to the parent table in tableTable (defines which table this column belongs to)

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
