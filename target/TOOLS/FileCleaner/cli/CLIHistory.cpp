/**
 * @file CLIHistory.cpp
 */

// @TAG #cli #history

#include <fstream>
#include <pugixml/pugixml.hpp>

#include "gd/expression/gd_expression_runtime.h"


#include "../Application.h"

#include "CLIHistory.h"

                           



NAMESPACE_CLI_BEGIN


// ## History operations

// ## Forward declarations

static std::pair<bool, std::string> HistoryPrepareXml_s(const gd::argument::arguments& argumentsXml);

std::pair<bool, std::string> History_g(const gd::cli::options* poptionsHistory)
{
   const gd::cli::options& options_ = *poptionsHistory;
   if( options_.exists("create") == true )
   {
      gd::argument::arguments argumentsCreate( {"create", options_["create"].as_string()} );
      auto result_ = HistoryCreate_g(argumentsCreate);
   }

   return { true, "" };
}


/**
 * @brief Set up folders for cleaner and creates history file if not exists
 */
std::pair<bool, std::string> HistoryCreate_g( const gd::argument::arguments& argumentsCreate )
{
   //auto result_ = papplication_g->CreateDirectory();                                               if( result_.first == false ) { return result_; }

   std::filesystem::path pathCurrentDirectory = std::filesystem::current_path() / ".cleaner";

   if( std::filesystem::exists(pathCurrentDirectory) == false )
   {
      std::filesystem::create_directory(pathCurrentDirectory); 
      std::ofstream ofstreamFile(pathCurrentDirectory / "history.xml");
      ofstreamFile.close();
   }
   else
   {
      HistoryDelete_g(argumentsCreate); // TODO: This is just temporary, we need to remove this later
   }

   return { true, "" };
}

std::pair<bool, std::string> HistoryDelete_g(const gd::argument::arguments& argumentsCreate)
{
   std::filesystem::path pathCurrentDirectory = std::filesystem::current_path() / ".cleaner";

   if( std::filesystem::exists(pathCurrentDirectory) == true  )
   {
      std::filesystem::remove_all(pathCurrentDirectory); // remove the history folder
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Prepare XML file for history
 * 
 * This function checks if the specified XML file exists, and if not, it creates a new XML document with a root node "history".
 * If the file exists, it loads the XML document and ensures that the root node is "history".
 * 
 * @param argumentsXml The arguments containing the file name.
 * @param argumentsXml.file {string}  The file name of the XML document to prepare.
 * @return A pair containing a boolean indicating success or failure, and a string with an error message if applicable.
 * 
 * @code
 * // Example usage: HistoryPrepareXml_s
 * gd::argument::arguments argumentsXml({{"file", "history.xml"}});
 * auto [success, message] = HistoryPrepareXml_s(argumentsXml);
 * if (!success) { std::cerr << "Error: " << message << std::endl; } 
 * else { std::cout << "History XML prepared successfully." << std::endl; }
 * @endcode
 */
std::pair<bool, std::string> c(const gd::argument::arguments& argumentsXml)
{ 
   std::string stringFileName = argumentsXml["file"].as_string();
   if( std::filesystem::exists(stringFileName) == false ) {  return { false, "History file does not exist: " + stringFileName }; }


   pugi::xml_document xmldocument;
   pugi::xml_parse_result result_ = xmldocument.load_file(stringFileName.c_str()); if (!result_) { return { false, "Failed to load XML file: " + stringFileName }; }

   // Check if the root node is "history"
   pugi::xml_node xmlnodeRoot = xmldocument.child("history");

   // If the root node is not "history", create it
   if( xmlnodeRoot.empty() == true ) { xmlnodeRoot = xmldocument.append_child("history"); }

   // save the modified XML document back to the file
   xmldocument.save_file(stringFileName.c_str(), "  ", pugi::format_default );

   return { true, "" };
}

NAMESPACE_CLI_END