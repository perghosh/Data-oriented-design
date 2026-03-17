// @FILE [tag: database, meta] [description: Database description and meta information] [type: source] [name: gd__database.cpp]


#include "gd__database.h"

_GD_MODULES_DBMETA_BEGIN


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