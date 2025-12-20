// @FILE [tag: dto, http] [description: Data transfer object for HTTP response] [type: source] [name: CDTOResponse.cpp]

#include "DTOResponse.h"

void CDTOResponse::Initialize()
{
   // Initialize datable that will hold response body parts

   if( m_pcolumnsBody_s == nullptr )
   {
      m_pcolumnsBody_s = new gd::table::detail::columns{};
      m_pcolumnsBody_s->add( "uint32", 0, "key" );
      m_pcolumnsBody_s->add( "uint32", 0, "type" );
      m_pcolumnsBody_s->add( "rstring", 0, "text" );
      m_pcolumnsBody_s->add( "pointer", 0, "object" );
      m_pcolumnsBody_s->add( "string", 32, "hint" );
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
      m_tableBody.cell_set( uRow, "key", uRow + 1 );
      uint32_t uType = (uint32_t)object_.type();
      m_tableBody.cell_set( uRow, "type", uType );

      auto* pobject = object_.release(); // detach pointer so we can move it into table
      m_tableBody.cell_set( uRow, "pointer", ( void* )pobject );
   }
   return { true, "" };
}
