// @FILE [tag: script, lua] [description: Script execution logic that may be used in the API system] [type: source] [name: API_Scripting.h]

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifndef SCRIPT_BEGIN
#  define SCRIPT_BEGIN namespace SCRIPT {
#  define SCRIPT_END }
#endif

class CAPIContext;

SCRIPT_BEGIN

using callback_lua_state = std::function<std::pair<bool, std::string>( sol::state* pstateLua, CAPIContext* pcontext_ )>;

std::pair<bool, std::string> LuaRequestExecute( std::string_view stringScript, CAPIContext* pcontext_, callback_lua_state callback = nullptr );
std::pair<bool, std::string> LuaRequestExecute( const std::vector<gd::variant_view>& vectorScript, CAPIContext* pcontext_, callback_lua_state callback = nullptr );


SCRIPT_END