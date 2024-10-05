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

std::pair<bool, std::string> command::add_arguments( const gd::variant_view& variantviewLocality, const gd::argument::arguments* pargumentsVariable ) 
{
   unsigned uPriority = ePriorityStack;

   if( variantviewLocality.is_string() == true )
   {
      uPriority = priority_g( variantviewLocality.as_string_view() );
   }
   else 
   {
      uPriority = variantviewLocality.as_uint();
   }


   // ## Check for integer, if integer this number represents any of the locality (priority) numbers.
   //    stack is the highest priority
   if( variantviewLocality.is_integer() == true )
   {
      unsigned uPriority = variantviewLocality.as_uint();                                          assert( uPriority >= ePriorityStack && uPriority <= ePriorityGlobal );
      if( uPriority == ePriorityStack )
      {
         m_vectorArgument.insert( m_vectorArgument.cbegin(), arguments( ePriorityStack, *pargumentsVariable ) ); // add first in argument vector
      }
      else
      {
         auto uPosition = find_last_priority_position( uPriority );
         m_vectorArgument.insert( m_vectorArgument.cbegin() + uPosition, arguments( uPriority, *pargumentsVariable ) ); // add first in argument vector
      }
   }
   // ## if string then it is probably related to command key value
   else if( variantviewLocality.is_string() == true )
   {
      unsigned uPriority = ePriorityStack;
      std::string_view stringPriority = variantviewLocality.as_string_view();
      if( stringPriority == "global" ) uPriority = ePriorityGlobal;

      if( uPriority == ePriorityStack )
      {
         m_vectorArgument.insert( m_vectorArgument.cbegin(), arguments( ePriorityStack, *pargumentsVariable ) ); // add first in argument vector
      }
      else
      {
         auto uPosition = find_last_priority_position( uPriority );
         m_vectorArgument.insert( m_vectorArgument.cbegin() + uPosition, arguments( uPriority, *pargumentsVariable ) ); // add first in argument vector
      }
   }

   return { true, "" };
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
 * @return 
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
      add_querystring( ePriorityStack, stringArguments );
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

/** ---------------------------------------------------------------------------
 * @brief get global argument from command object
 * @param index_ {string|integer} index to argument to return
 * @return gd::variant_view value for requested argument
 */
gd::variant_view command::get_argument( const gd::variant_view& index_ )
{
   gd::variant_view value_;

   if( index_.is_string() )
   {
      std::string_view stringName = index_.as_string_view();
      for( auto it = std::begin( m_vectorArgument ), itEnd = std::end( m_vectorArgument ); it != itEnd; it++ )
      {
         if( it->get_priority() != ePriorityCommand )                          // not for command values, only stack and global
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
std::pair<bool, std::string> command::get_arguments( const std::variant<size_t, std::string_view> index_, gd::argument::arguments* parguments_ )
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
         if( (it.get_priority() & uPriority) == 0 ) continue;

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
   auto result_ = query_select( ePriorityStack, stringSelector, &variantviewValue );
   if( result_.first == false )
   {
      query_select( ePriorityGlobal, stringSelector, &variantviewValue );   
   }

   return variantviewValue;
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


/**
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
std::pair<bool, std::string> response::add_return( gd::variant&& variantValue ) 
{
   m_vectorReturn.push_back( std::move( variantValue ) );
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
std::pair<bool, std::string> response::get_body( const std::variant<uint64_t, std::string_view>& index_, gd::com::server::body_i** ppload_ )
{
   if( index_.index() == 0 )
   {                                                                                               
      uint64_t uIndex = std::get<0>( index_ );                                                     assert( uIndex < get_body_count() );
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
std::pair<bool,std::string> response::add_body( gd::com::server::body_i* pload_)
{  
   pload_->add_reference();
   m_vectorBody.push_back( pload_ );
   return { true, "" };
}

/// ---------------------------------------------------------------------------
/// Get number of loads in response
uint64_t response::get_body_count()
{
   return (uint64_t)m_vectorBody.size();
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

std::pair<bool, std::string> server::get( const std::string_view* pstringCommandList, const gd::argument::arguments* pargumentsParameter, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse )
{
   if( pargumentsParameter != nullptr && pargumentsParameter->empty() == false )
   {
      {
         auto s0 = (*pargumentsParameter)["query"];
         auto s = s0.as_string();
         std::string stringtest( s );
      }
      pcommand->add_arguments(  ePriorityStack, pargumentsParameter );
      {
         auto s0 = pcommand->get_argument( "query" );
         auto s = s0.as_string();
         std::string stringtest( s );
      }
   }

   auto vectorCommands = gd::utf8::split( *pstringCommandList, m_uSplitChar );
   for( auto itCommand : vectorCommands )
   {
      for( auto itCallback : m_vectorCallback )
      {
         auto result_ = itCallback( itCommand, pcommand, presponse );
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
