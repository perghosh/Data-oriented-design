#include <variant>

#include "gd/gd_utf8.hpp"
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
 * @brief Add global arguments to command
 * @param argumentsGlobal list of arguments to add as global
 * @return true if ok, false and error information if something is wroing
 */
std::pair<bool, std::string> command::add_arguments( const gd::argument::arguments* pargumentsGlobal )
{
   if( m_argumentsGlobal.empty() == true ) { m_argumentsGlobal = *pargumentsGlobal; }
   else
   {
      m_argumentsGlobal += *pargumentsGlobal;
   }

   return { true, "" };
}

std::pair<bool, std::string> command::add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments* pargumentsLocal )
{
   int iIndex = (int)m_vectorLocal.size();
   m_vectorLocal.push_back( arguments( iIndex, stringKey, stringCommand, *pargumentsLocal ) );
   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief add command arguments without key, to access this match index
 * @param pargumentsLocal arguments for local command
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> command::add_command_arguments( const gd::argument::arguments* pargumentsLocal )
{
   std::string stringKey = std::to_string( m_vectorLocal.size() );
   m_vectorLocal.push_back( std::pair<std::string, gd::argument::arguments>( stringKey, *pargumentsLocal ) );

   return { true, "" };
}



/** ---------------------------------------------------------------------------
 * @brief add local arguments for specific command
 * @param stringName command name
 * @param pargumentsLocal arguments for local command
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> command::add_command_arguments( const std::string_view& stringName, const gd::argument::arguments* pargumentsLocal )
{
   gd::argument::arguments* parguments = find( stringName );
   if( parguments != nullptr )
   {
      *parguments += *pargumentsLocal;
   }
   else
   {
      m_vectorLocal.push_back( std::pair<std::string, gd::argument::arguments>( stringName, *pargumentsLocal ) );
   }
   return { true, "" };
}

/// ---------------------------------------------------------------------------
/// get pointer to internal arguments
void command::get_arguments( gd::argument::arguments** ppargumentsGlobal )
{                                                                                                  assert( ppargumentsGlobal );
   *ppargumentsGlobal = &m_argumentsGlobal;
}

/** ---------------------------------------------------------------------------
 * @brief get global argument from command object
 * @param index_ {string|integer} index to argument to return
 * @return gd::variant_view value for requested argument
 */
gd::variant_view command::get_argument( const gd::variant_view& index_ )
{
   gd::variant_view value_;
   if( index_.is_string() == true )
   {
      value_ = m_argumentsGlobal[index_.as_string_view()].as_variant_view();
   }
   else
   {                                                                                               assert( m_argumentsGlobal.size() > index_.as_uint() );
      value_ = m_argumentsGlobal[index_.as_uint()].as_variant_view();
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
   {                                                                                               assert( get<0>( index_ ) < m_vectorLocal.size() );
      auto uIndex = get<0>( index_ );
      const auto& arguments_ = m_vectorLocal.at( uIndex );
      parguments_->append( arguments_.get_arguments() );
   }

   return { true, "" };
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
{                                                                                                  assert( pcommand != nullptr );
   pcommand->add_arguments( pargumentsParameter );

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
