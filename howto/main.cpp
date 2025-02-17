#include <filesystem>

#include "gd/gd_file.h"

#include "catch2/catch_amalgamated.hpp"

#include "main.h"

MainArguments mainarguments_g;


/*----------------------------------------------------------------------------- 
 * start method for console test application
 * \param iArgumentCount number of arguments
 * \param ppbszArgumentValue argument values
 * \return int return 0 if no error, otherwise non zero value
 */
int main(int iArgumentCount, char* ppbszArgumentValue[])
{
   mainarguments_g = MainArguments( iArgumentCount, ppbszArgumentValue );

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


