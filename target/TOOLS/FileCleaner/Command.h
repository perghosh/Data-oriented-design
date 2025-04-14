/**
 * @file Command.cpp
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * 
 */

#pragma once


 // ## STL

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <filesystem>

 // ## GD

#include "gd/gd_uuid.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"


#include "Application.h"

int RowCount(const std::string& stringFile);

