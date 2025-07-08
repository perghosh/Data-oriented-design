#include <filesystem>

#include "gd/gd_variant_view.h"
#include "gd/gd_table_aggregate.h"

#include "gd/expression/gd_expression_value.h"
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_runtime.h"

#include "VS_Command.h"

namespace VS {


namespace detail {
   CComPtr<EnvDTE::TextDocument> GetTextDocumentFromDocument(EnvDTE::Document* pDocument) 
   {
      CComPtr<IDispatch> pDispatch;
      HRESULT hr = pDocument->QueryInterface(__uuidof(EnvDTE::TextDocument), (void**)&pDispatch);
      if (FAILED(hr) || !pDispatch) { return nullptr; }

      CComPtr<EnvDTE::TextDocument> pTextDocument;
      hr = pDispatch->QueryInterface(__uuidof(EnvDTE::TextDocument), (void**)&pTextDocument);
      if (FAILED(hr) || !pTextDocument) { return nullptr; }

      return pTextDocument;
   }
} // namespace detail



/** ---------------------------------------------------------------------------
 * @brief Connects to the active Visual Studio instance and retrieves its automation interface.
 *
 * This method searches the Running Object Table (ROT) for an active instance of Visual Studio
 * and retrieves its `EnvDTE::_DTE` interface. The method ensures that the correct instance
 * is identified by checking the display name of the monikers in the ROT.
 *
 * @param pDTE A reference to a `CComPtr<EnvDTE::_DTE>` that will be set to the active Visual Studio instance if found.
 * 
 * @return A pair containing:
 *         - `true` and a success message if the active Visual Studio instance is successfully retrieved.
 *         - `false` and an error message if no active instance is found or an error occurs.
 *
 * @note This method requires that at least one instance of Visual Studio is running. If no instance
 *       is found in the ROT, the method will return an appropriate error message.
 */
std::pair<bool, std::string> ConnectActiveVisualStudio(CComPtr<EnvDTE::_DTE>& pDTE)
{
   HRESULT iResult = S_OK;

   // Get the Running Object Table (ROT)
   CComPtr<IRunningObjectTable> pROT;
   iResult = GetRunningObjectTable(0, &pROT);
   if(FAILED(iResult) || !pROT) { return { false, "Failed to get running object table. HRESULT: " + std::to_string(iResult) }; }

   // Enumerate running objects
   CComPtr<IEnumMoniker> pEnumMoniker;
   iResult = pROT->EnumRunning(&pEnumMoniker);
   if(FAILED(iResult) || !pEnumMoniker) { return { false, "Failed to enumerate running objects. HRESULT: " + std::to_string(iResult) }; }

   CComPtr<IMoniker> pMoniker;
   ULONG uFetched = 0;

   // Iterate through the monikers
   while(pEnumMoniker->Next(1, &pMoniker, &uFetched) == S_OK)
   {
      CComPtr<IBindCtx> pBindCtx;
      iResult = CreateBindCtx(0, &pBindCtx);
      if(FAILED(iResult) || !pBindCtx) { return { false, "Failed to create bind context. HRESULT: " + std::to_string(iResult) }; }

      LPOLESTR puDisplayName = nullptr;
      iResult = pMoniker->GetDisplayName(pBindCtx, nullptr, &puDisplayName);
      if(SUCCEEDED(iResult) && puDisplayName)
      {
         // Check if the display name matches a Visual Studio instance
         if(std::wstring(puDisplayName).find(L"VisualStudio.DTE") != std::wstring::npos)
         {
            CComPtr<IUnknown> pUnk;
            iResult = pROT->GetObject(pMoniker, &pUnk);
            if(SUCCEEDED(iResult) && pUnk)
            {
               iResult = pUnk->QueryInterface(__uuidof(EnvDTE::_DTE), (void**)&pDTE);
               if(SUCCEEDED(iResult) && pDTE)
               {
                  // Found the active Visual Studio instance
                  CoTaskMemFree(puDisplayName);
                  return { true, "Successfully retrieved the active Visual Studio instance." };
               }
            }
         }
         CoTaskMemFree(puDisplayName);
      }
      pMoniker.Release();
   }

   return { false, "No active Visual Studio instance found." };
}

/** ---------------------------------------------------------------------------
 * @brief Connects to the active Visual Studio instance and retrieves its automation interface.
 *
 * This method connects to the active Visual Studio instance and retrieves its `EnvDTE::_DTE` interface.
 * It is a wrapper around the `ConnectActiveVisualStudio` function, which performs the actual connection logic.
 *
 * @return A pair containing:
 *         - `true` and a success message if the active Visual Studio instance is successfully retrieved.
 *         - `false` and an error message if no active instance is found or an error occurs.
 *
 * @note This method requires that at least one instance of Visual Studio is running. If no instance
 *       is found in the ROT, the method will return an appropriate error message.
 */
std::pair<bool, std::string> CVisualStudio::Connect()
{                                                                                                  assert( !m_pDTE );
   return ConnectActiveVisualStudio( m_pDTE );
}


/** --------------------------------------------------------------------------- @TAG #print #vs
 * @brief Outputs the specified text to the "General" pane of the Visual Studio Output window.
 *
 * This method connects to the active Visual Studio instance, retrieves the Output window,
 * and writes the provided text to the "General" pane. If the "General" pane does not exist,
 * it creates the pane before writing the text.
 *
 * @param stringText The text to be written to the Output window.
 * @param tag_vs_output A tag dispatcher to indicate the output operation (unused in this implementation).
 * 
 * @return A pair containing:
 *         - `true` and an empty string if the operation succeeds.
 *         - `false` and an error message if the operation fails.
 *
 * @note This method requires an active Visual Studio instance to function correctly.
 *       If no instance is found or an error occurs during the operation, an appropriate
 *       error message is returned.
 */
std::pair<bool, std::string> CVisualStudio::Print( const std::string_view& stringText, tag_vs_output)
{
   HRESULT iResult = S_OK;
   try 
   {
      // ## Get the Output window
      
      CComPtr<EnvDTE::Windows> pWindows;                                       // Get the Windows collection
      iResult = m_pDTE->get_Windows(&pWindows);
      if(FAILED(iResult) || !pWindows) { return { false, "Failed to get Windows collection. HRESULT: " + std::to_string(iResult) }; }

      CComPtr<EnvDTE::Window> pOutputWindow;                                   // Get the Output window
      iResult = pWindows->Item(CComVariant("Output"), &pOutputWindow);
      if(FAILED(iResult) || !pOutputWindow) { return { false, "Failed to get Output window. HRESULT: " + std::to_string(iResult) }; }

      CComPtr<IDispatch> pDispatch;                                            // Get the OutputWindow interface using get_Object
      iResult = pOutputWindow->get_Object(&pDispatch);
      if(FAILED(iResult) || !pDispatch) { return { false, "Failed to get OutputWindow object. HRESULT: " + std::to_string(iResult) }; }

      CComPtr<EnvDTE::OutputWindow> pOutputWindowInterface;
      iResult = pDispatch->QueryInterface(__uuidof(EnvDTE::OutputWindow), (void**)&pOutputWindowInterface);
      if(FAILED(iResult) || !pOutputWindowInterface) { return { false, "Failed to query OutputWindow interface. HRESULT: " + std::to_string(iResult) }; }
      
      CComPtr<EnvDTE::OutputWindowPanes> pOutputPanes;                         // Get the OutputWindowPanes collection
      iResult = pOutputWindowInterface->get_OutputWindowPanes(&pOutputPanes);
      if(FAILED(iResult) || !pOutputPanes) { return { false, "Failed to get OutputWindowPanes. HRESULT: " + std::to_string(iResult) }; }

      
      CComPtr<EnvDTE::OutputWindowPane> pOutputPane;
      iResult = pOutputPanes->Item(CComVariant("General"), &pOutputPane);      // Get or create the desired OutputWindowPane
      if(FAILED(iResult) || !pOutputPane)
      {
         // If the "General" pane does not exist, create it
         iResult = pOutputPanes->Add(CComBSTR("General"), &pOutputPane);
         if(FAILED(iResult) || !pOutputPane) { return { false, "Failed to create Output pane. HRESULT: " + std::to_string(iResult) }; }
      }

      // Activate the pane and write the text
      pOutputPane->Activate();
      pOutputPane->OutputString(CComBSTR(stringText.data()));

   }
   catch(_com_error& e) 
   {
      std::cerr << "COM Error: " << e.ErrorMessage() << std::endl;
   }
   catch( const std::exception& e )
   {
      std::cerr << "Exception: " << e.what() << std::endl;
   }
   catch( ... )
   {
      std::cerr << "Unknown error occurred." << std::endl;
   }

   return {true, ""};
}


/** ---------------------------------------------------------------------------
 * @brief Opens multiple files in Visual Studio using their file paths stored in a std::vector of std::string.
 *
 * This method connects to the active Visual Studio instance and opens each file specified in the provided
 * vector of file paths in the Visual Studio editor. The file paths must be valid and accessible on the system.
 *
 * @param vectorFile A std::vector containing std::string objects representing the full file paths to be opened.
 * 
 * @return A pair containing:
 *         - `true` and an empty string if all files are successfully opened.
 *         - `false` and an error message if the operation fails at any point (e.g., no Visual Studio instance or file not found).
 *
 * @note This method requires an active Visual Studio instance to function correctly. If no instance is found
 *       or if any file fails to open, an appropriate error message is returned.
 */
std::pair<bool, std::string> CVisualStudio::Open(const std::vector<std::string>& vectorFile)
{
   HRESULT iResult = S_OK;
   try 
   {
      // Iterate through the file paths
      for(const auto& stringFile : vectorFile)
      {
         if (!std::filesystem::exists(stringFile)) {
            return { false, "File not found: " + stringFile };
         }

         CComPtr<EnvDTE::Window> pWindow = nullptr;
         //CComBSTR bstrKind(L"Text");
         CComBSTR bstrKind(L"{00000000-0000-0000-0000-000000000000}"); // vsViewKindTextView
         CComBSTR bstrFile(stringFile.c_str());
         HRESULT hr = m_pDTE->OpenFile(bstrKind, bstrFile, &pWindow);
         if (FAILED(hr) || !pWindow) {
            return { false, "Failed to open file: " + stringFile + ". HRESULT: 0x" + std::format("{:08X}", static_cast<unsigned>(hr)) };
         }

         pWindow->Activate(); // Activate the window after opening it
      }
   }
   catch(_com_error& e) 
   {
      std::cerr << "COM Error: " << e.ErrorMessage() << std::endl;
      return { false, "COM Error: " + std::string(e.ErrorMessage()) };
   }
   catch(const std::exception& e)
   {
      std::cerr << "Exception: " << e.what() << std::endl;
      return { false, "Exception: " + std::string(e.what()) };
   }
   catch(...)
   {
      std::cerr << "Unknown error occurred." << std::endl;
      return { false, "Unknown error occurred." };
   }

   return { true, "" };
}

/** --------------------------------------------------------------------------- @TAG #bookmark #vs
* @brief Adds a bookmark at the specified line in a file in Visual Studio.
*
* This method connects to the active Visual Studio instance, opens the specified file if necessary,
* and adds a bookmark at the specified line with an optional description. The file doesn't need
* to be the active document.
*
* @param stringPath The full path to the file where the bookmark should be added.
* @param iLine The line number where the bookmark should be added (0-based).
* @param stringDescription Optional description for the bookmark.
* @param tag_vs_bookmark A tag dispatcher to indicate the bookmark operation (unused in this implementation).
* 
* @return A pair containing:
*         - `true` and an empty string if the operation succeeds.
*         - `false` and an error message if the operation fails.
*
* @note This method requires an active Visual Studio instance to function correctly.
*       If no instance is found or an error occurs during the operation, an appropriate
*       error message is returned. The file will be opened if it's not already open.
*/
std::pair<bool, std::string> CVisualStudio::AddBookmark(const std::string& stringPath, int iLine, const std::string& stringDescription)
{
   HRESULT iResult = S_OK;
   try
   {
      /*
      // Verify file exists
      if( std::filesystem::exists(stringPath) == false ) { return { false, "File not found: " + stringPath }; }
      // Open the file in Visual Studio
      CComPtr<EnvDTE::Window> pWindow = nullptr;
      iResult = m_pDTE->OpenFile(CComBSTR("Text"), CComBSTR(stringPath.c_str()), &pWindow);
      if(FAILED(iResult) || !pWindow) { return { false, "Failed to open file: " + stringPath + ". HRESULT: " + std::to_string(iResult) }; }
      // Get the document from the window
      CComPtr<EnvDTE::Document> pDocument = nullptr;
      iResult = pWindow->get_Document(&pDocument);
      if(FAILED(iResult) || !pDocument) { return { false, "Failed to get document from window. HRESULT: " + std::to_string(iResult) }; }
      // Get the TextDocument interface from the Document
      CComPtr<EnvDTE::TextDocument> pTextDocument = detail::GetTextDocumentFromDocument(pDocument);
      if(!pTextDocument) { return { false, "Failed to get TextDocument interface." }; }
      // Add a bookmark at the specified line
      CComPtr<EnvDTE::EditPoint> pStartPoint = nullptr;
      iResult = pTextDocument->CreateEditPoint(nullptr, &pStartPoint);
      if(FAILED(iResult) || !pStartPoint) { return { false, "Failed to create EditPoint. HRESULT: " + std::to_string(iResult) }; }
      // Move to the specified line and add a bookmark
      iResult = pStartPoint->MoveToLineAndOffset(iLine + 1, 1); // Line numbers are 1-based in Visual Studio
      if(FAILED(iResult)) { return { false, "Failed to move EditPoint to line. HRESULT: " + std::to_string(iResult) }; }
      CComPtr<EnvDTE::Bookmark> pBookmark = nullptr;
      iResult = pTextDocument->Bookmarks->Add(pStartPoint, CComBSTR(stringDescription.c_str()), &pBookmark);
      if( FAILED(iResult) || !pBookmark ) {
         return { false, "Failed to add bookmark.
         */
   }
   catch(_com_error& e) 
   {
      std::cerr << "COM Error: " << e.ErrorMessage() << std::endl;
      return { false, "COM Error: " + std::string(e.ErrorMessage()) };
   }
   catch( const std::exception& e )
   {
      std::cerr << "Exception: " << e.what() << std::endl;
      return { false, "Exception: " + std::string(e.what()) };
   }
   catch( ... )
   {
      std::cerr << "Unknown error occurred." << std::endl;
      return { false, "Unknown error occurred." };
   }
   return {true, ""};
}



using namespace gd::expression;


//============================================================================
//============================================================ script methods
//============================================================================

static std::pair<bool, std::string> open_s( runtime* pruntime, const std::vector<value>& vectorArgument )
{                                                                                                  assert(vectorArgument.size() > 0);
   std::string stringResult;
   const auto& vColumn = vectorArgument[0];

   CVisualStudio* pVS = pruntime->get_global_as<CVisualStudio>( "vs" );
   if( pVS != nullptr )
   {
      gd::table::dto::table* ptable_ = pVS->GetTable();
      auto stringColumn = vColumn.get_string();
      unsigned uColumn = ptable_->column_find_index(stringColumn);
      if( (int)uColumn == -1 ) { return { false, std::format( "Invalid column name: {}", vColumn.get_string() )}; }

      gd::table::aggregate<gd::table::dto::table> aggregate_(ptable_);
      std::vector<gd::variant_view> vectorFile = aggregate_.unique(uColumn, 0, ptable_->get_row_count());
      std::vector<std::string> vectorFilePath;
      for( const auto& itFile : vectorFile )
      {
         std::string stringFile = itFile.as_string();
         if( std::filesystem::exists(stringFile) == false ) { return { false, std::format("File not found: {}", stringFile) }; }
         vectorFilePath.push_back(stringFile);
      }
      auto result_ = pVS->Open(vectorFilePath);
      if( result_.first == false ) { return result_; }
      // return pVS->Open( vectorFile );
   }

   return { true, "" };
}


static std::pair<bool, std::string> print_s( runtime* pruntime, const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   std::string stringResult;
   const auto& vColumn = vectorArgument[0];

   CVisualStudio* pVS = pruntime->get_global_as<CVisualStudio>( "vs" );
   if( pVS != nullptr )
   {
      gd::table::dto::table* ptable_ = pVS->GetTable();
      unsigned uColumn = ptable_->column_find_index(vColumn.get_string());
      if( (int)uColumn == -1 ) { return { false, std::format( "Invalid column name: {}", vColumn.get_string() )}; }

      for( const auto& itRow : *ptable_ )
      {
         if( stringResult.empty() == false ) { stringResult += "\n"; }
         std::string stringRow = itRow.cell_get_variant_view(uColumn).as_string();
         stringResult += stringRow;
      }

      pVS->Print(stringResult, tag_vs_output{});
   }

   return { true, "" };
}


// Array of MethodInfo for visual studio operations
const method pmethodVisualStudio_g[] = {
   { (void*)&open_s, "open", 1, 0, method::eFlagRuntime },
   { (void*)&print_s, "print", 1, 0, method::eFlagRuntime },
};

std::pair<bool, std::string> CVisualStudio::ExecuteExpression(const std::string_view& stringExpression)
{
   // ## convert string to tokens
   std::vector<gd::expression::token> vectorToken;
   std::pair<bool, std::string> result = gd::expression::token::parse_s(stringExpression, vectorToken, gd::expression::tag_formula{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   // ## compile tokens and that means to convert tokens to postfix, place them in correct order to be processed
   std::vector<gd::expression::token> vectorPostfix;
   result = gd::expression::token::compile_s(vectorToken, vectorPostfix, gd::expression::tag_postfix{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   // ## calculate the result
   // NOTE: vectorVariable is not defined in this scope. Assuming member variable or needs to be passed in.
   //gd::expression::runtime runtime_(vectorVariable);
   gd::expression::runtime runtime_;

   runtime_.add_global( "vs", this );


   runtime_.add( { (unsigned)uMethodDefaultSize_g, gd::expression::pmethodDefault_g, ""});
   runtime_.add( { (unsigned)uMethodStringSize_g, gd::expression::pmethodString_g, std::string("str")});
   runtime_.add( { (unsigned)2, pmethodVisualStudio_g, std::string("vs")});

   gd::expression::value valueResult;
   result = gd::expression::token::calculate_s(vectorPostfix, &valueResult, runtime_);
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   return { true, "" };
}



}