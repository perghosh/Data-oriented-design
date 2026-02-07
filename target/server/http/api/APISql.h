// @FILE [tag: api, sql] [summary: API SQL command class] [type: header] [name: APISql.h]

#include "API_Base.h"

class CAPISql : public CAPI_Base
{
// ## construction -------------------------------------------------------------
public:
   CAPISql() {}
   CAPISql( const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter )
      : CAPI_Base( vectorCommand, argumentsParameter ) {}
   CAPISql( const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter, unsigned uCommandIndex )
      : CAPI_Base( vectorCommand, argumentsParameter, uCommandIndex ) {}
   CAPISql( std::vector<std::string_view>&& vectorCommand, gd::argument::arguments&& argumentsParameter )
      : CAPI_Base( std::move( vectorCommand ), std::move( argumentsParameter ) ) { }
   CAPISql(CApplication* pApplication, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter)
      : CAPI_Base( pApplication, vectorCommand, argumentsParameter ) {}
   CAPISql(CApplication* pApplication, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter, unsigned uCommandIndex)
      : CAPI_Base( pApplication, vectorCommand, argumentsParameter, uCommandIndex ) {}
   // copy - explicitly deleted to make class move-only (inherited from CAPI_Base)
   CAPISql( const CAPISql& ) = delete;
   CAPISql& operator=( const CAPISql& ) = delete;
   
   // move - only move operations allowed (inherited from CAPI_Base)
   CAPISql( CAPISql&& o ) noexcept : CAPI_Base( std::move( o ) ) {}
   CAPISql& operator=( CAPISql&& o ) noexcept { CAPI_Base::operator=( std::move( o ) ); return *this; }

   ~CAPISql() override {}

// ## methods ------------------------------------------------------------------
public:
   // @API [tag: database] [description: SQL operation methods]

   std::pair<bool, std::string> Execute() override;

   std::pair<bool, std::string> Execute_Add();
};
