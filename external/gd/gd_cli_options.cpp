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
         if( *pbszFindArgument == '-' ) pbszFindArgument++;
         poptionActive = find( pbszFindArgument );                             // move to argument name

         if(poptionActive == nullptr && bMayBeFlag == false )
         {
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
 * @param stringSplit string that splits arguments
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> options::parse(const std::string_view& stringArgument, const std::string_view& stringSplit)
{
   std::pair<bool, std::string> result_( true, "" );
   const char** ppbszArgument = nullptr;
   std::vector< std::string > vectorArgument;
   gd::parse::split_g( stringArgument, stringSplit, vectorArgument, gd::parse::csv{});

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
 * @brief parse vector with string objects acting similar to parsing arguments passed to applications executed in console
 * @param vectorArgument vector with string to parse 
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> options::parse(const std::vector<std::string>& vectorArgument)
{                                                                                                  assert( vectorArgument.empty() == false );
   const char** ppbszArgument = nullptr;
   std::pair<bool, std::string> result_( true, "" );

   std::unique_ptr<const char*, decltype([](auto p_){ delete [] p_; } )> ppArguments( new const char*[vectorArgument.size()]);

   for(auto it = std::begin(vectorArgument), itEnd = std::end(vectorArgument); it != itEnd; it++)
   {
      ppArguments.get()[std::distance( std::begin(vectorArgument), it )] = it->c_str();
   }

   ppbszArgument = ppArguments.get();
   result_ = parse( (int)vectorArgument.size(), ppbszArgument, nullptr );

   return result_;
}



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
bool options::iif( const std::string_view& stringName, std::function< void( const gd::variant_view& ) > callback_ )
{
   gd::variant_view variantviewValue = get_variant_view( stringName );
   if( variantviewValue.is_true() == true )
   {
      callback_( variantviewValue );
      return true;
   }
   
   return false;
}

void options::iif( const std::string_view& stringName, std::function< void( const gd::variant_view& ) > true_, std::function< void( const gd::variant_view& ) > false_ )
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
 * @brief Generate documentation 
 * Prints information about commands and arguments for each command
 * @param stringDocumentation reference to string getting documentation text
*/
void options::print_documentation( std::string& stringDocumentation ) const
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
      stringLine += " *";
      stringLine += it.description();
      stringLine += "*\n";
      stringDocumentation += stringLine;
      it.print_documentation( stringDocumentation );
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

