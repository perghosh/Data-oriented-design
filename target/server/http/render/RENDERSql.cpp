// @FILE [tag: sql, build] [description: sql render] ] [type: source] [name: RENDERSql.cpp]

#include <cassert>
#include <sstream>

#include "pugixml/pugixml.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

#include "gd/gd_arguments_io.h"
#include "gd/gd_table_io.h"

#include "RENDERSql.h"


void CRENDERSql::Initialize()
{
   // Initialize datable that will hold response body parts

   if( m_pcolumnsField_s == nullptr )
   {
      constexpr auto uSize = m_uMaxStringBufferLength_s;
      m_pcolumnsField_s = new gd::table::detail::columns{};                    /// static columns for body, remember to delete on shutdown (release)
      m_pcolumnsField_s->add( "uint32", 0, "key" );
      m_pcolumnsField_s->add( "string", uSize, "table" );                     // name for table field belongs to
      m_pcolumnsField_s->add( "string", uSize, "column" );                    // name for column in table
      m_pcolumnsField_s->add( "string", uSize, "alias" );                     // alias for column in table
      m_pcolumnsField_s->add( "string", uSize * 2, "value" );                 // value for column in table
      m_pcolumnsField_s->add_reference();
   }

   m_tableField.set_columns( m_pcolumnsField_s );
   m_tableField.prepare();
}

void CRENDERSql::AddValue( const gd::argument::arguments argumentsField )
{
   auto uRow = m_tableField.row_add_one();
   for( auto [key_, value_] : argumentsField.named() )
   {
      if( value_.is_string() == true ) { Add( key_, value_.as_string_view() ); }
      else                             { Add( key_, value_ ); }
   }
}

void CRENDERSql::Add( std::string_view stringName, std::string_view stringValue )
{
   uint64_t uRow = m_tableField.get_row_count() - 1;
   m_tableField.cell_set( uRow, stringName, stringValue, gd::table::tag_spill{});
}

void CRENDERSql::Add( std::string_view stringName, gd::variant_view variantviewValue )
{
   std::string stringValue = variantviewValue.as_string();
   Add( stringName, std::string_view( stringValue ) );
}


/// Destroy static members
void CRENDERSql::Destroy_s()
{
   if( m_pcolumnsField_s != nullptr )
   {                                                                          assert( m_pcolumnsField_s->get_reference() == 1 );
      m_pcolumnsField_s->release();
      m_pcolumnsField_s = nullptr;
   }
}
