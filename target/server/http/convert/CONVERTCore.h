// @FILE [tag: convert] [description: convert between core data objects used in http project] ] [type: source] [name: CONVERTCore.h]

#pragma once


#include "pugixml/pugixml.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

#include "gd/gd_variant.h"
#include "gd/gd_variant_view.h"
#include "gd/gd_arguments.h"

namespace CONVERT { 

gd::variant AsVariant( const jsoncons::json& json );
gd::variant_view AsVariantView( const jsoncons::json& json );


}
