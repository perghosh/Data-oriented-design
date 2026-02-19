
#include "gd_utf8.h"
#include "gd_sql_value.h"

#include "gd_sql_query.h"

/**
* \file gd_sql_query.h
* *Sample how to create SQL using two tables called `TActivity` and `TCustomer`.*
```cpp
{
   gd::sql::query query;      // create query object
   // add two tables
   query.table_add({ {"name","TActivity"}, {"schema","application"}, {"alias","Activity1"} });
   query.table_add({ {"name","TCustomer"}, {"schema","application"}, {"alias","Customer1"}, {"join","Activity1.ActivityK=Customer1.CustomerK"} });
   // add two fields
   query.field_add("Activity1", { {"name", "ActivityK"}, {"alias", "ID"} });
   query.field_add("Customer1", { {"name", "FName"}, {"alias", "CustomerName"} });

   // generate SQL
   auto stringSQL = std::string("SELECT ");
   stringSQL += query.sql_get_select();
   stringSQL += "\nFROM ";
   stringSQL += query.sql_get_from();
   std::cout << stringSQL << std::endl;
}
```
*/

#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wswitch"
   #pragma clang diagnostic ignored "-Wformat"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wswitch"
   #pragma GCC diagnostic ignored "-Wformat"
#elif defined( _MSC_VER )
   #pragma warning( disable : 4996 26812 )
#endif


_GD_SQL_QUERY_BEGIN

unsigned query::m_puPartOrder_s[] = 
{ 
   eSqlPartSelect, 
   eSqlPartInsert, 
   eSqlPartUpdate, 
   eSqlPartDelete, 
   eSqlPartFrom, 
   eSqlPartWhere, 
   eSqlPartGroupBy, 
   eSqlPartHaving,
   eSqlPartOrderBy, 
   eSqlPartLimit,
   0
};

void query::common_construct(const query& o)
{
   m_uNextKey = o.m_uNextKey;
   m_uFormatOptions = o.m_uFormatOptions;
   m_vectorTable = o.m_vectorTable;
   m_vectorField = o.m_vectorField;
   m_vectorCondition = o.m_vectorCondition;
}

void query::common_construct(query&& o) noexcept
{
   m_uNextKey = o.m_uNextKey;
   m_uFormatOptions = o.m_uFormatOptions;
   m_vectorTable = std::move( o.m_vectorTable );
   m_vectorField = std::move( o.m_vectorField );
   m_vectorCondition = std::move( o.m_vectorCondition );
}

/** ---------------------------------------------------------------------------
 * @brief get pointer to table for table id or index
 * 
 * @param variantTableIndex index to table, can be both integer index or id for table
 * @return pointer to table if found otherwise nullptr
*/
const query::table* query::table_get(const gd::variant_view& variantTableIndex) const
{
   if( variantTableIndex.is_null() == true )                                  // if index is null return first table, if there is no table return nullptr
   {
      if( m_vectorTable.empty() == false ) return &m_vectorTable[0];          // get first table, this is the default to simplify working with query when there is only one table, otherwise it is better to use index or name to get table
      else { assert(false); return nullptr; }
   }
   else if( variantTableIndex.is_integer() == true )
   {
      unsigned uIndex = variantTableIndex.get_uint();                                              assert( uIndex < m_vectorTable.size() );
      return &m_vectorTable[uIndex];
   }
   else
   {
      std::string stringTable = variantTableIndex.get_string();
      /// ## Find table matching table name
      for( auto it = std::begin(m_vectorTable); it != std::end(m_vectorTable); it++ )
      {
         if( it->get_arguments().compare({ "name", stringTable }) == true ) return &(*it);
      }

      // ## Find table matching alias
      for( auto it = std::begin(m_vectorTable); it != std::end(m_vectorTable); it++ )
      {
         if( it->get_arguments().compare({ "alias", stringTable }) == true ) return &(*it);
      }
   }

   return nullptr;
}


query::table* query::table_get(const std::pair<std::string_view, gd::variant_view>& pairTable)
{
   for( auto it = std::begin(m_vectorTable); it != std::end(m_vectorTable); it++ )
   {
      if( it->compare(pairTable) == true ) return &(*it);
   }

   return nullptr;
}

const query::table* query::table_get( const table& tableFind ) const noexcept
{
   auto alias_ = tableFind.get_arguments()["alias"];
   auto name_ = tableFind.get_arguments()["name"];

   for( auto it = std::begin(m_vectorTable); it != std::end(m_vectorTable); it++ )
   {
      if( alias_.empty() == false )
      { 
         if( it->get_arguments()["alias"] == alias_ ) return &(*it);
      }
      else
      {
         if( it->get_arguments()["name"] == name_ ) return &(*it);
      }
   }

   return nullptr;
}



/*----------------------------------------------------------------------------- table_add */ /**
 * Adds table with table properties found in vector
 * \param vectorTable vector with pair items where each pair represents one property for table
 * \return table* pointer to added table
 */
query::table* query::table_add(const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorTable)
{
   table tableAdd(next_key());
   for( const auto& it : vectorTable )
   {
      tableAdd.set(it.first, it.second);                                       // set property
   }

   m_vectorTable.push_back( std::move( tableAdd ) );                           // add to table list
   return &m_vectorTable.back();
}


query::table* query::table_add( const table& tableFrom )
{
   table tableAdd(next_key());

   tableAdd.set( tableFrom.get_arguments() );

   m_vectorTable.push_back( std::move( tableAdd ) );

   return &m_vectorTable.back();
}



/*----------------------------------------------------------------------------- field_add */ /**
 * Add field and set field name
 * \param variantTable index to table field belongs to
 * \param stringName name for field
 * \param stringAlias alias for field
 * \return query::field* pointer to added field
 */
query::field* query::field_add(const gd::variant_view& variantTable, std::string_view stringName, std::string_view stringAlias) 
{                                                                                                  assert( m_vectorTable.empty() == false );
   auto ptable = table_get(variantTable);                                                          assert(ptable != nullptr);

   m_vectorField.push_back(field(*ptable, stringName));

   auto pfield = &m_vectorField.back();
   if( stringAlias.empty() == false ) pfield->append("alias", stringAlias);

   return pfield;
}

/*----------------------------------------------------------------------------- field_add */ /**
 * Add field to query
 * \param variantTable index to table field belongs to
 * \param vectorField vector with field properties, each property has a name and a value
 * \return gd::sql::query::field* pointer to added field
 */
query::field* query::field_add(const gd::variant_view& variantTable, const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorField)
{
   auto ptable = table_get(variantTable);                                                          assert(ptable != nullptr);

   field fieldAdd( *ptable );                                                  // create field object that is added to query

   for( auto it : vectorField )
   {  // append values for field added to query
      fieldAdd.append_argument(it.first, it.second);                           // first = name, second = value
   }

   m_vectorField.push_back(std::move(fieldAdd));                               // add to list with fields
   return &m_vectorField.back();                                               // return pointer to added field
}

/** ---------------------------------------------------------------------------
 * @brief add field used in order by part of sql query
 * @param variantTable table field belongs to
 * @param variantviewField column index, column name or expression used to order result from query
 * @return query::field* pointer to added field
*/
query::field* query::field_add_as_orderby( const gd::variant_view& variantTable, const gd::variant_view& variantviewField )
{
   auto ptable = table_get(variantTable);                                                          assert(ptable != nullptr);

   field fieldAdd( *ptable, eSqlPartOrderBy );                                 // create field object that is added to query

   fieldAdd.append_argument("index", variantviewField);                        // add order index

   m_vectorField.push_back(std::move(fieldAdd));                               // add to list with fields
   return &m_vectorField.back();                                               // return pointer to added field
}


/*----------------------------------------------------------------------------- condition_add */ /**
 * Add condition to query, adds condition to first table (you need to add table before calling this method)
 * \param stringName condition field name
 * \param variantOperator condition operator
 * \param variantValue condition value
 * \return condition* pointer to added condition
 */
gd::sql::query::condition* query::condition_add(std::string_view stringName, const gd::variant_view& variantOperator, const gd::variant_view& variantValue)
{                                                                                                  assert( m_vectorTable.empty() == false );
   auto ptable = table_get();                                                                      assert(ptable != nullptr);
   return condition_add_(ptable, stringName, variantOperator, variantValue);
}


/*----------------------------------------------------------------------------- condition_add */ /**
 * Add condition to query
 * \param variantTable index to table condition belongs to
 * \param stringName condition field name
 * \param variantOperator condition operator
 * \param variantValue condition value
 * \return condition* pointer to added condition
 */
gd::sql::query::condition* query::condition_add(const gd::variant_view& variantTable, std::string_view stringName, const gd::variant_view& variantOperator, const gd::variant_view& variantValue)
{
   auto ptable = table_get(variantTable);                                                          assert(ptable != nullptr);
   return condition_add_(ptable, stringName, variantOperator, variantValue);
}

/*----------------------------------------------------------------------------- condition_add */ /**
 * Add condition to query
 * \param argumentsCondition vector with condition properties, each property has a name and a value, vector must contain "table" property with index to table condition belongs to
 * \return condition* pointer to added condition
 */
gd::sql::query::condition* query::condition_add( const gd::argument::arguments& argumentsCondition, tag_arguments )
{   
   auto ptable = table_get( argumentsCondition["table"].as_variant_view() );                       assert(ptable != nullptr);
   condition conditionAdd( *ptable ); // create condition object that is added to query
   for( const auto& it : argumentsCondition.named() )
   {  // append values for field added to query
      std::string_view stringName = it.first;                                                      assert( stringName.empty() == false );

      if( stringName[0] == 'o' && stringName == "operator" )
      {
         enumOperator eOperator = get_where_operator_number_s(it.second);                          assert( eOperator != eOperatorError ); // note that operator should be checked before calling this method
         conditionAdd.append("operator", eOperator);
      }
      else 
      { 
         conditionAdd.append_argument(it.first, it.second); 
      }             
   }
   m_vectorCondition.push_back(std::move(conditionAdd));                      // add condition to list
   return &m_vectorCondition.back();                                          // return pointer to added condition}
}

/*----------------------------------------------------------------------------- condition_add */ /**
 * Add condition to query
 * \param ptable pointer to table condition belongs to
 * \param stringName condition field name
 * \param variantOperator condition operator
 * \param variantValue condition value
 * \return condition* pointer to added condition
 */
gd::sql::query::condition* query::condition_add_(const table* ptable, std::string_view stringName, const gd::variant_view& variantOperator, const gd::variant_view& variantValue)
{                                                                                                  assert( ptable != nullptr );
   condition conditionAdd(*ptable);                                            // create condition object that is added to query
   conditionAdd.append("name", stringName);

   enumOperator eOperator = eOperatorEqual;                                    // default is = (equal)
   if( variantOperator.is_null() == false ) { eOperator = get_where_operator_number_s( variantOperator ); assert( eOperator != eOperatorError ); } 
   conditionAdd.append("operator", eOperator);
   if( variantValue.is_null() == false ) conditionAdd.append_argument("value", variantValue);

   m_vectorCondition.push_back(std::move(conditionAdd));                       // add to list with fields
   return &m_vectorCondition.back();                                           // return pointer to added condition
}

/*----------------------------------------------------------------------------- condition_add */ /**
 * @brief 
 * @param variantTable 
 * @param stringCondition 
 * @return 
*/
query::condition* query::condition_add_raw( const gd::variant_view& variantTable, const std::string_view& stringCondition )
{
   auto ptable = table_get(variantTable);                                                          assert(ptable != nullptr);

   condition conditionAdd( *ptable );                                          // create condition object that is added to query
   conditionAdd.append_argument("raw", stringCondition );

   m_vectorCondition.push_back(std::move(conditionAdd));                       // add condition to list
   return &m_vectorCondition.back();                                           // return pointer to added condition
}


/*----------------------------------------------------------------------------- condition_add */ /**
 * Add condition to query
 * \param variantTable index to table field belongs to
 * \param vectorField vector with field properties, each property has a name and a value
 * \return gd::sql::query::field* pointer to added field
 */
query::condition* query::condition_add(const gd::variant_view& variantTable, const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorCondition)
{
   auto ptable = table_get(variantTable);                                                          assert(ptable != nullptr);

   condition conditionAdd( *ptable );                                          // create condition object that is added to query

   for( auto it : vectorCondition )
   {  // append values for field added to query
      if( it.first[0] == 'o' && it.first == "operator" )
      {
         enumOperator eOperator = get_where_operator_number_s(it.second);                          assert( eOperator != eOperatorError );
         conditionAdd.append("operator", eOperator);
      }
      else { conditionAdd.append_argument(it.first, it.second); }              // first = name, second = value
   }

   m_vectorCondition.push_back(std::move(conditionAdd));                       // add condition to list
   return &m_vectorCondition.back();                                           // return pointer to added condition
}



/*----------------------------------------------------------------------------- field_get */ /**
 * return field based on property value for field
 * \param pairField name and value that are matched for field searched for
 * \return field* pointer to field if found, nullptr if not found
 */
query::field* query::field_get( const std::pair<std::string_view, gd::variant_view>& pairField )
{
   for( auto it = std::begin(m_vectorField); it != std::end(m_vectorField); it++ )
   {
      if( it->compare(pairField) == true ) return &(*it);                        // compare if field matches named property and property value, if match return pointer to field
   }

   return nullptr;
}


query& query::add( const query& queryFrom )
{
   auto pairResult = add_s( *this, queryFrom );

   return *this;
}



std::string query::sql_get_join_for_table( const table* ptable, const table* ptableParent ) const
{
   std::string stringJoin;

   if( ptable->has("alias") == true ) { stringJoin += ptable->alias(); }
   else                               { stringJoin += ptable->name(); }

   stringJoin += '.';

   if( ptableParent == nullptr )
   {
      if( ptable->has( "key" ) ) { stringJoin += ptable->key(); }
      else if( ptable->has( "fk" ) ) { stringJoin += ptable->fk(); }
      else { assert(false); }
   }
   else
   {
      if( ptable->has( "fk" ) ) { stringJoin += ptable->fk(); }                // foreign key set in current table
      else if( ptableParent->has( "key" ) ) { stringJoin += ptableParent->key(); }  // primary key to parent table

   }

   return stringJoin;
}

/*----------------------------------------------------------------------------- sql_get_select */ /**
 * Build `SELECT` text for fields in query
 * \return std::string text for fields selected in query
 */
std::string query::sql_get_select() const
{                                                                                                  assert( m_vectorField.empty() == false );
   std::string stringSelect; // generated select string with fields used in query
   stringSelect.reserve(24);

   unsigned uFormatOptions = m_uFormatOptions;                                 // default format options

   for( auto it = field_begin(); it != field_end(); it++ )
   {
      if( it->is_select() == false ) [[unlikely]] continue;                    // no select field ?

      if( stringSelect.empty() == false ) stringSelect += ", ";

      if( it->has("raw") == true )
      {
         stringSelect += it->raw();
      }
      else
      {
         const table* ptable = table_get_for_key(it->get_table_key());                             assert(ptable != nullptr); // no table found for key indicates internal error for query object, this shouldn't happen
         if( ptable->has("alias") == true )
         {
            stringSelect += ptable->alias();
            stringSelect += ".";
         }

         stringSelect += it->name();
         if( it->has("alias") == true )                                        // found alias ?
         {
            stringSelect += " ";
            if( flag_has_s(uFormatOptions, eFormatUseQuotes) == false ) stringSelect += it->alias();
            else format_add_and_surround_s(stringSelect, it->alias(), '\"');

         }
      }
   }
   return stringSelect;
}


/*----------------------------------------------------------------------------- sql_get_from */ /**
 * Build "FROM" text for tables in query
 * \return std::string Generated FROM part
 */
std::string query::sql_get_from() const
{                                                                                                  assert( m_vectorTable.empty() == false ); // don't call this if no tables added to query
   std::string stringFrom; // generated from string with tables used in query

   // ## Add table to to query, builds information from schema, name and alias if found
   //    *sample* schema = application, name = TCustomer, alias = Customer1 -> applicaton.TCustomer Customer1
   auto fAddTableName = [](const table* ptable, std::string& stringFrom) -> void {
      if( ptable->has("schema") == true )
      {
         stringFrom += ptable->schema();
         stringFrom += ".";
      }

      stringFrom += ptable->name();
      if( ptable->has("alias") == true )                                       // found alias ?
      {
         stringFrom += " ";
         stringFrom += ptable->alias();
      }
   };

   enumJoin eJoinDefault = eJoinInner;                                         // default join if join isn't, specified for table
   unsigned uTableIndex = 0;                                                   // active index for current table processed
   //std::string stringJoin;                                                     // used to build join text for table
   for( auto itTable = std::begin(m_vectorTable), itEnd = std::end(m_vectorTable); itTable != itEnd; itTable++ )
   {
      if( uTableIndex != 0 )                                                   // first table is allways the root table (no join)
      {
         stringFrom += "\n";                                                   // new line to simplfy reading

         enumJoin eJoin = eJoinDefault;
         stringFrom += sql_get_join_text_s(eJoin);                             // get join text for active table 
      }

      fAddTableName( &(*itTable), stringFrom);                                 // add table to string

      if( uTableIndex != 0 )
      {
         stringFrom += " ON ";
         if( itTable->has( "join" ) == true )
         {
            stringFrom += itTable->join();                                     // get join part for table, here it is stored in data for table
         }
         else
         {
            std::string stringParent = itTable->parent();                      assert( stringParent.empty() == false );
            stringFrom += sql_get_join_for_table( table_get( stringParent ) );
            stringFrom += std::string_view{ " = " };
            stringFrom += sql_get_join_for_table( &(*itTable), stringParent );
         }
      }

      uTableIndex++;
   }

   return stringFrom;
}

/*----------------------------------------------------------------------------- sql_get_where */ /**
 * Build "WHERE" text from conditions added to query
 * \return std::string
 */
std::string query::sql_get_where() const
{
   // If there are multiple "equal"- or "not equal" values for same field, these values will be
   // combined to `N` or `NOT IN` SQL operator.
   std::vector<bool> vectorSkipCondition(m_vectorCondition.size());
   char pbBuffer[16];       // buffer used for adding where operator and start or end parenthesis
   std::string stringWhere; // generated where string that gets condition values used to filter query

   unsigned uConditionIndex = 0;
   for( auto itCondition = std::begin(m_vectorCondition); itCondition != std::end(m_vectorCondition); itCondition++ )
   {
      if( vectorSkipCondition[uConditionIndex] == true ) { uConditionIndex++; continue; } // if condition has been processed then skip, this may happen when = or <> are combined

      if( uConditionIndex > 0 )
      {
         stringWhere += std::string_view{ " AND " };
      }

      if( itCondition->has("raw") == true )
      {
         stringWhere += itCondition->raw();
      }
      else
      {
         const table* ptable = table_get_for_key(itCondition->get_table_key());                    assert(ptable != nullptr); // no table found for key indicates internal error for query object, this shouldn't happen
         if( ptable->has("alias") == true )
         {
            stringWhere += ptable->alias();
            stringWhere += ".";
         }

         stringWhere += itCondition->name();

         enumOperatorTypeNumber eOperator = static_cast<enumOperatorTypeNumber>(itCondition->get_operator() & eOperatorMaskNumber);

         // ## Special case for equal or not equal. Looks for all values that has the equal 
         //    operator or not equal operator and groups them into IN or NOT IN.
         //    Values found is marked and will be skipped when it is their time to 
         //    be processed.
         if( eOperator == eOperatorTypeNumberEqual || eOperator == eOperatorTypeNumberNotEqual )
         {
            auto vectorIndex = condition_find_all_for_operator_s(m_vectorCondition, &(*itCondition), uConditionIndex + 1);
            if( vectorIndex.empty() != true )
            {
               if( eOperator == eOperatorTypeNumberEqual ) stringWhere += std::string_view{ " IN(" };
               if( eOperator == eOperatorTypeNumberNotEqual ) stringWhere += std::string_view{ " NOT IN(" };

               std::vector<const condition*> vectorCondition; 
               vectorCondition.reserve(vectorIndex.size() + 1);
               vectorCondition.push_back(&(*itCondition));
               for( auto it : vectorIndex ) { vectorCondition.push_back(&m_vectorCondition[it]); }

               print_condition_values_s( vectorCondition, stringWhere );       // print condition values that is added to where text
               
               stringWhere += ')';
               
               for( auto it : vectorIndex ) { vectorSkipCondition[it] = true; }// mark conditions as processed, this will disable condition next when activated

               uConditionIndex++;                                              // increase index for current condition
               continue; // go to loop and continue with next condition
            }
            
         }

         // string gets pointer to buffer, buffer is filled in method get_where_operator_text_s and buffer length is returned
         auto uOperator = itCondition->get_operator() & eOperatorMaskNumber;
         std::string_view stringOperator(pbBuffer, get_where_operator_text_s(uOperator, pbBuffer));
         stringWhere += stringOperator;                                        // add operator text
         if( uOperator != eOperatorTypeNumberNull && uOperator != eOperatorTypeNumberNotNull )
         {
            value_get_s( itCondition->value(), stringWhere );   // print where value to string
         }
      }

      vectorSkipCondition[uConditionIndex] = true;
      uConditionIndex++;
   }

   return stringWhere;
}


/** ---------------------------------------------------------------------------
 * @brief Generate sql INSERT query part without values
 * @return std::string sql insert string
*/
std::string query::sql_get_insert() const
{                                                                                                  assert( m_vectorTable.empty() == false );
   std::string stringInsert; // generated insert string with tables used in query

   //unsigned uTableIndex = 0;                                                     // active index for current table processed
   for( auto itTable = std::begin(m_vectorTable), itEnd = std::end(m_vectorTable); itTable != itEnd; itTable++ )
   {
      const auto ptable = &(*itTable);
      if( ptable->has( "schema" ) == true )
      {
         stringInsert += ptable->schema();
         stringInsert += ".";
         stringInsert += ptable->name();
      }
      else
      {
         stringInsert += ptable->name();
      }

      // ## Add fields for table
      unsigned uFieldIndex = 0;
      for( auto itField = std::begin(m_vectorField), itEndField = std::end(m_vectorField); itField != itEndField; itField++ )
      {
         if( itField->compare(ptable) == true )
         {
            if( uFieldIndex == 0 ) stringInsert += " (";
            else if( uFieldIndex > 0 ) stringInsert += ", ";
            stringInsert += itField->name();
            uFieldIndex++;
         }
      }

      if( uFieldIndex > 0 ) stringInsert += ")";
   }

   return stringInsert;
}

/**
 * @brief Generates the table and SET clause portion of an SQL UPDATE statement.
 * @return A string containing the table name (with optional schema prefix) and SET clause with field assignments for an UPDATE statement.
 */
std::string query::sql_get_update() const
{
   std::string stringUpdate;

   const auto ptable = &(*std::begin(m_vectorTable));
   if( ptable->has( "schema" ) == true )
   {
      stringUpdate += ptable->schema();
      stringUpdate += ".";
      stringUpdate += ptable->name();
   }
   else
   {
      stringUpdate += ptable->name();
   }

   stringUpdate += "\nSET ";

   // ## Add all set fields
   unsigned uFieldIndex = 0;
   for( auto itField = std::begin(m_vectorField), itEndField = std::end(m_vectorField); itField != itEndField; itField++ )
   {
      if( uFieldIndex > 0 ) stringUpdate += ", ";
      stringUpdate += itField->name();
      stringUpdate += std::string_view{ " = " };
      auto uType = itField->type();
      auto value_ = itField->value();
      if( uType == 0 ) { value_get_s( value_, stringUpdate); }
      else
      {
         if( value_.is_char_string() == true )
         {
            append_g( value_, uType, m_eSqlDialect, stringUpdate );
         }
         else
         {
            value_get_s( value_, stringUpdate );
         }
      }
      uFieldIndex++;
   }

   return stringUpdate;
}

/** ---------------------------------------------------------------------------
 * @brief Build SQL UPDATE set part using all added fields and values sent
 * @note
 * @param vectorValue values assigned to fields added to query
 * @return std::string sql update section without sql key words
*/
std::string query::sql_get_update( const std::vector<gd::variant_view>& vectorValue ) const
{                                                                                                  assert( m_vectorTable.empty() == false ); assert( m_vectorField.size() <= vectorValue.size() );
   std::string stringUpdate;

   const auto ptable = &(*std::begin(m_vectorTable));
   if( ptable->has( "schema" ) == true )
   {
      stringUpdate += ptable->schema();
      stringUpdate += ".";
      stringUpdate += ptable->name();
   }
   else
   {
      stringUpdate += ptable->name();
   }

   stringUpdate += "\nSET ";

   // ## Add all set fields
   unsigned uFieldIndex = 0;
   for( auto itField = std::begin(m_vectorField), itEndField = std::end(m_vectorField); itField != itEndField; itField++ )
   {
      if( uFieldIndex > 0 ) stringUpdate += ", ";
      stringUpdate += itField->name();
      stringUpdate += std::string_view{ " = " };
      auto uType = itField->type();
      if( uType == 0 ) { value_get_s( vectorValue[uFieldIndex], stringUpdate); }
      else
      {
         auto value_ = vectorValue[uFieldIndex];
         if( value_.is_char_string() == true )
         {
            append_g( value_, uType, m_eSqlDialect, stringUpdate );
         }
         else
         {
            value_get_s( vectorValue[uFieldIndex], stringUpdate );
         }
      }
      uFieldIndex++;
   }

   return stringUpdate;
}

std::string query::sql_get_delete() const
{
   std::string stringDelete; // generated delete string 

   const auto ptable = &(*std::begin(m_vectorTable));
   if( ptable->has( "schema" ) == true )
   {
      stringDelete += ptable->schema();
      stringDelete += ".";
      stringDelete += ptable->name();
   }
   else
   {
      stringDelete += ptable->name();
   }

   return stringDelete;
}

std::string query::sql_get_groupby() const
{
   std::string stringGroupBy; // generated group by string 

   return stringGroupBy;
}

/**
 * @brief Build sql order part used in select query
 * @return string with sql formated to order columns in query
*/
std::string query::sql_get_orderby() const
{
   std::string stringOrderBy; // generated order by string 

   for( auto it = field_begin(); it != field_end(); it++ )
   {
      if( it->is_orderby() == false ) [[likely]] continue;                     // no order by field ?

      if( stringOrderBy.empty() == false ) { stringOrderBy += std::string_view{ ", " }; }

      if( it->has( "index" ) == true )                                         // found index number for column
      {
         stringOrderBy += (*it)["index"].as_string();

         if( (*it)["desc"].is_true() == true ) stringOrderBy += std::string_view{ " DESC" };
      }
   }

   return stringOrderBy;
}

std::string query::sql_get_limit() const
{
   std::string stringLimit; // string with limit information

   return stringLimit;
}

std::string query::sql_get_with() const
{
   std::string stringWith; // with section

   return stringWith;
}

std::string query::sql_get_returning() const
{
   std::string stringReturning; // returning section
   return stringReturning;
}


std::string query::sql_get(enumSql eSql) const
{
   return sql_get(eSql, m_puPartOrder_s);
}

std::string query::sql_get(enumSql eSql, const unsigned* puPartOrder) const
{
   unsigned uSql = eSql;
   std::string stringSql;
   while( *puPartOrder != 0 )
   {
      unsigned uSqlPart = *puPartOrder & uSql;
      puPartOrder++;
      if( uSqlPart == 0 ) continue;
      switch( uSqlPart )
      {
      case eSqlPartSelect:
         stringSql += std::string_view{ "SELECT\n\t" };
         stringSql += sql_get_select();
         break;

      case eSqlPartInsert:
         stringSql += std::string_view{ "INSERT INTO\n\t" };
         stringSql += sql_get_insert();
         break;

      case eSqlPartUpdate:
         stringSql += std::string_view{ "UPDATE " };
         stringSql += sql_get_update();
         break;

      case eSqlPartDelete:
         stringSql += std::string_view{ "DELETE " };
         //stringSql += sql_get_delete();
         break;

      case eSqlPartFrom:
         stringSql += std::string_view{ " FROM " };
         stringSql += sql_get_from();
         break;

      case eSqlPartWhere:
         {
         if( m_vectorCondition.empty() == true ) continue;
         stringSql += std::string_view{ "\nWHERE\n\t" };
         stringSql += sql_get_where();
         }
         break;

      case eSqlPartGroupBy:
         break;

      case eSqlPartHaving:
         break;

      case eSqlPartOrderBy:
         break;

      case eSqlPartLimit:
         stringSql += sql_get_limit();
         break;

      default:
                                                                                                   assert( false );
      }
   }

   return stringSql;
}



/** --------------------------------------------------------------------------
 * @brief Clears all tables, fields, and conditions from the query.
 */
void query::clear()
{
   m_vectorTable.clear();
   m_vectorField.clear();
   m_vectorCondition.clear();
   m_argumentsAttribute.clear();
   m_uNextKey = 0;
}



/*----------------------------------------------------------------------------- sql_get_join_type_s */ /**
 * Get join type (constant value for join) for text
 * \param stringJoin join text join type is returned for
 * \return gd::sql::enumJoin type of join that join text is
 */
gd::sql::enumJoin query::get_join_type_s(const std::string_view& stringJoin)
{
   auto pbszJoin = stringJoin.data();
   while( *pbszJoin <= 32 && *pbszJoin != '\0' ) pbszJoin++;
   switch( *pbszJoin )
   {
   case 'i' :
   case 'I' :
   case 'j' :
   case 'J' :
      return eJoinInner;
   case 'l':
   case 'L':
      return eJoinLeft;
   case 'r':
   case 'R':
      return eJoinRight;
   case 'f':
   case 'F':
      return eJoinFull;
   }

   return eJoinUnknown;
}

/*----------------------------------------------------------------------------- sql_get_join_text_s */ /**
 * Return join string for join type
 * \param eJoinType type of join text is returned for
 * \return std::string_view string with join text
 */
std::string_view query::sql_get_join_text_s(enumJoin eJoinType)
{
   switch( eJoinType )
   {
   case eJoinInner: return std::string_view( " INNER JOIN " );
   case eJoinLeft:  return std::string_view( " LEFT JOIN " );
   case eJoinRight: return std::string_view( " RIGHT JOIN " );
   case eJoinFull:  return std::string_view( " FULL JOIN " );
   }
                                                                                                   assert(false);
   return std::string_view();
}

/*----------------------------------------------------------------------------- get_where_operator_number_s */ /**
 * get where operator number
 * This method tries to figure out the operator type for operator sent as text, id doesn't
 * validate operator text, and might return operator number for text that is misspelled, 
 * so it is recommended to validate operator text before calling this method.
 * \param stringOperator
 * \return gd::sql::enumOperator
 */
enumOperator query::get_where_operator_number_s(std::string_view stringOperator)
{                                                                                                  assert( stringOperator.empty() == false );
   auto pbszOperator = stringOperator.data();
   switch( *pbszOperator )
   {
   case '=': {
      if( stringOperator.length() == 1 ) return eOperatorEqual;
      else if( *( pbszOperator + 1 ) == '*' ) return eOperatorLikeBegin;
      return eOperatorEqual;
   }
   break;
   case '!': return eOperatorNotEqual;
   case '*': {
      if( stringOperator.length() == 1 ) return eOperatorLike;
      else if( *( pbszOperator + 1 ) == '=' ) return eOperatorLikeEnd;
      return eOperatorLike;
   }
   break;
   case '<': { 
      if( stringOperator.length() == 1 ) return eOperatorLess;
      else if( *(pbszOperator + 1) == '=' ) return eOperatorLessEqual;
      else if( *(pbszOperator + 1) == '>' ) return eOperatorNotEqual;
   }
   break;
   case '>': {
      if( stringOperator.length() == 1 ) return eOperatorGreater;
      else if( *(pbszOperator + 1) == '=' ) return eOperatorGreaterEqual;
   }
   break;
   case 'e': return eOperatorEqual;
   case 'g': {
      if( stringOperator.length() > sizeof("greater") ) return eOperatorGreaterEqual;
      return eOperatorGreater;
   }
   break;
   case 'i': return eOperatorIn;
   case 'l': {
      if( stringOperator[1] == 'e' && stringOperator.length() > sizeof("less") ) return eOperatorLessEqual;
      else if( stringOperator[1] == 'i' )
      {
         if( stringOperator.length() == (sizeof("like") - 1) ) return eOperatorLike;
         else if( stringOperator.length() == (sizeof("likebegin") - 1) ) return eOperatorLikeBegin;
         else if( stringOperator.length() == (sizeof("likeend") - 1) ) return eOperatorLikeEnd;
      }
      return eOperatorLess;
   }
   break;
   case 'n': {
      if( stringOperator.length() == (sizeof("notequal") - 1) ) return eOperatorNotEqual; // !=
      else if( stringOperator.length() == (sizeof("null") - 1)) return eOperatorNull; // IS NULL
      else if( stringOperator.length() == (sizeof("notnull") - 1) ) return eOperatorNotNull; // IS NOT NULL
      else if( stringOperator.length() == (sizeof("notin") - 1) ) return eOperatorNotIn; // NOT IN
   }
   break;

   }
   return eOperatorError;
}

enumOperator query::get_where_operator_number_s(const gd::variant_view& variantOperator)
{
   enumOperator eOperator;
   if( variantOperator.is_number() ) eOperator = static_cast<enumOperator>(variantOperator.get_uint());
   else if( variantOperator.is_char_string() ) eOperator = get_where_operator_number_s(std::string_view( variantOperator.c_str() ));
   else { eOperator = get_where_operator_number_s(std::string_view(variantOperator.get_string())); }

   return eOperator;
}

namespace {
std::string_view set_text(char* pbBuffer, const std::string_view& stringAdd)
{
   memcpy( pbBuffer, stringAdd.data(), stringAdd.length() + 1 );
   return std::string_view(pbBuffer, stringAdd.length());
}
}

unsigned query::get_where_operator_text_s(unsigned uOperator, char* pbBuffer)
{                                                                                                  assert( uOperator < eOperatorTypeNumberEND );
   std::string_view stringWhere;
   switch( uOperator )
   {
   case eOperatorTypeNumberEqual: stringWhere = set_text(pbBuffer, std::string_view{ " = " }); break;
   case eOperatorTypeNumberNotEqual: stringWhere = set_text(pbBuffer, std::string_view{ " <> " }); break;
   case eOperatorTypeNumberLess: stringWhere = set_text(pbBuffer, std::string_view{ " < " }); break;
   case eOperatorTypeNumberLessEqual: stringWhere = set_text(pbBuffer, std::string_view{ " <= " }); break;
   case eOperatorTypeNumberGreater: stringWhere = set_text(pbBuffer, std::string_view{ " > " }); break;
   case eOperatorTypeNumberGreaterEqual: stringWhere = set_text(pbBuffer, std::string_view{ " >= " }); break;
   case eOperatorTypeNumberLike: stringWhere = set_text(pbBuffer, std::string_view{ " LIKE " }); break;
   case eOperatorTypeNumberLikeBegin: stringWhere = set_text(pbBuffer, std::string_view{ " LIKE " }); break;
   case eOperatorTypeNumberLikeEnd: stringWhere = set_text(pbBuffer, std::string_view{ " LIKE " }); break;
   case eOperatorTypeNumberNull: stringWhere = set_text(pbBuffer, std::string_view{ " IS NULL " }); break;
   case eOperatorTypeNumberNotNull: stringWhere = set_text(pbBuffer, std::string_view{ " IS NOT NULL " }); break;
   case eOperatorTypeNumberIn: stringWhere = set_text(pbBuffer, std::string_view{ " IN " }); break;
   case eOperatorTypeNumberNotIn: stringWhere = set_text(pbBuffer, std::string_view{ " NOT IN " }); break;

   default:
      assert(false); *pbBuffer = '\0';
      break;
   }
   
   return (unsigned)stringWhere.length();
}

/*----------------------------------------------------------------------------- condition_find_all_for_operator_s */ /**
 * 
 * \param vectorCondtion
 * \param pconditionMatch
 * \param uBegin
 * \return std::vector<std::size_t>
 */
std::vector<std::size_t> query::condition_find_all_for_operator_s(const std::vector<condition>& vectorCondtion, const condition* pconditionMatch, unsigned uBegin)
{
   std::vector<std::size_t> vectorCondition;
   for( auto it = std::begin(vectorCondtion) + uBegin; it != std::end(vectorCondtion); it++ )
   {
      if( pconditionMatch->get_table_key() == it->get_table_key() &&
          pconditionMatch->get_operator() == it->get_operator() &&
          pconditionMatch->compare(std::string_view{ "name" }, &(*it)) == true )
      {
         vectorCondition.push_back( std::distance( std::begin(vectorCondtion), it ) );
      }
   }

   return vectorCondition;
}

void query::print_condition_values_s( const std::vector<const condition*>& vectorCondition, std::string& stringValues )
{
   unsigned uCount = 0;
   for( auto it : vectorCondition )
   {
      if( uCount > 0 ) stringValues += std::string_view{ ", " };
      auto value_ = it->value();
      value_get_s( value_, stringValues);
      uCount++;
   }
}

/** ---------------------------------------------------------------------------
 * @brief Variant value in is converted to string in a format that works for sql 
 * @param variantviewValue value converted to string
 * @return true and values as string if ok, false and error information if fail
*/
std::pair<bool, std::string> query::value_get_s( gd::variant_view& variantviewValue )
{
   std::string stringValue;

   if( variantviewValue.is_string() == true )
   {
      stringValue += '\'';
      append_g( variantviewValue, stringValue );
      stringValue += '\'';
   }
   else if( variantviewValue.is_binary() == true )
   {
      stringValue += std::string_view{ "0x" };
      append_g( variantviewValue, stringValue );
   }
   else
   {
      append_g( variantviewValue, stringValue );
   }

   return { true, std::move( stringValue ) };
}

/** ---------------------------------------------------------------------------
 * @brief Variant value in is converted and added to sql string in a format that works for sql 
 * @param variantviewValue value converted to string
 * @param stringValue string values are added to
*/
void query::value_get_s( const gd::variant_view& variantviewValue, std::string& stringSql )
{
   if( variantviewValue.is_string() == true )
   {
      append_g( variantviewValue, stringSql );
   }
   else if( variantviewValue.is_binary() == true )
   {
      stringSql += std::string_view{ "0x" };
      append_g( variantviewValue, stringSql );
   }
   else
   {
      append_g( variantviewValue, stringSql );
   }
}

/** ---------------------------------------------------------------------------
 * @brief Variant Values in vector is converted to string in a format that works for sql 
 * @param vectorValue values converted to string
 * @return true and values as string if ok, false and error information if fail
*/
std::pair<bool, std::string> query::values_get_s( const std::vector< gd::variant_view >& vectorValue )
{
   std::string stringValues;

   for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it != itEnd; it++ )
   {
      if( stringValues.empty() == false ) { stringValues += ','; }

      if( it->is_string() == true )
      {
         stringValues += '\'';
         append_g( *it, stringValues, tag_raw{} );
         stringValues += '\'';
      }
      else if( it->is_binary() == true )
      {
         stringValues += std::string_view{ "0x" };
         append_g( *it, stringValues );
      }
      else
      {
         append_g( *it, stringValues );
      }
   }

   return { true, std::move( stringValues ) };
}

std::pair<bool, std::string> query::values_get_s( std::vector< std::pair<uint32_t, gd::variant_view> >& vectorValue, unsigned uDialect )
{
   std::string stringValues;
   stringValues.reserve( vectorValue.size() * 16 );                          // preallocate to avoid smaller allocations, this is just one estimate

   for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it != itEnd; it++ )
   {
      if( stringValues.empty() == false ) { stringValues += ','; }
      unsigned uType = it->first;
      if( uType == 0 )                                                        // default is string
      {
         append_g( it->second, stringValues );
      }
      else
      {
         if( it->second.is_char_string() == true )
         {
            append_g( it->second, uType, uDialect, stringValues );
         }
         else if( it->second.is_integer() == true )
         {
            append_g( it->second, uType, uDialect, stringValues );
         }
      }
   }

   return { true, std::move( stringValues ) };
}

/** --------------------------------------------------------------------------
 * @brief Generates a RETURNING or OUTPUT clause string for SQL queries based on the specified SQL dialect.
 * @param vectorValue A vector of column name pairs, where the first element is the column name and the second element is an optional alias.
 * @param stringReturning The output string that will contain the generated RETURNING or OUTPUT clause.
 * @param uDialect The SQL dialect identifier that determines whether to use OUTPUT (SQL Server) or RETURNING (default) syntax.
 */
void query::returning_get_s( const gd::borrow::vector< std::pair< std::string_view, std::string_view > >& vectorValue, std::string& stringReturning, unsigned uDialect )
{
   auto add_returning = []( const auto& vectorValue, std::string& stringReturning ) -> void {
      unsigned uCount = 0;
      for( auto it : vectorValue ) 
      {
         if( uCount > 0 ) { stringReturning += ','; }
         stringReturning += it.first;
         if( it.second.empty() == false )
         {
            stringReturning += " AS ";
            stringReturning += it.second;
         }

         uCount++;
      }
   };

   switch( uDialect )
   {
   case eSqlDialectSqlServer:
      stringReturning += "OUTPUT ";
      add_returning( vectorValue, stringReturning );
      break;
   default:
      stringReturning += "RETURNING ";
      add_returning( vectorValue, stringReturning );
      break;
   }
}

/**
 * @brief Constructs a RETURNING clause string for a query based on the specified column and dialect.
 * 
 * Constructs gd::borrow::vector< std::pair< std::string_view, std::string_view > > from stringColumn and 
 * calls the other overload of returning_get_s to generate the RETURNING clause string.
 * 
 * It splits the stringColumn using the specified delimiters for columns and aliases, and creates
 * pairs of column names and aliases to be used in the RETURNING clause. The generated clause is stored in stringReturning.
 * 
 * ```cpp
 * // Example usage:
 * using namespace gd::sql;
 * std::string stringColumn = "FId;FName,Name;FAge";
 * std::string stringReturning;
 * query::returning_get_s(stringColumn, stringReturning, eSqlDialectPostgreSQL, ';', ',');
 * ```
 * 
 * @param stringColumn The column specification to include in the RETURNING clause.
 * @param stringReturning The output string where the RETURNING clause will be stored or appended.
 * @param uDialect The SQL dialect identifier used to format the clause appropriately.
 * @param iSplitColumn The delimiter character used to split column names.
 * @param iSplitAlias The delimiter character used to split column aliases.
 */
void query::returning_get_s( std::string_view stringColumn, std::string& stringReturning, unsigned uDialect, char iSplitColumn, char iSplitAlias )
{
   std::array< std::pair< std::string_view, std::string_view >, 32 > arrayColumn; // used as buffer to gd::borrow::vector
   gd::borrow::vector< std::pair< std::string_view, std::string_view > > vectorValue( arrayColumn );

   if( stringColumn.find( iSplitColumn ) != std::string_view::npos )
   {   
      auto vectorColumn = gd::utf8::split( stringColumn, iSplitColumn );
      for( auto it : vectorColumn )
      {
         std::string_view stringAlias;
         std::string_view stringName;
         if( it.find( iSplitAlias ) != std::string_view::npos )
         {
            auto vectorColumnAlias = gd::utf8::split( it, iSplitAlias );
            if( vectorColumnAlias.size() == 2 )
            {
               stringName = vectorColumnAlias[0];
               stringAlias = vectorColumnAlias[1];
            }
            else { stringName = it; }
         }
         else
         {
            stringName = it;
         }
         vectorValue.push_back( { stringName, stringAlias } );
      }
   }
   else
   {
      vectorValue.push_back( { stringColumn, std::string_view() } );
   }

   returning_get_s( vectorValue, stringReturning, uDialect );
}


/** ---------------------------------------------------------------------------
 * @brief Variant Values in vector is converted to string in a format that works for sql 
 * @param vectorValue values converted to string
 * @return true and values as string if ok, false and error information if fail
*/
std::pair<bool, std::string> query::values_get_s( const std::vector< gd::variant >& vectorValue )
{
   std::string stringValues;

   for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it != itEnd; it++ )
   {
      if( stringValues.empty() == false ) { stringValues += ','; }

      if( it->is_string() == true )
      {
         stringValues += '\'';
         append_g( *it, stringValues, tag_raw{} );
         stringValues += '\'';
      }
      else if( it->is_binary() == true )
      {
         stringValues += std::string_view{ "0x" };
         append_g( *it, stringValues );
      }
      else
      {
         append_g( *it, stringValues );
      }
   }

   return { true, std::move( stringValues ) };
}


/** ---------------------------------------------------------------------------
 * @brief Variant Values in vector is converted to string in a format that works for sql 
 * @param vectorValue values converted to string
 * @param stringValues string values are added to
*/
void query::values_get_s( const std::vector< gd::variant_view >& vectorValue, std::string& stringValues )
{
   for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it != itEnd; it++ )
   {
      if( stringValues.empty() == false ) { stringValues += ','; }
      value_get_s( *it, stringValues );
   }
}


/** ---------------------------------------------------------------------------
 * @brief Generate value list text for values in vector
 * @param vectorValue value from pairs in vector are used generating list text with values
 * @return true and values as string if ok, false and error information if fail
*/
std::pair<bool, std::string> query::values_get_s( const std::vector< std::pair<std::string, gd::variant> >& vectorValue )
{
   std::vector< gd::variant_view > vectorValueView;
   vectorValueView.reserve( vectorValue.size() );
   // !!! Remember to use reference in loop because otherwise variant will create a temporary object
   //     and that will be deleted. Then variant_view points to invalid data
   for( auto& it : vectorValue ) { vectorValueView.push_back( gd::variant_view( it.second ) ); }

   return values_get_s( vectorValueView );
}

/** ---------------------------------------------------------------------------
 * @brief Add (join) data from another query
 * 
 * @code
using namespace gd::sql;
gd::sql::query querySelect;
querySelect.add( "TAddress", "address_type", tag_table{} );
querySelect.add( "TAddress", "FId", tag_field{});
querySelect.add( "TAddress", { {"FStreet", "street"}, {"FCity", "city"}, {"FZipCode", "zip_code"}, {"FState", "state"}, {"FCountry", "country"}, {"FCoordinates", "coordinates"}  }, tag_field{});

gd::sql::query queryAdd;
queryAdd.add( "TAddress", "address_type", tag_table{} );
queryAdd.add( "TAddress", "FRegion = ?", tag_condition{} );

querySelect += queryAdd; // <- operator that use `add_s`

auto stringSelect = querySelect.sql_get( eSqlSelect );
std::cout << stringSelect << "\n";
 * @endcode
 * 
 * @param queryTo query that adds information
 * @param queryFrom query added information is taken from
 * @return true if ok, false and error information if not
*/
std::pair<bool, std::string> query::add_s( query& queryTo, const query& queryFrom )
{
   // ## add tables from "from" - query
   for( unsigned u = 0; u < queryFrom.table_size(); u++ )
   {
      auto ptable = queryFrom.table_get( u );
      if( queryTo.table_exists( *ptable ) == false )
      {
         queryTo.table_add( *ptable );
      }
   }

   // ## add fields from "from" - query
   for( auto it = queryFrom.field_begin(), itEnd = queryFrom.field_end(); it != itEnd; it++ )
   {
      const auto* ptableFrom = queryFrom.table_get_for_key( it->get_table_key() );                 assert( ptableFrom != nullptr );
      const auto* ptableTo = queryTo.table_get( *ptableFrom );                                     assert( ptableTo != nullptr );

      query::field fieldAdd( *ptableTo, it->get_useandtype(), it->get_arguments() );

      queryTo.field_add( std::move( fieldAdd ) );
   }


   // ## add conditions from "from" - query
   for( auto it = queryFrom.condition_begin(), itEnd = queryFrom.condition_end(); it != itEnd; it++ )
   {
      const auto* ptableFrom = queryFrom.table_get_for_key( it->get_table_key() );                 assert( ptableFrom != nullptr );
      const auto* ptableTo = queryTo.table_get( *ptableFrom );                                     assert( ptableTo != nullptr );

      query::condition conditionAdd( *ptableTo, it->get_arguments() );

      queryTo.condition_add( std::move( conditionAdd ) );
   }

   return { true, "" };
}


_GD_SQL_QUERY_END
