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


std::pair<bool, std::string> command::add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments* pargumentsLocal )
{
   int iIndex = (int)m_vectorArgument.size();
   m_vectorArgument.push_back( arguments( stringKey, stringCommand, *pargumentsLocal ) );
   m_vectorArgument.back().set_index( iIndex );
   return { true, "" };
}

/// ---------------------------------------------------------------------------
/// get pointer to internal arguments
void command::get_arguments( gd::argument::arguments** ppargumentsGlobal )
{                                                                                                  assert( ppargumentsGlobal );
   //*ppargumentsGlobal = &m_argumentsGlobal;
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
      for( auto it : m_vectorArgument )
      {
         if( it.get_priority() != ePriorityCommand )                           // not for command values, only stack and global
         {
            const gd::argument::arguments& arguments_ = it.get_arguments();
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
// ========================================================================================= server
// ================================================================================================

int32_t server::query_interface( const gd::com::guid& guidId, void** ppObject )
{
   return gd::com::S_Ok;
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
   if( pargumentsParameter != nullptr && pargumentsParameter->empty() == false ) pcommand->add_arguments(  ePriorityStack, pargumentsParameter );

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
