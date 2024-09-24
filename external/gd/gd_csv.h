#include <cassert>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "gd_variant.h"
#include "gd_arguments.h"

#ifndef _GD_CSV_BEGIN
#  define _GD_CSV_BEGIN namespace gd { namespace csv {
#  define _GD_CSV_END } }
#endif

_GD_CSV_BEGIN

struct no_quote {};

std::pair<gd::argument::arguments, std::vector<gd::variant>> read( const std::string_view& stringFileName, no_quote );

_GD_CSV_END