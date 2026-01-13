#include "../gd_database_record.h"

#include "gd_database_io.h"

_GD_DATABASE_BEGIN

/// Generate columns in table
static void prepare_columns_s(const gd::database::record* precord, gd::table::table_column_buffer* ptablecolumnbuffer )
{
   for( unsigned u = 0, uMax = (unsigned)precord->size(); u < uMax; u++ )
   {
      auto pcolumn = precord->get_column( u );
      std::string_view stringName = precord->name_get( u );
      unsigned uType = pcolumn->type();
#ifndef NDEBUG
      auto pbszTypeName_d = gd::types::type_name_g( uType );
#endif // !NDEBUG

      unsigned uSize = 0;
      if( pcolumn->is_fixed() == false ) { uType |= gd::types::eTypeDetailReference; }
      else                               { uSize = pcolumn->size_buffer(); }

      ptablecolumnbuffer->column_add( uType, uSize, stringName );
   }
}

/** ---------------------------------------------------------------------------
 * @brief Fill table with data from cursor
 * @param pcursor cursor with data that is inserted into table
 * @param ptable pointer to table that is filled with data
 * @return true if ok, false and error information if not
 */
std::pair<bool, std::string> to_table(gd::database::cursor_i* pcursor, gd::table::dto::table* ptable)
{                                                                                                  assert( pcursor != nullptr ); assert( ptable != nullptr );
   const auto* precord = pcursor->get_record();                                                    assert( precord != nullptr );

   if( ptable->empty() == true )                                              // check if table is empty, if empty then generate a perfect match against database cursor result
   {
      if( ptable->get_reserved_row_count() == 0 ) ptable->set_reserved_row_count( 10 ); //pre allocate data to hold 10 rows 
      ptable->set_flags(gd::table::tag_full_meta{});                          // set full meta data, all columns are added and full functionality within table
      prepare_columns_s( precord, ptable );
      ptable->prepare();
   }

   // ## table contains columns, match against tables in result to know what to add
   auto vectorTableName = ptable->column_get_name();
   auto vectorResultName = precord->name_get();

   // Match column names, only fill in columns with matching name in table and result
   auto vectorMatch = gd::table::table_column_buffer::column_match_s( vectorTableName, vectorResultName );

   if( vectorMatch.empty() == false )
   {
      std::vector<unsigned> vectorWriteTable;
      std::vector<unsigned> vectorReadResult;

      for( const auto& it : vectorMatch )
      {
         vectorWriteTable.push_back( it.first );
         vectorReadResult.push_back( it.second );
      }

      while( pcursor->is_valid_row() == true )
      {
         auto vectorValue = precord->get_variant_view( vectorReadResult );
         ptable->row_add( vectorValue, vectorWriteTable, gd::table::tag_convert{} );
         pcursor->next();
      }
   }
   else
   {
      while( pcursor->is_valid_row() == true )
      {
         auto vectorValue = precord->get_variant_view();
         ptable->row_add( vectorValue, gd::table::tag_convert{} );
         pcursor->next();
      }
   }

   return { true, std::string() };
}

_GD_DATABASE_END