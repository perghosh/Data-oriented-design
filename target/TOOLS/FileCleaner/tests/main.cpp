#include <filesystem>
#include <random>

#include "gd/gd_file.h"

#include "catch2/catch_amalgamated.hpp"

#include "main.h"

Main Main_g;

/*----------------------------------------------------------------------------- 
 * start method for console test application
 * \param iArgumentCount number of arguments
 * \param ppbszArgumentValue argument values
 * \return int return 0 if no error, otherwise non zero value
 */
int main(int iArgumentCount, char* ppbszArgumentValue[])
{
   Main_g = Main( iArgumentCount, ppbszArgumentValue );

	int iResult = Catch::Session().run(iArgumentCount, ppbszArgumentValue);

	return iResult;
}


/** ---------------------------------------------------------------------------
 * @brief Walk upp the folder tree and try to find folder containing file
 * @param stringSubfolder add this folder to found root folder, if empty then root folder is returned
 * @return std::string root folder name
*/
std::string FOLDER_GetRoot_g( const std::string_view& stringSubfolder )
{
   std::filesystem::path pathCurrentDirecotry = std::filesystem::current_path();
   auto [bFound, stringRootFolder] = gd::file::closest_having_file_g( pathCurrentDirecotry.string(), ROOT_MARKER );

   if( bFound == true ) stringRootFolder += stringSubfolder;

   std::filesystem::path path_( stringRootFolder );
   stringRootFolder = path_.make_preferred().string();
   return stringRootFolder;
}


/** ---------------------------------------------------------------------------
 * @brief Generate random name
 * @param uLength length of random name
 * @return std::string random name
 */
std::string GENERATE_RandomName( size_t uLength )
{
   static const char palphanum_s[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";

   // ## prepare random generator
   std::random_device RD;
   std::mt19937 mt19937(RD());
   std::uniform_int_distribution<> UID(0, sizeof(palphanum_s) - 2);

   // ## generate random name
   std::string stringName;
   stringName.reserve(uLength);
   for( size_t u = 0; u < uLength; u++ )
   {
      stringName += palphanum_s[UID(mt19937)];
   }
   return stringName;
}