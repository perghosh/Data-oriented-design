#pragma once

#include <cassert>
#include <functional>
#include <string>
#include <string_view>

#include "gd_types.h"
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
   ePriorityUnknown  = 0,
   ePriorityRegister = 0x01,     ///< register is the highest priority, named like this to mimic programming (only one register sequance is allowed)
   ePriorityStack    = 0x02,     ///< like the closest stack value, removed when command is executed
   ePriorityCommand  = 0x04,     ///< follow command
   ePriorityGlobal   = 0x08,     ///< global reach within command

   ePriorityMAX      = ePriorityGlobal, ///< max priority value
};

/// get priority contant from string 
constexpr enumPriority priority_g( const std::string_view& stringPriority )
{                                                                                                  assert( stringPriority.length() >= 3 );
   using namespace gd::types::detail;
   enumPriority ePriority = ePriorityUnknown;

   uint32_t uPriorityName = hash_type( stringPriority );
   switch( uPriorityName )
   {
   case hash_type("unkn"): ePriority = ePriorityUnknown;  break;               // unknown (0)
   case hash_type("regi"): ePriority = ePriorityRegister;  break;              // register priority, highest type
   case hash_type("stac"): ePriority = ePriorityStack;  break;                 // stack priority, like lokals
   case hash_type("comm"): ePriority = ePriorityCommand;  break;               // command or members for specific named command
   case hash_type("glob"): ePriority = ePriorityGlobal;  break;                // globals, accessible for all
   default: assert(false);
   }

   return ePriority;
}

/// all flags, used this to mask
constexpr unsigned uPriorityAll_g = ePriorityRegister + ePriorityStack + ePriorityCommand + ePriorityGlobal;


struct body_i : public unknown_i
{
   virtual unsigned type() = 0;
   virtual std::string_view type_name() = 0;
   virtual void* get() = 0;
   virtual void destroy() = 0;
};

// command format - command/sub-command/sub-sub-command

/** ---------------------------------------------------------------------------
 * @brief command information, what to execute in `server`
 * 
 * command interface is used to hold information about what type of operations to execute in server.
 * command is used command sequance to execute, this can be from one to any number of commands and these
 * commands are passed to server. server is trversing commans sent and executes them in order
 */
struct command_i : public unknown_i
{
   virtual server_i* get_server() = 0;
   virtual std::pair<bool, std::string> add_arguments( const gd::variant_view& variantviewLocality, const gd::argument::arguments* pargumentsValue ) = 0;
   virtual std::pair<bool, std::string> add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments* pargumentsLocal ) = 0;
   virtual gd::variant_view get_argument( const gd::variant_view& index_ ) = 0;
   virtual gd::argument::arguments get_all_arguments( const gd::variant_view& index_ ) = 0;
   virtual std::pair<bool, std::string> get_arguments( const std::variant<uint64_t, std::string_view> index_, gd::argument::arguments* parguments_ ) = 0;
   virtual std::pair<bool, std::string> query_select( unsigned uPriority, const gd::variant_view& selector_, gd::variant_view* pvariantview_ ) = 0;
   virtual std::pair<bool, std::string> query_select_all( const gd::variant_view& selector_, std::vector<gd::variant_view>* pvectorValue ) = 0;
   virtual void clear( const gd::variant_view& variantviewType ) = 0;
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
 * @brief stubs for command interface, to simplify getting started with command object
 */
struct command : public command_i
{
   server_i* get_server() override { return nullptr; }
   int32_t query_interface(const com::guid& guidId, void** ppObject) override { return 0; }
   unsigned add_reference() override { return 0; }
   unsigned release() override { return 0; }
   std::pair<bool, std::string> add_arguments( const gd::variant_view& variantviewLocality, const gd::argument::arguments* pargumentsGlobal ) override { return { true, "" }; }
   std::pair<bool, std::string> add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments* pargumentsGlobal ) override { return { true, "" }; }
   gd::variant_view get_argument( const gd::variant_view& index_ ) override { return gd::variant_view(); }
   gd::argument::arguments get_all_arguments( const gd::variant_view& index_ ) override { return gd::argument::arguments(); }
   std::pair<bool, std::string> get_arguments( const std::variant<uint64_t, std::string_view> index_, gd::argument::arguments* parguments_ ) override { return { true, "" }; }
   std::pair<bool, std::string> query_select( unsigned uPriority, const gd::variant_view& selector_, gd::variant_view* pvariantview_ ) override { return { true, "" }; }
   std::pair<bool, std::string> query_select_all( const gd::variant_view& selector_, std::vector<gd::variant_view>* pvectorValue ) override { return { true, "" }; }
   void clear( const gd::variant_view& variantviewType ) override {}
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
 * @brief stubs for response interface, to simplify getting started with respons object
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


/** ---------------------------------------------------------------------------
 * @brief stubs for server interface, to simplify getting started with server object
 */
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

/** ---------------------------------------------------------------------------
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
      arguments( unsigned uPriority, const std::string_view& stringKey, const gd::argument::arguments& arguments_ ): m_uPriority(uPriority), m_stringKey( stringKey ), m_arguments( arguments_ ) {}
      arguments( unsigned uPriority, const gd::argument::arguments& arguments_ ): arguments( uPriority, std::string_view(), arguments_ ) {}
      arguments( const std::string_view& stringKey, const gd::argument::arguments& arguments_ ): arguments( ePriorityCommand, stringKey, arguments_ ) {}
      arguments( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments& arguments_ ): arguments( ePriorityCommand, stringKey, arguments_ ) {}

      arguments( const std::pair< std::string, gd::argument::arguments >& pair_ ): m_stringKey(pair_.first), m_arguments( pair_.second ) {}

      arguments( const arguments& o ) { common_construct( o ); }
      arguments( arguments&& o ) noexcept { common_construct( std::move( o ) ); }

      arguments& operator=( const arguments& o ) { common_construct( o ); return *this; }
      arguments& operator=( arguments&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

      void common_construct( const arguments& o ) {
         m_uPriority = o.m_uPriority; m_iCommandIndex = o.m_iCommandIndex; m_stringKey = o.m_stringKey; m_stringCommand = o.m_stringCommand; m_arguments = o.m_arguments;
      }
      void common_construct( arguments&& o ) noexcept {
         m_uPriority = o.m_uPriority; m_iCommandIndex = o.m_iCommandIndex; m_stringKey = std::move( o.m_stringKey ); m_stringCommand = std::move( o.m_stringCommand ); m_arguments = std::move( o.m_arguments );
      }

      // ## operator
      operator int() const { return m_iCommandIndex; }
      // operator const gd::argument::arguments&() const { return m_arguments; }
      operator gd::argument::arguments*() { return &m_arguments; }
      operator const gd::argument::arguments*() const { return &m_arguments; }
      bool operator==( const std::string_view& stringMatch ) const { return m_stringKey == stringMatch; }

      // ## get/set operations
      const std::string& get_key() const { return m_stringKey; }
      const gd::argument::arguments& get_arguments() const { return m_arguments; }
      unsigned get_priority() const { return m_uPriority; }
      void set_index( int iIndex ) { m_iCommandIndex = iIndex; }

      unsigned m_uPriority = ePriorityGlobal; ///< priority, this is used to order values and in what order values are looked for
      int m_iCommandIndex = -1;     ///<
      std::string m_stringKey;      ///< command key to access command, this is also used to connect return values
      std::string m_stringCommand;  ///< command namen if command is specified here and not in uri
      gd::argument::arguments m_arguments; // parameters for command
   };

   command() {}
   command( gd::com::server::server_i* pserver ): m_pserver( pserver ) {}
   virtual ~command() {}

   // ## unknown_i interface

   int32_t query_interface(const gd::com::guid& guidId, void** ppObject) override;
   unsigned add_reference() override { m_iReference++; return (unsigned)m_iReference; }
   unsigned release() override;

   // ## get/set

   server_i* get_server() override { return m_pserver; }
   void set_command( const std::string_view& stringCommand ) { m_stringCommand = stringCommand; }
   /// add global arguments, all commands in command object are able to use global arguments
   std::pair<bool, std::string> add_arguments( const gd::variant_view& variantviewLocality, const gd::argument::arguments* pargumentsGlobal ) override;
   /// add query string variables as stack values
   std::pair<bool, std::string> add_querystring( const gd::variant_view& variantviewLocality, const std::string_view& stringQueryString );
   /// Wraper to manage full url sent internally, no runtime error checks
   std::vector< std::string_view > add_querystring( const std::string_view& stringQueryString );

   /// add command and arguments for that command
   std::pair<bool, std::string> add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments* pargumentsLocal ) override;
   std::pair<bool, std::string> add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments& argumentsLocal );
   //void get_arguments( gd::argument::arguments** ppargumentsGlobal ) override;
   gd::variant_view get_argument( const gd::variant_view& index_ ) override;
   gd::argument::arguments get_all_arguments( const gd::variant_view& index_ ) override;
   std::pair<bool, std::string> get_arguments( const std::variant<uint64_t, std::string_view> index_, gd::argument::arguments* parguments_ ) override;
   std::pair<bool, std::string> query_select( unsigned uPriority, const gd::variant_view& selector_, gd::variant_view* pvariantview_ ) override;
   /// wrapper to select first value for name
   gd::variant_view query_select( const std::string_view& stringSelector );
   std::pair<bool, std::string> query_select_all( const gd::variant_view& selector_, std::vector<gd::variant_view>* pvectorValue ) override;

   /// Clear internal data in command, based on whats passed different type of data can be cleared
   void clear( const gd::variant_view& variantviewToClear );


   /// find pointer to arguments for name (should match any of command name found in command object)
   const gd::argument::arguments* find( const std::string_view& stringName ) const;   
   gd::argument::arguments* find( const std::string_view& stringName );

   size_t find_last_priority_position( unsigned uPriority ) const;

   int m_iReference = 1;
   gd::com::server::server_i* m_pserver = nullptr;
   std::string m_stringCommand;
   std::vector< arguments > m_vectorArgument;// variables in command, priority decides how to search for value
};

/// adds command information to command object, command are able to hold more than one command
inline std::pair<bool, std::string> command::add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments& argumentsLocal ) {
   return add_command( stringKey, stringCommand, &argumentsLocal );
}

/// return pointer to arguments for selected name
inline const gd::argument::arguments* command::find( const std::string_view& stringKey ) const {
   for( const auto& it : m_vectorArgument ) { if( it == stringKey ) return (const gd::argument::arguments*)it; }
   return nullptr;
}

/// return pointer to arguments for selected name
inline gd::argument::arguments* command::find( const std::string_view& stringKey ) {
   for( auto& it : m_vectorArgument ) { if( it == stringKey ) return (gd::argument::arguments*)it; }
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
   std::pair<bool, std::string> get( const std::string_view& stringCommandList, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse ); 
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

/// Wrapper for interface method used to call `get` command sending command as string and arguments
inline std::pair<bool, std::string> server::get( const std::string_view& stringCommandList, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse ) {
   return get( &stringCommandList, nullptr, pcommand, presponse );
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
