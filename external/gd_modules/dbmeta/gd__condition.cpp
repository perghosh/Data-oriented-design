// @FILE [tag: database, sql, meta, condition] [description: Store information about conditions and rules how to use these] [type: header] [name: gd__condition.h]

#include "gd__condition.h"

_GD_MODULES_DBMETA_BEGIN


/// Initializes the `condition` object -------------------------------------- initialize
void condition::initialize()
{                                                                                                  assert( m_ptableCondition == nullptr );
   m_ptableCondition = std::make_unique<gd::table::arguments::table>();
   create_condition_s( *m_ptableCondition );
}


/** ------------------------------------------------------------------------ add
 * @brief Adds one condition row to the condition table.
 *
 * @param stringSetName   Name of the owning condition-set (e.g. "vote")
 * @param stringTable     Database table the condition-set applies to (e.g. "TPoll")
 * @param stringName      Name of this individual condition (e.g. "last_7_days")
 * @param stringField     Field / column the expression filters on (e.g. "CreateD")
 * @param stringExpression SQL expression fragment stored verbatim
 * @param stringDescription Optional human-readable description
 * @return Pair of success flag and generated UUID string (empty string on failure)
 */
std::pair<bool, std::string> condition::add( std::string_view stringSet,
                                             std::string_view stringTable,
                                             std::string_view stringName,
                                             std::string_view stringField,
                                             std::string_view stringExpression,
                                             std::string_view stringDescription )
{                                                                                                  assert( m_ptableCondition != nullptr ); assert( stringSet.empty() == false ); assert( stringName.empty() == false );
   auto uRow = m_ptableCondition->row_add_one();
   m_ptableCondition->cell_set( uRow, eColumnKey,         (uint32_t)uRow       );
   m_ptableCondition->cell_set( uRow, eColumnUuid,        gd::types::uuid_generate_g() );
   m_ptableCondition->cell_set( uRow, eColumnSet,         stringSet            );
   m_ptableCondition->cell_set( uRow, eColumnTable,       stringTable          );
   m_ptableCondition->cell_set( uRow, eColumnName,        stringName           );
   m_ptableCondition->cell_set( uRow, eColumnField,       stringField          );
   m_ptableCondition->cell_set( uRow, eColumnExpression,  stringExpression     );
   if( stringDescription.empty() == false )
      m_ptableCondition->cell_set( uRow, eColumnDescription, stringDescription );

   std::string stringUuid = m_ptableCondition->cell_get_variant_view( uRow, eColumnUuid ).as_string();
   return { true, stringUuid };
}


/** ------------------------------------------------------------------------ add
 * @brief Adds one condition row from a pre-filled `arguments` object.
 *
 * Expected keys: uuid (optional), set_name, table, name, field, expression, description (optional)
 *
 * @param argumentsCondition Arguments carrying the column values
 * @return Pair of success flag and error string (empty on success)
 */
std::pair<bool, std::string> condition::add( const gd::argument::arguments& argumentsCondition )
{                                                                                                  assert( m_ptableCondition != nullptr );
   auto uRow = m_ptableCondition->row_add_one();
   m_ptableCondition->cell_set( uRow, eColumnKey, (uint32_t)uRow );

   // ## UUID — generate if not supplied or invalid

   auto uuid_ = argumentsCondition["uuid"];
   if( uuid_.is_string() == true && uuid_.is_true() )
   {
      auto stringUuid = uuid_.as_string_view();
      if( stringUuid.length() == 36 || stringUuid.length() == 32 )
      {
         gd::types::uuid uuidValue = gd::types::from_string_g( stringUuid, gd::types::tag_uuid{} );
         m_ptableCondition->cell_set( uRow, eColumnUuid, uuidValue );
      }
      else { return { false, "Invalid UUID length: " + std::to_string( stringUuid.length() ) }; }
   }
   else
   {
      m_ptableCondition->cell_set( uRow, eColumnUuid, gd::types::uuid_generate_g() );
   }

   // ## String columns — copy if present

   auto set_string_ = [&]( auto eColumn_, std::string_view stringKey_ )
   {
      auto v_ = argumentsCondition[stringKey_];
      if( v_.is_string() == true ) m_ptableCondition->cell_set( uRow, eColumn_, v_.as_string_view() );
   };

   set_string_( eColumnSet,         "set"         );
   set_string_( eColumnTable,       "table"       );
   set_string_( eColumnName,        "name"        );
   set_string_( eColumnField,       "field"       );
   set_string_( eColumnExpression,  "expression"  );
   set_string_( eColumnDescription, "description" );

   return { true, "" };
}


/// @brief Get UUID for row -------------------------------------------------- get_id
gd::types::uuid condition::get_id( uint64_t uRow ) const
{                                                                                                  assert( m_ptableCondition != nullptr ); assert( uRow < m_ptableCondition->size() );
   gd::types::uuid uuid_ = m_ptableCondition->cell_get_variant_view( uRow, eColumnUuid ).as_binary_view();
   return uuid_;
}

/// @brief Get condition-set name for row ------------------------------------ get_set_name
std::string_view condition::get_set( uint64_t uRow ) const
{                                                                                                  assert( m_ptableCondition != nullptr ); assert( uRow < m_ptableCondition->size() );
   return m_ptableCondition->cell_get_variant_view( uRow, eColumnSet ).as_string_view();
}

/// @brief Get condition name for row ---------------------------------------- get_name
std::string_view condition::get_name( uint64_t uRow ) const
{                                                                                                  assert( m_ptableCondition != nullptr ); assert( uRow < m_ptableCondition->size() );
   return m_ptableCondition->cell_get_variant_view( uRow, eColumnName ).as_string_view();
}

/// @brief Get database table name for row ----------------------------------- get_table
std::string_view condition::get_table( uint64_t uRow ) const
{                                                                                                  assert( m_ptableCondition != nullptr ); assert( uRow < m_ptableCondition->size() );
   return m_ptableCondition->cell_get_variant_view( uRow, eColumnTable ).as_string_view();
}

/// @brief Get field name for row -------------------------------------------- get_field
std::string_view condition::get_field( uint64_t uRow ) const
{                                                                                                  assert( m_ptableCondition != nullptr ); assert( uRow < m_ptableCondition->size() );
   return m_ptableCondition->cell_get_variant_view( uRow, eColumnField ).as_string_view();
}

/// @brief Get SQL expression for row ---------------------------------------- get_expression
std::string_view condition::get_expression( uint64_t uRow ) const
{                                                                                                  assert( m_ptableCondition != nullptr ); assert( uRow < m_ptableCondition->size() );
   return m_ptableCondition->cell_get_variant_view( uRow, eColumnExpression ).as_string_view();
}


/** ------------------------------------------------------------------------- find
 * @brief Find first row whose condition name matches `stringName`.
 * @param stringName Condition name to search for.
 * @return Row index, or -1 if not found.
 */
int64_t condition::find( std::string_view stringName ) const
{                                                                                                  assert( m_ptableCondition != nullptr );
   return m_ptableCondition->find( eColumnName, stringName );
}

/** ------------------------------------------------------------------------- find
 * @brief Find row by UUID.
 * @param puuid Pointer to the UUID to search for.
 * @return Row index, or -1 if not found.
 */
int64_t condition::find( const gd::types::uuid* puuid ) const
{                                                                                                  assert( m_ptableCondition != nullptr ); assert( puuid != nullptr );
   return m_ptableCondition->find( eColumnUuid, *puuid );
}

/** ------------------------------------------------------------------------- find_set
 * @brief Find the first row that belongs to a named condition-set.
 * @param stringSetName Condition-set name to search for.
 * @return Row index of the first matching row, or -1 if not found.
 */
int64_t condition::find_set( std::string_view stringSetName ) const
{                                                                                                  assert( m_ptableCondition != nullptr );
   return m_ptableCondition->find( eColumnSet, stringSetName );
}


/// @brief Mark a row as deleted --------------------------------------------- erase
void condition::erase( uint64_t uRow )
{
   m_ptableCondition->erase( uRow );
}


/// @brief Build the column schema for the condition table ------------------ create_condition_s
void condition::create_condition_s( gd::table::arguments::table& tableCondition )
{
   tableCondition.set_flags( gd::table::tag_meta{} );
   tableCondition.column_prepare();
   tableCondition.column_add( {
      { "uint32",  0, "key"         }, ///< row key — mirrors row index for O(1) access
      { "uuid",    0, "uuid"        }, ///< unique identifier for this condition row
      { "rstring", 0, "set"         }, ///< owning condition-set name (e.g. "vote")
      { "rstring", 0, "table"       }, ///< database table the set applies to (e.g. "TPoll")
      { "rstring", 0, "name"        }, ///< individual condition name (e.g. "last_7_days")
      { "rstring", 0, "field"       }, ///< field / column the expression filters on
      { "rstring", 0, "expression"  }, ///< raw SQL expression fragment
      { "rstring", 0, "description" }, ///< optional human-readable description
   }, gd::table::tag_type_name{} );
   tableCondition.prepare();
}


_GD_MODULES_DBMETA_END