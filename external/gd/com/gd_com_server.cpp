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
      m_vectorArgument.push_back( arguments( uPriority, *pargumentsVariable ) );
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
 * @brief remove arguments in vector with priority flag
 * @param uPriority flags for arguments to remove
 */
void command::arguments_remove(unsigned uPriority)
{
   /// loop backwards because it is a bit faster
   for( auto it = m_vectorArgument.rbegin(); it != m_vectorArgument.rend(); ) 
   {
      if( it->get_priority() & uPriority )
      {
         // tricky for removing backwards iterators
         it = std::vector<arguments>::reverse_iterator( m_vectorArgument.erase( (it + 1).base() ));
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
std::pair<bool, std::string> command::append(const std::string_view& stringQueryString, gd::types::tag_uri)
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
   append( std::move(argumentsLocal) );

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
gd::variant_view command::get_argument( const gd::variant_view& index_, uint32_t uPriority )
{
   gd::variant_view value_;
   uint32_t uPriorityFilter = (ePriorityRegister | ePriorityStack | ePriorityCommand | ePriorityGlobal);
   
   if( uPriority != 0 ) uPriorityFilter = uPriority;                           // if priority is set use that

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

/** ---------------------------------------------------------------------------
 * @brief return all data in command. mostly stack and global values
 * @param index_ 
 * @return 
 */
gd::argument::arguments command::get_all_arguments( const gd::variant_view& index_ )
{
   gd::argument::arguments argumentsReturn;
   for( auto it = std::begin( m_vectorArgument ), itEnd = std::end( m_vectorArgument ); it != itEnd; it++ )
   {
      if( it->get_priority() != ePriorityCommand ) 
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
std::pair<bool, std::string> command::get_arguments( const std::variant<uint64_t, std::string_view> index_, gd::argument::arguments* parguments_ )
{                                                                                                  assert( parguments_ != nullptr );
   if( index_.index() == 1 )
   {
      std::string_view stringName = get<1>( index_ );
      const gd::argument::arguments* pargumentsFind = find( stringName );
      if( pargumentsFind != nullptr ) { parguments_->append( *pargumentsFind ); }
   }
   else
   {                                                                                               assert( get<0>( index_ ) < m_vectorArgument.size() );
      auto uIndex = get<0>( index_ );
      const auto& arguments_ = m_vectorArgument.at( uIndex );
      parguments_->append( arguments_.get_arguments() );
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

/** ---------------------------------------------------------------------------
 * @brief clear values used to execute commands
 * @param variantviewToClear what to delete, if number then `ePriorityStack`, `ePriorityCommand`, `ePriorityGlobal` are valid
 *        if string then "stack", "gommand", "global"
 */
void command::clear( const gd::variant_view& variantviewToClear )
{
   unsigned uType = 0;

   if( variantviewToClear.is_string() == true )
   {
      std::string_view stringType = variantviewToClear.as_string_view();
      if( stringType == "stack" ) uType = ePriorityStack;
      else if( stringType == "command" ) uType = ePriorityCommand;
      else if( stringType == "global" ) uType = ePriorityGlobal;
      else
      {
         for( auto it = std::begin( m_vectorArgument ), itEnd = std::end( m_vectorArgument ); it != itEnd; it++ )
         {
            if( it->get_key() == stringType )
            {
               it = m_vectorArgument.erase( it );
            }
         }
      }
   }
   else
   {
      uType = variantviewToClear.as_uint();
   }

   // ## Clear values in vector holding command values
   m_vectorArgument.erase(std::remove_if(m_vectorArgument.begin(), m_vectorArgument.end(), 
      [uType](const arguments a_ ) 
      { 
         bool b_ = (a_.get_priority() & uType) != 0;
         return b_;
      }), m_vectorArgument.end());
}

/// clear all values in command
void command::clear()
{
   m_vectorArgument.clear();
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
 * @brief Print all command arguments as a formatted string.
 * @return std::string A formatted string representing the command arguments.
 */
std::string command::print() const
{
   std::string stringReturn;
   for( auto it : m_vectorArgument )
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
