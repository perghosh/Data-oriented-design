// @FILE [tag: api, system] [summary: API System command class] [type: header] [name: APISystem.h]

#include "API_Base.h"

class CAPISystem : public CAPI_Base
{
// ## construction -------------------------------------------------------------
public:
   CAPISystem() {}
   CAPISystem( const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter )
      : CAPI_Base( vectorCommand, argumentsParameter ) {}
   CAPISystem( std::vector<std::string_view>&& vectorCommand, gd::argument::arguments&& argumentsParameter )
      : CAPI_Base( std::move( vectorCommand ), std::move( argumentsParameter ) ) { }
   CAPISystem(CApplication* pApplication, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter)
      : CAPI_Base( pApplication, vectorCommand, argumentsParameter ) {}
   // copy - explicitly deleted to make class move-only (inherited from CAPI_Base)
   CAPISystem( const CAPISystem& ) = delete;
   CAPISystem& operator=( const CAPISystem& ) = delete;
   
   // move - only move operations allowed (inherited from CAPI_Base)
   CAPISystem( CAPISystem&& o ) noexcept : CAPI_Base( std::move( o ) ) {}
   CAPISystem& operator=( CAPISystem&& o ) noexcept { CAPI_Base::operator=( std::move( o ) ); return *this; }

   ~CAPISystem() override {}

// ## methods ------------------------------------------------------------------
public:
   // @API [tag: database] [description: Database operation methods]

   std::pair<bool, std::string> Execute() override;

   std::pair<bool, std::string> Execute_FileDelete();
   std::pair<bool, std::string> Execute_FileDirectory();
   std::pair<bool, std::string> Execute_FileExists();

   std::pair<bool, std::string> Execute_MetadataQueryAdd();
   std::pair<bool, std::string> Execute_MetadataQueryDelete();
   std::pair<bool, std::string> Execute_MetadataQueryExists();

   std::pair<bool, std::string> Execute_SessionAdd();
   std::pair<bool, std::string> Execute_SessionDelete();
   std::pair<bool, std::string> Execute_SessionCount();
   std::pair<bool, std::string> Execute_SessionExists();
   std::pair<bool, std::string> Execute_SessionList();
       
};
