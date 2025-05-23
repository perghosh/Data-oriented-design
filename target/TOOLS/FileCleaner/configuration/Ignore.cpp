/**
* @file Ignore.cpp
*/

#include "Ignore.h"
#include <fstream>




NAMESPACE_CONFIGURATION_BEGIN

/** ---------------------------------------------------------------------------
 * @brief Reads and parses a .gitignore file to extract ignore patterns.
 *
 * Opens the specified .gitignore file, reads each line, trims whitespace,
 * skips comments and empty lines, and stores valid ignore patterns in the provided vector.
 *
 * @param stringPath Path to the .gitignore file.
 * @param pvectorPatterns Pointer to a vector to store ignore patterns.
 * @return std::pair<bool, std::string> True and empty string if successful, otherwise false and error message.
 */
std::pair<bool, std::string> CIgnore::Read_s(const std::string& stringPath, std::vector<std::string>* pvectorPatterns, tag_git)
{
   std::ifstream ifstreamFile(stringPath);                                      // open file stream
   if( ifstreamFile.is_open() == false ) { return { false, "Failed to open file: " + stringPath }; }

   std::string stringLine;
   while(std::getline(ifstreamFile, stringLine))                             // read each line
   {
      // Trim whitespace from start and end
      stringLine.erase(0, stringLine.find_first_not_of(" \t\r\n"));
      stringLine.erase(stringLine.find_last_not_of(" \t\r\n") + 1);

      // Skip empty lines or comments
      if(stringLine.empty() == true || stringLine[0] == '#') { continue; }

      // Skip lines that are too short or invalid
      if(stringLine.empty() == true ) { continue; }

      // Store valid pattern
      if(pvectorPatterns != nullptr)
      {
         pvectorPatterns->push_back(stringLine);                              // add pattern to vector
      }
   }

   ifstreamFile.close();                                                      // close file stream
   return { true, "" };
}

std::string_view CIgnore::Type_s(const std::string_view& stringPattern, tag_git)
{                                                                                                  assert(stringPattern.empty() == false); // check if string is not empty
   if( stringPattern.back() == '/' ) { return std::string_view("directory"); } // check if pattern is a directory

   if(stringPattern.find('.') != std::string::npos && stringPattern.find('/') == std::string::npos) 
   {
      return std::string_view("file");                                          // return file type
   }

   // Check for patterns like "dir/*" or "dir/**" which indicate directories
   if(stringPattern.find("/*") != std::string::npos || stringPattern.find("/**") != std::string::npos)
   {
      return std::string_view("directory");
   }

   // Patterns with directory separators but no file extension are likely directories
   if(stringPattern.find('/') != std::string::npos && stringPattern.find('.') == std::string::npos)
   {
      return std::string_view("directory");
   }

   return std::string_view("all");
}



NAMESPACE_CONFIGURATION_END