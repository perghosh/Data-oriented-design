#include <array>
#include <variant>

#include "gd/gd_utf8.h"
#include "gd_com_server.h"

_GD_BEGIN
namespace com { namespace server { namespace router {

// ================================================================================================
// ======================================================================================== command
// ================================================================================================

int32_t command::query_interface( const gd::com::guid& guidId, void** ppObject )
{
   return gd::com::S_Ok;
}

/// release decrease reference counter and if down to 0 object is deleted
unsigned command::release() 
{                                                                                                  assert( m_iReference > 0 );
   m_iReference--; 
   if( m_iReference == 0 )
   {
      delete this;
      return 0;
   }
   
   return (unsigned)m_iReference; 
}

/** ---------------------------------------------------------------------------
 * @brief Activates the next command in the sequence based on command priority.
 *
 * This method iterates through the argument vector to find and activate the next command
 * with priority flag ePriorityCommand. It maintains state in m_iCommandIndex.
 *
 * @return int Returns the index of the activated command, or -1 if no next command is found.
 */
int command::activate_next()  
{  
  int iCommandIndex = m_iCommandIndex;  
  for( const auto it : m_vectorArgument )  
  {  
     if( it.get_priority() & ePriorityCommand )  
     {  
        if( iCommandIndex == -1 ) { m_iCommandIndex = it.get_index(); return m_iCommandIndex; }  
        else if( iCommandIndex == it.get_index() ) { iCommandIndex = -1; }  
     }  
  }  
  return iCommandIndex;  
}

/** ---------------------------------------------------------------------------
 * @brief add arguments to internal list with arguments objects
 * command may hold a lot of arguments and these could be used depending on how
 * command's are packed. theare may be global, command arguments, stack or register
 * these type of arguments have different priorities, register is the first/highest
 * @param variantviewPriority pr
 * @param pargumentsVariable 
 * @return 
 */
std::pair<bool, std::string> command::add_arguments( const gd::variant_view& variantviewPriority, const gd::argument::arguments* pargumentsVariable ) 
{
   unsigned uPriority = ePriorityStack;

   if( variantviewPriority.is_string() == true )
   {
      uPriority = to_command_priority_g( variantviewPriority.as_string_view() );
   }
   else 
   {
      uPriority = variantviewPriority.as_uint();
   }

   if( (uPriority & ePriorityRegister) != 0 )                                  // register has top priority and only one register arguments is allowed
   {
      arguments_remove( ePriorityRegister );                                   // only one register item, remove rest
      m_vectorVariable.push_back(arguments(uPriority, *pargumentsVariable));
#ifndef NDEBUG
      [[maybe_unused]] std::string stringArguments_d = pargumentsVariable->print();
#endif
   }
   else
   {
      // ## loop backwards to speed (stack variables are most frequent to change here)
      //    Iterate from high (high priority has low number) to low
      bool bAdded = false;
      for(auto it = m_vectorArgument.rbegin(); it != m_vectorArgument.rend(); it++)
      {
         if(it->get_priority() >= uPriority)                                   // found priority value with higer number means that added should be inserted after
         {                                                                                         assert( !(uPriority & ePriorityRegister) ); // never register here, should be handled before
            m_vectorArgument.insert( it.base() + 1, arguments(uPriority, *pargumentsVariable));
            bAdded = true;
            break;
         }
      }

      // ## argumnets wasnt added so we need to add it
      if( bAdded == false )
      {
         m_vectorArgument.insert( m_vectorArgument.begin(), arguments(uPriority, *pargumentsVariable) );
      }
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief find arguments for key
 * @param uIndexKey key to search for
 * @return pointer to arguments if found, nullptr if not found
 */
command::arguments* command::find_arguments( uint32_t uIndexKey )
{
   for( auto& itArguments : m_vectorArgument )
   {
      if( itArguments == uIndexKey ) { return &itArguments; }
   }
   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief remove arguments in vector with priority flag
 * @param uPriority flags for arguments to remove
 */
void command::arguments_remove(unsigned uPriority)
{
   /// loop backwards because it is a bit faster
   for( auto it = m_vectorVariable.rbegin(); it != m_vectorVariable.rend(); ) 
   {
      if( it->get_priority() & uPriority )
      {
         // tricky for removing backwards iterators
         it = std::vector<arguments>::reverse_iterator( m_vectorVariable.erase( (it + 1).base() ));
      }
      else
      {
         it++;
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief Parse value from string that is in query-string format
 * This method parses values similar to values passed in url like `level=0&level=1&level=2&format=xml`
 * @param variantviewLocality level for parsed values
 * @param stringQueryString string to parse
 * @return true if ok, false and error informaton if error
 */
std::pair<bool, std::string> command::add_querystring( const gd::variant_view& variantviewLocality, const std::string_view& stringQueryString )
{
   gd::argument::arguments argumentsQueryString;
   auto vectorArgument = gd::utf8::split_pair( stringQueryString, '=', '&', gd::utf8::tag_string{} );
   argumentsQueryString.append( vectorArgument, gd::argument::tag_parse_type{} );
   return add_arguments( variantviewLocality, &argumentsQueryString );
}

/** ---------------------------------------------------------------------------
 * @brief Wrapper to extract command and arguments from url
 * @param stringQueryString url formated string used to extract command and arguments from
 * @return std::vector<std::string_view> returns vector with command parts
 */
std::vector<std::string_view> command::add_querystring(const std::string_view& stringQueryString) 
{
   std::vector<std::string_view> vectorCommand;
   std::string_view stringCommand;
   auto position_ = stringQueryString.find('?');
   if( position_ != std::string_view::npos )
   {
      stringCommand = stringQueryString.substr( 0, position_ );
      std::string_view stringArguments( stringQueryString.substr( position_ + 1 ) );
      add_querystring( ePriorityRegister, stringArguments );                   // query string arguments have the highest priority
   }
   else
   {
      stringCommand = stringQueryString;
   }

   vectorCommand = gd::utf8::split( stringCommand, '/' );

   return vectorCommand;
}

/** ---------------------------------------------------------------------------
 * @brief Append stack variables to command object
 * 
 * @param arguments_ object with variables to add with stack priority
 * @return true if ok, false and error informaton if error
 */
std::pair<bool, std::string> command::append(const gd::argument::arguments& arguments_, gd::types::tag_variable)
{
   m_vectorVariable.push_back(arguments( ePriorityStack, arguments_ ));
   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Append variables to command object
 * 
 * Command object may hold 0 or more commands, each command may have it's own arguments.
 * But the command object may also hold variables that are used for all commands. These variables
 * are stored in a separate vector and are associated with different priorities.
 * @param uPriority priority for variables added
 * @param arguments_ object with variables to add
 * @return true if ok, false and error informaton if error
 */
std::pair<bool, std::string> command::append(uint32_t uPriority, const gd::argument::arguments& arguments_, gd::types::tag_variable)
{
   m_vectorVariable.push_back(arguments(uPriority, arguments_));
   return { true, "" };
}

std::pair<bool, std::string> command::add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments* pargumentsLocal )
{
   int iIndex = (int)m_vectorArgument.size();
   m_vectorArgument.push_back( arguments( stringKey, stringCommand, *pargumentsLocal ) );
   m_vectorArgument.back().set_index( iIndex );
   return { true, "" };
}

/** --------------------------------------------------------------------------
 * @brief Appends a command and its arguments parsed from a query string to this command object.
 *
 * This function parses a query string in URI format, separates the command path from 
 * arguments, splits them into components, handles URL encoding for argument values, 
 * and appends these to the current command structure.
 *
 * @param stringQueryString A string view containing the query string in URI format.
 * @param arguments_ additional arguments to append to the command.
 * @param tag_uri An tag dispatcher indicating that this is a URI formatted query string.
 * @return std::pair<bool, std::string> Returns a pair where the boolean indicates success 
 *         (always true in this implementation) and the string is empty or could contain 
 *         an error message if needed for future extensions.
 *
 * @details 
 * - Splits the query string into command path and arguments using '?' as separator.
 * - Further splits the command path by '/' to get command components.
 * - Parses arguments separated by '&' and '=' into key-value pairs.
 * - Decodes URL-encoded values in the arguments.
 * - Appends the parsed command and arguments to this command object.
 */
std::pair<bool, std::string> command::append(const std::string_view& stringQueryString, const gd::argument::arguments& arguments_, gd::types::tag_uri)
{
   std::string_view stringCommandPath;
   std::string_view stringArguments;

   auto position_ = stringQueryString.find('?');
   if( position_ != std::string_view::npos )
   {
      stringCommandPath = stringQueryString.substr( 0, position_ );
      stringArguments = stringQueryString.substr( position_ + 1 );
   }
   else
   {
      stringCommandPath = stringQueryString;
   }

   std::vector<std::string_view> vectorCommand = gd::utf8::split( stringCommandPath, '/' );
   command::arguments argumentsLocal( gd::com::server::to_command_priority_g( std::string_view( "command" ) ), vectorCommand );

   std::vector< std::pair<std::string, std::string> > vectorArguments = gd::utf8::split_pair( stringArguments, '=', '&', gd::utf8::tag_string{} );
   // ## Convert values that need to be converted because of url encoding
   for( auto& it : vectorArguments )
   {
      if( it.second.find('%') != std::string::npos )
      {
         std::string stringValue;
         gd::utf8::uri::convert_uri_to_uf8(it.second, stringValue);
         it.second = stringValue;
      }
      argumentsLocal.append(it);
   }
   argumentsLocal.append(vectorArguments);

   if( arguments_.empty() == false ) argumentsLocal.append( arguments_ );

   argumentsLocal.set_index(next_command_index());
   append( std::move(argumentsLocal) );

   return { true, "" };
}

std::pair<bool, std::string> command::append(enumPriority ePriority, const gd::argument::arguments& arguments_)
{
   arguments argumentsLocal(ePriority, arguments_);
   append( std::move(argumentsLocal) );
   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Retrieves a variable value based on a command and variable identifier with priority.
 *
 * This function retrieves a variable value by first identifying a command (either by index or name)
 * and then looking up the specified variable within that command's arguments. It supports multiple
 * types for both the command and variable identifiers, falling back to a priority-based search
 * if the variable is not found in the specified command.
 *
 * @param command_ The variant view identifying the command. Can be a number (index) or string (name).
 * @param variable_ The variant view containing the variable identifier to look up.
 *                  Can be a character string (name), string (name), or unsigned integer (index).
 * @param uPriority The priority bit mask used for fallback variable lookup if the variable
 *                  is not found in the specified command.
 *
 * @return A gd::variant_view containing the found value, or an empty gd::variant_view
 *         if no matching variable is found.
 *
 * @details
 * - If command_ is a number (checked via is_number()), it retrieves the command by index using get_command().
 * - If command_ is a string (checked via is_string()), it retrieves the command by name using get_command().
 * - If a value is found and not empty, it is returned immediately.
 * - If no value is found in the command's arguments, it falls back to a priority-based search
 *   using the overloaded get_variable() method with the variable_ and uPriority.
 * - If the command is not found or the variable lookup fails, an empty variant view is returned.
 */
gd::variant_view command::get_variable(const gd::variant_view& command_, const gd::variant_view& variable_, uint32_t uPriority) const
{
   arguments* parguments_ = nullptr;

   if( command_.is_number() == true )                                          // if command is accessed using index
   {
      parguments_ = get_command( command_.get_uint64() );
   }
   else if( command_.is_string() == true )                                       // if command is named with name( key value for command )
   {
      std::string stringName = command_.as_string();
      parguments_ = get_command(stringName);
   }

   // ## try to find variable
   if( parguments_ != nullptr )
   {
      std::string_view stringVariable;
      if( variable_.is_char_string() == true )
      {
         stringVariable = variable_.as_string_view();
         gd::variant_view value_ = parguments_->get_variant_view( stringVariable );
         if( value_.empty() != true ) return value_;
         return get_variable(variable_, uPriority);
      }
      else if( variable_.is_string() == true )
      {
         std::string stringVariable = variable_.as_string();
         gd::variant_view value_ = parguments_->get_variant_view( stringVariable );
         if( value_.empty() != true ) return value_;
         return get_variable(variable_, uPriority);
      }
      else if( variable_.is_number() == true )
      {
         unsigned uIndex = variable_.as_uint();
         auto uCount = parguments_->get_arguments().size();
         if( uCount > uIndex )
         {
            return parguments_->get_arguments()[uIndex].get_variant_view();
         }

         uIndex -= (unsigned)uCount;
         return get_variable(uIndex, uPriority);
      }
   }

   return gd::variant_view();
}

/// get variable values from command and then if not found it searches in register, stack or global based on passed arguments
gd::variant_view command::get_variable(const gd::variant_view& command_, const gd::variant_view& variable_, const std::string_view& stringPriority) const
{
   uint32_t uPriority = to_command_priority_g(stringPriority);
   return get_variable(command_, variable_, uPriority);
}


/** ---------------------------------------------------------------------------
 * @brief Retrieves a variable value based on the provided variant view and priority.
 *
 * This function searches for a variable within the internal variable vector (m_vectorVariable)
 * based on the input variant view and priority mask. It supports three types of variable
 * lookups: character string, string, and unsigned integer index.
 *
 * @param variable_ The variant view containing the variable identifier to look up.
 *                  Can be a character string (name), string (name), or unsigned integer (index).
 * @param uPriority The priority bit mask used to filter which variables to consider.
 *
 * @return A gd::variant_view containing the found value, or an empty gd::variant_view
 *         if no matching variable is found.
 *
 * @details
 * - If variable_ is a character string (checked via is_char_string()), it uses the string
 *   view to look up the value in each variable container that matches the priority.
 * - If variable_ is a string (checked via is_string()), it converts it to a std::string
 *   and performs the same lookup.
 * - If variable_ is neither, it treats it as an unsigned integer index and searches
 *   through argument lists of matching priority variables, subtracting the size of
 *   each argument list until the correct index is found.
 * - The function returns the first non-empty value found or an empty variant view
 *   if no match is found or the index is out of bounds.
 */
gd::variant_view command::get_variable(const gd::variant_view& variable_, uint32_t uPriority) const
{
   if( variable_.is_char_string() == true )
   {
      std::string_view stringVariable = variable_.as_string_view();
      for( const auto& it : m_vectorVariable )
      {
         if( it.get_priority() & uPriority )
         {
            gd::variant_view value_ = it.get_variant_view(stringVariable);
            if( value_.empty() != true ) return value_;
         }
      }
   }
   else if( variable_.is_string() == true )
   {
      std::string stringVariable = variable_.as_string();
      for( const auto& it : m_vectorVariable )
      {
         if( it.get_priority() & uPriority )
         {
            gd::variant_view value_ = it.get_variant_view(stringVariable);
            if( value_.empty() != true ) return value_;
         }
      }
   }
   else
   {
      unsigned uIndex = variable_.as_uint();
      for( const auto& it : m_vectorVariable )
      {
         if( it.get_priority() & uPriority )
         {
            auto uCount = it.get_arguments().size();
            if( uIndex < uCount )
            {
               return it.get_arguments()[uIndex].get_variant_view();
            }
            
            uIndex -= (unsigned)uCount;
         }
      }
   }

   return gd::variant_view();
}

/** ---------------------------------------------------------------------------
 * @brief Retrieves variables based on the given priority and appends their values to the provided argument list.
 * @param parguments_ A pointer to an argument list where the retrieved variables' arguments will be appended.
 * @param priority_ A variant that can either be a size_t representing the priority or a string_view representing the priority as a string.
 * @return A pair consisting of a boolean indicating success and a string (currently empty).
 */
std::pair<bool, std::string> command::get_variables(gd::argument::arguments* parguments_, const std::variant<size_t, std::string_view>& priority_) const
{
   uint32_t uPriority = 0;
   if( priority_.index() == 0 )
   {
      uPriority = (unsigned)std::get<size_t>(priority_);
   }
   else if( priority_.index() == 1 )
   {
      std::string_view stringPriority = std::get<std::string_view>(priority_);
      uPriority = to_command_priority_g(stringPriority);
   }


   for( const auto& it : m_vectorVariable )
   {
      if( it.get_priority() & uPriority )
      {
         *parguments_ += it.get_arguments();
      }
   }
   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Retrieves command variables based on index and priority specifications.
 *
 * This method fetches variables associated with a command identified by either an index or name,
 * and optionally includes variables from specified priority levels. The results are stored in
 * the provided arguments object.
 *
 * @param index_ Identifier for the command to retrieve variables from.
 *        - If size_t: Numeric index of the command
 *        - If string_view: Name of the command
 * @param priority_ Specification of priority levels to include variables from.
 *        - If size_t: Numeric priority value (bitmask)
 *        - If string_view: Priority name/key to be converted to numeric value
 * @param pargumentsVariable Pointer to arguments object where retrieved variables will be stored.
 *        The command's arguments and priority-level variables (if specified) are appended to this object.
 *
 * @return std::pair<bool, std::string> containing:
 *         - bool: Success indicator (true if successful)
 *         - string: Error message (empty string if successful)
 *
 * The method first retrieves the command based on the index parameter, then adds its arguments
 * to pargumentsVariable. If a priority is specified, it also adds variables from the matching
 * priority levels.
 */
std::pair<bool, std::string> command::get_command_variable( const std::variant<size_t, std::string_view> index_, const std::variant<size_t, std::string_view>& priority_, gd::argument::arguments* pargumentsVariable )
{
   size_t uPriority = 0;
   arguments* parguments_ = nullptr;
   if( index_.index() == 0 )                                                  // if command is accessed using index
   {
      size_t uIndex = std::get<size_t>(index_);
      parguments_ = get_command(uIndex);
   }
   else if( index_.index() == 1 )                                              // if command is named with name( key value for command )
   {
      std::string_view stringName = std::get<std::string_view>(index_);
      parguments_ = get_command(stringName);
   }

   // ## Harvest variables from command and if specified also from variables in different levels

   // ### variables from command
   if( parguments_ != nullptr ) 
   { 
      *pargumentsVariable += parguments_->get_arguments();                     // add command arguments to passed arguments
   }

   if( priority_.index() == 0 )
   {
      uPriority = std::get<size_t>(priority_);
   }
   else if( priority_.index() == 1 )
   {
      std::string_view stringPriority = std::get<std::string_view>(priority_);
      uPriority = to_command_priority_g(stringPriority);
   }

   // ### variables from different levels
   if( uPriority != 0 )
   {
      get_variables(pargumentsVariable, uPriority);                            // add variables with priority to passed arguments
   }

   return { true, "" };
}




/** ---------------------------------------------------------------------------
 * @brief Retrieves an argument value based on the index and priority.
 *
 * This function searches for an argument within the command's argument collection
 * based on either a string name or an index.
 * The search can be filtered by priority levels, which determine the
 * scope or context of the argument (e.g., register, stack, command, global).
 *
 * @param index_ The identifier of the argument, expected to be a string.
 * @param iCommandIndex The index of the command to search within. If set to -1 
 *        skips to find for selected command.
 * @param uPriority Specifies the priority filter for searching arguments. If set to 
 *        non-zero, it overrides the default behavior where all priorities except 
 *        'ePriorityCommand' are considered. The priority can be a combination of:
 *        - ePriorityRegister - think of a processor register
 *        - ePriorityStack - think of a processor method local stack, variables within a method
 *        - ePriorityCommand - like parameters to a command
 *        - ePriorityGlobal - global values
 *
 * @return gd::variant_view A variant view of the found argument. If no argument 
 *         matches the given criteria, it returns an empty or default-initialized 
 *         variant_view.
 *
 * @note 
 * - If `uPriority` is 0, the function uses a default priority filter excluding 
 *   'ePriorityCommand'. The reason for this it to handle command settings separately.
 * - The method first checks if `index_` is a string before proceeding with the search.
 * - For low priority filters (< 0x0100), it searches through all arguments with 
 *   matching priorities.
 * - For higher priority filters, it processes each byte of `uPriority` to determine 
 *   the priority for searching, ensuring each priority is checked in sequence.
 */
/*
gd::variant_view command::get_argument( const gd::variant_view& index_, int32_t iCommandIndex, uint32_t uPriority )
{
   gd::variant_view value_;
   uint32_t uPriorityFilter = (ePriorityRegister | ePriorityStack | ePriorityCommand | ePriorityGlobal);

   if( iCommandIndex != -1 )
   {
      arguments* parguments = find_arguments(iCommandIndex);                                       assert(parguments != nullptr);
      if( parguments != nullptr )
      {
         // ## Try to find value in arguments

         if( index_.is_string() )                                              // named value ?
         {
            std::string_view stringName = index_.as_string_view();
            value_ = parguments->get_variant_view(stringName);
         }
         else if( index_.is_integer() == true )                                // indexed value ?
         {
            value_ = parguments->get_variant_view(index_.as_uint());
         }

         if( value_.empty() == false ) { return value_; }                      // If value is found, return it
      }

      // If value is not found in command arguments, try to find it in local, stack or global values
      uPriorityFilter = (ePriorityRegister | ePriorityStack | ePriorityGlobal);
   }
   
   if( uPriority != 0 ) uPriorityFilter = uPriority;                           // if priority is sent use that

   if( index_.is_string() )
   {
      std::string_view stringName = index_.as_string_view();
      if( uPriorityFilter < 0x0100 ) 
      {
         for( auto it = std::begin( m_vectorArgument ), itEnd = std::end( m_vectorArgument ); it != itEnd; it++ )
         {
            if( (it->get_priority() & uPriorityFilter) != 0 )                  // not for command values, only stack and global
            {
               const gd::argument::arguments& arguments_ = it->get_arguments();
               if( arguments_.exists( stringName ) == true )
               {
                  value_ = arguments_[stringName].as_variant_view();
                  break;
               }
            }
         }
      }
      else
      {
         std::array<unsigned, 4> arrayPriority = { ePriorityRegister, ePriorityStack, ePriorityCommand, ePriorityGlobal };
         // ## Extract each byte from uPriority and place in array
         for( unsigned uByte = 0; uByte < sizeof(uint32_t); uByte++ )
         {
            arrayPriority[uByte] = ( uPriority >> ( uByte * 8 ) ) & 0xFF;      // extract byte and put in array
         }

         // ## loop through all priority values and try to find value, first value found is returned
         for( auto it : arrayPriority )
         {
            uPriorityFilter = it;
            for( auto it = std::begin(m_vectorArgument), itEnd = std::end(m_vectorArgument); it != itEnd; it++ )
            {
               if( it->get_priority() == uPriorityFilter )                     // not for command values, only stack and global
               {
                  const gd::argument::arguments& arguments_ = it->get_arguments();
                  if( arguments_.exists(stringName) == true )
                  {
                     value_ = arguments_[stringName].as_variant_view();
                     return value_;
                  }
               }
            }
         }
      }
   }
   else if( index_.is_integer() == true )
   {
      // TODO: implement integer index logic
      unsigned uIndex = index_.as_uint();
      if( uIndex < m_vectorArgument.size() )
      {
         // value_ = m_vectorArgument.at(uIndex).get_arguments();
      }
   }

   return value_;
}
*/

/** ---------------------------------------------------------------------------
 * @brief Retrieves all arguments from the command object.
 *
 * This function retrieves all arguments from the command object, including 
 * command-specific arguments, stack values, and global values. The function 
 * filters the arguments based on the priority level specified in the index.
 * 
 * If m_iCommandIndex is set then it tages arguments from that command and add
 * those first in returned arguments object.
 *
 * @param index_ The index or priority level to filter the arguments.
 * @return gd::argument::arguments A collection of arguments based on the priority level.
 *
 * @note 
 * - If `index_` is a boolean `true`, the function uses the default priority level 
 *   `ePriorityALL` to retrieve all arguments.
 * - If `index_` is a string, the function converts it to a priority level using 
 *   `to_command_priority_g()` and retrieves arguments based on that level.
 * - If `index_` is an integer, the function retrieves arguments based on that index.
 * - The function first retrieves command-specific arguments and then appends 
 *   stack and global values based on the priority level.
 */
gd::argument::arguments command::get_all_arguments( const gd::variant_view& index_ )
{
   unsigned uPriority = ePriorityALL;

   if( index_.is_true() == true )
   {
      if( index_.is_string() == true )
      {
         uPriority = to_command_priority_g(index_.as_string_view());
      }
      else if( index_.is_integer() == true )
      {
         uPriority = index_.as_uint();
      }
   }

   gd::argument::arguments argumentsReturn;

   if( m_iCommandIndex != -1 )
   {
      arguments* parguments = find_arguments(m_iCommandIndex);                                     assert(parguments != nullptr);
      if( parguments != nullptr )
      {
         argumentsReturn.append(parguments->get_arguments());
      }

      uPriority &= ~ePriorityCommand;
   }

   for( auto it = std::begin( m_vectorVariable ), itEnd = std::end( m_vectorVariable ); it != itEnd; it++ )
   {
      if( it->get_priority() & uPriority ) 
      {
         const gd::argument::arguments& arguments_ = it->get_arguments();
         if( arguments_.empty() == false )
         {
            argumentsReturn += arguments_;
         }
      }
   }

   return argumentsReturn;
}

/** ---------------------------------------------------------------------------
 * @brief return values from selected item, if no found return global values
 * @param index_ index to selected part
 * @param parguments_ arguments item where values are placed
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> command::get_command( const gd::variant_view& index_, void** ppCommand )
{                                                                                                  assert( ppCommand != nullptr );
   *ppCommand = nullptr; // set to null to indicate that no command was found

   if( index_.is_number() == true )
   {
      unsigned uIndex = index_.as_uint();
      *ppCommand = (void*)get_command(uIndex);
   }
   else if( index_.is_string() == true )
   {
      std::string_view stringName = index_.as_string_view();
      for( auto it : m_vectorArgument )
      {
         if( it.get_key() == stringName )
         {
            *ppCommand = &it;
            break;
         }
      }
   }

   return { true, "" };
}

std::pair<bool, std::string> command::query_select( unsigned uPriority, const gd::variant_view& selector_, gd::variant_view* pvariantview_ )
{
   if( uPriority == 0 ) uPriority = uPriorityAll_g;

   gd::variant_view value_;

   if( selector_.is_string() )
   {
      std::string_view stringName = selector_.as_string_view();
      for( auto it : m_vectorArgument )
      {
         auto uPriorityArgument = it.get_priority();
         if( (uPriorityArgument & uPriority) == 0 ) continue;

         if( it.get_priority() != ePriorityCommand )                           // not for command values, only stack and global
         {
            const gd::argument::arguments& arguments_ = it.get_arguments();
            if( arguments_.exists( stringName ) == true  )
            {
               value_ = arguments_[stringName].as_variant_view();
               if( pvariantview_ != nullptr ) *pvariantview_ = value_;
               return { true, "" };
            }
         }
      }
   }
   else if( selector_.is_integer() == true )
   {
      unsigned uIndex = selector_.as_uint();
      
   }

   return { false, "" };
}

/** ---------------------------------------------------------------------------
 * @brief wrapper to simplify query_select to get value from variable name
 * @param stringSelector name for value to select
 * @return gd::variant_view value for selector name or null if not found
 */
gd::variant_view command::query_select( const std::string_view& stringSelector )
{
   gd::variant_view variantviewValue;
   // try to find in stack
   auto result_ = query_select( (ePriorityRegister|ePriorityStack), stringSelector, &variantviewValue );
   if( result_.first == false )
   {
      query_select( ePriorityGlobal, stringSelector, &variantviewValue );   
   }

   return variantviewValue;
}


/** ---------------------------------------------------------------------------
 * @brief Retrieves a variant view of an argument based on a selector and a key.
 *
 * This function searches for an argument within the command's argument collection
 * based on a string selector and a key, which can be either an index or a string.
 * The search can be filtered by priority levels, which determine the
 * scope or context of the argument (e.g., register, stack, command, global).
 *
 * @param stringSelector The name of the argument to search for.
 * @param variantKey The key to search for, which can be either an index (size_t) or a string.
 * @return gd::variant_view A variant view of the found argument. If no argument 
 *         matches the given criteria, it returns an empty or default-initialized 
 *         variant_view.
 *
 * @note 
 * - If `variantKey` is an index, the function searches for the argument at that index.
 * - If `variantKey` is a string, the function searches for the argument with that key.
 * - If no argument matches the given criteria, the function performs a secondary search
 *   using the `query_select` method with the provided string selector.
 */
gd::variant_view command::query_select(const std::string_view& stringSelector, const std::variant<size_t, std::string>& variantKey) 
{
   gd::variant_view variantviewValue;
   if( variantKey.index() == 0 )
   {
      size_t uIndex = std::get<0>(variantKey);
      if( uIndex < m_vectorArgument.size() )
      {
         const auto& arguments_ = m_vectorArgument.at(uIndex);
         variantviewValue = arguments_.get_arguments().get_argument(stringSelector).as_variant_view();
      }
   }
   else
   {
      std::string_view stringName = std::get<1>(variantKey);
      for( auto it : m_vectorArgument )
      {
         if( it.get_key() == stringName )
         {
            variantviewValue = it.get_arguments().get_argument(stringSelector).as_variant_view();
            break;
         }
      }
   }

   if( variantviewValue.empty() == true )
   {
      variantviewValue = query_select( stringSelector );
   }

   return variantviewValue;
}

gd::argument::arguments command::query_select( const std::initializer_list<std::string_view>& listSelector )
{
   gd::argument::arguments argumentsReturn;
   for( auto itName : listSelector )
   {
      auto value_ = query_select( itName );
      if( value_.empty() == false ) argumentsReturn.append_argument(itName, value_);
   }
   return argumentsReturn;
}


gd::argument::arguments command::query_select( const std::initializer_list<std::string_view>& listSelector, const std::variant<size_t,std::string>& variantKey )
{
   gd::argument::arguments argumentsReturn;
   for( auto itName : listSelector )
   {
      auto value_ = query_select( itName, variantKey );
      if( value_.empty() == false ) argumentsReturn.append_argument(itName, value_);
   }
   return argumentsReturn;
}

std::pair<bool, std::string> command::query_select_all( const gd::variant_view& selector_, std::vector<gd::variant_view>* pvectorValue )
{
   if( selector_.is_string() )
   {
      std::string_view stringName = selector_.as_string_view();
      for( auto it : m_vectorArgument )
      {
         if( it.get_priority() != ePriorityCommand )                           // not for command values, only stack and global
         {
            const gd::argument::arguments& arguments_ = it.get_arguments();
            if( arguments_.exists( stringName ) == true)
            {
               auto vector_ = arguments_.get_argument_all( stringName, gd::argument::arguments::tag_view{} );
               if( pvectorValue != nullptr ) pvectorValue->insert( pvectorValue->end(), vector_.begin(), vector_.end());
            }
         }
      }
   }

   return { true, "" };
}

 /** --------------------------------------------------------------------------
  * @brief Clears values used to execute commands based on the specified variant view.
  * 
  * This method clears command-related data depending on the provided variant view parameter.
  * It supports both numeric and string-based specifications of what to clear.
  * 
  * @param variantviewToClear The specification of what to delete. 
  *        - If a number: Valid values are `ePriorityRegister`, `ePriorityStack`, `ePriorityCommand`, `ePriorityGlobal`, or combinations thereof
  *        - If a string: Valid values are "stack", "command", "global", "register", "all", or a variable name
  * 
  * When a string is provided:
  * - "register": Clears register priority values
  * - "stack": Clears stack priority values
  * - "command": Clears command priority values and arguments
  * - "global": Clears global priority values
  * - "all": Clears register, stack, and global priority values
  * - Other strings: Treated as a variable name to remove from m_vectorVariable
  * 
  * When a number is provided, it uses bitwise operations to determine which priority levels to clear.
  */
void command::clear( const gd::variant_view& variantviewToClear )
{
   unsigned uType = 0;

   if( variantviewToClear.is_string() == true )
   {
      std::string_view stringType = variantviewToClear.as_string_view();
      uType = to_command_priority_g(stringType);
      if( stringType == "register" )      uType = ePriorityRegister;
      else if( stringType == "stack" )    uType = ePriorityStack;
      else if( stringType == "command" )  uType = ePriorityCommand;
      else if( stringType == "global" )   uType = ePriorityGlobal;
      else if( stringType == "all" )      uType = (ePriorityRegister | ePriorityStack | ePriorityGlobal);
      else
      {
         for( auto it = std::begin( m_vectorVariable), itEnd = std::end( m_vectorVariable ); it != itEnd; it++ )
         {
            if( it->get_key() == stringType )
            {
               it = m_vectorVariable.erase( it );
            }
         }
      }
   }
   else
   {
      uType = variantviewToClear.as_uint();
   }

   if( uType & ePriorityCommand )                                             // if command values are to be cleared
   {
      m_vectorArgument.clear();
   }

   if( uType & ( ePriorityRegister | ePriorityStack | ePriorityGlobal ) )     // if variables are to be cleared
   {
      // ## Clear values in vector holding variables
      m_vectorVariable.erase(std::remove_if(m_vectorVariable.begin(), m_vectorVariable.end(), 
         [uType](const arguments& a_ ) 
         { 
            bool b_ = (a_.get_priority() & uType) != 0;
            return b_;
         }), m_vectorVariable.end());
   }
}

/// clear all values in command
void command::clear()
{
   m_vectorArgument.clear();
   m_vectorVariable.clear();
}


/** ---------------------------------------------------------------------------
 * @brief Find last position for priority among arguments
 * Arguments are ordered in priority, it starts with low and increase
 * global priority is the highest value
 * @param uPriority 
 * @return size_t index to last position for priority
 */
size_t command::find_last_priority_position( unsigned uPriority ) const
{
   unsigned uPositionPriority = ePriorityStack; // stack priority is allways first
   auto itPosition = m_vectorArgument.begin();

   while( itPosition != std::end( m_vectorArgument ) )
   {                                                                                               assert( itPosition->get_priority() >= uPositionPriority ); // need to be in priority order
      uPositionPriority = itPosition->get_priority();
      
      // if next priority is larger then jump out, we have the last position
      if( uPriority < uPositionPriority ) { break;  }
      
      itPosition++;
   }

   return std::distance( m_vectorArgument.begin(), itPosition );
}

/** ---------------------------------------------------------------------------
 * @brief Sorts the command arguments based on their priority.
 *
 * This method sorts the internal vector of command arguments in ascending order
 * of their priority values. The sorting ensures that arguments with lower priority
 * values come before those with higher priority values.
 * 
 * To get values it is importan t that values are placed in correct order
 */
void command::sort()
{
   std::sort(m_vectorArgument.begin(), m_vectorArgument.end(), [](const arguments& arguments1_, const arguments& arguments2_) 
   {
      return arguments1_.get_priority() < arguments2_.get_priority();
   });
}

/** ---------------------------------------------------------------------------
 * @brief Print all command arguments as a formatted string.
 * @return std::string A formatted string representing the command arguments.
 */
std::string command::print() const
{
   std::string stringReturn;
   for( const auto& it : m_vectorArgument )
   {
      if( stringReturn.empty() == false ) stringReturn += '\n';
      stringReturn += it.print();
   }
   return stringReturn;
}
/** ---------------------------------------------------------------------------
 * @brief Prints the command arguments as a formatted string.
 * 
 * This method iterates through the command path and formats them into a single string.
 * Each command is separated by " / " and followed by its respective arguments.
 * The arguments are indented and printed on a new line.
 * 
 * @return std::string A formatted string representing the command arguments.
 */
std::string command::arguments::print() const
{ 
   std::string stringReturn;
   for( auto it : m_strings32Command )
   {
      if( it.empty() == false ) stringReturn += std::string_view{ " / " };
      stringReturn += it;
   }

   stringReturn += "\n    ";                                                   // add new line and indent   

   stringReturn += m_arguments.print();

   return stringReturn;
}

// ================================================================================================
// ======================================================================================= response
// ================================================================================================

int32_t response::query_interface( const gd::com::guid& guidId, void** ppObject )
{
   return gd::com::E_NoInterface;
}

/// release decrease reference counter and if down to 0 object is deleted
unsigned response::release() 
{                                                                                                  assert( m_iReference > 0 );
   m_iReference--; 
   if( m_iReference == 0 )
   {
      delete this;
      return 0;
   }

   return (unsigned)m_iReference; 
}

std::pair<bool, std::string> response::add( const gd::variant_view& key_, const gd::argument::arguments& argumentsValue )
{
   //m_vectorValue.push_back( { key_.as_string(), argumentsValue } );
   return { true, "" };
}

std::pair<bool, std::string> response::add( const gd::variant_view& key_, const gd::argument::arguments&& argumentsValue )
{
   //m_vectorValue.push_back( { key_.as_string(), std::move(argumentsValue) } );
   return { true, "" };
}

/** ---------------------------------------------------------------------------
* @brief Add to collection of return values
* This method adds return values, values that are not named, just values harvested
* from methods executed that return one single value
* @param variantValue value added as return value
* @return true if ok, false and error information on error
*/
std::pair<bool, std::string> response::return_add( gd::variant* pvariantKey, gd::variant* pvariantValue ) 
{
   gd::variant variantKey;
   gd::variant variantValue;

   if( pvariantKey != nullptr ) { variantKey = *pvariantKey; }
   if( pvariantValue != nullptr ) { variantValue = *pvariantValue; }

   m_vectorReturn.push_back( std::pair<gd::variant, gd::variant>( std::move( variantKey ), std::move( variantValue ) ) );
   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Get pointer to internal arguments value that holds return data from called methods
 * @param index_ index to internal arguments value with return information
 * @param ppArguments pointer to pointer for arguments thet gets pointer to arguments object
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> response::get(const gd::variant_view& index_, gd::argument::arguments** ppArguments)
{                                                                                                  assert( ppArguments != nullptr );
/*
   size_t uIndex = 0;
   if(index_.is_integer() == true) { uIndex = index_.as_uint64(); }
   else
   {
      std::string stringCommand = index_.as_string();
      for(size_t u = 0, uMax = m_vectorValue.size(); u < uMax; u++)
      {

         std::pair<std::string, gd::argument::arguments>* ppair_ = &m_vectorValue.at( u );
         if(stringCommand == ppair_->first )
         {
            *ppArguments = &ppair_->second;
            return { true, "" };
         }
      }
   }

   if(m_vectorValue.size() > uIndex)
   {
      // ## Get pointer to internal arguments object with return information from method.
      //    Internal list holds returned data and full name (path) for command
      std::pair<std::string, gd::argument::arguments>* ppair_ = &m_vectorValue.at( uIndex );
      *ppArguments = &ppair_->second;
      return { true, "" };                                                     // returns ok, arguments for index was found
   }
  */
   return { false, index_.as_string() };
}


/// ---------------------------------------------------------------------------
/// get pointer to load for index or name
std::pair<bool, std::string> response::body_get( const std::variant<uint64_t, std::string_view>& index_, gd::com::server::body_i** ppload_ )
{
   if( index_.index() == 0 )
   {                                                                                               
      uint64_t uIndex = std::get<0>( index_ );                                                     assert( uIndex < body_size() );
      body_i* pload = m_vectorBody.at( uIndex );
      pload->add_reference();
      *ppload_ = pload;
   }
   else
   {
      std::string_view stringName = std::get<1>( index_ );
      for( auto it : m_vectorBody )
      {
         if( ((body_i*)it)->name() == stringName )
         {
            it->add_reference();
            *ppload_ = it;
            return { true, "" };      
         }
      }
   }

   return { false, std::string( "`get_body` invalid index, no load found" ) };
}

/// ---------------------------------------------------------------------------
/// add load to response object
std::pair<bool,std::string> response::body_add( gd::com::server::body_i* pload_)
{  
   pload_->add_reference();
   m_vectorBody.push_back( pload_ );
   return { true, "" };
}

/// ---------------------------------------------------------------------------
/// Get number of loads in response
uint32_t response::body_size()
{
   return (uint32_t)m_vectorBody.size();
}

/// Clear all internal data
void response::clear_all()
{
   m_vectorReturn.clear();

   // ## release interface for load added to response
   for( auto it : m_vectorBody )
   {
      it->release();
   }
   m_vectorBody.clear();
}







// ================================================================================================
// ========================================================================================= server
// ================================================================================================

int32_t server::query_interface( const gd::com::guid& guidId, void** ppObject )
{
   return gd::com::E_NoInterface;
}

/// release decrease reference counter and if down to 0 object is deleted
unsigned server::release() 
{                                                                                                  assert( m_iReference > 0 );
   m_iReference--; 
   if( m_iReference == 0 )
   {
      delete this;
      return 0;
   }
   
   return (unsigned)m_iReference; 
}

/** ---------------------------------------------------------------------------
 * @brief `get` is to mimic get requets from browser requests, like when you call "get" or "post"
 * `get` takes arguments in one single string, they are split based on the `m_uSplitChar`
 * *sample*
 * @code
std::pair<bool, std::string> Sum(const std::string_view& stringCommand, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse )
{
   gd::com::server::router::command* pcommand_ = (gd::com::server::router::command*)pcommand;
   std::vector< std::string_view > vectorCommand = pcommand_->add_querystring( stringCommand );

   std::string_view stringCommandName = vectorCommand.back();
   if( stringCommandName == "sum" )
   {
      std::vector< gd::variant_view > vectorValue;
      pcommand->query_select_all( "value", &vectorValue );
      uint64_t uSum = 0;
      for( auto it : vectorValue ) { uSum += it.as_uint64(); }
      gd::variant variantSum( uSum );
      presponse->return_add( nullptr, &variantSum );
   }

   return { true, "" };
}
 
auto pserver = gd::com::pointer< gd::com::server::router::server >( new gd::com::server::router::server );
pserver->callback_add( Sum );
auto pcommand = gd::com::pointer< gd::com::server::router::command >( new gd::com::server::router::command( pserver ) );
std::string stringCommand = "sum?value=1&value=10&value=100&value=1000&value=10000;sum?value=2&value=20";

 * @endcode
 * @param pstringCommandList 
 * @param pargumentsParameter 
 * @param pcommand 
 * @param presponse 
 * @return 
 */
std::pair<bool, std::string> server::get( const std::string_view* pstringCommandList, const gd::argument::arguments* pargumentsParameter, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse )
{
   if( pargumentsParameter != nullptr && pargumentsParameter->empty() == false )
   {
      pcommand->add_arguments(  ePriorityStack, pargumentsParameter );
   }
   /*
   else
   {
      ((gd::com::server::router::command*)(pcommand))->add_querystring( *pstringCommandList );
   }
   */

   auto vectorCommands = gd::utf8::split(*pstringCommandList, m_uSplitChar);   // extract commands from string that are separated by `m_uSplitChar`. Each command is then formated similar to url query string
   for( auto stringCommand : vectorCommands )
   {
      for( auto itCallback : m_vectorCallback )
      {
         auto result_ = itCallback( stringCommand, pcommand, presponse );      // call callback with command and response object
         if( result_.first == false ) 
         {
            add_error( result_.second );
            return result_;
         }
      }
      
   }
   return { true, "" };
}

/// ---------------------------------------------------------------------------
/// Add to internal error list
void server::add_error( const std::variant<std::string_view, const gd::argument::arguments*>& error_ )
{
   if( error_.index() == 0 )
   {
      m_vectorError.push_back( std::string( std::get<0>( error_ ) ) );
   }
   else if( error_.index() == 1 )
   {
      const auto* parguments_ = std::get<1>( error_ );
      std::string stringError = parguments_->print_json();
      m_vectorError.push_back( stringError );
   }
}

/** ---------------------------------------------------------------------------
 * @brief return error information
 * Passing a nullpointer for vector with string, then only number of error messages are return
 * @param pvectorError pointer to vector that gets error information
 * @param bRemove if errors should be removed
 * @return number of errors
 */
unsigned server::get_error(std::vector<std::string>* pvectorError, bool bRemove)
{
   unsigned uErrorCount = (unsigned)m_vectorError.size();

   if( pvectorError != nullptr ) { pvectorError->insert( pvectorError->end(), m_vectorError.begin(), m_vectorError.end() ); } // copy errors

   if( bRemove == true ) m_vectorError.clear();

   return uErrorCount;
}


} } } // com::server::router
_GD_END
