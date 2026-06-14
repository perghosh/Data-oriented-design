// @FILE [tag: router, http] [summary: Router class for http server] [type: source]

#include "gd/parse/gd_parse_uri.h"
#include "gd/gd_utf8.h"

#include "pugixml/pugixml.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

#include "api/APIDatabase.h"
#include "api/APISql.h"
#include "api/APISystem.h"
#include "api/APIView.h"


#include "Router.h"

//static std::tuple< gd::com::pointer< gd::com::server::router::command >, gd::com::pointer< gd::com::server::router::response > > make_command_s(gd::com::server::server_i* pserver );

/*
@TASK [tag: url, format, command][status: ongoing] [assigned: per]
[title: design url format for commands]
[description: """
Design a URL format for commands that allows easy parsing and execution of server commands.
sample http://127.0.0.1/!db/create?name=testdb&user=admin
- db/create?name=testdb&user=admin
- db/delete?name=testdb
- db/table/create?name=TUser
- db/column/create?table=TUser&name=FColumnName&type=int32
- db/select?query=SELECT * FROM TUser WHERE FColumnName=100
- db/insert?table=TUser&values=(FColumnName=100,FAnotherColumn='text')
- db/update?table=TUser&set=(FAnotherColumn='newtext')&where=(FColumnName=100)
"""]
*/

CRouter::~CRouter()
{
   // ## clean up request data if any

   if( m_pairRequestData.second != nullptr )
   {
      // @CRITICAL [tag: router, request data] [description: Clean up request data based on its type to prevent memory leaks]
      unsigned uType = m_pairRequestData.first;
      void* pData = m_pairRequestData.second;
      if( uType == eRequestFormatXml )
      {
         pugi::xml_document* pdocument = static_cast<pugi::xml_document*>( pData );
         delete pdocument;
      }
      else if( uType == eRequestFormatJson )
      {
         jsoncons::json* pjson = static_cast<jsoncons::json*>( pData );
         delete pjson;
      }
   }
}


/** @CRITICAL [tag: router, parse] [description: Parse query string to determine if it's a command and extract command path and arguments]
 * @brief Parses the query string into internal arguments that hold parameters.
 * 
 * Parse the query string to determine if it represents a command and extract the
 * command path and arguments. The method checks if the query string starts with '!' to 
 * identify it as a command, then removes the '!' prefix and sets the appropriate flag.
 * It also prepares the query string for further processing by command execution.
 * 
 * @return A pair with true if parsing was successful, and an error message if it was not.
 */
std::pair<bool, std::string> CRouter::Parse()
{                                                                                                   assert( m_stringQueryString.empty() == false );
   std::string_view stringQueryStringView = m_stringQueryString;
   if( stringQueryStringView[0] == '!' )
   {
      stringQueryStringView.remove_prefix( 1 );
      m_stringQueryString = stringQueryStringView;
      m_uFlags |= eFlagCommand;
   }
   else
   {
      m_uFlags &= ~eFlagCommand;
   }

   return { true, "" };
}


template<typename APIObject, typename Configure>
std::pair<bool, std::string> CRouter::ExecuteCommand_( const std::vector<std::string_view>& vectorPath, const gd::argument::arguments& arguments_, unsigned& uCommandIndex, Configure&& configure )
{
   // [CONTEXT] Construct handler with shared context instead of raw application pointer.
   APIObject apiobject_( m_context, vectorPath, arguments_, uCommandIndex );

   if constexpr(!std::is_same_v<std::decay_t<Configure>, std::monostate>)
   {
      std::forward<Configure>(configure)(static_cast<CAPI_Base&>(apiobject_));
   }

   if(m_functionConfigure)
   {
      m_functionConfigure(&apiobject_, vectorPath[uCommandIndex], "before");
   }
 
   auto result_ = apiobject_.Execute();
 
   if( result_.first == true )
   {
      // Objects are already inside m_context.m_objects via the handler's
      // AddObject() / GetObjects() calls — no separate transfer step needed
      // when the context is shared.  The check below handles the legacy path
      // where a handler that does not yet use context-based construction still
      // fills its own m_objects.
      Types::Objects* pobjectsResult = apiobject_.GetObjects();                                    assert( pobjectsResult );
      if( pobjectsResult != nullptr && pobjectsResult->Empty() == false )
      {
         result_ = m_pdtoresponse->AddTransfer( pobjectsResult );
      }
   }

   if(m_functionConfigure)
   {
      m_functionConfigure(&apiobject_, vectorPath[uCommandIndex], "after");
   }

 
   uCommandIndex = apiobject_.GetCommandIndex();
   return result_;
}


std::pair<bool, std::string> CRouter::Run()
{
   return Run(m_stringQueryString, true);
}

 /**  -------------------------------------------------------------------------- Run
  * @CRITICAL [tag: router, command, query] [summary: Execute command chain from query string]
  * @brief Execute route commands parsed from `stringQueryString`.
  *
  * Parse command path and query arguments, optionally reuse fallback query arguments when
  * `bInternal` is `false`, attach parsed body payload objects (`xml`/`json`) to arguments,
  * and dispatch each command segment to its matching API handler.
  *
  * @param stringQueryString Route and query text to parse and execute.
  * @param bInternal Internal-call marker that disables fallback argument parsing.
  * @return std::pair<bool, std::string> `first` is success; `second` contains an error message on failure.
  */
std::pair<bool, std::string> CRouter::Run( std::string_view stringQueryString, bool bInternal )
{                                                                                                  assert( stringQueryString.empty() == false );
   std::pair<bool, std::string> result_;

   result_ = Prepare();                                                       // prepare for running command to ensure that all necessary data is ready and in correct format
   if( result_.first == false ) { return result_; }

   if(IsCommand() == true)
   {
      // ## parse command path and query arguments and prepare important variables
      auto [vectorPath, arguments_] = gd::parse::uri::parse_path_and_query(stringQueryString);
      if(vectorPath.empty() == true) { return { false, std::string("No command found in query string: " + std::string(stringQueryString)) }; }

      // ## If nog arguments, then check if member query string differ and parse that for arguments.
      if(arguments_.empty() == true && m_stringQueryString.empty() == false && bInternal == false)
      {
         // move to ? and extract string view from there, if not found then parse all
         std::string_view stringQueryView = m_stringQueryString;
         std::size_t uPosition = stringQueryView.find('?');
         if(uPosition != std::string_view::npos) { stringQueryView.remove_prefix(uPosition + 1); }

         result_ = gd::parse::uri::parse_query(stringQueryView, arguments_);// if query parsing did not find any arguments, try parsing query string as arguments directly
         if(result_.first == false) { return result_; }
      }        

#ifndef NDEBUG
      std::string stringArguments_d = gd::argument::debug::print( arguments_ );
#endif // NDEBUG

      if( IsBody() == true )
      {
         if( IsRequestFormatXml() == true )
         {
            pugi::xml_document* pdocument = static_cast<pugi::xml_document*>( m_pairRequestData.second );
            arguments_.append( "xml", (void*)pdocument );                    // add parsed xml document to arguments for command execution
         }
         else if( IsRequestFormatJson() == true )
         {
            jsoncons::json* pjson = static_cast<jsoncons::json*>( m_pairRequestData.second );
            arguments_.append( "json", (void*)pjson );                       // add parsed json document to arguments for command execution
         }
      }

      // ## Prepare the response object for request, here result data is placed
      if( !m_pdtoresponse )
      {
         // ## double checked locking pattern to ensure that response object is created only once and is thread safe

         std::lock_guard<std::mutex> lock( m_mutexRouter );                  // lock mutex to ensure thread safety when creating response object
         if( !m_pdtoresponse )
         {
            m_pdtoresponse = std::make_unique<CDTOResponse>();
            m_pdtoresponse->Initialize();
         }
      }

      result_ = Run(vectorPath, arguments_);                                   // run command with parsed path and arguments, this will execute the command chain and fill response data in m_pdtoresponse
   }

   return { true, "" };
}

std::pair<bool, std::string> CRouter::Run( const std::vector<std::string_view>& vectorPath, const gd::argument::arguments& arguments_ )
{
   unsigned uCommandIndex = 0;
   std::pair<bool, std::string> result_(true, "");

   while(uCommandIndex < vectorPath.size())
   {
      std::string_view stringCommand = vectorPath[uCommandIndex];
      if(stringCommand == "db")                                           // database related commands, select, create, delete, open, close database
      {
         result_ = ExecuteCommand_<CAPIDatabase>(vectorPath, arguments_, uCommandIndex);
      }
      else if(stringCommand == "sql")                                     // sql commands are logic related to sql queries, adding, remove or edit sql queries
      {
         result_ = ExecuteCommand_<CAPISql>(vectorPath, arguments_, uCommandIndex);
      }
      else if(stringCommand == "sys")                                     // system related commands, thing that affects the complete system
      {
         result_ = ExecuteCommand_<CAPISystem>(vectorPath, arguments_, uCommandIndex);
      }
      else if(stringCommand == "view")                                      // view related commands, like server side rendering, generating html, etc.
      {
         result_ = ExecuteCommand_<CAPIView>(vectorPath, arguments_, uCommandIndex);
      }
      else if(stringCommand == "xml")                                       // commands are packed in xml, this to enable more complex commands that can not be easily represented in url
      {
         auto* pxmldocument_ = arguments_["xml"].get_pointer<pugi::xml_document>();
         result_ = RunXml(pxmldocument_);
      }
      else
      {
         return { false, "Unknown command: " + std::string(stringCommand) };
      }

      uCommandIndex++;

      if(result_.first == false) { return result_; }
   }

   return { true, "" };
}


/**  -------------------------------------------------------------------------- RunXml
 * @brief Execute one or more route commands stored in an XML request document.
 *
 * Accept a parsed XML document in `pxmldocument_`, locate `<command>` entries,
 * read each command query string from the `qs` or `querystring` attribute,
 * convert that query into path segments and arguments, and dispatch the command
 * through `Run( vectorPath, arguments_ )`.
 *
 * If a command node contains nested XML content, pointers to the full XML
 * document and the current command node are attached to the command arguments
 * so downstream handlers can inspect command-specific payload data.
 *
 * Supported layouts are:
 * - a root `<commands>` element with multiple `<command>` children
 * - a root `<command>` element
 * - a wrapper root that contains one or more `<commands>` elements
 *
 * @param pxmldocument_ Parsed XML request document to execute.
 * @return std::pair<bool, std::string> `first` is `true` on success; `second`
 *         contains an error message if parsing or command execution fails.
 */
std::pair<bool, std::string> CRouter::RunXml(pugi::xml_document* pxmldocument_)
{                                                                                                  assert( pxmldocument_ != nullptr );

   auto run_command_ = [this, pxmldocument_](pugi::xml_node& xmlnodeCommand) -> std::pair<bool, std::string>
   {
      // ## run command from xml node, this will execute the command and fill response data in m_pdtoresponse

      // ### get the qs attribute from command node, this will be used as query string for command execution
      pugi::xml_attribute xmlattributeQs = xmlnodeCommand.attribute("qs");
      if(!xmlattributeQs) 
      { 
         xmlattributeQs = xmlnodeCommand.attribute("querystring"); 
         if(!xmlattributeQs) { return { false, "No qs attribute found in command node" }; }
      }

      std::string_view stringQueryString = xmlattributeQs.value();
      
      auto [vectorPath, arguments_] = gd::parse::uri::parse_path_and_query(stringQueryString);
      if(vectorPath.empty() == true) { return { false, std::string("No command found in query string: " + std::string(stringQueryString)) }; }

      // check if command node has child nodes
      if(xmlnodeCommand.first_child().empty() == false)
      {
         arguments_.append("xml", (void*)pxmldocument_);
         arguments_.append("xml_command", (void*)&xmlnodeCommand);
      }

      auto result_ = Run(vectorPath, arguments_);                              // run command with parsed path and arguments, this will execute the command chain and fill response data in m_pdtoresponse

      return { true, "" };
   }; 

   // ## Get all "commands" elements from xml, they are direct bellow the root node
   pugi::xml_node xmlnodeRoot = pxmldocument_->document_element();
   if(!xmlnodeRoot) { return { false, "No root element found" }; }

   if(std::string(xmlnodeRoot.name()) == "commands")
   {
      for(pugi::xml_node xmlnodeCommand : xmlnodeRoot.children("command"))
      {
         auto result_ = run_command_(xmlnodeCommand);
         if(result_.first == false) { return result_; }
      }
   }
   else if(std::string(xmlnodeRoot.name()) == "command")
   {
      auto result_ = run_command_(xmlnodeRoot);
      if(result_.first == false) { return result_; }
   }
   else // check for commands children
   {
      // multiple commands in root, this is to support sending multiple commands in one request without wrapping them in <commands> element
      for(pugi::xml_node xmlnodeCommands : xmlnodeRoot.children("commands"))
      {
         for(pugi::xml_node xmlnodeCommand : xmlnodeCommands.children("command"))
         {
            auto result_ = run_command_(xmlnodeCommand);
            if(result_.first == false) { return result_; }
         }
      }
   }
   
   return { true, "" };
}

/** --------------------------------------------------------------------------
 *  @brief Prepare for running command, this can include things like encoding values in arguments, validating arguments, etc.
 */
std::pair<bool, std::string> CRouter::Prepare()
{                                                                                                  assert( IsPrepared() == false );
   // ## prepare for running command, this can include things like encoding values in arguments, validating arguments, etc.

   SetFlag( eFlagPrepared );                                                  // set prepared flag to indicate that preparation is done

   /// ### Prepare request data if sent with request
   if( m_stringBody.empty() == false )
   {
#ifndef NDEBUG
      const char* piBody_d = m_stringBody.data();
#endif // NDEBUG


      if( IsRequestFormatXml() == true )
      {
         pugi::xml_document* pdocument = new pugi::xml_document;
         pugi::xml_parse_result parseResult = pdocument->load_string( m_stringBody.data() );
         if( parseResult.status != pugi::xml_parse_status::status_ok )
         {
            delete pdocument;
            return { false, "Failed to parse XML body: " + std::string(parseResult.description()) };
         }

         SetResponseData( eRequestFormatXml, pdocument );                     // store parsed xml in response data to be used later in command execution
      }
      else if( IsRequestFormatJson() == true )
      {
         jsoncons::json* pjson = new jsoncons::json;
         try
         {
            (*pjson) = jsoncons::json::parse( m_stringBody );
         }
         catch( const std::exception& e )
         {
            delete pjson;
            return { false, "Failed to parse JSON body: " + std::string(e.what()) };
         }

         SetResponseData( eRequestFormatJson, pjson );                        // store parsed json in response data to be used later in command execution
      }
   }

   return { true, "" };
}

std::pair<bool, std::string> CRouter::Encode_s( gd::argument::arguments& arguments_, const std::vector<std::string>& vectorName )
{
   std::string stringValueEncoded; // encoded value

   // ## encode values in arguments for specified names in vectorName
   for( const auto& stringName : vectorName )
   {
      decltype( arguments_.next() ) positionNext = nullptr;
      
      for(auto* position_ = arguments_.next(); position_ != nullptr; position_ = arguments_.next(position_) ) 
      {
         auto name_ = gd::argument::arguments::get_name_s(position_);
         if( name_ != stringName ) { continue; }

         auto stringValue = gd::argument::arguments::get_argument_s( position_ ).as_variant_view().as_string_view(); // get string value as string view to avoid alocate
         stringValueEncoded.clear();
         auto result_ = gd::utf8::uri::convert_uri_to_uf8( stringValue, stringValueEncoded );
         for( char& i_ : stringValueEncoded ) { if( i_ == '+' ) { i_ = ' '; } }
         arguments_.set( position_, stringValueEncoded, &positionNext );
         position_ = positionNext;
      }
   }
   return { true, "" };
}
