// @FILE
// 


#pragma once


#ifndef LUA_BEGIN
#  define LUA_BEGIN namespace LUA {
#  define LUA_END }
#endif

LUA_BEGIN

void RegisterApplication( sol::state& stateLua );
void RegisterDocument( sol::state& stateLua );
void RegisterDatabase( sol::state& stateLua );
void RegisterRequest( sol::state& stateLua );
void RegisterCursor( sol::state& stateLua );
void RegisterTable( sol::state& stateLua );


LUA_END
