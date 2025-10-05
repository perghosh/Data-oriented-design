/**
 * \file gd_cli_options.h
 * @brief Header file for command-line interface (CLI) options management.
 * 
 * 
 | Area                | Methods (Examples)                                                                                                 | Description                                                                                                   |
 |---------------------|-----------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------|
 | Construction        | options(...), common_construct(...), set_parent(...), parent(...), clone_arguments()                            | Constructors and copy/assignment for creating, copying, and initializing CLI options and subcommands.         |
 | Appending           | add(...), add_flag(...), add_global(...), sub_add(...), add_value(...)                                          | Methods for adding options, flags, global options, subcommands, and argument values.                          |
 | Setting             | set(...), set_first(...), set_flag(...), set_active(), set_name(...), set_type(...), set_letter(...)            | Methods for updating option values, flags, types, names, and activation state.                                |
 | Removal             | clear(...), clear_all(), remove(...)                                                                            | Methods for removing argument values by name, clearing all values, and clearing subcommand values.            |
 | Retrieval           | find(...), find_active(), find_active(), find_sub(...), get_variant(...), get_variant_view(...), exists(...)    | Methods for retrieving options, active options, subcommands, argument values, and checking existence.         |
 | Iteration           | begin(), end(), option_begin(), option_end(), sub_size(), sub_count_active(), sub_find_active(), sub_find(...)  | Methods for traversing options and subcommands using iterators and search functions.                          |
 | Comparison          | is_flag(...), is_active(), is_parent(), is_sub(), is_single_dash(), sub_is_active(...), sub_exists(...)         | Methods for checking flags, activation, parent/subcommand status, and single-dash option allowance.           |
 | Printing/Debug      | print_documentation(...), print_suboption_options(...), to_string(), to_string_s(...), error_s(...)             | Methods for formatting, printing, and converting options and argument values for documentation and debugging. |
 | Utility/Meta        | empty(), size(), at(...), get_parent(), name(), description(), flag_s(...), type(...), flags(...), sub_get(...) | Utility methods for clearing, checking, querying metadata, and accessing option/subcommand properties.        |
 * 
 *
 * ### 0TAG0 File navigation, mark and jump to common parts *
 * - `0TAG0option.options` - option object, manage each option value
 * - `0TAG0construct.options` - construct options
 * - `0TAG0sub.options` - methods to access sub options
 * 
 */

#pragma once

#include <cassert>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "gd_types.h"
#include "gd_variant.h"
#include "gd_variant_view.h"
#include "gd_arguments.h"


#ifndef _GD_CLI_BEGIN
#define _GD_CLI_BEGIN namespace gd { namespace cli {
#define _GD_CLI_END } }
_GD_CLI_BEGIN
#else
_GD_CLI_BEGIN
#endif


 /** --------------------------------------------------------------------------
 * \class options
 * \brief A class for handling and parsing command-line arguments passed to an executable.
 *
 * The `options` class manages arguments passed to an executable, supporting subcommands and adhering to standard command-line conventions. 
 * It provides functionality to define, parse, and retrieve option values, including support for ordinal positions of arguments. 
 * Options can be configured with flags, types, and descriptions, and the class supports hierarchical structures through subcommands, 
 * enabling complex argument parsing scenarios.
 * 
 * Example usage:
 * \code
 * #include "gd_cli_options.h"
 * 
 * int main(int argc, const char* argv[]) {
 *     gd::cli::options cli_options;
 *     
 *     // Define options
 *     cli_options.add({ "help", 'h', "Display help information" });
 *     cli_options.add({ "version", 'v', "Display version information" });
 *     cli_options.add({ "input", 'i', "Specify input file" });
 *     cli_options.add({ "output", 'o', "Specify output file" });
 *     
 *     // Parse command-line arguments
 *     auto [success, error_message] = cli_options.parse(argc, argv);
 *     if(!success) {
 *         std::cerr << "Error: " << error_message << std::endl;
 *         return 1;
 *     }
 *     
 *     // Check for options
 *     if(cli_options.exists("help")) {
 *         std::cout << "Help information..." << std::endl;
 *         return 0;
 *     }
 *     
 *     if(cli_options.exists("version")) {
 *         std::cout << "Version 1.0.0" << std::endl;
 *         return 0;
 *     }
 *     
 *     if(cli_options.exists("input")) {
 *         std::string input_file = cli_options.get_variant("input").as_string();
 *         std::cout << "Input file: " << input_file << std::endl;
 *     }
 *     
 *     if(cli_options.exists("output")) {
 *         std::string output_file = cli_options.get_variant("output").as_string();
 *         std::cout << "Output file: " << output_file << std::endl;
 *     }
 *     
 *     return 0;
 * }
 * \endcode
 */
class options
{
public:
   /// Format for printing options in a table format
   struct tag_documentation_table {};
   /// Format for printing options in a dense format
   struct tag_documentation_dense {};
   /// Format for printing options in a verbose format
   struct tag_documentation_verbose {};


public:
   enum enumFlag
   {
      eFlagActive          = 0b0000'0000'0000'0001,
      eFlagParent          = 0b0000'0000'0000'0010,   ///< if options should find valid option values in parent
      eFlagUnchecked       = 0b0000'0000'0000'0100,   ///< when parsing argument values 
      eFlagSingleDash      = 0b0000'0000'0000'1000,   ///< option value marked with single dash is allowed (normal for options is double dashed)
   };

   enum enumOptionFlag
   {
      eOptionFlagGlobal    = 0b0000'0000'0000'0001,   ///< option is global, valid for all commands 
      eOptionFlagSingle    = 0b0000'0000'0000'0010,   ///< option is only allowed if alone
      eOptionFlagFlag      = 0b0000'0000'0000'0100,   ///< option can be used as flag
		eOptionFlagOption    = 0b0000'0000'0000'1000,   ///< option is used with value
   };

   enum enumOptionType
   {
      eOptionTypeCommand   = 1, ///< command type
      eOptionTypeOption    = 2, ///< option type
      eOptionTypeFlag      = 4, ///< flag type
   };

   // 0TAG0option.options

   /**
    * \brief Manage data for valid options
    * 
    * `option` store information about each valid option. `options` store these
    * `option` values in list and these are matched when application arguments 
    * is parsed.
    */
   struct option
   {
      // ## construction -------------------------------------------------------------

      option() {}
      option( const std::string_view& stringName ) { set_name( stringName ); }
      option( const std::string_view& stringName, unsigned uFlags ) { set_name( stringName ); set_flags( uFlags ); }
      option( const std::string_view& stringName, unsigned uFlags, const std::string_view& stringDescription ): m_stringDescription( stringDescription ) { set_name( stringName ); set_flags( uFlags ); }
      option( const std::string_view& stringName, const std::string_view& stringFlag, const std::string_view& stringDescription ): m_stringDescription( stringDescription ) { set_name( stringName ); set_flags( flag_s(stringFlag) ); }
      option( const std::string_view& stringName, unsigned uType, unsigned uFlags, const std::string_view& stringDescription ): m_uType(uType), m_stringDescription( stringDescription ) { set_name( stringName ); set_flags( uFlags ); }
      option( const std::string_view& stringName, const std::string_view& stringType, unsigned uFlags, const std::string_view& stringDescription ): m_uType(gd::types::type_g(stringType)), m_stringDescription( stringDescription ) { set_name( stringName ); set_flags( uFlags ); }
      option( const std::string_view& stringName, const std::string_view& stringDescription ): m_stringDescription( stringDescription ) { set_name(stringName); }
      option( const std::string_view& stringName, char chLetter ): m_chLetter(chLetter) { set_name(stringName); }
      option( const std::string_view& stringName, char chLetter, const std::string_view& stringDescription ): m_chLetter(chLetter), m_stringDescription( stringDescription ) { set_name(stringName); }
      option( const option& o ) {
         m_uType = o.m_uType; m_uFlags = o.m_uFlags; m_stringName = o.m_stringName; m_chLetter = o.m_chLetter; m_stringDescription = o.m_stringDescription; m_argumentsRule = o.m_argumentsRule;
      }
      option( option&& o ) noexcept {
         m_uType = o.m_uType; m_uFlags = o.m_uFlags; m_stringName = std::move(o.m_stringName); m_chLetter = o.m_chLetter; m_stringDescription = std::move(o.m_stringDescription); m_argumentsRule = std::move(o.m_argumentsRule);
      }

      option& operator=( const option& o ) {
         m_uType = o.m_uType; m_uFlags = o.m_uFlags; m_stringName = o.m_stringName; m_chLetter = o.m_chLetter; m_stringDescription = o.m_stringDescription; m_argumentsRule = o.m_argumentsRule;
         return *this;
      }
      option& operator=( option&& o ) noexcept {
         m_uType = o.m_uType; m_uFlags = o.m_uFlags; m_stringName = std::move(o.m_stringName); m_chLetter = o.m_chLetter; m_stringDescription = std::move(o.m_stringDescription); m_argumentsRule = std::move(o.m_argumentsRule);
         return *this;
      }

      ~option() {}

      bool is_flag() const noexcept { return m_uType & gd::types::eTypeNumberBool || (m_uFlags & eOptionFlagFlag) == eOptionFlagFlag; }
      bool is_global() const noexcept { return (m_uFlags & eOptionFlagGlobal) == eOptionFlagGlobal; }

      std::string_view name() const { return m_stringName; }
      char letter() const { return m_chLetter; }
      void set_letter(char chLetter) { m_chLetter = chLetter; }
      void set_name( const std::string_view& stringName );
      void set_type( unsigned uType ) { m_uType = uType; }
      option& type( unsigned uType ) { m_uType = uType; return *this; }
      option& type( const std::string_view& stringType ) { m_uType = gd::types::type_g( stringType ); return *this; }
      void set_flags( unsigned uFlags ) { m_uFlags = uFlags; }
		void set_flags(unsigned uSet, unsigned uClear) { m_uFlags |= uSet; m_uFlags &= ~uClear; }
      option& flags( unsigned uFlags ) { m_uFlags = uFlags; return *this; }
      std::string_view description() const { return m_stringDescription; }

      // ## attributes
      unsigned m_uType = 0;      ///< value type if specified (no type = string)
      unsigned m_uFlags = 0;     ///< option flags, could be that option is global or only allowed if single or something else
      std::string m_stringName;  ///< option name
      char m_chLetter = '\0';    ///< option abbreviation (letter)
      std::string m_stringDescription; ///< option description
      gd::argument::arguments m_argumentsRule;  ///< rules for option value if any
   };



// 0TAG0construct.options
// ## construction -------------------------------------------------------------
public:
   options() {}
   options( const std::string_view& stringName ): m_stringName{ stringName } {}
   options( const std::string_view& stringName, const std::string_view& stringDescription ): m_stringName{ stringName }, m_stringDescription{ stringDescription } {}
   options( unsigned uFlags ) : m_uFlags{uFlags} {}
   options( unsigned uFlags, unsigned uFirstToken ) : m_uFirstToken{uFirstToken}, m_uFlags{uFlags} {}
   options( unsigned uFlags, const std::string_view& stringName ) : m_uFlags{uFlags}, m_stringName{ stringName } {}
   options( unsigned uFlags, const std::string_view& stringName, const std::string_view& stringDescription ) : m_uFlags{uFlags}, m_stringName{ stringName }, m_stringDescription{ stringDescription } {}
   options( const std::initializer_list<option>& listOption ) { add( listOption ); }
   options( const std::string_view& stringName, const std::initializer_list<option>& listOption ): m_stringName{ stringName } { add( listOption ); }
   options( unsigned uFlags, const std::string_view& stringName, const std::initializer_list<option>& listOption ): m_uFlags{uFlags}, m_stringName{ stringName } { add( listOption ); }
   options( const std::string_view& stringName, const gd::argument::arguments& arguments_ ): m_stringName{ stringName }, m_argumentsValue(arguments_) {}
// copy
   options( const options& o ) { common_construct( o ); }
   options( options&& o ) noexcept { common_construct( std::move( o ) ); }
// assign
   options& operator=( const options& o ) { common_construct( o ); return *this; }
   options& operator=( options&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~options() {}
private:
// common copy
   void common_construct( const options& o ) {
      m_uFirstToken = o.m_uFirstToken;
      m_uFlags = o.m_uFlags;
      m_iArgumentCount = o.m_iArgumentCount;
      m_stringName = o.m_stringName;
      m_stringDescription = o.m_stringDescription;
      m_vectorOption = o.m_vectorOption;
      m_vectorSubOption = o.m_vectorSubOption;
      m_argumentsValue = o.m_argumentsValue;
      m_poptionsParent = o.m_poptionsParent;
   }
   void common_construct( options&& o ) noexcept {
      m_uFirstToken = o.m_uFirstToken;
      m_uFlags = o.m_uFlags;
		m_iArgumentCount = o.m_iArgumentCount;
      m_stringName = std::move( o.m_stringName );
      m_stringDescription = std::move( o.m_stringDescription );
      m_vectorOption = std::move( o.m_vectorOption );
      m_vectorSubOption = std::move( o.m_vectorSubOption );
      m_argumentsValue = std::move( o.m_argumentsValue );
      m_poptionsParent = o.m_poptionsParent;
   }

// ## operator -----------------------------------------------------------------
public:
   gd::variant_view operator[]( const std::string_view& stringName ) const noexcept { return get_variant_view( stringName ); }
   gd::variant_view operator[]( const std::initializer_list<std::string_view>& listName ) const noexcept { return get_variant_view( listName ); }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
// ## flags and settings

   void set_first( unsigned uFirst ) { m_uFirstToken = uFirst; }
   void set_flag( unsigned uSet, unsigned uClear ) noexcept { m_uFlags |= uSet; m_uFlags &= ~uClear;  }
   bool is_flag( enumFlag eFlag ) const noexcept { return (m_uFlags & (unsigned)eFlag) == eFlag; }                                                                                            
   /// if this options section is the one passed to application
   bool is_active() const { return is_flag( eFlagActive ); }
   /// If parent flag is set then rules set for parent is valid for sub options
   bool is_parent() const { return is_flag( eFlagParent ); }
   void set_active() { set_flag( eFlagActive, 0 ); }
   bool is_sub() const;
   /// Check if single dash options is allowed
   bool is_single_dash() const { return is_flag( eFlagSingleDash ); }

	// ## argument values

	/// number of arguments passed to parse function (if set)
	int get_argument_count() const { return m_iArgumentCount; }
   void set_argument_count(int iCount) { m_iArgumentCount = iCount; }

   const gd::argument::arguments& get_arguments() const { return m_argumentsValue; }
   gd::argument::arguments& get_arguments() { return m_argumentsValue; }

	// ## meta data about active sub command

   const std::string& name() const { return m_stringName; }
   const std::string& description() const { return m_stringDescription; }

   void parent(const options* poptionsParent) { m_poptionsParent = poptionsParent; }
   void set_parent(const options* poptionsParent) { m_poptionsParent = poptionsParent; }
   const options* get_parent() const { return m_poptionsParent; }

//@}

/** \name OPERATION
*///@{
   /// Add option rule
   options& add( const option& option ) { m_vectorOption.push_back( option ); return *this; }
   options& add( const std::initializer_list<std::string_view>& listName, const std::string_view& stringDescription ); 
   /// Add list of option rules
   options& add( const std::initializer_list<option>& listOption );
   /// Add option flag (option marked as boolean value, don't need a value)
   void add_flag( const option& optionSource );
   /// Add globals option rules from passed options (checks all global option in options )
   void add_global( const options& options_ );
	/// Add option that can be flag or option with value
   void add_flag_or_option(const option& option_);


   /// Parse application arguments (like they are sent to `main`)
   std::pair<bool, std::string> parse( int iArgumentCount, const char* const* ppbszArgumentValue, const options* poptionsRoot );
   std::pair<bool, std::string> parse( int iArgumentCount, const char* const* ppbszArgumentValue ) { return parse( iArgumentCount, ppbszArgumentValue, nullptr ); }
   
   /// parse single string, splits string into parts and parse as normal
   std::pair<bool, std::string> parse( const std::string_view& stringArgument );
   /// parse vector of strings as normal
   std::pair<bool, std::string> parse( const std::vector<std::string>& vectorArgument );
   /// parse single terminal line, splits string into parts and parse as normal, this is the exakt format as you type in terminal
   std::pair<bool, std::string> parse_terminal( const std::string_view& stringArgument );

   std::string to_string() const;
   


   template <typename VALUE>
   void add_value( const option* poption, const VALUE& v ) { assert( poption != nullptr ); add_value( poption->name(), v); }
   void add_value( const std::string_view& stringName, const char* pbszValue ) { m_argumentsValue.append( stringName, pbszValue ); }
   template <typename VALUE>
   void add_value( const std::string_view& stringName, const VALUE& v ) { m_argumentsValue.append( stringName, v ); }
   void set_value( const std::string_view& stringName, std::string_view v ) { m_argumentsValue.set( stringName, v ); }

   std::vector<gd::variant_view> get_all(const std::string_view& stringName) const;

   // ## find added option's

   option* find( const std::string_view& stringName );
   const option* find( const std::string_view& stringName ) const;
   option* find( char chLetter );

   // find parsed values
   bool find( const std::string_view& stringName, const gd::variant_view& variantviewValue ) const;

   /// find active option
   const options* find_active() const;
   options* find_active();

   void set( const gd::argument::arguments& arguments_ ) { m_argumentsValue = arguments_; }
   void set( gd::argument::arguments&& arguments_ ) { m_argumentsValue = std::move( arguments_ ); }

   /// empty is same as no arguments values added
   bool empty() const noexcept { return m_argumentsValue.empty(); }

   void clear(const std::string_view& stringName) { m_argumentsValue.remove(stringName); }
   /// remove values in options
   void clear() { m_argumentsValue.clear(); }
   /// remove all values, also from sub options
   void clear_all();

   // ## get parsed values

   gd::variant get_variant( const std::string_view& stringName ) const;
   gd::variant_view get_variant_view( const std::string_view& stringName ) const noexcept;
   gd::variant_view get_variant_view( const std::string_view& stringName, gd::types::tag_state_active ) const noexcept;
   gd::variant_view get_variant_view( const std::string_view* pstringName ) const noexcept;
   gd::variant_view get_variant_view( const std::string_view& stringName, unsigned uindex ) const noexcept;
   gd::variant_view get_variant_view( const std::string_view* pstringName, unsigned uindex ) const noexcept;
   gd::variant_view get_variant_view( const std::initializer_list<std::string_view>& stringName ) const noexcept;
   // gd::variant_view get_variant_view( const std::initializer_list<std::string_view>& stringName, unsigned uindex ) const noexcept; TODO: impement this i `arguments` object

   std::vector<gd::variant> get_variant_all( const std::string_view& stringName ) const;
   std::vector<gd::variant_view> get_variant_view_all( const std::string_view& stringName ) const;

   // ## if/then for option value, call callback if option exists

   bool iif( const std::string_view& stringName, std::function< void( const gd::variant_view& ) > callback_ ) const;
   void iif( const std::string_view& stringName, std::function< void( const gd::variant_view& ) > true_, std::function< void( const gd::variant_view& ) > false_ ) const;

   // ## print documentation

   /// print all options and their values to get information about options
   void print_documentation( std::string& stringDocumentation, tag_documentation_table ) const;
   void print_documentation( std::string& stringDocumentation, tag_documentation_dense ) const;
   void print_documentation( std::string& stringDocumentation, tag_documentation_verbose ) const;
   void print_suboption_options(const options& optionSub, std::string& stringDocumentation) const;

   /// send documentation to callback function
   void print_documentation( std::function<void(unsigned uType, const std::string_view, std::string_view, const option*)> callback_ ) const;

   /// get option at specified index
   option* at( size_t uIndex ) { return &m_vectorOption.at( uIndex ); }
   const option* at( size_t uIndex ) const { return &m_vectorOption.at( uIndex ); }

   /// Check if arguments exists
   bool exists( const std::string_view stringName ) const noexcept { return m_argumentsValue.exists( stringName ); }
   /// Check if arguments exists in active options
   bool exists( const std::string_view stringName, gd::types::tag_state_active ) const noexcept;

   void remove(const std::string_view& stringName) { m_argumentsValue.remove(stringName); }

   // ## Iterators, default name and one version that specifically names option

   std::vector<option>::iterator option_begin() { return m_vectorOption.begin(); }
   std::vector<option>::iterator option_end() { return m_vectorOption.end(); }
   std::vector<option>::const_iterator option_begin() const { return m_vectorOption.begin(); }
   std::vector<option>::const_iterator option_end() const { return m_vectorOption.end(); }

   std::vector<option>::iterator begin() { return m_vectorOption.begin(); }
   std::vector<option>::iterator end() { return m_vectorOption.end(); }
   std::vector<option>::const_iterator begin() const { return m_vectorOption.begin(); }
   std::vector<option>::const_iterator end() const { return m_vectorOption.end(); }


   /// number of valid options for options command
   size_t size() const { return m_vectorOption.size(); }

   // 0TAG0sub.options
   // ## sub options

   bool sub_exists( const std::string_view& stringName );
   bool sub_is_active( const std::string_view& stringName ) const;
   options& sub_add( const options& options_ ) { m_vectorSubOption.push_back( options_ ); return *this; }
   options& sub_add( options&& options_ ) { m_vectorSubOption.push_back( std::move( options_ ) ); return *this; }
   std::size_t sub_size() const { return m_vectorSubOption.size(); }
   /// find active sub command
   const options* sub_find_active() const;
   options* sub_find_active();
   /// find active sub command name
   std::string_view sub_find_active_name() const;
   /// find sub command for specified name
   options* sub_find( const std::string_view& stringName );
   /// count active sub commands
   size_t sub_count_active() const;
   /// find sub command for specified name
   const options* sub_find( const std::string_view& stringName ) const;
   /// return sub options for specified name
   options sub_get( const std::string_view& stringName ) const;

   /// copy needed data to work with options
   options clone_arguments() const { return options(m_stringName, m_argumentsValue); }
//@}

protected:
/** \name INTERNAL
*///@{

//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   unsigned m_uFirstToken = 1;         ///< Where to start parsing values, when arguments are passed to application then first argument is normally the file name for executable
   unsigned m_uFlags = 0;              ///< How `options` should work internally
	int      m_iArgumentCount = -1;     ///< Number of arguments passed to application
   std::string m_stringName;           ///< If named child command or for other type of identification
   std::string m_stringDescription;    ///< Describe (sub) command
   std::vector<option> m_vectorOption; ///< Valid option values
   gd::argument::arguments m_argumentsValue;///< Argument values
   std::vector< options > m_vectorSubOption;///< Attached subcommands 

   const options* m_poptionsParent = nullptr; ///< Pointer to parent options (if this is sub command)


// ## free functions ------------------------------------------------------------
public:
   /// convert argument values to string
   static std::string to_string_s(int iCount, const char* const* ppbszArgumentValue, int iOffset = 0);
   static std::string to_string_s(const options* poptions_, int iOffset = 0);
   static std::string to_string_s( const gd::argument::arguments& arguments_ );
   /// Parse string into command line arguments, this do not have specific rules more than it can handle quoted strings
   static std::pair<bool, std::string> parse_s(const std::string_view& stringCommandLine, std::vector<std::string>& vectorArguments );
   /// parse terminal formated command line into vector of strings
   static std::pair<bool, std::string> parse_terminal_s(const std::string_view& stringCommandLine, std::vector<std::string>& vectorArguments);
   /// parse terminal formated command line into vector of strings
   static std::vector<std::string> parse_s(const std::string_view& stringCommandLine );
   /// print error message and return it
   static std::pair<bool, std::string> error_s( std::initializer_list<gd::variant_view> listPrint );

   /// return flag
   static constexpr unsigned flag_s( const std::string_view& stringFlag )
   {
      // ## flags for complete object
      if( stringFlag == "active" )           return eFlagActive;
      else if( stringFlag == "parent" )      return eFlagParent;
      else if( stringFlag == "unchecked" )   return eFlagUnchecked;
      else if( stringFlag == "single-dash" ) return eFlagSingleDash;

      // ## flags for each option
      else if( stringFlag == "global" )      return eOptionFlagGlobal;
      else if( stringFlag == "single" )      return eOptionFlagSingle;
                                                                                                   assert( false );
      return 0;
   }


};

inline options::option* options::find( char chLetter ) {
   for( auto it = std::begin( m_vectorOption ), itEnd = std::end( m_vectorOption ); it != itEnd; it++ ) {
      if( it->letter() == chLetter ) return &(*it);
   }

   return nullptr;
}

inline options::option* options::find( const std::string_view& stringName ) {
   for( auto it = std::begin( m_vectorOption ), itEnd = std::end( m_vectorOption ); it != itEnd; it++ ) {
      if( it->name() == stringName ) return &(*it);                            // compare full option name
      if( stringName.length() == 1 && it->letter() == stringName[0] ) return &( *it );// compare single letter
   }

   // ## check for comma separated string, if found then match all names within string
   for( auto it = std::begin( m_vectorOption ), itEnd = std::end( m_vectorOption ); it != itEnd; it++ ) {
      const char* pbszName = it->name().data();
      const char* pbszNext = strchr( pbszName, ',' );
      while(pbszNext != nullptr)
      {
         std::string_view stringMatch( pbszName, pbszNext - pbszName );
         if( stringMatch == stringName ) return &(*it);
         pbszName = pbszNext + 1;
         pbszNext = strchr( pbszName, ',' );
         if( pbszNext == nullptr && *pbszName != '\0' ) pbszNext = pbszName + strlen( pbszName );
      }
   }

   return nullptr;
}

/// find options for selected name
inline const options::option* options::find( const std::string_view& stringName ) const {
   for( auto it = std::begin( m_vectorOption ), itEnd = std::end( m_vectorOption ); it != itEnd; it++ ) {
      if( it->name() == stringName ) return &(*it);
   }

   return nullptr;
}

/// find the active options
inline const options* options::find_active() const {
   const options* poptionsActive = sub_find_active();
   if( poptionsActive != nullptr ) return poptionsActive;
   return this;
}

/// find the active options
inline options* options::find_active() {
   options* poptionsActive = sub_find_active();
   if( poptionsActive != nullptr ) return poptionsActive;
   return this;
}


/// clear data from all sub options and root options
inline void options::clear_all() {
   for( auto it = std::begin( m_vectorSubOption ), itEnd = std::end( m_vectorSubOption ); it != itEnd; it++ ) {
      it->clear();
   }
   clear();
}

_GD_CLI_END
