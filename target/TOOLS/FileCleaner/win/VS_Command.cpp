#include <filesystem>

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
std::pair<bool, std::string> CVisualStudio::Open_s(const std::vector<std::string>& vectorFile)
{
   HRESULT iResult = S_OK;
   try 
   {
      CComPtr<EnvDTE::_DTE> pDTE;
      auto result_ = ConnectActiveVisualStudio(pDTE);
      if(!result_.first) { return result_; }

      // Iterate through the file paths
      for(const auto& stringFile : vectorFile)
      {
         // Verify file exists
         if( std::filesystem::exists(stringFile) == false ) { return { false, "File not found: " + stringFile }; }

         // Open the file in Visual Studio
         CComPtr<EnvDTE::Window> pWindow = nullptr;
         iResult = pDTE->OpenFile(CComBSTR("Text"), CComBSTR(stringFile.c_str()), &pWindow);

         // If you need the document, get it from the window
         if (SUCCEEDED(iResult) && pWindow) {
            CComPtr<EnvDTE::Document> pDocument = nullptr;
            pWindow->get_Document(&pDocument);
         }

         if(FAILED(iResult)) { return { false, "Failed to open file: " + stringFile + ". HRESULT: " + std::to_string(iResult) }; }

         /*
         CComPtr<EnvDTE::Document> pDocument;
         iResult = pDTE->OpenFile(CComBSTR("Text"), CComBSTR(stringFile.c_str()), &pDocument);
         if(FAILED(iResult) || !pDocument)
         {
            return { false, "Failed to open file: " + stringFile + ". HRESULT: " + std::to_string(iResult) };
         }
         */
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


};


/** ---------------------------------------------------------------------------
* @brief Modifies text in an open document at a specified position.
*
* This method connects to the active Visual Studio instance, opens the specified file if not already open,
* locates the text at the given position, and replaces it with the new text. The operation can be performed
* either by position coordinates (line, column) or by character offset from the beginning of the document.
*
* @param filePath The path to the file to be modified.
* @param line The 1-based line number where the modification starts (use -1 if using offset instead).
* @param column The 1-based column number where the modification starts (use -1 if using offset instead).
* @param offset The character offset from the beginning of the document (use -1 if using line/column instead).
* @param deleteLength The number of characters to delete starting from the specified position.
* @param insertText The text to insert at the specified position after deletion.
* 
* @return A pair containing:
*         - `true` and an empty string if the modification is successful.
*         - `false` and an error message if the operation fails.
*
* @note This method requires an active Visual Studio instance to function correctly.
*       Either (line, column) or offset must be provided, but not both.
*/
/*
std::pair<bool, std::string> CVisualStudio::ModifyText_s(
   const std::string& filePath, 
   int line, 
   int column, 
   int offset, 
   int deleteLength, 
   const std::string& insertText)
{
   HRESULT iResult = S_OK;
   try 
   {
      // Validate parameters
      if ((line <= 0 || column <= 0) && offset <= 0) {
         return { false, "Either valid line/column or offset position must be provided." };
      }

      if ((line > 0 && column > 0) && offset > 0) {
         return { false, "Only either line/column or offset should be provided, not both." };
      }

      // First ensure the file exists
      if (!std::filesystem::exists(filePath)) {
         return { false, "File not found: " + filePath };
      }

      // Connect to Visual Studio
      CComPtr<EnvDTE::_DTE> pDTE;
      auto result_ = ConnectActiveVisualStudio(pDTE);
      if (!result_.first) { return result_; }

      // Open the file if it's not already open
      CComPtr<EnvDTE::Document> pDocument = nullptr;

      // First try to get the document if it's already open
      CComPtr<EnvDTE::Documents> pDocuments;
      iResult = pDTE->get_Documents(&pDocuments);
      if (FAILED(iResult) || !pDocuments) {
         return { false, "Failed to get Documents collection. HRESULT: " + std::to_string(iResult) };
      }

      long documentCount = 0;
      pDocuments->get_Count(&documentCount);
      for (long i = 1; i <= documentCount; i++) {  // DTE collections are 1-based
         CComPtr<EnvDTE::Document> pTempDoc;
         pDocuments->Item(CComVariant(i), &pTempDoc);
         if (pTempDoc) {
            CComBSTR bstrFullName;
            pTempDoc->get_FullName(&bstrFullName);
            std::wstring docPath(bstrFullName);
            if (docPath == std::wstring(filePath.begin(), filePath.end())) {
               pDocument = pTempDoc;
               break;
            }
         }
      }

      // If document isn't open, open it
      if (!pDocument) {
         // Open the file in Visual Studio
         CComPtr<EnvDTE::Window> pWindow = nullptr;
         iResult = pDTE->OpenFile(CComBSTR("Text"), CComBSTR(filePath.c_str()), &pWindow);
         if (FAILED(iResult) || !pWindow) {
            return { false, "Failed to open file: " + filePath + ". HRESULT: " + std::to_string(iResult) };
         }

         // Get the document from the window
         iResult = pWindow->get_Document(&pDocument);
         if (FAILED(iResult) || !pDocument) {
            return { false, "Failed to get document from window. HRESULT: " + std::to_string(iResult) };
         }
      }

      // Get the TextDocument object
      CComPtr<IDispatch> pTextDocDispatch;
      iResult = pDocument->Object(CComBSTR("TextDocument"), &pTextDocDispatch);
      if (FAILED(iResult) || !pTextDocDispatch) {
         return { false, "Failed to get TextDocument object. HRESULT: " + std::to_string(iResult) };
      }

      CComPtr<EnvDTE::TextDocument> pTextDoc;
      iResult = pTextDocDispatch->QueryInterface(__uuidof(EnvDTE::TextDocument), (void**)&pTextDoc);
      if (FAILED(iResult) || !pTextDoc) {
         return { false, "Failed to query TextDocument interface. HRESULT: " + std::to_string(iResult) };
      }

      // Get the selection
      CComPtr<EnvDTE::TextSelection> pSelection;
      iResult = pTextDoc->get_Selection(&pSelection);
      if (FAILED(iResult) || !pSelection) {
         return { false, "Failed to get TextSelection. HRESULT: " + std::to_string(iResult) };
      }

      // Move the selection to the specified position
      if (line > 0 && column > 0) {
         // Use line and column positioning
         iResult = pSelection->MoveToLineAndOffset(line, column);
         if (FAILED(iResult)) {
            return { false, "Failed to move to line " + std::to_string(line) + " column " + std::to_string(column) + 
               ". HRESULT: " + std::to_string(iResult) };
         }
      } else {
         // Use character offset positioning
         iResult = pSelection->MoveToAbsoluteOffset(offset);
         if (FAILED(iResult)) {
            return { false, "Failed to move to offset " + std::to_string(offset) + 
               ". HRESULT: " + std::to_string(iResult) };
         }
      }

      // Select the text to be replaced
      if (deleteLength > 0) {
         CComVariant vTrue(true);  // Extend selection
         iResult = pSelection->CharRight(vTrue, deleteLength);
         if (FAILED(iResult)) {
            return { false, "Failed to select text for deletion. HRESULT: " + std::to_string(iResult) };
         }
      }

      // Replace the selected text with the new text
      iResult = pSelection->Text(CComBSTR(insertText.c_str()));
      if (FAILED(iResult)) {
         return { false, "Failed to insert text. HRESULT: " + std::to_string(iResult) };
      }

      // Save the document if needed
      // Uncommment the following if you want to automatically save after modifications
      // pDocument->Save();

      return { true, "Successfully modified text in " + filePath };
   }
   catch (_com_error& e) 
   {
      std::cerr << "COM Error: " << e.ErrorMessage() << std::endl;
      return { false, "COM Error: " + std::string(e.ErrorMessage()) };
   }
   catch (const std::exception& e)
   {
      std::cerr << "Exception: " << e.what() << std::endl;
      return { false, "Exception: " + std::string(e.what()) };
   }
   catch (...)
   {
      std::cerr << "Unknown error occurred." << std::endl;
      return { false, "Unknown error occurred." };
   }
} */