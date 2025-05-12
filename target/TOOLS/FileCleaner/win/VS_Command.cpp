#include "VS_Command.h"

namespace VS {

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

/** --------------------------------------------------------------------------- @TAG #print
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
std::pair<bool, std::string> CVisualStudio::Print_s( const std::string_view& stringText, tag_vs_output)
{
   HRESULT iResult = S_OK;
   try 
   {
      CComPtr<EnvDTE::_DTE> pDTE;
      auto result_ = ConnectActiveVisualStudio( pDTE );                             
      if( result_.first == false ) { return result_; }

      // ## Get the Output window
      
      CComPtr<EnvDTE::Windows> pWindows;                                       // Get the Windows collection
      iResult = pDTE->get_Windows(&pWindows);
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
   catch (_com_error& e) 
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


};