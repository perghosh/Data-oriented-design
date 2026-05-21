#include "gd__expression.h"


_GD_MODULES_DBMETA_BEGIN

/// @API [tag: initialize]
std::pair<bool, std::string> expression::initialize()
{
   // ## Create the necessary tables ..........................................

   m_ptableExpression = std::make_unique<gd::table::arguments::table>();        // create table for table metadata
   create_expression_s(*m_ptableExpression  );

   return { true, "" };
}



/** -------------------------------------------------------------------------- add
 * @brief Adds a new expression to the internal expression table using the provided arguments.
 * @param argumentsTable An arguments object containing the details of the expression to be added. Must include "id" and "expression" keys, and optionally "uuid" and "table-key".
 * @return A pair containing a boolean success indicator and an error message string (empty on success).
 */
std::pair<bool, std::string> expression::add(const gd::argument::arguments& argumentsTable)
{                                                                                                  assert(m_ptableExpression != nullptr); assert(m_ptableExpression->empty() == false);
   std::array<uint8_t, 256> buffer_;
   gd::argument::arguments argumentsRow(buffer_);

   uint32_t uKey = (uint32_t)m_ptableExpression->size() + 1u;
   argumentsRow.append("key", uKey);
   argumentsRow.append(argumentsTable, { "uuid", "table-key", "id", "column", "expression", "description" }); assert(argumentsRow.exists({ "id", "expression" }) == true);
   if(argumentsRow.exists("uuid") == false) argumentsRow.append("uuid", gd::types::uuid_generate_g()); // if table-key is not provided then generate

   m_ptableExpression->row_add(argumentsRow, gd::table::tag_arguments{}, gd::table::tag_convert{});

   return { true, "" };
}

/** -------------------------------------------------------------------------- add
 * @brief Adds rows from a table expression to the internal expression table.
 * @param tableExpression The source table containing rows to be added to the expression table.
 * @return A pair containing a boolean success indicator and an error message string (empty on success).
 */
std::pair<bool, std::string> expression::add(gd::table::dto::table& tableExpression)
{
   std::array<uint8_t, 512> buffer_;
   gd::argument::arguments argumentsRow(buffer_);

   for(auto itRow = tableExpression.row_begin(); itRow != tableExpression.row_end(); itRow++)
   {
      argumentsRow.clear();
      itRow.get_arguments(argumentsRow);

      argumentsRow.append("key", (uint32_t)itRow.get_row());
      m_ptableExpression->row_add(argumentsRow, gd::table::tag_arguments{}, gd::table::tag_convert{});
   }

   return { true, "" };
}

/** -------------------------------------------------------------------------- find
 * @brief Finds the row index of an expression in the internal expression table based on provided search criteria.
 * @param argumentsFind An arguments object containing the search criteria. Valid keys are "schema", "table", and "column". At least "column" must be provided.
 * @return The row index of the found expression, or -1 if no matching expression is found or if invalid keys are provided.
 */
int64_t expression::find(const gd::argument::arguments& argumentsFind) const
{   assert(m_ptableExpression != nullptr); assert(m_ptableExpression->empty() == false);
#ifndef NDEBUG
   std::string stringSearch_d = gd::argument::debug::print(argumentsFind);
   // ## Validate keys
   auto vectorKey = argumentsFind.get_keys();
   for(const auto& key_ : vectorKey)
   {
      if(key_ != "id" && key_ != "uuid" && key_ != "table-key" && key_ != "column")
      {
         assert(false && "Invalid key in argumentsFind, expected keys are 'id', 'uuid', 'table-key', 'column'");
         return -1; // Invalid key found
      }
   }
#endif // NDEBUG

   int64_t iRow = 0;

   std::string_view stringUuid = argumentsFind["uuid"].as_string_view();
   if(stringUuid.empty() == false)
   {                                                                                                assert((stringUuid.size() == 32 || stringUuid.size() == 36) && "Invalid uuid format, expected 32 or 36 character string"); // ## Validate sie of uuid string, it should be either 32 characters (without dashes) or 36 characters (with dashes)
      // ## If uuid is provided, it should be unique, so we can directly find the expression by uuid
      gd::types::uuid uuidValue = gd::types::from_string_g(stringUuid, gd::types::tag_uuid{}); // convert to binary
      iRow = m_ptableExpression->find(eColumnUuid, uuidValue);
      return iRow; // Return immediately since uuid is unique identifier
   }
   else
   {
      iRow = m_ptableExpression->find(argumentsFind);
   }

   return iRow;
}



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
      { "uint32",   0, "type"        }, // expression type
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
      { "uint32",   0, "type" }, // expression type
      { "rutf8",    0, "expression" }, // raw data for expression
      { "rstring",  0, "description" }, // optional description for statement
   });
   table_.prepare();

   return table_;
}

_GD_MODULES_DBMETA_END
