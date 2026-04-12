// @FILE
// 


#pragma once


#ifndef LUA_BEGIN
#  define LUA_BEGIN namespace LUA {
#  define LUA_END }
#endif

LUA_BEGIN

void RegisterTable( sol::state& stateLua );
void RegisterDocument( sol::state& stateLua );
void RegisterApplication( sol::state& stateLua );

LUA_END
