/*
| name | description |
| ---- | ----------- |
| `` |  |
| `` |  |
| `` |  |
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


/**
 * \brief Manage command line options/arguments
 *
 *
 *
 \code
 \endcode
 */
class options
{
public:
   enum enumFlag
   {
      eFlagActive          = 0b0000'0000'0000'0001,
      eFlagParent          = 0b0000'0000'0000'0010,   ///< if options should find valid option values in parent
      eFlagUnchecked       = 0b0000'0000'0000'0100,   ///< when parsing argument values 
   };

   enum enumOptionFlag
   {
      eOptionFlagGlobal    = 0b0000'0000'0000'0001,   ///< option is global, valid for all commands 
      eOptionFlagSingle    = 0b0000'0000'0000'0010,   ///< option is only allowed if alone
   };


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

      bool is_flag() const noexcept { return m_uType & gd::types::eTypeNumberBool; }
      bool is_global() const noexcept { return (m_uFlags & eOptionFlagGlobal) == eOptionFlagGlobal; }

      std::string_view name() const { return m_stringName; }
      char letter() const { return m_chLetter; }
      void set_name( const std::string_view& stringName );
      void set_type( unsigned uType ) { m_uType = uType; }
      option& type( unsigned uType ) { m_uType = uType; return *this; }
      option& type( const std::string_view& stringType ) { m_uType = gd::types::type_g( stringType ); return *this; }
      void set_flags( unsigned uFlags ) { m_uType = uFlags; }
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




// ## construction -------------------------------------------------------------
public:
   options() {}
   options( const std::string_view& stringName ): m_stringName{ stringName } {}
   options( const std::string_view& stringName, const std::string_view& stringDescription ): m_stringName{ stringName }, m_stringDescription{ stringDescription } {}
   options( unsigned uFlags ) : m_uFlags{uFlags} {}
   options( unsigned uFlags, unsigned uFirstToken ) : m_uFlags{uFlags}, m_uFirstToken{uFirstToken} {}
   options( unsigned uFlags, const std::string_view& stringName ) : m_uFlags{uFlags}, m_stringName{ stringName } {}
   options( unsigned uFlags, const std::string_view& stringName, const std::string_view& stringDescription ) : m_uFlags{uFlags}, m_stringName{ stringName }, m_stringDescription{ stringDescription } {}
   options( const std::initializer_list<option>& listOption ) { add( listOption ); }
   options( const std::string_view& stringName, const std::initializer_list<option>& listOption ): m_stringName{ stringName } { add( listOption ); }
   options( unsigned uFlags, const std::string_view& stringName, const std::initializer_list<option>& listOption ): m_uFlags{uFlags}, m_stringName{ stringName } { add( listOption ); }
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
      m_stringName = o.m_stringName;
      m_stringDescription = o.m_stringDescription;
      m_vectorOption = o.m_vectorOption;
      m_vectorSubOption = o.m_vectorSubOption;
      m_argumentsValue = o.m_argumentsValue;
   }
   void common_construct( options&& o ) noexcept {
      m_uFirstToken = o.m_uFirstToken;
      m_uFlags = o.m_uFlags;
      m_stringName = std::move( o.m_stringName );
      m_stringDescription = std::move( o.m_stringDescription );
      m_vectorOption = std::move( o.m_vectorOption );
      m_vectorSubOption = std::move( o.m_vectorSubOption );
      m_argumentsValue = std::move( o.m_argumentsValue );
   }

// ## operator -----------------------------------------------------------------
public:
   gd::variant_view operator[]( const std::string_view& stringName ) const noexcept { return get_variant_view( stringName ); }
   gd::variant_view operator[]( const std::initializer_list<std::string_view>& listName ) const noexcept { return get_variant_view( listName ); }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   const gd::argument::arguments& get_arguments() const { return m_argumentsValue; }
   const std::string& name() const { return m_stringName; }
   const std::string& description() const { return m_stringDescription; }
   void set_first( unsigned uFirst ) { m_uFirstToken = uFirst; }
   void set_flag( unsigned uSet, unsigned uClear ) noexcept { m_uFlags |= uSet; m_uFlags &= ~uClear;  }
   bool is_flag( enumFlag eFlag ) const noexcept { return m_uFlags & (unsigned)eFlag; }                                                                                            
   /// if this options section is the one passed to application
   bool is_active() const { return is_flag( eFlagActive ); }
   /// If parent flag is set then rules set for parent is valid for sub options
   bool is_parent() const { return is_flag( eFlagParent ); }
   void set_active() { set_flag( eFlagActive, 0 ); }
   bool is_sub() const;

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


   /// Parse application arguments (like they are sent to `main`)
   std::pair<bool, std::string> parse( int iArgumentCount, const char* const* ppbszArgumentValue, const options* poptionsRoot );
   std::pair<bool, std::string> parse( int iArgumentCount, const char* const* ppbszArgumentValue ) { return parse( iArgumentCount, ppbszArgumentValue, nullptr ); }
   
   /// parse single string, splits string into parts and parse as normal
   std::pair<bool, std::string> parse( const std::string_view& stringArgument, const std::string_view& stringSplit );

   template <typename VALUE>
   void add_value( const option* poption, const VALUE& v ) { assert( poption != nullptr ); add_value( poption->name(), v); }
   void add_value( const std::string_view& stringName, const char* pbszValue ) { m_argumentsValue.append( stringName, pbszValue ); }
   template <typename VALUE>
   void add_value( const std::string_view& stringName, const VALUE& v ) { m_argumentsValue.append( stringName, v ); }

   // ## find added option's

   option* find( const std::string_view& stringName );
   const option* find( const std::string_view& stringName ) const;
   option* find( char chLetter );

   void set( const gd::argument::arguments& arguments_ ) { m_argumentsValue = arguments_; }
   void set( gd::argument::arguments&& arguments_ ) { m_argumentsValue = std::move( arguments_ ); }

   /// empty is same as no arguments values added
   bool empty() const noexcept { return m_argumentsValue.empty(); }

   // ## get parsed values

   gd::variant get_variant( const std::string_view& stringName ) const;
   gd::variant_view get_variant_view( const std::string_view& stringName ) const noexcept;
   gd::variant_view get_variant_view( const std::string_view* pstringName ) const noexcept;
   gd::variant_view get_variant_view( const std::string_view& stringName, unsigned uindex ) const noexcept;
   gd::variant_view get_variant_view( const std::string_view* pstringName, unsigned uindex ) const noexcept;
   gd::variant_view get_variant_view( const std::initializer_list<std::string_view>& stringName ) const noexcept;

   std::vector<gd::variant> get_variant_all( const std::string_view& stringName ) const;
   std::vector<gd::variant_view> get_variant_view_all( const std::string_view& stringName ) const;

   // ## find parsed values
   bool find( const std::string_view& stringName, const gd::variant_view& variantviewValue ) const;

   bool iif( const std::string_view& stringName, std::function< void( const gd::variant_view& ) > callback_ );
   void iif( const std::string_view& stringName, std::function< void( const gd::variant_view& ) > true_, std::function< void( const gd::variant_view& ) > false_ );

   void print_documentation( std::string& stringDocumentation ) const;

   /// get option at specified index
   option* at( size_t uIndex ) { return &m_vectorOption.at( uIndex ); }
   const option* at( size_t uIndex ) const { return &m_vectorOption.at( uIndex ); }

   /// Check if arguments exists
   bool exists( const std::string_view stringName ) const noexcept { return m_argumentsValue.exists( stringName ); }

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

   bool sub_exists( const std::string_view& stringName );
   bool sub_is_active( const std::string_view& stringName ) const;
   options& sub_add( const options& options_ ) { m_vectorSubOption.push_back( options_ ); return *this; }
   options& sub_add( options&& options_ ) { m_vectorSubOption.push_back( std::move( options_ ) ); return *this; }
   std::size_t sub_size() const { return m_vectorSubOption.size(); }
   /// find active sub command
   const options* sub_find_active() const;
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
   std::string m_stringName;           ///< If named child command or for other type of identification
   std::string m_stringDescription;    ///< Describe (sub) command
   std::vector<option> m_vectorOption; ///< Valid option values
   gd::argument::arguments m_argumentsValue;///< Argument values
   std::vector< options > m_vectorSubOption;///< Attached subcommands 


// ## free functions ------------------------------------------------------------
public:

   static std::pair<bool, std::string> error_s( std::initializer_list<gd::variant_view> listPrint );

   /// return flag
   static constexpr unsigned flag_s( const std::string_view& stringFlag )
   {
      if( stringFlag == "active" )           return eFlagActive;
      else if( stringFlag == "parent" )      return eFlagParent;
      else if( stringFlag == "unchecked" )   return eFlagUnchecked;

      else if( stringFlag == "global" )       return eOptionFlagGlobal;
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
      if( it->name() == stringName ) return &(*it);
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

inline const options::option* options::find( const std::string_view& stringName ) const {
   for( auto it = std::begin( m_vectorOption ), itEnd = std::end( m_vectorOption ); it != itEnd; it++ ) {
      if( it->name() == stringName ) return &(*it);
   }

   return nullptr;
}






_GD_CLI_END