// @FILE [tag: api, base] [summary: Base class for API commands] [type: header] [name: API_Base.h]

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_arguments.h"
#include "gd/gd_variant_view.h"
#include "gd/gd_database.h"

#include "../Types.h"


// #include "APIContext.h"    // client/session context object

class CApplication;
class CDocument;

/**
 * @brief Client context object that carries per-request identity and result state across API sections.
 *
 * CAPIContext separates the two orthogonal concerns that were previously mixed inside CAPI_Base:
 *
 *  - **Execution state** (command path, index, parameters) belongs to each API handler and lives
 *    in CAPI_Base / the concrete subclass.
 *
 *  - **Client context** (who is calling, which document/application they are bound to, accumulated
 *    result objects, shared global arguments, and last error) belongs here.
 *
 * This lets chained or packed endpoints share one context without reconstructing the heavier
 * execution machinery. A context can be passed by pointer into each API section so objects
 * accumulate naturally across the chain, and so the final response has everything it needs.
 *
 * Ownership model
 * ---------------
 * - CAPIContext owns m_objects (result accumulator) and m_argumentsGlobal.
 * - m_papplication and m_pdocument are non-owning raw pointers; their lifetimes are
 *   managed by the server/session layer that created them.
 *
 * State flags
 * -----------
 * - eFlagBound    : set when both application and document pointers are valid.
 * - eFlagHasError : set when SetError() is called; cleared by ClearError().
 * - eFlagHasResult: set when at least one object has been added to m_objects.
 *
 * Example – single section:
 * @code
 * CAPIContext ctx( pApplication, pDocument );
 * CAPIDatabase dbCmd( ctx, {"db","select"}, {{"query","SELECT * FROM TUser"}} );
 * auto [ok, err] = dbCmd.Execute();
 * if( ok && ctx.HasResult() ) { ... ctx.GetObjects() ... }
 * @endcode
 *
 * Example – chained sections sharing one context:
 * @code
 * CAPIContext ctx( pApplication, pDocument );
 * CAPIDatabase dbCmd( ctx, {"db","insert"}, insertArgs );
 * dbCmd.Execute();
 * CAPISystem  sysCmd( ctx, {"sys","session-add"}, sessionArgs );
 * sysCmd.Execute();
 * // ctx.GetObjects() now contains results from both sections
 * @endcode
 */
class CAPIContext
{
// ## construction -----------------------------------------------------------
public:
   CAPIContext() {}

   explicit CAPIContext( CApplication* papplication )
      : m_papplication( papplication ) {}

   CAPIContext( CApplication* papplication, CDocument* pdocument )
      : m_papplication( papplication ), m_pdocument( pdocument )
   { if( papplication && pdocument ) { SetFlag( eFlagBound ); } }

   // copy - deleted; context is move-only to prevent accidental aliasing of m_objects
   CAPIContext( const CAPIContext& ) = delete;
   CAPIContext& operator=( const CAPIContext& ) = delete;

   // move
   CAPIContext( CAPIContext&& o ) noexcept { common_construct( std::move( o ) ); }
   CAPIContext& operator=( CAPIContext&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~CAPIContext() = default;

private:
   void common_construct( CAPIContext&& o ) noexcept
   {
      m_papplication    = std::exchange( o.m_papplication, nullptr );
      m_pdocument       = std::exchange( o.m_pdocument,    nullptr );
      m_objects         = std::move( o.m_objects );
      m_argumentsGlobal = std::move( o.m_argumentsGlobal );
      m_stringLastError = std::move( o.m_stringLastError );
      m_uFlags          = std::exchange( o.m_uFlags, 0u );
   }

// ## flags ------------------------------------------------------------------
public:
   enum enumFlag : unsigned
   {
      eFlagNone            = 0x00000000,
      eFlagBound           = 0x00000001,  ///< application and document pointers are valid
      eFlagHasError        = 0x00000002,  ///< an error has been stored via SetError()
      eFlagHasResult       = 0x00000004,  ///< at least one object was added to m_objects
      eFlagDatabaseOwner   = 0x00000008,  ///< Ownss the database, releases it on destruction (not yet implemented)
   };

// ## methods ----------------------------------------------------------------
public:
   // @API [tag: get] [description: Accessors for context state]

   CApplication*       GetApplication()       { return m_papplication; }
   const CApplication* GetApplication() const { return m_papplication; }

   CDocument*          GetDocument()          { return m_pdocument; }
   const CDocument*    GetDocument() const    { return m_pdocument; }

   gd::database::database_i* GetDatabase() { return m_pdatabase; }
   const gd::database::database_i* GetDatabase() const { return m_pdatabase; }

   /// Bind a document pointer; also sets eFlagBound when application is already set
   void SetDocument( CDocument* pdocument )
   {
      m_pdocument = pdocument;
      if( m_papplication && m_pdocument ) { SetFlag( eFlagBound ); }
      else                                { ClearFlag( eFlagBound ); }
   }

   /// Bind both pointers in one call
   void Bind( CApplication* papplication, CDocument* pdocument )
   {
      m_papplication = papplication;
      m_pdocument    = pdocument;
      if( m_papplication && m_pdocument ) { SetFlag( eFlagBound ); }
      else                                { ClearFlag( eFlagBound ); }
   }

   bool IsBound()    const { return ( m_uFlags & eFlagBound )     != 0; }
   bool HasError()   const { return ( m_uFlags & eFlagHasError )  != 0; }
   bool HasResult()  const { return ( m_uFlags & eFlagHasResult ) != 0; }
   bool IsDatabaseOwner() const { return ( m_uFlags & eFlagDatabaseOwner ) != 0; }

   // ## Error helpers .......................................................

   void SetError( std::string stringError )
   {
      m_stringLastError = std::move( stringError );
      SetFlag( eFlagHasError );
   }

   /// Clear error state and message
   void ClearError() { m_stringLastError.clear(); ClearFlag( eFlagHasError ); }

   const std::string& GetLastError() const { return m_stringLastError; }

   // ## Result objects ......................................................

   Types::Objects*       GetObjects()       { return &m_objects; }
   const Types::Objects* GetObjects() const { return &m_objects; }

   /// Convenience: add an object and mark eFlagHasResult
   template<typename T>
   void AddObject( T* p )
   {
      m_objects.Add( p );
      SetFlag( eFlagHasResult );
   }

   /// True when no objects have been added yet
   bool IsObjectsEmpty() const { return m_objects.Empty(); }

   // ## Global arguments (shared across chained API sections) ................

   gd::argument::arguments&       GlobalArguments()       { return m_argumentsGlobal; }
   const gd::argument::arguments& GlobalArguments() const { return m_argumentsGlobal; }

   /// Read a global value by name (set by one section, readable by the next)
   gd::variant_view GetGlobal( std::string_view stringName ) const { return m_argumentsGlobal.get_argument( stringName ); }

   /// Write a global value; typically called by an API section to expose a key/id downstream
   void SetGlobal( std::string_view stringName, gd::variant_view value_ ) { m_argumentsGlobal.set( std::string(stringName), value_ ); }

   bool HasGlobal( std::string_view stringName ) const { return m_argumentsGlobal.exists( stringName ); }

   // ## Reset ................................................................

   /// Clear accumulated results and error state; keeps application/document binding intact.
   /// Useful when re-using a context across unrelated request cycles (e.g. keep-alive connections).
   void ResetResults();

   void ResetDatabase();

   /// Full reset; unbinds all pointers and clears every field.
   void Reset();

// ## internal flag helpers --------------------------------------------------
private:
   void SetFlag( unsigned uFlag )   { m_uFlags |=  uFlag; }
   void ClearFlag( unsigned uFlag ) { m_uFlags &= ~uFlag; }

// ## attributes -------------------------------------------------------------
public:
   CApplication*           m_papplication{};       ///< non-owning; lifetime managed by server layer
   CDocument*              m_pdocument{};          ///< non-owning; resolves to the calling user's document
   gd::database::database_i* m_pdatabase{};        ///< database used for this request
   Types::Objects          m_objects;              ///< accumulates result objects across chained API sections
   gd::argument::arguments m_argumentsGlobal;      ///< global values shared across sections (e.g. insert key passed to next section)
   std::string             m_stringLastError;      ///< last error message recorded in this context
   unsigned                m_uFlags{ eFlagNone };  ///< state flags (bound, has-error, has-result)
};



/**
 * @brief Base class for API command classes.
 *
 * CAPI_Base owns the execution-scoped state for a single API invocation:
 *   - command path vector and the index currently being processed
 *   - per-request parameters (m_argumentsParameter)
 *   - the raw request body
 *   - internal argument counters
 *
 * Client / session state (who is calling, accumulated result objects, shared
 * globals, last error) has been moved to CAPIContext so it can be shared across
 * chained API sections without copying or reconstructing the heavier execution
 * machinery.
 *
 * Context ownership
 * -----------------
 * CAPI_Base holds a *non-owning* pointer to CAPIContext.  The context lifetime
 * must exceed the lifetime of every API handler that references it.  The typical
 * pattern is that CRouter owns the context and passes it to each handler.
 *
 * Backward-compatible constructors are provided for the common single-section
 * case where the caller does not yet have a CAPIContext and passes CApplication
 * / CDocument directly.  In that case CAPI_Base creates and owns a local context
 * stored in m_contextOwned, and m_pcontext points into it.
 *
 * Preferred construction (shared context):
 * @code
 * CAPIContext ctx( pApp, pDoc );
 * CAPIDatabase cmd( ctx, {"db","select"}, queryArgs );
 * @endcode
 *
 * Legacy / convenience construction (local context):
 * @code
 * CAPIDatabase cmd( pApp, {"db","select"}, queryArgs );
 * @endcode
 */
class CAPI_Base
{
// ## construction -----------------------------------------------------------
public:
   CAPI_Base() : m_pcontext( &m_contextOwned ) {}

   // -- shared-context constructors (preferred for chained endpoints) -------

   CAPI_Base( CAPIContext& context, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter )
      : m_vectorCommand( vectorCommand ), m_argumentsParameter( argumentsParameter ), m_pcontext( &context ) {}

   CAPI_Base( CAPIContext& context, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter, unsigned uCommandIndex )
      : m_vectorCommand( vectorCommand ), m_uCommandIndex( uCommandIndex ), m_argumentsParameter( argumentsParameter ), m_pcontext( &context ) { assert( uCommandIndex < vectorCommand.size() ); }

   CAPI_Base( CAPIContext& context, std::vector<std::string_view>&& vectorCommand,gd::argument::arguments&& argumentsParameter )
      : m_vectorCommand( std::move( vectorCommand ) ) , m_argumentsParameter( std::move( argumentsParameter ) ), m_pcontext( &context ) {}

   // -- legacy / convenience constructors (local context owned by this object) --

   CAPI_Base( const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter )
      : m_vectorCommand( vectorCommand ), m_argumentsParameter( argumentsParameter ), m_pcontext( &m_contextOwned ) {}

   CAPI_Base( const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter,unsigned uCommandIndex )
      : m_vectorCommand( vectorCommand ), m_uCommandIndex( uCommandIndex ), m_argumentsParameter( argumentsParameter ), m_pcontext( &m_contextOwned ) { assert( uCommandIndex < vectorCommand.size() ); }

   CAPI_Base( std::vector<std::string_view>&& vectorCommand, gd::argument::arguments&& argumentsParameter )
      : m_vectorCommand( std::move( vectorCommand ) ) , m_argumentsParameter( std::move( argumentsParameter ) ), m_pcontext( &m_contextOwned ) {}

   CAPI_Base( CApplication* papplication, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter )
      : m_vectorCommand( vectorCommand ), m_argumentsParameter( argumentsParameter ), m_contextOwned( papplication ), m_pcontext( &m_contextOwned ) {}

   CAPI_Base( CApplication* papplication, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter, unsigned uCommandIndex )
      : m_vectorCommand( vectorCommand ) , m_uCommandIndex( uCommandIndex ) , m_argumentsParameter( argumentsParameter ) , m_contextOwned( papplication ), m_pcontext( &m_contextOwned ) { assert( uCommandIndex < vectorCommand.size() ); }

   // copy - explicitly deleted; class is move-only
   CAPI_Base( const CAPI_Base& ) = delete;
   CAPI_Base& operator=( const CAPI_Base& ) = delete;

   // move
   CAPI_Base( CAPI_Base&& o ) noexcept { common_construct( std::move( o ) ); }
   CAPI_Base& operator=( CAPI_Base&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   virtual ~CAPI_Base() {}

private:
   void common_construct( CAPI_Base&& o ) noexcept;

// ## operator ---------------------------------------------------------------
public:
   /// Get argument by name (first value for that name from URI parameters)
   gd::variant_view operator[]( const char* piName ) { 
      return m_argumentsParameter.get_argument( std::string_view( piName ) ).as_variant_view(); }

   gd::variant_view operator[]( std::tuple<std::string_view, size_t> index_ ) { 
      return m_argumentsParameter.find_argument( std::get<0>( index_ ), (unsigned)std::get<1>( index_ ) ).as_variant_view(); }

// ## methods ----------------------------------------------------------------
public:
   // @API [tag: get] [description: Get methods]

   // -- context accessors (delegate to m_pcontext) --------------------------

   CAPIContext*       GetContext()       { return m_pcontext; }
   const CAPIContext* GetContext() const { return m_pcontext; }

   /// Bind an external shared context; clears the locally owned context.
   /// Must be called before Execute() if the handler was default-constructed.
   void SetContext( CAPIContext& context ) { m_pcontext = &context; }

   // Convenience pass-throughs so existing call-sites inside Execute_* methods
   // do not need to change.

   const CDocument* GetDocument() const;
   CDocument*       GetDocument();

   CApplication*       GetApplication()       { return m_pcontext->GetApplication(); }
   const CApplication* GetApplication() const { return m_pcontext->GetApplication(); }

   Types::Objects&       Objects()          { return *m_pcontext->GetObjects(); }
   const Types::Objects& Objects() const    { return *m_pcontext->GetObjects(); }
   Types::Objects*       GetObjects()       { return m_pcontext->GetObjects(); }
   const Types::Objects* GetObjects() const { return m_pcontext->GetObjects(); }

   std::string GetLastError() const { return m_pcontext->GetLastError(); }

   // -- execution-scoped accessors ------------------------------------------

   /// Current command segment being processed
   std::string_view GetCommand() const { assert( m_uCommandIndex < m_vectorCommand.size() ); return m_stringCommand; }

   void SetCommand( std::string_view stringCommand ) { m_stringCommand = stringCommand; }

   unsigned GetCommandIndex() const { assert( m_uCommandIndex <= m_vectorCommand.size() ); return m_uCommandIndex; }

   void SetCommandIndex( unsigned uIndex ) { m_uCommandIndex = uIndex; assert( m_uCommandIndex <= m_vectorCommand.size() ); }

   bool IsLastCommand() const { return m_uCommandIndex >= (unsigned)m_vectorCommand.size(); }

   // -- parameter helpers ---------------------------------------------------

   gd::variant_view Get( std::string_view stringName ) const { return m_argumentsParameter.get_argument( stringName ); }

   gd::variant_view GetArgument( std::string_view stringName ) const { return m_argumentsParameter.get_argument( stringName ); }
   gd::variant_view GetNextArgument( std::string_view stringName );

   /// Count the uses of a keyed argument based on current command index
   size_t GetArgumentIndex( const std::string_view& stringName ) const;

   [[deprecated]] size_t GetArgumentIndex( const std::string_view& stringFirst, const std::string_view& stringSecond ) const;

   /// Increment the internal counter for an argument name (supports multiple
   /// occurrences of the same key across chained operations)
   void IncrementArgumentCounter( std::string_view stringName );

   /// True if the named argument exists in m_argumentsParameter
   bool Exists( const std::string_view& stringName ) const;
   const gd::argument::arguments& GetParameterArguments() const { return m_argumentsParameter; }

   // -- global argument helpers (forwarded to context) ----------------------

   gd::variant_view GetGlobal( std::string_view stringName ) const { return m_pcontext->GetGlobal( stringName ); }

   void SetGlobal( std::string_view stringName, gd::variant_view value_ ) { m_pcontext->SetGlobal( stringName, value_ ); }

   bool HasGlobal( std::string_view stringName ) const { return m_pcontext->HasGlobal( stringName ); }
   bool IsGlobalEmpty() const { return m_pcontext->GlobalArguments().empty(); }
   gd::argument::arguments& GetGlobalArguments() { return m_pcontext->GlobalArguments(); }

   // -- pure virtual interface ----------------------------------------------

   virtual std::pair<bool, std::string> Execute() = 0;

// ## attributes -------------------------------------------------------------
public:
   // Execution-scoped (per-handler, not shared)
   std::string_view               m_stringCommand;        ///< active command segment (set by Execute loop)
   std::vector<std::string_view>  m_vectorCommand;        ///< full command path segments parsed from URL
   unsigned                       m_uCommandIndex{};      ///< index into m_vectorCommand currently being dispatched
   gd::argument::arguments        m_argumentsParameter;   ///< per-request URL parameters
   std::string_view               m_stringBody;           ///< raw request body (XML/JSON forwarded from router)

   // Argument counter – small stack buffer avoids heap for typical use
   std::array<std::byte, 64>      m_arrayBufferCounter;
   gd::argument::arguments        m_argumentsArgumentCount{ m_arrayBufferCounter };

private:
   // Locally owned context used when no external context is supplied (legacy constructors)
   CAPIContext   m_contextOwned;

   // Non-owning pointer; either points to m_contextOwned or to an external shared context
   CAPIContext*  m_pcontext{ &m_contextOwned };

// ## free functions ---------------------------------------------------------
public:
};


// ---------------------------------------------------------------------------
// Inline helpers that were previously reaching into flat members — now delegate
// to context so code in the Execute_* methods continues to compile unchanged.
// ---------------------------------------------------------------------------

inline const CDocument* CAPI_Base::GetDocument() const
{
   return m_pcontext->GetDocument();
}
