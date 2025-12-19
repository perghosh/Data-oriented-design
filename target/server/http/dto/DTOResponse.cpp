// @FILE [tag: dto, http] [description: Data transfer object for HTTP response] [type: source] [name: CDTOResponse.cpp]

#include "DTOResponse.h"

void CDTOResponse::Initialize()
{
   // Initialize datable that will hold response body parts

   if( m_pcolumnsBody_s == nullptr )
   {
      gd::table::detail::columns* m_pcolumnsBody_s = new gd::table::detail::columns{};
      m_pcolumnsBody_s->add( "uint32", 0, "key" );
      m_pcolumnsBody_s->add( "uint32", 0, "type" );
      m_pcolumnsBody_s->add( "rstring", 0, "text" );
      m_pcolumnsBody_s->add( "pointer", 0, "object" );
   }

   m_tableBody.set_columns( m_pcolumnsBody_s );
   m_tableBody.prepare();
}