#pragma once

#include <cassert>
#include <functional>
#include <string>
#include <string_view>

#include "../gd_types.h"
#include "../gd_arguments.h"
#include "../gd_variant_view.h"

#include "../gd_table_column-buffer.h"

#include "../gd_database.h"

_GD_DATABASE_BEGIN

std::pair<bool, std::string> to_table(gd::database::cursor_i* pcursor, gd::table::dto::table* ptable);


_GD_DATABASE_END