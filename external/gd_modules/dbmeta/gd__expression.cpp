
#include "gd__expression.h"


_GD_MODULES_DBMETA_BEGIN



void expression::create_expression_s(gd::table::arguments::table& tableExpression)
{
   // ## Create table with meta information about expressions, this also works as a type of index for expressions

   tableExpression.set_flags(gd::table::tag_meta{});
   tableExpression.column_prepare();
   tableExpression.column_add({
      { "uint32",   0, "key"         }, // key (these keys do also represent row number to be fast)
      { "uuid",     0, "uuid"        }, // unique identifier for statement
      { "uint32",   0, "table-key"   }, // key to the parent table in `m_ptableTable` (defines which table this expression belongs to) and try som match this with index to be fast to lookup
      { "string",   32,"id"          }, // expression name, this is used to identify expression and also used in query templates to refer to expression
      { "string",   32,"column"      }, // column information
      { "rutf8",    0, "expression"  }, // raw data for expression, this is used to store the actual sql expression template, this can also be used to store other types of expressions if needed{}
      { "rstring",  0, "description" }, // optional description for statement, this can be used to provide additional information about statement, like what it does, or how to use it
      }, gd::table::tag_type_name{});
   tableExpression.prepare();
}

/// @brief create expression dto table for expression structure used to transport expression data.
gd::table::dto::table expression::create_expression_s()
{
   gd::table::dto::table table_((gd::table::dto::table::eTableFlagNull32 | gd::table::dto::table::eTableFlagRowStatus), {
      { "uuid",     0, "uuid" }, // unique identifier for statement
      { "uint32",   0, "table-key" }, // key to the parent table in `m_ptableTable`
      { "rstring",  0, "table" }, //table name if table name 
      { "rstring",  0, "id" }, // expression id
      { "rstring",  0, "column" }, // column information
      { "rutf8",    0, "expression" }, // raw data for expression
      { "rstring",  0, "description" }, // optional description for statement
   });
   table_.prepare();

   return table_;
}

_GD_MODULES_DBMETA_END
