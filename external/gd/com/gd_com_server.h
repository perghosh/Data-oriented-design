#pragma once

#include <cassert>
#include <functional>
#include <string>
#include <string_view>

#include "../gd_types.h"
#include "../gd_arguments.h"
#include "../gd_strings.h"
#include "../gd_variant_view.h"

#include "../gd_com.h"

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
   ePriorityALL      = ePriorityRegister + ePriorityStack + ePriorityCommand + ePriorityGlobal, ///< all priorities

   ePriorityMAX      = ePriorityGlobal, ///< max priority value
};

/// get priority contant from string 
constexpr unsigned to_command_priority_g( const std::string_view& stringPriority )
{                                                                                                  assert( stringPriority.length() >= 3 );
   using namespace gd::types::detail;
   unsigned uPriority = ePriorityUnknown;

   uint32_t uPriorityName = hash_type16( stringPriority );
   switch( uPriorityName )
   {
   case hash_type16( "un" ): uPriority = ePriorityUnknown;  break;               // unknown (0)
   case hash_type16( "re" ): uPriority = ePriorityRegister;  break;              // register priority, highest type
   case hash_type16( "st" ): uPriority = ePriorityStack;  break;                 // stack priority, like lokals
   case hash_type16( "co" ): uPriority = ePriorityCommand;  break;               // command or members for specific named command
   case hash_type16( "gl" ): uPriority = ePriorityGlobal;  break;                // globals, accessible for all
   case hash_type16( "al" ): uPriority = ( ePriorityRegister | ePriorityStack | ePriorityGlobal );  break;
   default: assert(false);
   }

   return uPriority;
}

/// all flags, used this to mask
constexpr unsigned uPriorityAll_g = ePriorityRegister + ePriorityStack + ePriorityCommand + ePriorityGlobal;


struct body_i : public unknown_i
{
   virtual unsigned type() = 0;
   virtual std::string_view name() = 0;
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

/**
 * @brief Defines the interface for command management operations.
 *
 * This interface provides methods for managing commands and their associated arguments
 * within a server environment. It inherits from `unknown_i`, suggesting it can be used
 * with dynamic casting or interface querying mechanisms.
 *
 * @note All methods in this interface are pure virtual, requiring implementation by derived classes.
 *
 * @method get_server
 *   - Retrieves the server instance associated with this command interface.
 *   - @return Pointer to a server_i object, or nullptr if not available.
 *
 * @method add_arguments
 *   - Adds arguments to the command based on the provided locality and argument set.
 *   - @param variantviewLocality The locality context for the arguments.
 *   - @param pargumentsValue Pointer to the arguments to be added.
 *   - @return std::pair<bool, std::string> where bool indicates success, and string provides any error message.
 *
 * @method add_command
 *   - Adds a new command with its key, command string, and local arguments.
 *   - @param stringKey The unique identifier for the command.
 *   - @param stringCommand The command string to execute.
 *   - @param pargumentsLocal Pointer to local arguments for this command. Arguments contains values for the command. These gets the priority `ePriorityCommand`.
 *   - @return std::pair<bool, std::string> where bool indicates if the command was added successfully.
 *
 * @method get_argument
 *   - Retrieves a specific argument based on an index and priority.
 *   - @param index_ The index or identifier for the argument.
 *   - @param uPriority The priority level for argument selection. How this priority is used is up to the implementation.
 *   - @return gd::variant_view representing the argument value.
 *
 * @method get_all_arguments
 *   - Retrieves all arguments associated with a given index.
 *   - @param index_ The index or identifier for the arguments.
 *   - @return gd::argument::arguments object containing all related arguments.
 *
 * @method get_arguments
 *   - Fetches arguments for a given index, which can be either numeric or string-based.
 *   - @param index_ A variant that could be either uint64_t or std::string_view to specify the argument set.
 *   - @param parguments_ Pointer to where the arguments should be stored.
 *   - @return std::pair<bool, std::string> indicating success and any error message.
 *
 * @method query_select
 *   - Executes a query with specific priority and selector to return a single result.
 *   - @param uPriority The priority level for the query.
 *   - @param selector_ The selector used for the query.
 *   - @param pvariantview_ Pointer to store the result of the query.
 *   - @return std::pair<bool, std::string> indicating if the query was successful.
 *
 * @method query_select_all
 *   - Executes a query to retrieve all matching results based on the given selector.
 *   - @param selector_ The selector for the query.
 *   - @param pvectorValue Pointer to a vector where results will be stored.
 *   - @return std::pair<bool, std::string> indicating if the query operation was successful.
 *
 * @method clear
 *   - Clears all commands or arguments of a specific type.
 *   - @param variantviewType The type of commands or arguments to clear.
 */
struct command_i : public unknown_i
{
   virtual server_i* get_server() = 0;
   virtual std::pair<bool, std::string> add_arguments( const gd::variant_view& variantviewLocality, const gd::argument::arguments* pargumentsValue ) = 0;
   virtual std::pair<bool, std::string> add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments* pargumentsLocal ) = 0;
   //virtual uint64_t get_command_count() = 0;
   //virtual gd::variant_view get_argument( const gd::variant_view& index_, int32_t iCommandIndex, uint32_t uPriority ) = 0;
   virtual gd::argument::arguments get_all_arguments( const gd::variant_view& index_ ) = 0;
   virtual std::pair<bool, std::string> get_command( const gd::variant_view& index_, void** ppCommand ) = 0;
   virtual std::pair<bool, std::string> query_select( unsigned uPriority, const gd::variant_view& selector_, gd::variant_view* pvariantview_ ) = 0;
   virtual std::pair<bool, std::string> query_select_all( const gd::variant_view& selector_, std::vector<gd::variant_view>* pvectorValue ) = 0;
   virtual void clear( const gd::variant_view& variantviewType ) = 0;
};    

struct response_i : public unknown_i
{
   /// add load to response from string, format descrbe what type of format it is
   virtual uint64_t size() = 0;
   virtual std::pair<bool, std::string> add( const gd::variant_view& key_, const gd::argument::arguments& argumentsValue ) = 0;
   virtual std::pair<bool, std::string> add( const gd::variant_view& key_, const gd::argument::arguments&& argumentsValue ) = 0;

   virtual std::pair<bool, std::string> return_add( gd::variant* pvariantKey, gd::variant* pvariantValue ) = 0;
   virtual gd::variant_view return_at( uint32_t uIndex ) = 0;
   virtual uint32_t return_size() = 0;

   virtual std::pair<bool, std::string> get( const gd::variant_view& index_, gd::argument::arguments** ppArguments ) = 0;
   virtual std::pair<bool, std::string> body_get( const std::variant<uint64_t, std::string_view>& index_, body_i** ppload_ ) = 0;
   virtual std::pair<bool, std::string> body_add( body_i* pload_ ) = 0;
   virtual uint32_t body_size() = 0;
   virtual void clear_all() = 0;
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
   virtual bool is_endpoint( const std::string_view& stringCommand ) = 0;
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
   //gd::variant_view get_argument( const gd::variant_view& index_, int32_t iCommandIndex, uint32_t uPriority ) override { return gd::variant_view(); }
   gd::argument::arguments get_all_arguments( const gd::variant_view& index_ ) override { return gd::argument::arguments(); }
   std::pair<bool, std::string> get_command( const gd::variant_view& index_, void** ppCommand ) override { return { true, "" }; }
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
   std::string_view name() override { return std::string_view(); }
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
   //std::pair<bool, std::string> add( const std::string_view& stringName, const std::string_view& stringLoad, unsigned uFormat ) override { return { false, "" }; }
   //std::pair<bool, std::string> add_return( gd::variant&& variantValue ) override { return { false, "" }; }
   std::pair<bool, std::string> return_add( gd::variant* pvariantKey, gd::variant* pvariantValue ) override { return { false, "" }; }
   gd::variant_view return_at( uint32_t uIndex ) override { return gd::variant_view(); };
   uint32_t return_size() override { return 0; };
   std::pair<bool, std::string> get( const gd::variant_view& index_, gd::argument::arguments** ppArguments ) override { return { false, "" }; }
   std::pair<bool, std::string> body_get( const std::variant<uint64_t, std::string_view>& index_, body_i** ppload_ ) override { return { false, "" }; }
   std::pair<bool, std::string> body_add( body_i* pload_ ) override { return { false, "" }; }
   uint32_t body_size() override { return 0; }
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
   bool is_endpoint(const std::string_view& stringCommand) override { return false; }
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
      arguments( unsigned uPriority, const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments& arguments_ ): m_uPriority(uPriority), m_stringKey( stringKey ), m_arguments( arguments_ ) { m_strings32Command.append( stringCommand ); }
      arguments( unsigned uPriority, const std::string_view& stringKey, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& arguments_ ): m_uPriority(uPriority), m_stringKey( stringKey ), m_strings32Command(vectorCommand), m_arguments( arguments_ ) {}
      arguments( unsigned uPriority, const gd::argument::arguments& arguments_ ): arguments( uPriority, std::string_view(), arguments_ ) {}
      arguments(unsigned uPriority, const std::vector<std::string_view>& vectorCommand) : m_uPriority(uPriority), m_strings32Command(vectorCommand) {}
      arguments( const std::string_view& stringKey, const gd::argument::arguments& arguments_ ): arguments( ePriorityCommand, stringKey, arguments_ ) {}
      arguments( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments& arguments_ ): arguments( ePriorityCommand, stringKey, stringCommand, arguments_ ) {}
      arguments( const std::string_view& stringKey, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& arguments_ ): arguments( ePriorityCommand, stringKey, vectorCommand, arguments_ ) {}

      arguments( const std::pair< std::string, gd::argument::arguments >& pair_ ): m_stringKey(pair_.first), m_arguments( pair_.second ) {}

      arguments( const arguments& o ) { common_construct( o ); }
      arguments( arguments&& o ) noexcept { common_construct( std::move( o ) ); }

      arguments& operator=( const arguments& o ) { common_construct( o ); return *this; }
      arguments& operator=( arguments&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

      void common_construct( const arguments& o ) {
         m_uPriority = o.m_uPriority; m_iCommandIndex = o.m_iCommandIndex; m_stringKey = o.m_stringKey; m_strings32Command = o.m_strings32Command; m_arguments = o.m_arguments;
      }
      void common_construct( arguments&& o ) noexcept {
         m_uPriority = o.m_uPriority; m_iCommandIndex = o.m_iCommandIndex; m_stringKey = std::move( o.m_stringKey ); m_strings32Command = std::move( o.m_strings32Command ); m_arguments = std::move( o.m_arguments );
      }

      // ## operator
      std::string_view operator[](size_t uIndex) const { return m_strings32Command[uIndex]; }

      operator int() const { return m_iCommandIndex; }
      operator std::vector<std::string>() const { std::vector<std::string> v_; m_strings32Command.get( v_ ); return v_; }
      operator std::vector<std::string_view>() const { std::vector<std::string_view> v_; m_strings32Command.get( v_ ); return v_; }
      // operator const gd::argument::arguments&() const { return m_arguments; }
      operator gd::argument::arguments*() { return &m_arguments; }
      operator const gd::argument::arguments*() const { return &m_arguments; }
      bool operator==( uint32_t uIndexKey ) const { return compare( uIndexKey ); }
      bool operator==( const std::string_view& stringMatch ) const { return m_stringKey == stringMatch; }

      // ## get/set operations
      bool is_command() const { return m_uPriority & ePriorityCommand; }
      const std::string& get_key() const { return m_stringKey; }
      const gd::argument::arguments& get_arguments() const { return m_arguments; }
      gd::variant_view get_variant_view(const std::string_view& stringName) const { return m_arguments[stringName]; }
      gd::variant_view get_variant_view(unsigned uIndex) const { return m_arguments[uIndex]; }
      unsigned get_priority() const { return m_uPriority; }
      void set_index( int iIndex ) { m_iCommandIndex = iIndex; }
      int32_t get_index() const { return (uint32_t)m_iCommandIndex; }
      bool compare(uint32_t uIndexKey) const { return m_iCommandIndex == (int32_t)uIndexKey; }

      arguments& append(const std::vector<std::pair<std::string_view,std::string_view>>& vectorArgument) { m_arguments.append(vectorArgument, gd::argument::tag_parse_type{}); return *this; }
      arguments& append(const std::vector<std::pair<std::string,std::string>>& vectorArgument) { m_arguments.append(vectorArgument, gd::argument::tag_parse_type{}); return *this; }
      arguments& append(const std::pair<std::string_view, gd::variant>& pairArgument) { m_arguments.append(pairArgument); return *this; }
      arguments& append(const gd::argument::arguments& arguments_ ) { m_arguments.append(arguments_); return *this; }

      std::string print() const;

   // ## attributes ----------------------------------------------------------------
      unsigned m_uPriority = ePriorityGlobal; ///< priority, this is used to order values and in what order values are looked for
      int m_iCommandIndex = -1;     ///<
      std::string m_stringKey;      ///< command key to access command, this is also used to connect return values
      gd::strings32 m_strings32Command; ///< command name sequence, like command/sub-command/sub-sub-command
      gd::argument::arguments m_arguments; ///< parameters for command
   };

// ## typedefs -----------------------------------------------------------------
   using iterator = std::vector<arguments>::iterator;
   using const_iterator = std::vector<arguments>::const_iterator; 

// ## construction -------------------------------------------------------------
   command() {}
   command( gd::com::server::server_i* pserver ): m_pserver( pserver ) {}
   virtual ~command() {}

   // ## unknown_i interface

   int32_t query_interface(const gd::com::guid& guidId, void** ppObject) override;
   unsigned add_reference() override { m_iReference++; return (unsigned)m_iReference; }
   unsigned release() override;

   // ## get/set
   arguments& operator[](size_t uIndex) { return m_vectorArgument[uIndex]; }
   arguments& at(size_t uIndex) { return m_vectorArgument[uIndex]; }

/** \name GET/SET
*///@{
   /// get server object for command (each command object is connected to one server object)
   server_i* get_server() override { return m_pserver; }
   /// activate specific command in command object, -1 is used to deactivate command
   void activate(int iIndex) { m_iCommandIndex = iIndex; assert( iIndex >= -1 && iIndex < size()); }
   /// Activate next command in command object, returns index of activated command
   int activate_next();
   /// get active command index
   int get_active() const { return m_iCommandIndex; }
//@}

/** \name OPERATION
*///@{
   /// add global arguments, all commands in command object are able to use global arguments
   std::pair<bool, std::string> add_arguments( const gd::variant_view& variantviewPriority, const gd::argument::arguments* pargumentsGlobal ) override;
   arguments* find_arguments( uint32_t uIndexKey );
   arguments* find_active_arguments() { return find_arguments(m_iCommandIndex); }
   void arguments_remove( unsigned uPriority );
   /// add query string variables as stack values
   std::pair<bool, std::string> add_querystring( const gd::variant_view& variantviewPriority, const std::string_view& stringQueryString );
   /// Wraper to manage full url sent internally, no runtime error checks
   std::vector< std::string_view > add_querystring( const std::string_view& stringQueryString );


/** \name APPEND
* Group of methods for appending or adding commands and arguments to an internal collection.
* These overloaded methods facilitate the addition of commands, arguments, some in uri format to a managed container (e.g., `m_vectorArgument`).
*///@{
   std::pair<bool, std::string> append(arguments&& argumentsCommand) { m_vectorArgument.push_back(std::move(argumentsCommand)); return { true, "" }; }
   std::pair<bool, std::string> append(const std::string_view& stringQueryString, gd::types::tag_uri) { return append(stringQueryString, gd::argument::arguments(), gd::types::tag_uri()); }
   std::pair<bool, std::string> append( const std::string_view& stringQueryString, const gd::argument::arguments& arguments_, gd::types::tag_uri );
   std::pair<bool, std::string> append( enumPriority ePriority, const gd::argument::arguments& arguments_ );
   std::pair<bool, std::string> append(uint32_t uPriority, const gd::argument::arguments& arguments_) { return append((enumPriority)uPriority, arguments_); }
   /// append variable with priority stack (mimics local variables)
   std::pair<bool, std::string> append(const gd::argument::arguments& arguments_, gd::types::tag_variable);
   std::pair<bool, std::string> append(uint32_t uPriority, const gd::argument::arguments& arguments_, gd::types::tag_variable);
   std::pair<bool, std::string> add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments* pargumentsLocal ) override;
   std::pair<bool, std::string> add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments& argumentsLocal );
   void add_command( const std::vector<std::string_view>& vectorCommand );
   void add_command( const std::string_view& stringKey, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsLocal );

   std::pair<bool, std::string> get_variable(gd::argument::arguments* parguments_, const std::variant<size_t, std::string_view>& priority_ );
   gd::argument::arguments get_variable(uint32_t uPriority, gd::types::tag_variable) { gd::argument::arguments arguments_; get_variable(&arguments_, uPriority); return arguments_; }
   /// get variables for command, it also returns variables for specified priorities
   std::pair<bool, std::string> get_command_variable(const std::variant<size_t, std::string_view> index_, const std::variant<size_t, std::string_view>& priority_, gd::argument::arguments* pargumentsVariable );
//@}


   // ## Methods to access internal command information, command may hold more than one command

   /// get command for index, index is used to find command in command object
   arguments* get_command(size_t uIndex) const { return uIndex < m_vectorArgument.size() ? const_cast<arguments*>(&m_vectorArgument[uIndex]) : nullptr; }
   arguments* get_command(const std::string_view& stringKey) const;

/** \name ARGUMENT
* Command holds argument values in different levels and these levels mimics how values are stored
* in computer. Register values have the highest priority, stack values are next and then command and last global.
* Get argument are mostly used by finding value for name but can be found using index.
* Running a command the normal way will be to first try to find value connected to the active command within
* command (this is the command that is active). If value is not found then register and stack values are searched and lastly
* global argument values.
*///@{
   gd::argument::arguments get_all_arguments( const gd::variant_view& index_ ) override;
   gd::argument::arguments get_all_arguments() { return get_all_arguments(gd::variant_view()); }
   /// get arguments for index, index can be numeric or string that identifies command
   std::pair<bool, std::string> get_command( const gd::variant_view& index_, void** ppCommand ) override;
   std::pair<bool, std::string> query_select( unsigned uPriority, const gd::variant_view& selector_, gd::variant_view* pvariantview_ ) override;
   /// wrapper to select first value for name
   gd::variant_view query_select( const std::string_view& stringSelector );
   gd::variant_view query_select( const std::string_view& stringSelector, const std::variant<size_t,std::string>& variantKey );
   gd::argument::arguments query_select( const std::initializer_list<std::string_view>& listSelector );
   gd::argument::arguments query_select( const std::initializer_list<std::string_view>& listSelector, const std::variant<size_t,std::string>& variantKey );
   std::pair<bool, std::string> query_select_all( const gd::variant_view& selector_, std::vector<gd::variant_view>* pvectorValue ) override;
//@}

/** \name MISCELLANEOUS
*///@{

   /// Get number of commands in command object (note that one command object could hold more than one command)
   size_t size() const { return m_vectorArgument.size(); }
   /// Count number of commands with specific priority
   size_t count( uint32_t uPriority ) const;
   /// Clear internal data in command, based on whats passed different type of data can be cleared
   void clear( const gd::variant_view& variantviewToClear ) override;
   void clear();
   /// Check if command object is empty
   bool empty() const { return m_vectorArgument.empty(); }
//@}

   /// find pointer to arguments for name (should match any of command name found in command object)
   const gd::argument::arguments* find( const std::string_view& stringName ) const;   
   gd::argument::arguments* find( const std::string_view& stringName );

   size_t find_last_priority_position( unsigned uPriority ) const;
   void sort();
   int next_command_index() { return m_iNextCommandIndex++; }

   // ## iterator methods

   iterator begin() { return m_vectorArgument.begin(); }
   iterator end() { return m_vectorArgument.end(); }
   const_iterator begin() const { return m_vectorArgument.begin(); }
   const_iterator end() const { return m_vectorArgument.end(); }
   const_iterator cbegin() const { return m_vectorArgument.cbegin(); }
   const_iterator cend() const { return m_vectorArgument.cend(); }

   /// print command object, useful for debugging
   std::string print() const;
//@}

// ## attributes ----------------------------------------------------------------
public:
   int m_iReference = 1;   ///< reference counter
   int m_iCommandIndex = -1; ///< index for active command
   int m_iNextCommandIndex = 0; ///< index for next free command index
   gd::com::server::server_i* m_pserver = nullptr; ///< server object that command is connected to
   std::vector< arguments > m_vectorArgument; ///< command and arguments or only arguments, priority decides how to search for argument value
   std::vector< arguments > m_vectorVariable; ///< variables that are used in command/commands stored in server
};

/// adds command information to command object, command are able to hold more than one command
inline std::pair<bool, std::string> command::add_command( const std::string_view& stringKey, const std::string_view& stringCommand, const gd::argument::arguments& argumentsLocal ) {
   return add_command( stringKey, stringCommand, &argumentsLocal );
}

/// adds command information to command object, command are able to hold a sequence of command names
inline void command::add_command(const std::vector<std::string_view>& vectorCommand) {
   m_vectorArgument.push_back( arguments( std::string(), vectorCommand, gd::argument::arguments()) );
}
/// adds command information to command object, command are able to hold a sequence of command names
inline void command::add_command(const std::string_view& stringKey, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsLocal) {
   m_vectorArgument.push_back( arguments( stringKey, vectorCommand, argumentsLocal ) );
}
/// get pointer to command for key value
inline command::arguments* command::get_command(const std::string_view& stringKey) const {
   for( auto& it : m_vectorArgument ) { 
      if( it == stringKey ) return (command::arguments*)&it;
   }
   return nullptr;
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

/// count number of commands with specific priority bits set and return how many commands that are found
inline size_t gd::com::server::router::command::count(uint32_t uPriority) const {
   size_t uCount = 0;
   for( const auto& a_ : m_vectorArgument ) { if( a_.get_priority() & uPriority ) { ++uCount; } }
   return uCount;
}



// ================================================================================================
// ======================================================================================= response
// ================================================================================================

/** ---------------------------------------------------------------------------
 * @brief `response` object is used to store responses from executed commands passed
 * to server. 
 */
struct response : public gd::com::server::response
{
   int32_t query_interface(const gd::com::guid& guidId, void** ppObject) override;
   unsigned add_reference() override { m_iReference++; return (unsigned)m_iReference; }
   unsigned release() override;

   // ## response_i
   /// return number of return objects
   uint64_t size() override { return 0; }
   /// add response data to internal list of respons information
   std::pair<bool, std::string> add( const gd::variant_view& key_, const gd::argument::arguments& argumentsValue ) override;
   std::pair<bool, std::string> add( const gd::variant_view& key_, const gd::argument::arguments&& argumentsValue ) override;
   //std::pair<bool, std::string> add_return( gd::variant&& variantValue ) override;
   //std::pair<bool, std::string> return_add( gd::variant&& variantValue ) override;
   std::pair<bool, std::string> return_add( gd::variant* pvariantKey, gd::variant* pvariantValue ) override;
   gd::variant_view return_at( uint32_t uIndex ) override { assert( uIndex < m_vectorReturn.size() ); return m_vectorReturn.at( uIndex ).second.as_variant_view(); }
   uint32_t return_size() override { return (uint32_t)m_vectorReturn.size(); }
   /// Get response data for index (named index that should match id in arguments or index)
   std::pair<bool, std::string> get( const gd::variant_view& index_, gd::argument::arguments** ppArguments ) override;
   std::pair<bool, std::string> body_get( const std::variant<uint64_t, std::string_view>& index_, gd::com::server::body_i** ppbody_ ) override;
   std::pair<bool, std::string> body_add( gd::com::server::body_i* pload_ ) override;
   uint32_t body_size() override;
   void clear_all() override;

   
   int m_iReference = 1;
   std::vector< std::pair<gd::variant, gd::variant> > m_vectorReturn; ///< store primitive return values from executed operations, each return can have some sort of key (reason for pair)
   std::vector< gd::com::server::body_i* > m_vectorBody;
};

// ================================================================================================
// ======================================================================================= server
// ================================================================================================

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
   bool is_endpoint(const std::string_view& stringCommand) override { return false; }
   void add_error( const std::variant<std::string_view, const gd::argument::arguments*>& error_ ) override;
   unsigned get_error( std::vector< std::string >* pvectorError, bool bRemove ) override;

   template<typename FUNCTION>
   void callback_add( FUNCTION&& callback_ ) { m_vectorCallback.push_back( std::forward<FUNCTION>( callback_ ) ); }
   /// check if callbacks are added to server
   bool callback_empty() const { return m_vectorCallback.empty(); }
   /// return number of callbacks added to server
   std::size_t callback_size() const { return m_vectorCallback.size(); }
   /// remove all callbacks
   void callback_clear() { return m_vectorCallback.clear(); }

// ## attributes ----------------------------------------------------------------
   int m_iReference = 1; ///< "user" count
   uint8_t m_uSplitChar = ';'; ///< character used to split commands
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
