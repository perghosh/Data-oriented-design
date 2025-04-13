/**
 * @file Application.cpp
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0DATABASE.Application` - database operations
 * 
 */

#include <filesystem>
#include <format>

#include "gd/gd_cli_options.h"

#include "Application.h"





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

/** ---------------------------------------------------------------------------
 * @brief Prepares the application by setting up command-line options.
 *
 * @param optionsApplication The options object to prepare.
 */
std::pair<bool, std::string> CApplication::Main(int iArgumentCount, char* ppbszArgument[], std::function<bool(const std::string_view&, const gd::variant_view&)> process_)
{
   if( iArgumentCount > 1 )
   {
      gd::cli::options optionsApplication;
      CApplication::Prepare_s(optionsApplication);
      // Parse the command-line arguments
      auto [bOk, stringError] = optionsApplication.parse(iArgumentCount, ppbszArgument);
      if( bOk == false ) { return { false, stringError }; }

      std::tie(bOk, stringError) = Initialize(optionsApplication);
      if( bOk == false ) { return { false, stringError }; }


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

std::pair<bool, std::string> CApplication::Initialize( gd::cli::options& optionsApplication )
{
   const gd::cli::options* poptionsActive = optionsApplication.find_active();
   if( poptionsActive == nullptr ) { return { false, "No active options found" }; }

   /// ## prepare command

#ifndef NDEBUG
   auto stringName_d = poptionsActive->name();
#endif // !NDEBUG

   std::string stringCommandName = poptionsActive->name();
   PROPERTY_Set("command", stringCommandName);

   auto* pdocument = DOCUMENT_Add(stringCommandName);
   if( pdocument == nullptr ) { return { false, "Failed to add document" }; }

   if( stringCommandName == "count" )
   {
      std::string stringSource = (*poptionsActive)["source"].as_string();
      auto result_ = pdocument->HarvestFile( { {"source", stringSource} } );
      if( result_.first == false ) return result_;
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Adds a new document with the specified name.
 * 
 * @param stringName The name of the document to add.
 */
CDocument* CApplication::DOCUMENT_Add(const std::string_view& stringName) 
{
   auto pdocument = std::make_unique<CDocument>( this, stringName );
   m_vectorDocument.push_back(std::move(pdocument));
   return m_vectorDocument.back().get();
}

/** ---------------------------------------------------------------------------
 * @brief Adds a new document based on the provided arguments.
 * 
 * @param arguments_ The arguments used to create the document.
 */
CDocument* CApplication::DOCUMENT_Add(const gd::argument::shared::arguments& arguments_) 
{
   auto pdocument = std::make_unique<CDocument>( arguments_ );
   // Assuming CDocument has a method to initialize from arguments
   // doc->Initialize(arguments_);
   m_vectorDocument.push_back(std::move(pdocument));
   return m_vectorDocument.back().get();
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


void CApplication::COMMAND_Read(char** ppbszArgument)
{
   std::string stringCommand = ppbszArgument[1];
   if( stringCommand == "print" )
   {
      COMMAND_Print(ppbszArgument[2]);
   }
   else if( stringCommand == "count" )
   {
      COMMAND_Count(ppbszArgument[2]);
   }
   else
   {
      std::cout << "Invalid command\n";
   }
}

void CApplication::COMMAND_Print(const std::string& stringArgument)
{
   std::ifstream ifstreamFile(stringArgument);
   std::string stringText;
   while( std::getline(ifstreamFile, stringText) )
   {
      std::cout << stringText << std::endl;
   }

   ifstreamFile.close();
}

void CApplication::COMMAND_Count(const std::string& stringArgument)
{
   std::ifstream ifstreamFile(stringArgument);
   std::string stringText;
   int iCount = 0;
   while( std::getline(ifstreamFile, stringText) )
   {
      iCount++;
   }

   std::cout << iCount << " Rows" << std::endl;

   ifstreamFile.close();
}

// 0TAG0DATABASE.Application

std::pair<bool, std::string> CApplication::DATABASE_Open(const gd::argument::shared::arguments& argumentsOpen)
{
   std::string stringPath = argumentsOpen["path"].as_string();
   if( stringPath.empty() == false )
   {
      if( std::filesystem::exists(stringPath) == false )
      {
      }

   }
   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Connect to database
 * Connect to database based on string that holds information on how to connect
 * 
 * @param stringConnect connection string
 * @return true if ok, false and error information if failed
 */
std::pair<bool, std::string> CApplication::DATABASE_Connect( const std::string_view& stringConnect )
{
   std::string stringConnectionName;
   std::string stringConnectionName2;

   DATABASE_CloseActive();

   if( stringConnect.empty() == true )
   {
      stringConnectionName = PROPERTY_Get("database").as_string();             assert( stringConnectionName.empty() == false );
   }
   else
   {
      stringConnectionName = stringConnect;
   }

   // ## connect main database
   bool bOk = false;
   std::string stringError;
   gd::database::database_i* pdatabaseMain = nullptr;

   
   if(true)
   {
#ifdef GD_DATABASE_ODBC_USE      
      gd::database::odbc::database_i* pdatabase = new gd::database::odbc::database_i();  // create database interface
      pdatabase->add_reference();
      std::tie(bOk, stringError) = pdatabase->open( stringConnectionName );
      pdatabaseMain = pdatabase;
#else
      bOk = false;
      stringError = "Only file connection is enabled, make sure to connect to file database (sqlite)";
#endif      
   }
   else
   {
      gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i();  // create database interface
      pdatabase->add_reference();
      std::tie(bOk, stringError) = pdatabase->open( stringConnectionName );
      pdatabaseMain = pdatabase;
   }

   if( bOk == true )
   {
      m_pdatabase = pdatabaseMain;
   }
   else
   {
      if( pdatabaseMain != nullptr ) pdatabaseMain->release();
      if( stringError.empty() == true ) stringError = "Failed to collect information about database connection";
      return { false, stringError };
   }

   return { true, "" };
}

void CApplication::DATABASE_Append( gd::database::database_i* pdatabase, bool bActivate ) 
{                                                                                                  assert( pdatabase != nullptr );
   pdatabase->add_reference();
   m_vectorDatabase.push_back( pdatabase ); 
   if( bActivate == true )
   {
      if( m_pdatabase != nullptr ) m_pdatabase->release();
      m_pdatabase = pdatabase;
      if( m_pdatabase != nullptr ) m_pdatabase->add_reference();
   }
}


/** ---------------------------------------------------------------------------
 * @brief Close active database
*/
void CApplication::DATABASE_CloseActive()
{
   if( m_pdatabase != nullptr )
   {
      m_pdatabase->release();
      m_pdatabase = nullptr;
   }
}



void CApplication::Prepare_s(gd::cli::options& optionsApplication)
{
   {  // ## `copy` command, copies file from source to target
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "count", "count lines in file" );
      optionsCommand.add({"source", 's', "File to count lines in"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }

   {  // ## `count` command, count number of lines in file
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "copy", "Copy file from source to target" );
      optionsCommand.add({"source", 's', "File to copy"});
      optionsCommand.add({"destination", 'd', "Destination, where file is copied to"});
      optionsCommand.add({"backup", 'b', "If destination file exits then make a backup"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }


   {  // ## `join` command, joins two or more files
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
 