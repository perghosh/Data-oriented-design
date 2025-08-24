/**
 * \file gd_cli_options.cpp
 * 
 * \brief Definitions for gd command-line options
 * 
 */


#include <memory>
#include <sstream>

#include "gd_parse.h"
#include "gd_cli_options.h"


_GD_CLI_BEGIN

bool options::is_sub() const
{
   if( is_active() == true ) return false;
   else
   {
      if( sub_find_active() != nullptr ) return true;
   }

   return false;
}

/** ---------------------------------------------------------------------------
 * @brief duplicate option arguments added to 
 * @code
 * // sample adding three option arguments with same description
 * gd::cli::options optionsCommand( "select", "run select query against connected database in web server" );
 * optionsCommand.add({"url","qs","querystring"}, "arguments passed to script");
 * @endcode
 * @param listName list with option names that get same description
 * @param stringDescription description name
 * @return 
 */
options& options::add(const std::initializer_list<std::string_view>& listName, const std::string_view& stringDescription)
{
   for(auto it : listName)
   {
      m_vectorOption.push_back( option( it, stringDescription ) );
   }
   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Add multiple options to internal list of valid options
 * @param listOption list of possible options
 */
options& options::add(const std::initializer_list<option>& listOption)
{
   for(const auto itOption : listOption)
   {
      m_vectorOption.push_back( itOption );
   }
   return *this;
}

/// Add flag option (boolean type)
void options::add_flag( const option& optionSource )
{
   option optionAdd( optionSource );
   optionAdd.set_type( gd::types::eTypeBool );

   // ## if option name is only one character and letter is not set then set letter to first character
   if( optionAdd.name().length() == 1 && optionAdd.letter() == '\0' )
   {
      optionAdd.set_letter(optionAdd.name()[0]);                              // set letter to first character
   }
   m_vectorOption.push_back( std::move( optionAdd ) );
}

/// Add all global option values from sent options_
void options::add_global( const options& options_ )
{
   for( auto it = options_.option_begin(), itEnd = options_.option_end(); it != itEnd; it++ )
   {
      if( it->is_global() == true )
      {
         add( *it );
      }
   }
}

/// Get all values for name as variant_view's in list
std::vector<gd::variant_view> options::get_all(const std::string_view& stringName) const
{
   auto vectorValues = m_argumentsValue.get_argument_all(stringName, gd::types::tag_view{}); // get all values for name

   return vectorValues;
}


/** ---------------------------------------------------------------------------
 * @brief Parse application arguments
 * @param iArgumentCount number of arguments to parse
 * @param ppbszArgumentValue pointer to charpointers (array of pointers)
 * @return true if ok, false and error information if error
*/
std::pair<bool, std::string> options::parse( int iArgumentCount, const char* const* ppbszArgumentValue, const options* poptionsRoot )
{                                                                                                  assert( poptionsRoot != nullptr || iArgumentCount > 1 );
   enum { state_unknown, state_option, state_value };

   const option* poptionActive = nullptr; // current option that is beeing processed

   int iOptionState = state_unknown;                 
   int iPositionalArgument = -1;                                               // if argument is set as positional
   bool bAllowPositional = true;                                               // if positional arguments are alowed, when first named argument is found this is disabled

#ifndef NDEBUG
   std::string stringCommand_d;
   const char* pbszCommand_d = nullptr;
   stringCommand_d = name(); 
   pbszCommand_d = stringCommand_d.c_str();
#endif   

   // ## Loop arguments sent to application (starts at 1, first is the application name)
   for( int iPosition = m_uFirstToken; iPosition != iArgumentCount; iPosition++ )
   {                                                                                               assert( iPosition < iArgumentCount ); // iPosition cant pass iArgumentCount
      const char* pbszArgument = ppbszArgumentValue[iPosition];                // current argument

      bool bOption = false;      // test if option
      bool bMayBeFlag = false;   // if no option then test for flag or abreviated option
      if( pbszArgument[0] == '-' && pbszArgument[1] == '-' ) bOption = true;
      else if( is_single_dash() == true && pbszArgument[0] == '-' ) { bOption = true; bMayBeFlag = true; }

      if( bOption == true )                                                    // found option
      {
         bAllowPositional = false;                                             // no more positional arguments are allowed
         const char* pbszFindArgument = pbszArgument + 1;                      // move past first dash
         if( *pbszFindArgument == '-' ) pbszFindArgument++;                    // move to argument name
         poptionActive = find( pbszFindArgument );                             

         if(poptionActive == nullptr && bMayBeFlag == false )
         {
            /// Checks if parent options state is enabled and if parent option, then try to find value in parent
            if( is_parent() == true && poptionsRoot != nullptr ) poptionActive = poptionsRoot->find( pbszFindArgument );
         }
         
         // ## if no option found and single dash options is not allowed 
         if( poptionActive != nullptr )
         {
            if( poptionActive == nullptr && is_flag( eFlagUnchecked ) == false )// unknown argument and we do not allow unknown arguments?
            {
               return error_s( { "Unknown option : ", pbszArgument } );
            }

            // ## check for flag if single dash is allowed
            if( bMayBeFlag == true && poptionActive->is_flag() == true )
            {
               add_value( poptionActive, true );
               iOptionState = state_unknown;
               continue;
            }

            iOptionState = state_option;                                       // set function state to `option`

            // ## option values should hold a matching value, read value and add option
            if( iOptionState == state_option )
            {
               iPosition++;
               if( iPosition != iArgumentCount )
               {
                  const char* pbszValue = ppbszArgumentValue[iPosition];
                  if( poptionActive != nullptr ) add_value( poptionActive, pbszValue );// add value for option if option is found (when all options are allowed it could be option that do not exist)
                  iOptionState = state_unknown;
               }
               else
               {
                  return error_s( { "miss match arguments and values: ", pbszArgument} );
               }
            }

            continue;                                                          // continue with loop
         }
      }
      
      if( pbszArgument[0] == '-'  )                                            // find abbreviated option
      {
         pbszArgument++;                                                       // move to character
         bAllowPositional = false;                                             // no more positional arguments are allowed

         // ## try to find option flag
         poptionActive = find( pbszArgument );
         if( poptionActive == nullptr && poptionsRoot != nullptr && is_parent() == true )
         {
            poptionActive = poptionsRoot->find( pbszArgument );
         }

         if( poptionActive != nullptr )
         {
            add_value( poptionActive, true );
         }
         else
         {
            // ## It could be multiple options packed after "-" used as flags so we check for that
            while( *pbszArgument &&
               *pbszArgument > ' ' &&
               (poptionActive = find( *pbszArgument )) != nullptr )
            {
               if( poptionActive == nullptr && is_flag( eFlagUnchecked ) == false )// unknown option and unknown names is not allowed ?
               {
                  return error_s( { "Unknown option : ", pbszArgument } );
               }

               if( poptionActive->is_flag() == true )                          // Is it a flag then add value
               {
                  add_value( poptionActive, true );
               }
               else
               {
                  //iOptionState = state_option;                                    // We have one active option, next might be values for this option 
               }
               pbszArgument++;                                                 // move to next character, could be more abbreviated options if flag option
            }

            if(poptionActive == nullptr && is_flag(eFlagUnchecked) == false) { return error_s( { "Unknown flag : ", ppbszArgumentValue[iPosition] } ); }
         }

         iOptionState = state_unknown;
      }
      else
      {
         if( poptionsRoot == nullptr )                                         // check for sub options ? if root is null it means that it can be sub command if any is added
         {
            options* poptions = sub_find( pbszArgument );
            if(poptions != nullptr)
            {  // ## found sub command, set to active and parse rest with rules from the sub command
               poptions->set_active();
               if( poptionsRoot == nullptr ) { poptionsRoot = this; }
               return poptions->parse( iArgumentCount - iPosition, &ppbszArgumentValue[iPosition], poptionsRoot );
            }
         }

         // ## try to set as positional argument
         if( iPositionalArgument == -1 ) iPositionalArgument = 0;

         if( bAllowPositional == true && (size_t)iPositionalArgument < size() )
         {
            const option* poptionPositional = at( iPositionalArgument );
            add_value( poptionPositional, pbszArgument );
            iPositionalArgument++;
         }
         else
         {
            if( iOptionState != state_option ) { return error_s( { "Order missmatch, value need option name to know what to do, current value is: ", pbszArgument } ); }

            if( poptionActive != nullptr )
            {
               add_value( poptionActive, pbszArgument );
            }
            else { return error_s( { "No active option for value: ", pbszArgument} ); }
         }
      }
   }


   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief parse complete string similar to parsing arguments passed to applications executed in console
 * @param stringArgument string with arguments passed
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> options::parse(const std::string_view& stringArgument)
{
   std::vector< std::string > vectorArgument;
   std::pair<bool, std::string> result_ = parse_s(stringArgument, vectorArgument); // parse string into vector of strings 

   if( result_.first == false ) return result_;                               // if error then return error

   const char** ppbszArgument = nullptr;
   
   if(vectorArgument.empty() == false)
   {
      ppbszArgument = new const char*[vectorArgument.size()];                  // allocate pointer to pointer buffer to point to all found parts in string.

      for(auto it = std::begin(vectorArgument), itEnd = std::end(vectorArgument); it != itEnd; it++)
      {
         ppbszArgument[std::distance( std::begin(vectorArgument), it )] = it->c_str();
      }

      result_ = parse( (int)vectorArgument.size(), ppbszArgument, nullptr );


      delete [] ppbszArgument;
   }

   return result_;
}


/** ---------------------------------------------------------------------------
* @brief parse complete string similar to parsing arguments passed to applications executed in console
* @param stringArgument string with arguments passed, formatted like command line arguments
* @return true if ok, false and error information on error
*/
std::pair<bool, std::string> options::parse_terminal(const std::string_view& stringArgument)
{
   std::vector< std::string > vectorArgument;
   std::pair<bool, std::string> result_ = parse_terminal_s(stringArgument, vectorArgument); // parse string into vector of strings 

   if( result_.first == false ) return result_;                               // if error then return error

   const char** ppbszArgument = nullptr;

   if(vectorArgument.empty() == false)
   {
      ppbszArgument = new const char*[vectorArgument.size()];                  // allocate pointer to pointer buffer to point to all found parts in string.

      for(auto it = std::begin(vectorArgument), itEnd = std::end(vectorArgument); it != itEnd; it++)
      {
         ppbszArgument[std::distance( std::begin(vectorArgument), it )] = it->c_str();
      }

      result_ = parse( (int)vectorArgument.size(), ppbszArgument, nullptr );


      delete [] ppbszArgument;
   }

   return result_;
}



/** ---------------------------------------------------------------------------
 * @brief Parse command line argument string into vector of individual arguments
 * 
 * Handles quoted arguments, escaped characters, and whitespace separation.
 * Supports both single and double quotes with proper escaping.
 * 
 * @param stringCommandLine The command line string to parse
 * @param vectorArguments Vector to store parsed arguments
 * @return std::pair<bool, std::string> indicating success or failure with error message if any
 */
std::pair<bool, std::string> options::parse_s(const std::string_view& stringCommandLine, std::vector<std::string>& vectorArguments )
{
   std::string stringCurrentArgument;

   enum class enumState 
   {
      eStateNormal,          // Outside quotes
      eStateDoubleQuoted,   // Inside double quotes
      eStateSingleQuoted    // Inside single quotes
   };

   enumState eState = enumState::eStateNormal;
   bool bEscapeNext = false;

   // ## Parse the command line string character by character

   for( auto uPosition = 0u; uPosition < stringCommandLine.length(); uPosition++ )
   {
      char iCharacter = stringCommandLine[uPosition];

      // ## Handle escaped character
      if( bEscapeNext == true )
      {
         stringCurrentArgument += iCharacter;
         bEscapeNext = false;
         continue;
      }

      switch( eState )
      {
      case enumState::eStateNormal:
         if( iCharacter == '\\' ) { bEscapeNext = true; }
         else if( iCharacter == '"' ) { eState = enumState::eStateDoubleQuoted; }
         else if( iCharacter == '\'' ) { eState = enumState::eStateSingleQuoted; }
         else if( isspace(iCharacter) != 0 )
         {
            if( stringCurrentArgument.empty() == false )                      // End of current argument
            {
               vectorArguments.push_back(stringCurrentArgument);
               stringCurrentArgument.clear();
            }
            // Skip consecutive whitespace
            while( uPosition + 1 < stringCommandLine.length() && isspace(stringCommandLine[uPosition + 1]) != 0 ) { uPosition++; }
         }
         else { stringCurrentArgument += iCharacter; }
         break;

      case enumState::eStateDoubleQuoted:
         if( iCharacter == '\\' ) { bEscapeNext = true;  }
         else if( iCharacter == '"' ) {eState = enumState::eStateNormal; }
         else { stringCurrentArgument += iCharacter; }
         break;

      case enumState::eStateSingleQuoted:
         if( iCharacter == '\'' ) { eState = enumState::eStateNormal; }
         else { stringCurrentArgument += iCharacter; }                        // In single quotes, everything is literal (no escaping)
         break;
      }
   }

   // Check for unmatched quotes
   if( eState != enumState::eStateNormal )
   {
      return { false, "Unmatched quotes in command line" };                   // Return error if quotes are unmatched
   }

   // Add final argument if any
   if( stringCurrentArgument.empty() == false )
   {
      vectorArguments.push_back(stringCurrentArgument);
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Parse command line argument string into vector of individual arguments
 * 
 * Handles quoted arguments, escaped characters, and whitespace separation following
 * POSIX shell parsing rules more closely. Supports both single and double quotes
 * with proper escaping behavior that matches terminal shells.
 * 
 * @param stringCommandLine The command line string to parse
 * @param vectorArguments Vector to store parsed arguments (will be cleared first)
 * @return std::pair<bool, std::string> indicating success or failure with error message if any
 */
std::pair<bool, std::string> options::parse_terminal_s(const std::string_view& stringCommandLine, std::vector<std::string>& vectorArguments)
{
   vectorArguments.clear(); // Clear existing arguments

   if( stringCommandLine.empty() == true ) { return { true, "" }; }

   std::string stringCurrentArgument;
   enum class enumState
   {
      eStateNormal,          // Outside quotes
      eStateDoubleQuoted,   // Inside double quotes
      eStateSingleQuoted    // Inside single quotes
   };

   enumState eState = enumState::eStateNormal;
   bool bEscapeNext = false;

   // Parse the command line string character by character
   for( size_t uPosition = 0; uPosition < stringCommandLine.length(); ++uPosition )
   {
      char iCharacter = stringCommandLine[uPosition];

      // Handle escaped character
      if( bEscapeNext )
      {
         // In terminal, some escape sequences have special meaning
         switch( iCharacter )
         {
         case 'n':  stringCurrentArgument += '\n'; break;
         case 't':  stringCurrentArgument += '\t'; break;
         case 'r':  stringCurrentArgument += '\r'; break;
         case '\\': stringCurrentArgument += '\\'; break;
         case '"':  stringCurrentArgument += '"'; break;
         case '\'': stringCurrentArgument += '\''; break;
         case ' ':  stringCurrentArgument += ' '; break;
         default:
            // For other characters, include the backslash (terminal behavior)
            stringCurrentArgument += '\\';
            stringCurrentArgument += iCharacter;
            break;
         }
         bEscapeNext = false;
         continue;
      }

      switch( eState )
      {
      case enumState::eStateNormal:
         if( iCharacter == '\\' ) { bEscapeNext = true; }
         else if( iCharacter == '"' ) { eState = enumState::eStateDoubleQuoted; }
         else if( iCharacter == '\'' ) { eState = enumState::eStateSingleQuoted; }
         else if( std::isspace(static_cast<unsigned char>( iCharacter )) )
         {
            if( !stringCurrentArgument.empty() )
            {
               vectorArguments.push_back(stringCurrentArgument);
               stringCurrentArgument.clear();
            }
            // Skip consecutive whitespace
            while( uPosition + 1 < stringCommandLine.length() &&
               std::isspace(static_cast<unsigned char>( stringCommandLine[uPosition + 1] )) )
            {
               ++uPosition;
            }
         }
         else
         {
            stringCurrentArgument += iCharacter;
         }
         break;

      case enumState::eStateDoubleQuoted:
         if( iCharacter == '\\' )
         {
            // In double quotes, only certain characters can be escaped
            if( uPosition + 1 < stringCommandLine.length() )
            {
               char iNextChar = stringCommandLine[uPosition + 1];
               if( iNextChar == '"' || iNextChar == '\\' || iNextChar == '$' ||
                   iNextChar == '`' || iNextChar == '\n' )
               {
                  bEscapeNext = true;
               }
               else
               {
                  // Backslash is literal if not followed by escapable character
                  stringCurrentArgument += iCharacter;
               }
            }
            else
            {
               // Backslash at end of string is literal
               stringCurrentArgument += iCharacter;
            }
         }
         else if( iCharacter == '"' )
         {
            eState = enumState::eStateNormal;
         }
         else
         {
            stringCurrentArgument += iCharacter;
         }
         break;

      case enumState::eStateSingleQuoted:
         if( iCharacter == '\'' )
         {
            eState = enumState::eStateNormal;
         }
         else
         {
            // In single quotes, everything is literal (no escaping possible)
            stringCurrentArgument += iCharacter;
         }
         break;
      }
   }

   // Check for unmatched quotes
   if( eState != enumState::eStateNormal )
   {
      std::string stringError = ( eState == enumState::eStateDoubleQuoted ) ?
         "Unmatched double quote in command line" :
         "Unmatched single quote in command line";
      return { false, stringError };
   }

   // Check for trailing escape
   if( bEscapeNext ) { return { false, "Trailing escape character in command line" }; }

   // Add final argument if any
   if( stringCurrentArgument.empty() == false ) { vectorArguments.push_back(stringCurrentArgument); }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief parse command line argument string into vector of individual arguments
 * @param stringCommandLine The command line string to parse
 * @return std::vector<std::string> vector with parsed arguments
 */
std::vector<std::string> options::parse_s(const std::string_view& stringCommandLine )
{
   std::vector<std::string> vectorArguments;
   std::pair<bool, std::string> result_ = parse_s(stringCommandLine, vectorArguments); // parse string into vector of strings

   if( result_.first == false ) return {};                                    // if error then return error

   return vectorArguments;
}



/** ---------------------------------------------------------------------------
 * @brief parse vector with string objects acting similar to parsing arguments passed to applications executed in console
 * @param vectorArgument vector with string to parse 
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> options::parse(const std::vector<std::string>& vectorArgument)
{                                                                                                  assert( vectorArgument.empty() == false );
   const char** ppbszArgument = nullptr;
   std::pair<bool, std::string> result_( true, "" );

   // allocate pointer to pointer buffer to point to all found parts in string.
   // the custom deleter will delete the buffer when it goes out of scope
   // this should mimic the argc and argv from main function
   std::unique_ptr<const char*, decltype([](auto p_){ delete [] p_; } )> ppArguments( new const char*[vectorArgument.size()]);

   for(auto it = std::begin(vectorArgument), itEnd = std::end(vectorArgument); it != itEnd; it++)
   {
      // set pointer to string in vector
      ppArguments.get()[std::distance( std::begin(vectorArgument), it )] = it->c_str();
   }

   ppbszArgument = ppArguments.get();                                          // get pointer to pointer buffer, similar to argv
   result_ = parse( (int)vectorArgument.size(), ppbszArgument, nullptr );

   return result_;
}

/** ---------------------------------------------------------------------------
 * @brief convert current options and values to string that can be used to start application with same options
 * @return std::string with options and values
 */
std::string options::to_string() const
{
   using namespace gd::argument;
   std::string stringResult;

   // ## convert all options and values to string .............................

   std::vector<std::string> vectorOption;
   for( auto it = m_argumentsValue.named_begin(), itEnd = m_argumentsValue.named_end(); it != itEnd; it++ )
   {
      auto [stringName, argument_] = *it;
      if( argument_.is_bool() == false )
      {
         vectorOption.push_back(std::string("--") + std::string(stringName)); // add option name
         vectorOption.push_back(argument_.to_string());                      // add option value
      }
      else                                                                    // only add boolean options that are true
      {
         vectorOption.push_back(std::string("-") + std::string(stringName));  // add flag
      }
   }

   // ## get pointers to simulate char* const* argv ...........................

   std::vector<const char*> vectorOptionPointer;
   for( const auto& it : vectorOption )
   {
      vectorOptionPointer.push_back( it.c_str() );
   }

   stringResult = to_string_s( (unsigned)vectorOptionPointer.size(), vectorOptionPointer.data() );
   return stringResult;
}


/// return option value if found as variant
gd::variant options::get_variant( const std::string_view& stringName ) const 
{                                                                                                  assert( stringName.empty() == false );
   auto value_ = m_argumentsValue[stringName].get_variant();
   return value_;
}

/// return option value if found for name
gd::variant_view options::get_variant_view( const std::string_view& stringName ) const noexcept
{                                                                                                  assert( stringName.empty() == false );
   auto value_ = m_argumentsValue[stringName].get_variant_view();
   return value_;
}

/// return option value if found for name, if sub command is active then search in sub command
gd::variant_view options::get_variant_view( const std::string_view& stringName, gd::types::tag_state_active ) const noexcept
{                                                                                                  assert( stringName.empty() == false );
   const options* poptions_ = sub_find_active();
   if( poptions_ != nullptr )
   {
      return poptions_->get_variant_view( stringName );
   }
   auto value_ = m_argumentsValue[stringName].get_variant_view();
   return value_;
}

/// return option value if found for name
gd::variant_view options::get_variant_view( const std::string_view* ptringName ) const noexcept
{                                                                                                  assert( ptringName->empty() == false );
   auto value_ = m_argumentsValue[*ptringName].get_variant_view();
   return value_;
}

/// return option value based on index if multiple options with same name
gd::variant_view options::get_variant_view( const std::string_view& stringName, unsigned uIndex ) const noexcept
{                                                                                                  assert( stringName.empty() == false );
   auto value_ = m_argumentsValue.find_argument( stringName, uIndex ).as_variant_view();
   return value_;
}

/// return option value based on index if multiple options with same name
gd::variant_view options::get_variant_view( const std::string_view* pstringName, unsigned uIndex ) const noexcept
{                                                                                                  assert( pstringName->empty() == false );
   auto value_ = m_argumentsValue.find_argument( *pstringName, uIndex ).as_variant_view();
   return value_;
}

/// return option value for first found name or empty value if not found
gd::variant_view options::get_variant_view(const std::initializer_list<std::string_view>& listName) const noexcept
{
   for(auto it = listName.begin(); it != listName.end(); it++)
   {
      gd::variant_view v_ = get_variant_view( it );
      if( v_.empty() == false ) return v_;
   }

   return gd::variant_view();
}

/** ---------------------------------------------------------------------------
 * @brief get all values for name as variant's in list
 * @param stringName collected values for name
 * @return std::vector<gd::variant> list of variants with values from name
*/
std::vector<gd::variant> options::get_variant_all( const std::string_view& stringName ) const
{                                                                                                  assert( stringName.empty() == false );
   auto values_ = m_argumentsValue.get_argument_all( stringName );             // get list with argument't
   auto variants_ = gd::argument::arguments::get_variant_s( values_ );         // convert list with argument's to list of variant's
   return variants_;
}

/** ---------------------------------------------------------------------------
 * @brief get all values for name as variant_view's in list
 * @param stringName collected values for name
 * @return std::vector<gd::variant_view> list of variant_view's with values from name
*/
std::vector<gd::variant_view> options::get_variant_view_all( const std::string_view& stringName ) const
{                                                                                                  assert( stringName.empty() == false );
   auto values_ = m_argumentsValue.get_argument_all( stringName );             // get list with argument't
   auto variants_ = gd::argument::arguments::get_variant_view_s( values_ );    // convert list with argument's to list of variant_view's
   return variants_;
}

/** ---------------------------------------------------------------------------
 * @brief Try to find value
 * 
 * @code
   gd::cli::options optionsConfiguration;
   optionsConfiguration.add( {"database", "describe how to connect to main database"});

   // parse command line arguments
   auto [bOk, stringError] = optionsConfiguration.parse( iArgumentCount, ppbszArgument );
   if( bOk == false ) { return { bOk, stringError }; }

   if( optionsConfiguration.find( "database", "print" ) == true ) // found "print" among database arguments?               
   { 
	   auto vectorDatabase = optionsConfiguration.get_variant_view_all("database");
      // ...
   }
 * @endcode 
 * 
 * @param stringName option name value is attached to
 * @param variantviewValue value to find
 * @return true if value is found, false if not
*/
bool options::find( const std::string_view& stringName, const gd::variant_view& variantviewValue ) const
{
   auto values_ = get_variant_view_all( stringName );
   for( auto it = std::begin( values_ ), itEnd = std::end( values_ ); it != itEnd; it++ )
   {
      if( variantviewValue.compare( *it ) == true ) return true;
   }
   return false;
}

/// ---------------------------------------------------------------------------
/// @brief  if value for name is found then call callback method passing value as reference
/// @param stringName name for value to check if found
/// @param callback_ callback method to execute with value if value is found
/// @return true if value was faound
bool options::iif( const std::string_view& stringName, std::function< void( const gd::variant_view& ) > callback_ ) const
{
   gd::variant_view variantviewValue = get_variant_view( stringName );
   if( variantviewValue.is_true() == true )
   {
      callback_( variantviewValue );
      return true;
   }
   
   return false;
}

void options::iif( const std::string_view& stringName, std::function< void( const gd::variant_view& ) > true_, std::function< void( const gd::variant_view& ) > false_ ) const
{
   gd::variant_view variantviewValue = get_variant_view( stringName );
   if( variantviewValue.is_true() == true )
   {
      true_( variantviewValue );
   }
   else
   {
      false_( variantviewValue );
   }
}


/** ---------------------------------------------------------------------------
 * @brief Check if an option exists and is set.
 *
 * This method checks if the specified option name exists and is set in the current options context.
 * If there is an active subcommand, it checks within the active subcommand; otherwise, it checks in the current options.
 *
 * @param stringName The name of the option to check.
 * @param gd::types::tag_state_active Tag to indicate checking in the active subcommand if present.
 * @return true if the option exists and is set, false otherwise.
 *
 * @code
 * gd::cli::options cli_options;
 * cli_options.add({ "input", 'i', "Input file" });
 * cli_options.add({ "output", 'o', "Output file" });
 * 
 * // Parse command-line arguments
 * auto [ok, err] = cli_options.parse(argc, argv);
 * if(!ok) { std::cerr << err << std::endl; return 1; }
 * 
 * // Check if "input" option exists (is set)
 * if(cli_options.exists("input", gd::types::tag_state_active{})) {
 *     std::cout << "Input option is set." << std::endl;
 * }
 * @endcode
 */
bool options::exists( const std::string_view stringName, gd::types::tag_state_active ) const noexcept
{                                                                                                  assert( stringName.empty() == false );
   const auto* poptions_ = sub_find_active();
   if( poptions_ != nullptr )
   {
      return poptions_->exists( stringName );
   }

   return exists( stringName );
}



/** ---------------------------------------------------------------------------
 * @brief Generate documentation 
 * 
 * Documentation is generated in a in columns or almost as columns
 * 
 * Prints information about commands and arguments for each command
 * @param stringDocumentation reference to string getting documentation text
*/
void options::print_documentation( std::string& stringDocumentation, tag_documentation_table ) const
{
   std::string stringLine;
   std::string stringPrint;   /// 

   for( auto it : m_vectorOption )
   {
      stringLine = "[";
      stringLine += it.name();

      if( stringLine.size() < 25 ) stringLine.append( 25 - stringLine.size(), ' ' );
      stringLine += "]   *";
      stringLine += it.description();
      stringLine += "*\n";

      stringPrint += stringLine;
   }

   stringDocumentation += stringPrint;

   for(const auto& it : m_vectorSubOption)
   {
      stringLine = "\n\n## ";
      stringLine += it.name();
      stringLine += "   *";
      stringLine += it.description();
      stringLine += "*\n- - - - - - - - - - - - - - - - - - - - - - - - -\n";
      stringDocumentation += stringLine;
      it.print_documentation( stringDocumentation, tag_documentation_table{});
   }
}

/** ---------------------------------------------------------------------------
 * @brief Generate documentation
 * 
 * Prints information not using to many lines, a bit denser
 * 
 * @param stringDocumentation reference to string getting documentation text
 */
void options::print_documentation( std::string& stringDocumentation, tag_documentation_dense ) const
{
   constexpr size_t uTotalColumnWidth = 80; // Total width of the output
   constexpr size_t uNameColumnWidth = 25; // Adjust column width for option names
   constexpr size_t uDescriptionColumnWidth = uTotalColumnWidth - uNameColumnWidth; // Adjust column width for descriptions
   constexpr char chSeparator = '-';      // Separator for sections

   std::string string_; // temporary string for formatting
   std::ostringstream OSSDocumentation; // String stream used to produce documentation

   OSSDocumentation << std::string(uTotalColumnWidth, chSeparator) << "\n"; // Dynamic separator line

   std::string stringName = name(); // Get the name of the command

   if( stringName.empty() == false )
   {
      OSSDocumentation << "\n"; // Blank line before command name
      string_ = "## " + stringName; // Command name as a section header
      string_.append(uNameColumnWidth - string_.size(), ' ');
      OSSDocumentation << string_ ; // Command name as a section header
      OSSDocumentation << description() << "\n"; // Description of the command
      OSSDocumentation << "\n"; // Blank line before command name
   }

   // Header for options
   string_ = "command options ";
   string_.append(uNameColumnWidth - string_.size(), ' ');
   OSSDocumentation << string_;
   OSSDocumentation << std::string(uNameColumnWidth + uDescriptionColumnWidth - string_.length(), chSeparator) << "\n"; // Dynamic separator line

   // Format and print each option
   for(const auto& option : m_vectorOption)
   {
      std::string stringName = "[" + std::string(option.name()) + "]";
      if(stringName.size() < uNameColumnWidth)
      {
         stringName.append(uNameColumnWidth - stringName.size(), ' '); // Align names to column width
      }

      OSSDocumentation << stringName << option.description() << "\n";
   }

   if( get_parent() != nullptr )
   {
      string_ = "global options";
      string_.append(uNameColumnWidth - string_.size(), ' ');
      OSSDocumentation << "\n" << string_;
      OSSDocumentation << std::string(uNameColumnWidth + uDescriptionColumnWidth - string_.length(), chSeparator) << "\n"; // Dynamic separator line

      // Format and print each option from the parent
      for(const auto& option : get_parent()->m_vectorOption)
      {
         std::string stringName = "[" + std::string(option.name()) + "]";
         if(stringName.size() < uNameColumnWidth)
         {
            stringName.append(uNameColumnWidth - stringName.size(), ' '); // Align names to column width
         }
         OSSDocumentation << stringName << option.description() << "\n";
      }
   }

   // Append the formatted string to the provided documentation
   stringDocumentation += OSSDocumentation.str();

}

void options::print_documentation( std::function<void(unsigned uType, std::string_view, std::string_view, const option*)> callback_ ) const
{
   callback_( eOptionTypeCommand, m_stringName, m_stringDescription, nullptr );

   for( auto it : m_vectorOption )
   {
      unsigned uType = eOptionTypeOption;
      if( it.is_flag() == true ) { uType = eOptionTypeFlag; }                 // if flag then set type to flag

      callback_( uType, it.name(), it.description(), &it );
   }

   for(const auto& it : m_vectorSubOption)
   {
      it.print_documentation( callback_ );
   }
}

void options::print_documentation(std::string& stringDocumentation, tag_documentation_verbose) const
{
   // ## Header
   stringDocumentation += "\n";
   stringDocumentation += "HELP - Command Line Documentation\n";
   stringDocumentation += "=================================\n";

   stringDocumentation += "\nGlobal options:\n------------------------------\n";

   // ## Global options (e.g., logging, print)
   for(const auto& option : m_vectorOption)
   {
      stringDocumentation += option.name().data() + std::string("\n");

      // Description (indented)
      stringDocumentation += std::string( "   " ) + option.description().data() + std::string( "\n" );
   }

   stringDocumentation += "\n\nCommands:\n------------------------------\n";

   // Sub-options (e.g., count, copy)
   for(const auto& option_ : m_vectorSubOption)
   {
      stringDocumentation += "## " + option_.name() + "\n";                    // Sub-option (global option value) name as a section header
      stringDocumentation += "    " + option_.description() + "\n";            // Description (indented)

      print_suboption_options(option_, stringDocumentation);

      stringDocumentation += "\n";
   }

   stringDocumentation += "=================================\n";               // footer
}

// Helper function to print options for a sub-option (e.g., count’s source, pattern)
void options::print_suboption_options(const options& optionsSub, std::string& stringDocumentation) const
{
   bool bHasOptions = false;
   size_t uMaxOptionNameLength = 15; // For aligning option names

   // Calculate max length for alignment (optional, for better formatting)
   for(const auto& option : optionsSub)                                           // Adjust based on actual method
   {
      uMaxOptionNameLength = std::max(uMaxOptionNameLength, option.name().size());
   }

   for(const auto& option : optionsSub)                                           // Adjust based on actual method
   {
      if(bHasOptions == false)
      {
         stringDocumentation += "  Options:\n";
         bHasOptions = true;
      }

      std::string stringName( option.name() );
      std::string stringDescription( option.description() );

      // Format option
      stringDocumentation += "    - " + stringName;
      stringDocumentation += std::string(uMaxOptionNameLength - stringName.size() + 2, ' ');
      stringDocumentation += stringDescription + "\n";
   }
}


bool options::sub_exists(const std::string_view& stringName)
{
   for(auto it = std::begin( m_vectorSubOption ), itEnd = std::end( m_vectorSubOption ); it != itEnd; it++)
   {
      if( it->name() == stringName ) return true;
   }
   return false;
}

/// Check if sub command is active, true if active, false if not active
bool options::sub_is_active(const std::string_view& stringName) const
{
   for(auto it = std::begin( m_vectorSubOption ), itEnd = std::end( m_vectorSubOption ); it != itEnd; it++)
   {
      if( it->name() == stringName && it->is_active() ) return true;
   }

   return false;
}

/// Get pointer to active sub command if any, nullpointer is returned if no active sub command
const options* options::sub_find_active() const
{
   for(auto it = std::begin( m_vectorSubOption ), itEnd = std::end( m_vectorSubOption ); it != itEnd; it++)
   {
      if( it->is_active() ) return &(*it);
   }
   return nullptr;
}

/// Get pointer to active sub command if any, nullpointer is returned if no active sub command (non const version)
options* options::sub_find_active()
{
   for(auto it = std::begin( m_vectorSubOption ), itEnd = std::end( m_vectorSubOption ); it != itEnd; it++)
   {
      if( it->is_active() ) return &(*it);
   }
   return nullptr;
}


/// Return name for active sub command if any active is found, no active sub command returns empty string
std::string_view options::sub_find_active_name() const
{
   for(auto it = std::begin( m_vectorSubOption ), itEnd = std::end( m_vectorSubOption ); it != itEnd; it++)
   {
      if( it->is_active() ) return it->name();
   }
   return std::string_view();
}

/// Find sub command for specified name
options* options::sub_find(const std::string_view& stringName)
{
   for(auto it = std::begin( m_vectorSubOption ), itEnd = std::end( m_vectorSubOption ); it != itEnd; it++)
   {
      if( it->name() == stringName ) return &(*it);
   }
   return nullptr;
}

/// count number of active sub commands
size_t options::sub_count_active() const
{
   size_t uCount = 0;
   for(auto it = std::begin( m_vectorSubOption ), itEnd = std::end( m_vectorSubOption ); it != itEnd; it++)
   {
      if( it->is_active() ) uCount++;
   }
   return uCount;
}

const options* options::sub_find(const std::string_view& stringName) const
{
   for(auto it = std::begin( m_vectorSubOption ), itEnd = std::end( m_vectorSubOption ); it != itEnd; it++)
   {
      if( it->name() == stringName ) return &(*it);
   }
   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief return sub options with values if any is added
 * @param stringName name for sub options to return
 * @return options sub options object with values
 */
options options::sub_get( const std::string_view& stringName ) const
{
   const options* poptions = sub_find( stringName );
   if( poptions != nullptr )
   {
      options optionsSub( *poptions );
      optionsSub.add_global( *this );

      // ## if values within sub options is empty then add those from parent options (this)
      if( m_argumentsValue.empty() == true ) { optionsSub.set( poptions->m_argumentsValue ); }

      return optionsSub;
   }

   return options();
}

/** ---------------------------------------------------------------------------
 * @brief convert argument values to string, reconstruct command-line arguments into a string 
 * 
 * Convert all arguments to string and add to stringArguments that is returned.
 * Useful for printing command line arguments to application
 * 
 * @param iCount number of arguments
 * @param ppbszArgumentValue pointer to charpointers (array of pointers)
 * @param iOffset offset for first argument to convert
 * @return std::string with all arguments converted to string
 */
std::string options::to_string_s(int iCount, const char* const* ppbszArgumentValue, int iOffset)
{                                                                                                  assert( iCount >= 0 );
   std::string stringArguments; // generated string with arguments


   // ## Go through all arguments and add to string
   for( int iPosition = iOffset; iPosition < iCount; iPosition++ )
   {
      if(ppbszArgumentValue[iPosition] == nullptr) continue;                  // Skip null pointers

      bool bQuote = false; // if space or special characters are found then add quotes
      auto uLength = (int)strlen(ppbszArgumentValue[iPosition]);
      const auto* pbszArgument = ppbszArgumentValue[iPosition];

      // ## Check if argument needs to be quoted
      for( int i = 0; i != uLength; i++ )
      {
         char iCharacter = pbszArgument[i];
         if(isspace(static_cast<unsigned char>(iCharacter)) || 
            iCharacter == '"' || iCharacter == '\'' || iCharacter == '\\' || 
            iCharacter == '(' || iCharacter == ')' || iCharacter == '|' || 
            iCharacter == '&' || iCharacter == ';' || iCharacter == '<' || 
            iCharacter == '>' || iCharacter == '*' || iCharacter == '?' || 
            iCharacter == '[' || iCharacter == ']' || iCharacter == '{' || 
            iCharacter == '}' || iCharacter == '$' || iCharacter == '`')
         { bQuote = true; break; }
      }

      if( stringArguments.empty() == false ) stringArguments += " ";

      if( bQuote == true )                                                     // if space or special characters are found then add quotes
      {
         stringArguments += "\"";                                              // add quote
         for( decltype(uLength) u = 0; u != uLength; u++ )
         {
            char iCharacter = ppbszArgumentValue[iPosition][u];
            if(iCharacter == '"' || iCharacter == '\\') 
            {
               stringArguments += '\\';                                        // escape special characters
            }
            
            stringArguments += iCharacter;
         }
         stringArguments += "\"";                                              // add quote
      }
      else
      {
         stringArguments += ppbszArgumentValue[iPosition];                     // no special characters, just add
      }
   }

   return stringArguments;
}

std::string options::to_string_s(const options* poptions_, int iOffset)
{
   std::string stringArguments; // generated string with arguments
   return stringArguments;
}


/** ---------------------------------------------------------------------------
 * @brief Generate error message and return a pair that works for other methods
 * @param listPrint list of values converted to generated text
 * @return std::pair<bool, std::string> false and generated text
*/
std::pair<bool, std::string> options::error_s( std::initializer_list<gd::variant_view> listPrint )
{
   std::string stringError;
   for( const auto& it : listPrint )
   {
      stringError += it.as_string();
   }
   
   return { false, stringError };
}


void options::option::set_name( const std::string_view& stringName )
{                                                                                                  assert( stringName.length() > 0 );
   const char* pbszName = stringName.data();
   if( stringName.length() > 2 && stringName[1] == ',' )
   {                                                                                               assert( stringName.length() > 3 ); // need to be over "X,X"
      pbszName += 2;
      m_chLetter = stringName[0];
   }

   m_stringName = pbszName;
}


_GD_CLI_END

