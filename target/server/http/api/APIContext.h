// @FILE [tag: api, context] [summary: Client context object for API execution] [type: header] [name: APIContext.h]

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>

#include "../Types.h"

#include "gd/gd_arguments.h"
#include "gd/gd_variant_view.h"

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
      eFlagNone      = 0x00000000,
      eFlagBound     = 0x00000001,  ///< application and document pointers are valid
      eFlagHasError  = 0x00000002,  ///< an error has been stored via SetError()
      eFlagHasResult = 0x00000004,  ///< at least one object was added to m_objects
   };

// ## methods ----------------------------------------------------------------
public:
   // @API [tag: get] [description: Accessors for context state]

   CApplication*       GetApplication()       { return m_papplication; }
   const CApplication* GetApplication() const { return m_papplication; }

   CDocument*          GetDocument()          { return m_pdocument; }
   const CDocument*    GetDocument() const    { return m_pdocument; }

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

   // ## Error helpers .......................................................

   void SetError( std::string stringError )
   {
      m_stringLastError = std::move( stringError );
      SetFlag( eFlagHasError );
   }

   void ClearError()
   {
      m_stringLastError.clear();
      ClearFlag( eFlagHasError );
   }

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
   void ResetResults()
   {
      m_objects.Clear();
      m_argumentsGlobal.clear();
      m_stringLastError.clear();
      ClearFlag( eFlagHasResult );
      ClearFlag( eFlagHasError );
   }

   /// Full reset; unbinds all pointers and clears every field.
   void Reset()
   {
      m_papplication = nullptr;
      m_pdocument    = nullptr;
      m_uFlags       = eFlagNone;
      ResetResults();
   }

// ## internal flag helpers --------------------------------------------------
private:
   void SetFlag( unsigned uFlag )   { m_uFlags |=  uFlag; }
   void ClearFlag( unsigned uFlag ) { m_uFlags &= ~uFlag; }

// ## attributes -------------------------------------------------------------
public:
   CApplication*           m_papplication{};       ///< non-owning; lifetime managed by server layer
   CDocument*              m_pdocument{};          ///< non-owning; resolves to the calling user's document
   Types::Objects          m_objects;              ///< accumulates result objects across chained API sections
   gd::argument::arguments m_argumentsGlobal;      ///< global values shared across sections (e.g. insert key passed to next section)
   std::string             m_stringLastError;      ///< last error message recorded in this context
   unsigned                m_uFlags{ eFlagNone };  ///< state flags (bound, has-error, has-result)
};
