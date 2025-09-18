#include "gd/gd_cli_options.h"

#include "Application.h"

#include <vector>
#include <fstream>
#include <iostream>


std::pair<bool, std::string> ReadFile(std::string_view stringLocation, std::vector<std::string> vectorContent)
{

   std::ifstream ifstreamFile(stringLocation.data());

   if( ifstreamFile.is_open() == false )
   {
      return { false, "Failed to open file: " + std::string(stringLocation) };
   }

   std::string stringLine;
   while( std::getline(ifstreamFile, stringLine) )
   {
      vectorContent.push_back(stringLine);
   }

   ifstreamFile.close();

   return { true, "" };
}

std::pair<bool, std::string> ReadFile(std::string_view stringLocation, std::vector<char>& vectorBuffer)
{
   std::ifstream ifstreamFile(stringLocation.data(), std::ios::binary | std::ios::ate);

   if( ifstreamFile.is_open() == false )
   {
      return { false, "Failed to open file: " + std::string(stringLocation) };
   }

   std::streamsize streamsizeSize = ifstreamFile.tellg();   // Get the size of the file
   ifstreamFile.seekg(0, std::ios::beg);                    // Move to the beginning of the file

   vectorBuffer.resize(streamsizeSize);                       // Resize the buffer to fit the file content

   if( !ifstreamFile.read(vectorBuffer.data(), streamsizeSize) )
   {
      return { false, "Failed to read file: " + std::string(stringLocation) };
   }

   return { true, "" };
}

std::pair<bool, std::string> PrintContent(const std::vector<std::string>& vectorContent)
{
   auto uSize = vectorContent.size();
   //std::vector<char> vectorCharacters;

   for( unsigned int u = 0; u < uSize; u++ )
   {
      std::string stringLine = vectorContent[u];
      std::cout << stringLine << std::endl;
   }



   return { true, "" };
}

std::pair<bool, std::string> PrintContent(const std::vector<char>& vectorBuffer)
{
   auto uSize = vectorBuffer.size();
   //unsigned int uCount = 0;
   std::vector<char> vectorCharacters;

   vectorCharacters.resize(uSize);

   std::cout.write(vectorBuffer.data(), vectorBuffer.size());
   std::cout << std::endl;

   for( char cTemp : vectorBuffer )
   {
      if( std::find(vectorCharacters.begin(), vectorCharacters.end(), cTemp) == vectorCharacters.end() )
      {
         vectorCharacters.push_back(cTemp);
      }
   }

   /*for( unsigned int u = 0; u < uSize; u++ )
   {
      char cTemp = vectorBuffer[u];

      for( unsigned int u2 = 0; u2 < uSize; u2++ )
      {
         if( cTemp == vectorBuffer[u2] && cTemp != vectorCharacters[u2] )
         {
            vectorCharacters[uCount] = cTemp;
            uCount++;
         }
      }
   }*/



   std::cout.write(vectorCharacters.data(), vectorCharacters.size());


   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Common construction logic for copy constructor and copy assignment operator.
 * 
 * @param o The source object to copy from.
 */
void CApplication::common_construct(const CApplication& o) 
{
    // Copy the document vector
    m_vectorDocument.clear();
    for(const auto& document_ : o.m_vectorDocument) 
    {
        m_vectorDocument.push_back(std::make_unique<CDocument>(*document_));
    }
}

/** ---------------------------------------------------------------------------
 * @brief Common construction logic for move constructor and move assignment operator.
 * 
 * @param o The source object to move from.
 */
void CApplication::common_construct(CApplication&& o) noexcept 
{
    // Move the document vector
    m_vectorDocument = std::move(o.m_vectorDocument);
}

std::pair<bool, std::string> CApplication::Main(int iArgumentCount, char* ppbszArgument[], std::function<bool(const std::string_view&, const gd::variant_view&)> process_)
{
   /*if( iArgumentCount > 1 )
   {
      gd::cli::options optionsApplication;
      CApplication::Prepare_s(optionsApplication);
      // Parse the command-line arguments
      auto [bOk, stringError] = optionsApplication.parse(iArgumentCount, ppbszArgument);
      if( bOk == false ) { return { false, stringError }; }

      // Get the active options
      const gd::cli::options* poptions = optionsApplication.find_active();
      if( poptions != nullptr )
      {
         std::string stringName = poptions->name();
         auto vectorValue = poptions->get_arguments().get_argument_all( gd::types::tag_view{}, gd::types::tag_pair{} );
         gd::argument::shared::arguments arguments_( vectorValue );

         CCommands commands_;
         commands_.Add(CCommand(stringName, arguments_));

         CDocument document_(std::move(commands_));
         DOCUMENT_Add(document_);
      }
   } */


   if( iArgumentCount > 1 )
   {
      std::cout << ppbszArgument[0] << std::endl;
      std::cout << ppbszArgument[1] << std::endl;

      std::vector<char> vectorBuffer;
      ReadFile(ppbszArgument[1], vectorBuffer);
      PrintContent(vectorBuffer);
   }

   // Process the command-line arguments
   return { true, "" };
}


std::pair<bool, std::string> CApplication::Initialize()
{
   // Perform initialization tasks here
   // For example, you might want to initialize documents or other resources

   // Example: Initialize documents
   // DOCUMENT_Add("example_document");

   // If initialization is successful
   return {true, ""};

   // If initialization fails, return an appropriate error message
   // return {false, "Initialization failed: <error details>"};
}

std::pair<bool, std::string> CApplication::Exit()
{
   // Perform cleanup tasks here
   // For example, you might want to clear documents or release resources

   // Example: Clear documents
   DOCUMENT_Clear();

   // If cleanup is successful
   return {true, ""};

   // If cleanup fails, return an appropriate error message
   // return {false, "Exit failed: <error details>"};
}

/** ---------------------------------------------------------------------------
 * @brief Adds a new document with the specified name.
 * 
 * @param stringName The name of the document to add.
 */
void CApplication::DOCUMENT_Add(const std::string_view& stringName) 
{
   auto pdocument = std::make_unique<CDocument>( stringName );
   m_vectorDocument.push_back(std::move(pdocument));
}


/** ---------------------------------------------------------------------------
 * @brief Retrieves a document by its name.
 * 
 * @param stringName The name of the document to retrieve.
 * @return CDocument* Pointer to the document if found, otherwise nullptr.
 */
const CDocument* CApplication::DOCUMENT_Get(const std::string_view& stringName) const 
{
   for( const auto& pdocument : m_vectorDocument ) 
   {
      if(pdocument->GetName() == stringName) 
      {
         return pdocument.get();
      }
   }
   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Retrieves a document by its name.
 * 
 * @param stringName The name of the document to retrieve.
 * @return CDocument* Pointer to the document if found, otherwise nullptr.
 */
CDocument* CApplication::DOCUMENT_Get(const std::string_view& stringName) 
{
   for( const auto& pdocument : m_vectorDocument ) 
   {
#ifndef NDEBUG
      auto stringName_d = pdocument->GetName();
#endif // !NDEBUG

      if(pdocument->GetName() == stringName) 
      {
         return pdocument.get();
      }
   }
   return nullptr;
}


/** ---------------------------------------------------------------------------
 * @brief Removes a document by its name.
 * 
 * @param stringName The name of the document to remove.
 */
void CApplication::DOCUMENT_Remove(const std::string_view& stringName) 
{
   m_vectorDocument.erase(
      std::remove_if(m_vectorDocument.begin(), m_vectorDocument.end(),
         [&stringName](const std::unique_ptr<CDocument>& doc) {
            return doc->GetName() == stringName;
         }),
      m_vectorDocument.end());
}

/** ---------------------------------------------------------------------------
 * @brief Gets the number of documents.
 * 
 * @return size_t The number of documents.
 */
size_t CApplication::DOCUMENT_Size() const 
{
   return m_vectorDocument.size();
}

/** ---------------------------------------------------------------------------
 * @brief Checks if there are no documents.
 * 
 * @return bool True if there are no documents, otherwise false.
 */
bool CApplication::DOCUMENT_Empty() const 
{
   return m_vectorDocument.empty();
}

/** -
 * @brief Clears all documents.
 */
void CApplication::DOCUMENT_Clear() 
{
   m_vectorDocument.clear();
}





void CApplication::Prepare_s(gd::cli::options& optionsApplication)
{
   {  // ## `copy` command
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "ls", "List files" );
      optionsCommand.add({"filter", 'f', "Filter files that is shown"});
      optionsCommand.add({"recursive", 'r', "List files recursive"});
      optionsCommand.add({"level", 'l', "Levels deep when list files recursive"});
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }

   {  // ## `copy` command
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "copy", "Copy file from source to target" );
      optionsCommand.add({"source", 's', "File to copy"});
      optionsCommand.add({"destination", 'd', "Destination, where file is copied to"});
      optionsCommand.add({"backup", 'b', "If destination file exits then make a backup"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }

   {  // ## `join` command
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "join", "join two or more files" );
      optionsCommand.add({"source", 's', "Files to join"});
      optionsCommand.add({"destination", 'd', "Destination, joined files result"});
      optionsCommand.add({"backup", 'b', "If destination file exits then make a backup"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }


   {  // ## `help` print help about champion
      gd::cli::options optionsCommand( "help", "Print command line help" );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }


}
 