// @FILE [tag: binary] [description: Handle xml and html documents] [type: source] [name: gd_tools_html_document.cpp]

#include "gd_tools_html_document.h"

_GD_TOOLS_HTML_BEGIN

void element::common_construct( const element& o )
{
   m_stringName = o.m_stringName;
   m_stringContent = o.m_stringContent;
   m_pelementParent = nullptr;
   m_argumentsAttribute = o.m_argumentsAttribute;
   
   m_vectorElement.reserve( o.m_vectorElement.size() );
   for( const auto& pelementChild : o.m_vectorElement )
   {
      auto pelement_ = std::make_unique<element>();
      pelement_->common_construct( *pelementChild );                           // note that this will copy children
      pelement_->set_parent( this );
      m_vectorElement.push_back( std::move( pelement_ ));
   }
}


element* element::add(std::unique_ptr<element> pelementChild)
{
   pelementChild->set_parent( this );
   m_vectorElement.push_back(std::move(pelementChild));
   return m_vectorElement.back().get();
}

void element::add_attribute( std::string_view stringName, std::string_view stringContent )
{
   m_argumentsAttribute[stringName] = stringContent;
}

_GD_TOOLS_HTML_END
