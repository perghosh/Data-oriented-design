// @FILE [tag: dto, http] [description: Data transfer object for HTTP response] [type: source] [name: CDTOResponse.cpp]

#include <cassert>
#include <sstream>

#include "pugixml/pugixml.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

#include "gd/gd_arguments_io.h"
#include "gd/gd_table_io.h"

#include "DTOResponse.h"

CDTOResponse::~CDTOResponse() 
{ 
   Clear(); 
}

void CDTOResponse::Initialize()
{
   // Initialize datable that will hold response body parts

   if( m_pcolumnsBody_s == nullptr )
   {
      m_pcolumnsBody_s = new gd::table::detail::columns{};                    /// static columns for body, remember to delete on shutdown (release)
      m_pcolumnsBody_s->add( "uint32", 0, "key" );
      m_pcolumnsBody_s->add( "uint32", 0, "type" );
      m_pcolumnsBody_s->add( "rstring", 0, "text" );
      m_pcolumnsBody_s->add( "pointer", 0, "object" );
      m_pcolumnsBody_s->add( "string", 32, "hint" );
      m_pcolumnsBody_s->add_reference();
   }

   m_tableBody.set_columns( m_pcolumnsBody_s );
   m_tableBody.prepare();
}

/// Move objects into response table
std::pair<bool, std::string> CDTOResponse::AddTransfer( Types::Objects* pobjects_ )
{                                                                                                  assert( pobjects_ ); assert( pobjects_->Empty() == false );
   for( auto& object_ : pobjects_->m_vectorObjects )
   {
      auto uRow = m_tableBody.row_add_one();
      m_tableBody.cell_set( uRow, "key", (uint32_t)(uRow + 1) );
      uint32_t uType = (uint32_t)object_.type();
      m_tableBody.cell_set( uRow, "type", uType );

      auto* pobject = object_.release(); // detach pointer so we can move it into table
#ifndef NDEBUG
      intptr_t iTableAddress_d = (intptr_t)pobject;                           // @BOOKMARK [tag: debug, memory, response] [description: check memory adress response data]
#endif
      m_tableBody.cell_set( uRow, "object", ( void* )pobject );
#ifndef NDEBUG
      auto* pobject_d = ( void* )m_tableBody.cell_get_variant_view( uRow, "object" );
      intptr_t iTableAddress2_d = (intptr_t)pobject_d;
      assert( iTableAddress_d == iTableAddress2_d );
#endif
   }
   return { true, "" };
}

std::pair<bool, std::string> CDTOResponse::PrintXml( std::string& stringXml, const gd::argument::arguments* parguments_ )
{
   using namespace pugi;
   using namespace gd::table;
   xml_document xmldocument;   // xml document used to load xml from file

   pugi::xml_node xmlnodeResults = xmldocument.append_child( m_stringResults_s );

   // ## iterate all objects in table and serialize them to xml

   for( uint64_t uRow = 0; uRow < m_tableBody.size(); uRow++ )
   {
      auto* pobject = ( void* )m_tableBody.cell_get_variant_view( uRow, "object" );
#ifndef NDEBUG
      intptr_t iTableAddress_d = (intptr_t)pobject;
#endif
      if( pobject != nullptr )
      {
         std::string stringJson;                                              // json string used for serialization and most clients are able to read json
         stringJson.reserve( 512 );                                           // preallocate 512 byte for json string

         auto xmlnodeResult = xmlnodeResults.append_child( m_stringResult_s );// create result node

         // ### If table then serialize table object

         uint32_t uType = m_tableBody.cell_get_variant_view( uRow, "type" );
         if( Types::TypeNumber_g("table") == uType )
         {
            gd::table::dto::table* ptable = (gd::table::dto::table*)pobject;  //  cast to table object
            to_string( *ptable, stringJson, tag_io_json{}, tag_io_name{});

            // #### add json as xml node as cdata
            xmlnodeResult.append_child(node_cdata).set_value(stringJson);
         }
         else if( Types::TypeNumber_g("arguments") == uType )
         {
            gd::argument::arguments* parguments = (gd::argument::arguments*)pobject;  //  cast to arguments object
            gd::argument::to_string( *parguments, stringJson, gd::argument::tag_io_json{});
            // #### add json as xml node as cdata
            xmlnodeResult.append_child(node_cdata).set_value(stringJson);
         }
         else
         {
            return { false, "unsupported type for xml serialization" };
         }
      }
   }
   
   // ## serialize xml document to `stringXml`
   std::stringstream stringstream_;
   xmldocument.save( stringstream_ );
   stringXml += stringstream_.str();

   return { true, "" };
}



/// Clear response body (table with objects)
void CDTOResponse::Clear()
{
   // ## iterate all objects in table and delete them
   for( uint64_t uRow = 0; uRow < m_tableBody.get_row_count(); uRow++ )
   {
      auto* pobject = ( void* )m_tableBody.cell_get_variant_view( uRow, "object" );
      if( pobject != nullptr )
      {
         uint32_t uType = m_tableBody.cell_get_variant_view( uRow, "type" );
#ifndef NDEBUG
         if( Types::TypeNumber_g("table") == uType )
         {   
            gd::table::dto::table* ptable = (gd::table::dto::table*)pobject;  //  cast to table object
         }
#endif //NDEBUG
         
         m_tableBody.cell_set( uRow, "object", (void*)nullptr );

         Types::Clear_g( (Types::enumType)uType, pobject );
      }
   }


   m_tableBody.clear();
}

/// Destroy static members
void CDTOResponse::Destroy_s()
{
   if( m_pcolumnsBody_s != nullptr )
   {                                                                          assert( m_pcolumnsBody_s->get_reference() == 1 );
      m_pcolumnsBody_s->release();
      m_pcolumnsBody_s = nullptr;
   }
}