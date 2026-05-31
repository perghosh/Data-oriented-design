// @FILE [tag: api, view] [summary: APIView class for handling view-related commands] [type: header] [name: APIView.h]


#include "API_Base.h"

/** @CLASS [name: CAPIView] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CAPIView : public CAPI_Base
{
   // @API [tag: construction]
public:
   CAPIView() {}

   CAPIView(CAPIContext& context, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter)
      : CAPI_Base(context, vectorCommand, argumentsParameter) {
   }
   CAPIView(CAPIContext& context, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter, unsigned uCommandIndex)
      : CAPI_Base(context, vectorCommand, argumentsParameter, uCommandIndex) {
   }
   CAPIView(CAPIContext& context, std::vector<std::string_view>&& vectorCommand, gd::argument::arguments&& argumentsParameter)
      : CAPI_Base(context, std::move(vectorCommand), std::move(argumentsParameter)) {
   }
   CAPIView(const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter)
      : CAPI_Base(vectorCommand, argumentsParameter) {
   }
   CAPIView(const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter, unsigned uCommandIndex)
      : CAPI_Base(vectorCommand, argumentsParameter, uCommandIndex) {
   }
   CAPIView(std::vector<std::string_view>&& vectorCommand, gd::argument::arguments&& argumentsParameter)
      : CAPI_Base(std::move(vectorCommand), std::move(argumentsParameter)) {
   }
   CAPIView(CApplication* pApplication, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter)
      : CAPI_Base(pApplication, vectorCommand, argumentsParameter) {
   }

   // move - only move operations allowed (inherited from CAPI_Base)
   CAPIView(CAPIView&& o) noexcept : CAPI_Base(std::move(o)) {}
   CAPIView& operator=(CAPIView&& o) noexcept { CAPI_Base::operator=(std::move(o)); return *this; }

   ~CAPIView() override {}


// @API [tag: operator]
public:


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]

// @API [tag: operation]
   std::pair<bool, std::string> Execute() override;

   std::pair<bool, std::string> Execute_RenderPage(std::string& stringRendered);

protected:
// @API [tag: internal]

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   std::string m_stringPage;
   std::string m_stringPath;


// @API [tag: free-functions]
public:



};
