#include "METADatabase.h"

NAMESPACE_META_BEGIN

void CDatabase::CreateTable_s( gd::table::arguments::table& tableDatabase )
{                                                                                                  assert( tableDatabase.empty() == true );
   tableDatabase.set_flags( gd::table::tag_meta{} );
   tableDatabase.column_prepare();
   tableDatabase.column_add( {
      { "uint32",  0, "key"         },
      { "rstring", 0, "schema"      }, 
      { "rstring", 0, "table_name"  }, 
      { "rstring", 0, "column_name" }, 
      { "uint32",  0, "type"        }, 
      { "uint32",  0, "size"        }, 
      { "uint32",  0, "flags"       }, 
      { "uint32",  0, "types"       }, 
      { "uint32",  0, "properties"  }, 
      { "rstring", 0, "alias"       }, 
      { "rstring", 0, "default"     },
      { "rutf8",   0, "description" } }, 
      gd::table::tag_type_name{}
   );
   tableDatabase.prepare();
}

void CDatabase::CreateConnections_s( gd::table::arguments::table& tableConnections )
{                                                                                                  assert( tableConnections.empty() == true );
   gd::table tableConnections;
   tableConnections.set_flags( gd::table::tag_meta{} );
   tableConnections.column_prepare();
   tableConnections.column_add( {
      // Parent Side (Where we start)
      { "rstring", 0, "parent_schema" }, 
      { "rstring", 0, "parent_table"  }, 

      // Connection Definition (The "Edge")
      { "rstring", 0, "child_alias"   }, // Crucial: Identifies this specific link (e.g. "Manager", "BillingAddress")

      // Child Side (Where we go)
      { "rstring", 0, "child_schema"  }, 
      { "rstring", 0, "child_table"   }, 

      // Join Logic
      { "rstring", 0, "join_on"       }, // The logic: "parent.id = child.parent_id"
      { "uint32",  0, "join_type"     }, // Enum: Inner, Left, Right, etc.
      { "uint32",  0, "cardinality"   }, // Enum: 1:1, 1:N, N:M

      // Metadata
      { "rutf8",   0, "description"   } 
   }, 
   gd::table::tag_type_name{}
   );
   tableConnections.prepare();
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