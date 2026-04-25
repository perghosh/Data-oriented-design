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

/** ------------------------------------------------------------------------- Initialize
 * @brief Initializes the response CDTOResponse for work
 */
void CDTOResponse::Initialize()
{
   // Initialize datable that will hold response body parts

   // ## Magic Static (Meyers' Singleton)
   // The compiler guarantees thread-safe initialization exactly once.
   // This is typically faster than manual mutex locking.
   static gd::table::detail::columns* pcolumnsBody_s = []() -> gd::table::detail::columns* {
      auto* p = new gd::table::detail::columns{};
      p->add( "uint32", 0, "key" );
      p->add( "uint32", 0, "type" );
      p->add( "rstring", 0, "text" );
      p->add( "pointer", 0, "object" );
      p->add( "string", 16, "command" );
      p->add( "string", 16, "echo" );
      p->add_reference();

      CDTOResponse::m_pcolumnsBody_s = p; // assign to static member for use in other instances of CDTOResponse

      return p;
   }();
                                                                                                   assert( CDTOResponse::m_pcolumnsBody_s->get_reference() == 1 ); // ensure reference column is added and stay there
   m_tableBody.set_columns( pcolumnsBody_s, gd::table::tag_static_columns{} );
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
   
      // ## Check for default values in arguments ...........................

      const auto& arguments_ = object_.arguments();
      for( const auto [key_, value_] : arguments_.named() )
      {
         if( key_ == "command" || key_ == "echo" )
         {                                                                                         assert( value_.length() < 12 );
            m_tableBody.cell_set( uRow, key_, value_.as_string_view() );
         }
         else
         {
            // ## @OPTIMIZED [tag: if] [description: check if value is string and use string_view to avoid unnecessary copy]
            if( value_.is_string() == true ) { m_tableBody.cell_set_argument( uRow, key_, value_.as_string_view() ); }
            else { m_tableBody.cell_set_argument( uRow, key_, value_.as_string() ); }
         }

      }

   }
   return { true, "" };
}

/** --------------------------------------------------------------------------
 * @brief Serializes the contents of the table to XML format, storing the result in a string and returning a status indicator and message.
 * @param stringXml A reference to a string where the resulting XML will be appended.
 * @param parguments_ A pointer to a gd::argument::arguments object, used for additional serialization context (may be unused in this function).
 * @return A std::pair where the first element is a boolean indicating success (true) or failure (false), and the second element is a string containing an error message if serialization failed, or an empty string on success.
 */
std::pair<bool, std::string> CDTOResponse::PrintXml( std::string& stringXml, const gd::argument::arguments* parguments_ )
{
   using namespace pugi;
   using namespace gd::table;
   xml_document xmldocument;   // xml document used to load xml from file

   auto xmlnodeDeclaration = xmldocument.append_child(pugi::node_declaration);
   xmlnodeDeclaration.append_attribute("version") = "1.0";
   xmlnodeDeclaration.append_attribute("encoding") = "UTF-8";

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

         // ### Check for command and echo
         auto command_ = m_tableBody.cell_get_variant_view( uRow, "command" );// "command" attribute
         if( command_.is_string() == true )
         {
            xmlnodeResult.append_attribute("command").set_value( command_.as_string_view() );
         }

         auto echo_ = m_tableBody.cell_get_variant_view( uRow, "echo" );     // "echo" attribute
         if( echo_.is_string() == true )
         {
            xmlnodeResult.append_attribute("echo").set_value( echo_.as_string_view() );
         }

         // ### Check for arguments attributes and add them as xml attributes
         const auto* parguments = m_tableBody.row_get_arguments_pointer( uRow ); // ensure arguments are loaded for row
         if( parguments != nullptr )
         {
            for( const auto& [key_, value_] : parguments->named() )
            {                                                                                      assert( value_.is_string() == true && "Only string values are supported for XML attributes" ); // for now we only support string
               if( value_.is_string() == true ) { xmlnodeResult.append_attribute( key_ ).set_value( value_.as_string_view() ); }
            }
         }

         // ### If table then serialize table object

         uint32_t uType = m_tableBody.cell_get_variant_view( uRow, "type" );
         if( Types::TypeNumber_g("table") == uType )
         {
            gd::table::dto::table* ptable = (gd::table::dto::table*)pobject;  //  cast to table object
            stringJson += '[';
            gd::argument::arguments argumentsJson( { "format", "escape" } ); // arguments for json serialization, we want to escape special characters in json string to be safely embedded in xml
            to_string( *ptable, 0, ptable->get_row_count(), argumentsJson, nullptr, stringJson, tag_io_header{}, tag_io_json{});

            //to_string( *ptable, stringJson, tag_io_json{}, tag_io_name{}); // @TODO OLD: Remove comment when tested
            stringJson += ']';
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
         else if( Types::TypeNumber_g("text/plain") == uType )
         {
            std::string* pstring = (std::string*)pobject;                     //  cast to string object
            xmlnodeResult.append_child(node_cdata).set_value(pstring->c_str());
         }
         else
         {
            return { false, "unsupported type for xml serialization" };
         }
      }
   }
   
   // ## serialize xml document to `stringXml`
   std::stringstream stringstream_;
   xmldocument.save( stringstream_, "", pugi::format_raw, pugi::encoding_utf8);
   stringXml.append( std::istreambuf_iterator<char>( stringstream_.rdbuf() ), std::istreambuf_iterator<char>() ); // @OPTIMIZED [tag: string] [description: avoid unnecessary copy by appending directly to stringXml]

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
