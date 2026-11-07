// @FILE [tag: expression, method, console] [summary: Header file for console expression methods] [description: This header file defines console/terminal methods (print, input, colors, terminal info) for the GD expression engine.] [name: gd_expression_method_02.h] [type: header]

/**
 * @file gd_expression_method_02.h
 *
 * @brief Console methods for the expression engine.
 *
 * Methods are meant to be registered as namespace `console`, for example:
 * `runtime.add( { uMethodConsoleSize_g, pmethodConsole_g, "console" } );`
 * and are then called like `console::println("hello")`.
 *
 * Works on Windows (console API), Linux and macOS (POSIX termios/ioctl). ANSI escape
 * sequences are only written when standard output is an interactive terminal.
 */

#pragma once

#include <cassert>
#include <functional>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "gd_expression.h"
#include "gd_expression_value.h"
#include "gd_expression_runtime.h"

#ifndef _GD_EXPRESSION_BEGIN
#define _GD_EXPRESSION_BEGIN namespace gd { namespace expression {
#define _GD_EXPRESSION_END } }
#endif

_GD_EXPRESSION_BEGIN

// ## output methods, no return value

std::pair<bool, std::string> beep_g(const std::vector<value>& vectorArgument);
std::pair<bool, std::string> clear_g(const std::vector<value>& vectorArgument);
std::pair<bool, std::string> error_g(const std::vector<value>& vectorArgument);
std::pair<bool, std::string> pause_g(const std::vector<value>& vectorArgument);
std::pair<bool, std::string> print_g(const std::vector<value>& vectorArgument);
std::pair<bool, std::string> println_g(const std::vector<value>& vectorArgument);
std::pair<bool, std::string> title_g(const std::vector<value>& vectorArgument);

// ## methods that return a value

std::pair<bool, std::string> choose_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> color_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> confirm_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> input_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> input_number_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> is_tty_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> password_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> term_height_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> term_width_g(const std::vector<value>& vectorArgument, value* pvalueResult);


//============================================================================
// Console method registry
// Format: { function_pointer, "method_name", parameter_count, return_count, flags }
// NOTE: table must be sorted by name, runtime uses binary search to find methods
//============================================================================

const method pmethodConsole_g[] = {
   { (void*)&beep_g,         "beep",         0, 0, method::eFlagVoid },                                // beep() - terminal bell
   { (void*)&choose_g,       "choose",       2, 1, method::eFlagVarArgs },                             // choose(prompt, option1, option2, ...) - numbered menu, returns selected number (1-based)
   { (void*)&clear_g,        "clear",        0, 0, method::eFlagVoid },                                // clear() - clear screen
   { (void*)&color_g,        "color",        2, 1 },                                                   // color(text, style) - text wrapped in ANSI style, style like "red", "bold green", "bg_blue"
   { (void*)&confirm_g,      "confirm",      2, 1 },                                                   // confirm(prompt, default) - yes/no question, returns bool
   { (void*)&error_g,        "error",        0, 0, method::eFlagVoid | method::eFlagVarArgs },         // error(a, ...) - print values and newline to standard error
   { (void*)&input_g,        "input",        1, 1 },                                                   // input(prompt) - read line as string
   { (void*)&input_number_g, "input_number", 2, 1 },                                                   // input_number(prompt, default) - read number, default is returned for empty or invalid input
   { (void*)&is_tty_g,       "is_tty",       0, 1 },                                                   // is_tty() - true if standard output is an interactive terminal
   { (void*)&password_g,     "password",     1, 1 },                                                   // password(prompt) - read line without echo
   { (void*)&pause_g,        "pause",        0, 0, method::eFlagVoid | method::eFlagVarArgs },         // pause(prompt?) - wait for enter
   { (void*)&print_g,        "print",        0, 0, method::eFlagVoid | method::eFlagVarArgs },         // print(a, ...) - print values separated by space
   { (void*)&println_g,      "println",      0, 0, method::eFlagVoid | method::eFlagVarArgs },         // println(a, ...) - print values separated by space and newline
   { (void*)&term_height_g,  "term_height",  0, 1 },                                                   // term_height() - terminal rows, 24 if unknown
   { (void*)&term_width_g,   "term_width",   0, 1 },                                                   // term_width() - terminal columns, 80 if unknown
   { (void*)&title_g,        "title",        1, 0, method::eFlagVoid },                                // title(text) - set terminal window title
};

// Calculate array size at compile time
constexpr size_t uMethodConsoleSize_g = sizeof(pmethodConsole_g) / sizeof(method);

_GD_EXPRESSION_END