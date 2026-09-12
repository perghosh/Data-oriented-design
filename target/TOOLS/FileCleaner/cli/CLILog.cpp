/** @FILE [tag: cli, command, log] [description:implementation for CLI log operations] [type: source]
 * @file CLILog.cpp
 * @brief Implementation file for CLI log operations.
 */

#include "../Command.h"
#include "../Application.h"

#include "CLILog.h"

NAMESPACE_CLI_BEGIN 

// count  --source "C:\dev\home\DOD\external\gd" -R --sort count --stats "sum"


std::pair<bool, std::string> Log_g( const gd::cli::options* poptionsLog, gd::cli::options* poptionsApplication )
{
    const gd::cli::options& options_ = *poptionsList;

#ifndef NDEBUG
    [[maybe_unused]] std::string string_d = gd::argument::debug::print(poptionsList->get_arguments());
#endif // NDEBUG


    if (options_.exists("clip") == true && options_["clip"].is_true() == true) // test for clip argument
    {
        std::string stringFile;
        OS_ReadClipboard_g(stringFile);                                          // read clipboard content

        // ### Check if clipboard content is a valid file path and informtion user if found
        if (stringFile.empty() == false && std::filesystem::exists(stringFile) == true)
        {
            pdocument->MESSAGE_Display(std::format("File from clipboard as source: {}", stringFile));
            poptionsList->set_value("source", stringFile);                       // set source to the file from clipboard
        }
    }

    std::string stringCommandName = options_.name();
    if (stringCommandName == "log")
    {
        auto result_ =  LogPattern_g(poptionsList, pdocument);
        if (result_.first == false) return result_;
    }

    return { true, "" };
}

std::pair<bool, std::string> ListPattern_g(const gd::cli::options* poptionsList, CDocument* pdocument)
{                                                                                                   assert(poptionsList != nullptr); assert(pdocument != nullptr);
    size_t uSearchPatternCount = 0; // count of patterns to search for
    const gd::cli::options& options_ = *poptionsList;
    return { true, "" }; // return success

}


NAMESPACE_CLI_END
