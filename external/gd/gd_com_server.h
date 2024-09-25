#pragma once

#include <cassert>
#include <functional>
#include <string>
#include <string_view>


#include "gd_arguments.h"
#include "gd_variant_view.h"

#include "gd_com.h"

#if defined( __clang__ )
   #pragma clang diagnostic push
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
#elif defined( _MSC_VER )
   #pragma warning(push)
#endif


_GD_BEGIN

/**
 * @brief Interfaces to have different objects to comunicate without knowing what the communicate with.
 */

namespace com { namespace server {

struct server_i;   

enum enumFormat
{
   eNULL          = 0,
   eASCII         = 1,
   eUTF8          = 2,
   eJSON          = 3,
   eXML           = 4,
   eTABLE         = 5,
};

/// The reach for values added to command, reach means priority
enum enumPriority
{
   ePriorityStack = 0x01,     ///< like the closest stack value, removed when command is executed
   ePriorityCommand= 0x02,    ///< follow command
   ePriorityGlobal= 0x04,     ///< global reach within command
   ePriorityAll   = 0x07,
};

struct body_i : public unknown_i
{
   virtual unsigned type() = 0;
   virtual std::string_view type_name() = 0;
   virtual void* get() = 0;
   virtual void destroy() = 0;
};

// command format - command/sub-command/sub-sub-command

struct command_i : public unknown_i
{
   virtual server_i* get_server() = 0;
   //virtual std::string add( const std::string_view& stringName, const std::string_view& stringUriCommand ) = 0;
   //virtual std::string add( const std::string_view& stringName, const gd::argument::arguments& argumentsCommand ) = 0;
   virtual std::pair<bool, std::string> add_arguments( const gd::argument::arguments* pargumentsGlobal ) = 0;
   virtual std::pair<bool, std::string> add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments* pargumentsGlobal ) = 0;
   virtual std::pair<bool, std::string> add_command_arguments( const gd::argument::arguments* pargumentsGlobal ) = 0;
   virtual std::pair<bool, std::string> add_command_arguments( const std::string_view& stringName, const gd::argument::arguments* pargumentsGlobal ) = 0;
   virtual void get_arguments( gd::argument::arguments** ppargumentsGlobal ) = 0;
   //virtual gd::variant_view get_command() = 0;
   virtual gd::variant_view get_argument( const gd::variant_view& index_ ) = 0;
   // virtual std::pair<bool, std::string> get_list( const std::variant<size_t, std::string_view> index_, std::vector< std::pair<std::string, gd::variant> >** ppvector_ ) = 0;
   // virtual uint64_t get_list_count() = 0;
   virtual std::pair<bool, std::string> get_arguments( const std::variant<uint64_t, std::string_view> index_, gd::argument::arguments* parguments_ ) = 0;
   // virtual std::pair<bool, std::string> as_list( const std::variant<size_t, std::string_view> index_, std::vector< std::pair<std::string, gd::variant> >* pvector_ ) = 0;
};    

struct response_i : public unknown_i
{
   //virtual std::pair<bool, std::string> read( const gd::argument::arguments& argumentsRecepie ) = 0; 
   //virtual std::pair<bool, std::string> write( const gd::argument::arguments& argumentsRecepie ) = 0;

   /// add load to response from string, format descrbe what type of format it is
   virtual uint64_t size() = 0;
   virtual std::pair<bool, std::string> add( const gd::variant_view& key_, const gd::argument::arguments& argumentsValue ) = 0;
   virtual std::pair<bool, std::string> add( const gd::variant_view& key_, const gd::argument::arguments&& argumentsValue ) = 0;
   virtual std::pair<bool, std::string> add( const std::string_view& stringName, const std::string_view& stringLoad, unsigned uFormat ) = 0;
   virtual std::pair<bool, std::string> add_return( gd::variant&& variantValue ) = 0;
   virtual std::pair<bool, std::string> get( const gd::variant_view& index_, gd::argument::arguments** ppArguments ) = 0;
   virtual std::pair<bool, std::string> get_body( const std::variant<uint64_t, std::string_view>& index_, body_i** ppload_ ) = 0;
   virtual std::pair<bool, std::string> add_body( body_i* pload_ ) = 0;
   virtual uint64_t get_body_count() = 0;
   virtual void clear_all() = 0;
   // virtual size_t load_size() = 0;
   // virtual load_i* load_get( size_t uIndex ) = 0;
};    

struct request_i : public unknown_i
{
   virtual std::pair<bool, std::string> read( const gd::argument::arguments& argumentsRecepie ) = 0; 
   virtual std::pair<bool, std::string> write( const gd::argument::arguments& argumentsRecepie ) = 0;
};    

struct server_i : public unknown_i
{
   virtual std::pair<bool, std::string> get( command_i* pcommand_, response_i* presponse ) = 0; 
   virtual std::pair<bool, std::string> get( const char* pbszCommand, response_i* presponse ) = 0; 
   virtual void add_error( const std::variant<std::string_view, const gd::argument::arguments*>& error_ ) = 0;
   virtual unsigned get_error( std::vector< std::string >* pvectorError, bool bRemove ) = 0;
   //virtual std::pair<bool, std::string> get( command_i* pcommand_, const gd::argument::arguments& argumentsRecepie, const request_i* prequest, const response_i* pesponse ) = 0; 
   //virtual std::pair<bool, std::string> post( command_i* pcommand_, const gd::argument::arguments& argumentsRecepie, const request_i* prequest, const response_i* pesponse ) = 0; 
};

/** ---------------------------------------------------------------------------
 * @brief 
 */
struct command : public command_i
{
   server_i* get_server() override { return nullptr; }
   int32_t query_interface(const com::guid& guidId, void** ppObject) override { return 0; }
   unsigned add_reference() override { return 0; }
   unsigned release() override { return 0; }
   //std::string add( const std::string_view& stringName, const std::string_view& stringUriCommand ) override { return std::string(); }
   //std::string add( const std::string_view& stringName, const gd::argument::arguments& argumentsCommand ) override { return std::string(); }
   std::pair<bool, std::string> add_arguments( const gd::argument::arguments* pargumentsGlobal ) override { return { true, "" }; }
   std::pair<bool, std::string> add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments* pargumentsGlobal ) override { return { true, "" }; }
   std::pair<bool, std::string> add_command_arguments( const gd::argument::arguments* pargumentsGlobal ) override { return { true, "" }; }
   std::pair<bool, std::string> add_command_arguments( const std::string_view& stringName, const gd::argument::arguments* pargumentsGlobal ) override { return { true, "" }; }
   void get_arguments( gd::argument::arguments** ppargumentsGlobal ) override { *ppargumentsGlobal = nullptr; }
   //gd::variant_view get_command() override { return gd::variant_view(); }
   gd::variant_view get_argument( const gd::variant_view& index_ ) override { return gd::variant_view(); }
   //std::pair<bool, std::string> get_list( const std::variant<size_t, std::string_view> index_, std::vector< std::pair<std::string, gd::variant> >** ppvector_ ) override { return { true, "" }; }
   //uint64_t get_list_count() override { return 0; }
   std::pair<bool, std::string> get_arguments( const std::variant<uint64_t, std::string_view> index_, gd::argument::arguments* parguments_ ) override { return { true, "" }; }
   // std::pair<bool, std::string> as_list( const std::variant<size_t, std::string_view> index_, std::vector< std::pair<std::string, gd::variant> >* pvector_ ) override { return { true, "" }; }
};

/** ---------------------------------------------------------------------------
 * @brief body is used to transport some sort of dataobject with information
 * @note access data by getting pointer to it, what pointer it is depends on implementation
 */
struct body : public body_i
{
   int32_t query_interface(const com::guid& guidId, void** ppObject) override { return 0; }
   unsigned add_reference() override { return 0; }
   unsigned release() override { return 0; }

   unsigned type() override { return 0; }
   std::string_view type_name() override { return std::string_view(); }
   void* get() override { return nullptr; };
   void destroy() override {};
};


/** ---------------------------------------------------------------------------
 * @brief 
 */
struct response : public response_i
{
   int32_t query_interface(const com::guid& guidId, void** ppObject) override { return 0; }
   unsigned add_reference() override { return 0; }
   unsigned release() override { return 0; }

   uint64_t size() override { return 0; }
   std::pair<bool, std::string> add( const gd::variant_view& key_, const gd::argument::arguments& argumentsValue ) override { return { false, "" }; }
   std::pair<bool, std::string> add( const gd::variant_view& key_, const gd::argument::arguments&& argumentsValue ) override { return { false, "" }; }
   std::pair<bool, std::string> add( const std::string_view& stringName, const std::string_view& stringLoad, unsigned uFormat ) override { return { false, "" }; }
   std::pair<bool, std::string> add_return( gd::variant&& variantValue ) override { return { false, "" }; }
   std::pair<bool, std::string> get( const gd::variant_view& index_, gd::argument::arguments** ppArguments ) override { return { false, "" }; }
   std::pair<bool, std::string> get_body( const std::variant<uint64_t, std::string_view>& index_, body_i** ppload_ ) override { return { false, "" }; }
   std::pair<bool, std::string> add_body( body_i* pload_ ) override { return { false, "" }; }
   uint64_t get_body_count() override { return 0; }
   void clear_all() override {};
};


struct server : public server_i
{
   int32_t query_interface(const com::guid& guidId, void** ppObject) override { return 0; }
   unsigned add_reference() override { return 0; }
   unsigned release() override { return 0; }
   std::pair<bool, std::string> get( command_i* pcommand_, response_i* presponse ) override { return { false, "" }; }
   std::pair<bool, std::string> get( const char* pbszCommand, response_i* presponse ) override { return { false, "" }; }
   void add_error( const std::variant<std::string_view, const gd::argument::arguments*>& perror_ ) override {}
   unsigned get_error( std::vector< std::string >* pvectorError, bool bRemove ) override { return 0; }
};


} } // com::server 

_GD_END

_GD_BEGIN
namespace com { namespace server { namespace router {

/**
 * \brief server implementation that with similar logics found in web routers
 *
 *
 *
 \code
 \endcode
 */
struct command : public gd::com::server::command_i
{
   /**
    * @brief arguments is used to hold arguments connected to one specifc command
    */
   struct arguments
   {
      arguments( const std::string_view& stringKey, const gd::argument::arguments& arguments_ ): m_stringKey( stringKey ), m_arguments( arguments_ ) {}
      arguments( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments& arguments_ ): m_stringKey( stringKey ), m_stringCommand( stringCommand ), m_arguments( arguments_ ) {}
      arguments( int iIndex, const std::string_view& stringCommand, const gd::argument::arguments& arguments_ ): m_iCommandIndex( iIndex ), m_stringCommand( stringCommand ), m_arguments( arguments_ ) {}
      arguments( int iIndex, const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments& arguments_ ): m_iCommandIndex( iIndex ), m_stringKey(stringKey), m_stringCommand( stringCommand ), m_arguments( arguments_ ) {}
      arguments( const std::pair< std::string, gd::argument::arguments >& pair_ ): m_stringKey(pair_.first), m_arguments( pair_.second ) {}
      arguments( const arguments& o ): m_iCommandIndex(o.m_iCommandIndex), m_stringKey( o.m_stringKey ), m_stringCommand( o.m_stringCommand ), m_arguments( o.m_arguments ) {}
      arguments( arguments&& o ): m_iCommandIndex(o.m_iCommandIndex), m_stringKey( std::move( o.m_stringKey ) ), m_stringCommand( std::move( o.m_stringCommand ) ), m_arguments( std::move( o.m_arguments ) ) {}
      operator int() const { return m_iCommandIndex; }
      operator const gd::argument::arguments&() const { return m_arguments; }
      operator gd::argument::arguments*() { return &m_arguments; }
      operator const gd::argument::arguments*() const { return &m_arguments; }
      bool operator==( const std::string_view& stringMatch ) const { return m_stringKey == stringMatch; }
      const gd::argument::arguments& get_arguments() const { return m_arguments; }
      int m_iCommandIndex = -1;
      std::string m_stringKey;      ///< command key to access command, this is also used to connect return values
      std::string m_stringCommand;  ///< command namen if command is specified here and not in uri
      gd::argument::arguments m_arguments; // parameters for command
   };

   command() {}
   command( gd::com::server::server_i* pserver ): m_pserver( pserver ) {}
   virtual ~command() {}

   int32_t query_interface(const gd::com::guid& guidId, void** ppObject) override;
   unsigned add_reference() override { m_iReference++; return (unsigned)m_iReference; }
   unsigned release() override;

   server_i* get_server() override { return m_pserver; }
   /// add global arguments, all commands in command object are able to use global arguments
   std::pair<bool, std::string> add_arguments( const gd::argument::arguments* pargumentsGlobal ) override;   
   std::pair<bool, std::string> add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments* pargumentsLocal ) override;
   std::pair<bool, std::string> add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments& argumentsLocal );
   std::pair<bool, std::string> add_command_arguments( const gd::argument::arguments* pargumentsGlobal ) override;
   std::pair<bool, std::string> add_command_arguments( const std::string_view& stringName, const gd::argument::arguments* pargumentsLocal ) override;
   void get_arguments( gd::argument::arguments** ppargumentsGlobal ) override;
   gd::variant_view get_argument( const gd::variant_view& index_ ) override;
   std::pair<bool, std::string> get_arguments( const std::variant<uint64_t, std::string_view> index_, gd::argument::arguments* parguments_ ) override;

   /// find pointer to arguments for name (should match any of command name found in command object)
   const gd::argument::arguments* find( const std::string_view& stringName ) const;   
   gd::argument::arguments* find( const std::string_view& stringName );   

   int m_iReference = 1;
   gd::com::server::server_i* m_pserver = nullptr;
   std::string m_stringCommand;
   gd::argument::arguments m_argumentsGlobal;                                  ///< global arguments
   std::vector< arguments > m_vectorLocal;// local arguments for methods
};

/// adds command information to command object, command are able to hold more than one command
inline std::pair<bool, std::string> command::add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments& argumentsLocal ) {
   return add_command( stringKey, stringCommand, &argumentsLocal );
}

/// return pointer to arguments for selected name
inline const gd::argument::arguments* command::find( const std::string_view& stringName ) const {
   for( const auto& it : m_vectorLocal ) { if( it == stringName ) return (const gd::argument::arguments*)it; }
   return nullptr;
}

/// return pointer to arguments for selected name
inline gd::argument::arguments* command::find( const std::string_view& stringName ) {
   for( auto& it : m_vectorLocal ) { if( it == stringName ) return (gd::argument::arguments*)it; }
   return nullptr;
}

/** ---------------------------------------------------------------------------
 * \brief server implementation that with similar logics found in web routers
 *
 * @code
// adding callback to member method in some object, note that you need placeholders
auto pserver = gd::com::pointer< gd::com::server::router::server >( new gd::com::server::router::server );
pserver->callback_add( std::bind( &CServer::ExecuteCommand, &serverApplication, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );
 * @endcode
 *
 */
struct server : public gd::com::server::server_i
{
   /// to not make the code to messy this long callback declaration is used
   using type_callback = std::function< std::pair<bool, std::string>( const std::string_view&, gd::com::server::command_i*, gd::com::server::response_i*) >;

   server() {}
   server( char chSplitChar ): m_uSplitChar( chSplitChar ) {  }
   int32_t query_interface(const gd::com::guid& guidId, void** ppObject) override;
   unsigned add_reference() override { m_iReference++; return (unsigned)m_iReference; }
   unsigned release() override;

   std::pair<bool, std::string> get( command_i* pcommand_, response_i* presponse ) override { return { true, "" }; }
   std::pair<bool, std::string> get( const char* pbszCommand, response_i* presponse ) override { return { true, "" }; } 
   virtual std::pair<bool, std::string> get( const std::string_view* stringCommandList, const gd::argument::arguments* pargumentsParameter, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse );
   std::pair<bool, std::string> get( const std::string_view& stringCommandList, const gd::argument::arguments& argumentsParameter, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse ); 
   void add_error( const std::variant<std::string_view, const gd::argument::arguments*>& error_ ) override;
   unsigned get_error( std::vector< std::string >* pvectorError, bool bRemove ) override;

   template<typename FUNCTION>
   void callback_add( FUNCTION&& callback_ ) { m_vectorCallback.push_back( std::forward<FUNCTION>( callback_ ) ); }
   bool callback_empty() const { return m_vectorCallback.empty(); }
   std::size_t callback_size() const { return m_vectorCallback.size(); }
   void callback_clear() { return m_vectorCallback.clear(); }

// ## attributes ----------------------------------------------------------------
   int m_iReference = 1;
   uint8_t m_uSplitChar = ';';
   std::vector< type_callback > m_vectorCallback;
   std::vector<std::string> m_vectorError;   ///< list of errors if something went wrong
};

/// Wrapper for interface method used to call `get` command sending command as string and arguments
inline std::pair<bool, std::string> server::get( const std::string_view& stringCommandList, const gd::argument::arguments& argumentsParameter, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse ) {
   return get( &stringCommandList, &argumentsParameter, pcommand, presponse );
}


} } } // com::server::router
_GD_END

#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
