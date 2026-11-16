// @FILE [tag: cli, command, help] [description:implementation for CLI help operations] [type: source]

#include <cassert>

#include "CLIHelp.h"

NAMESPACE_CLI_BEGIN

namespace
{
   // ## LLM help
   // Compact, no decoration. Only options an agent needs are listed. Output format is stated so it can be parsed.

   constexpr std::string_view stringHelpListLLM_g = R"help(list: line-by-line search. Prints matching lines as `path:line - [text]`.
Use for single-line matches (names, calls, declarations). Use `find` for patterns spanning lines.

Usage:
  cleaner list "<filter>" -R --pattern "<text>" --current-directory "<folder>" --max 50 -llm

Required:
  "<filter>"             First positional argument. File wildcards separated by ';', e.g. "*.h;*.cpp"
  --current-directory    Folder to search in
  --pattern "<text>"     Literal text. Several values separated by ';'
  --rpattern "<regex>"   Regular expression. Repeat the option for several. Use --pattern OR --rpattern

Options:
  -R                     Search subfolders. Without it only the top folder is searched
  --max N                Max matches returned. Always set it, e.g. 50
  --context N            Also show N lines after each match
  --context S,N          Show N lines starting at offset S from the match. Negative S starts before it: --context -2,5
  --segment <type>       code (default, skips comments and strings) | comment | string | all
  --ignore "a;b"         Skip folders with these names
  --icase                Ignore case
  --word                 Whole words only
  --match-all            Line must match ALL patterns (default: any pattern)
  -llm                   Output adapted for LLM, match lines marked with '>'. Use together with --context

Notes:
  Progress lines ("Find in files: ...") are noise. Exit code -1 is normal, judge by the output.
  Start with a small --max, then narrow the pattern or filter instead of raising --max.

Examples:
  cleaner list "*.h;*.cpp" -R --pattern CRouter --max 30 -llm --current-directory "C:\dev\project"
  cleaner list "*.cpp" -R --rpattern "^\s*void\s+\w+::Start" --context 0,6 -llm --current-directory "C:\dev\project"
  cleaner list "*.h" -R --pattern "Start;Stop" --match-all --word --max 20 --current-directory "C:\dev\project"
  cleaner list "*.cpp" -R --pattern "TODO" --segment comment --max 20 --current-directory "C:\dev\project"
)help";

   constexpr std::string_view stringHelpFindLLM_g = R"help(find: searches the whole content of each file, so a match can span several lines.
Use for multi-line patterns (function bodies, blocks, declarations split over lines). Use `list` for single lines.

Usage:
  cleaner find "<filter>" -R --rpattern "<regex>" --current-directory "<folder>" --max 20 -llm

Required:
  "<filter>"             First positional argument. File wildcards separated by ';', e.g. "*.h;*.cpp"
  --current-directory    Folder to search in
  --pattern "<text>"     Literal text. Several values separated by ';'
  --rpattern "<regex>"   Regular expression, matched against whole file content. Use --pattern OR --rpattern

Options:
  -R                     Search subfolders. Without it only the top folder is searched
  --max N                Max matches returned. Always set it, e.g. 20
  --context N            Also show N lines around each match
  --segment <type>       code (default, skips comments and strings) | comment | string
  --ignore "a;b"         Skip folders with these names
  --match-all            Result must match ALL patterns (default: any pattern)
  -llm                   Output adapted for LLM. Use together with --context

Notes:
  Matches can be large. Keep patterns specific and always set --max.
  Progress lines ("Find in files: ...") are noise. Exit code -1 is normal, judge by the output.

Examples:
  cleaner find "*.cpp" -R --rpattern "if\(.*\)\s*\{[^}]*return" --max 10 -llm --current-directory "C:\dev\project"
  cleaner find "*.h" -R --pattern "class CRouter" --context 10 -llm --current-directory "C:\dev\project"
)help";

   constexpr std::string_view stringHelpCountLLM_g = R"help(count: counts matches per file. Use for scoping only (where is something used, how often). Output is a table with counts per file.
Then use `list` or `find` to get the exact lines before reading or editing code.

Usage:
  cleaner count "<filter>" -R --pattern "<text>" --current-directory "<folder>"

Required:
  "<filter>"             First positional argument. File wildcards separated by ';', e.g. "*.h;*.cpp"
  --current-directory    Folder to search in
  --pattern "<text>"     Literal text. Several values separated by ';'
  --rpattern "<regex>"   Regular expression. Repeat the option for several. Use --pattern OR --rpattern

Options:
  -R                     Search subfolders. Without it only the top folder is searched
  --segment <type>       code (default, skips comments and strings) | comment | string | all
  --ignore "a;b"         Skip folders with these names
  --sort <column>        Sort result on this column
  --page-size N          Max rows per page (default 10)
  --page N               Page to print, for reading further results
  -llm                   Output adapted for LLM

Examples:
  cleaner count "*.h;*.cpp" -R --pattern CRouter --current-directory "C:\dev\project"
  cleaner count "*.cpp" -R --pattern "set;compare" --page-size 20 --current-directory "C:\dev\project"
)help";

   // ## User help
   // Fuller descriptions for a person at a terminal. Only the commonly used options are listed.

   constexpr std::string_view stringHelpListUser_g = R"help(list - Search files and list lines matching patterns. Searches line by line.

Usage: cleaner list [filter] [options]

Arguments:
  filter              File name wildcards, separated by ';' (for example "*.h;*.cpp"). All files if omitted
  --pattern           Text to search for. Separate several with ',' or ';'
  --rpattern          Regular expression to search for
  --source            File(s) or folder(s) to search in, separated by ';'
  --ignore            Folder names to exclude, separated by ';'
  --context N[,N]     Show context lines around each match (one or two numbers)
  --max N             Max number of matching lines returned
  --segment           Segment of code to search: code, comment, string or all
  --expression        Inline script expression for custom result handling
  --script            External script file for custom result handling

Flags:
  -R                  Search all subfolders (depth 16)
  --match-all         All patterns must match in the same line
  --icase             Ignore case
  --word              Match whole words only
  -llm                Output adapted for LLM
  --vs                Visual Studio output window format (clickable files)

Global:
  --current-directory Folder to run in
)help";

   constexpr std::string_view stringHelpFindUser_g = R"help(find - Search patterns in files. All file content is searched, so patterns can span lines.

Usage: cleaner find [filter] [options]

Arguments:
  filter              File name wildcards, separated by ';' (for example "*.h;*.cpp"). All text files if omitted
  --pattern           Text to search for, separated by ';'
  --rpattern          Regular expression to search for
  --source            Directory(ies) to start in, separated by ';'
  --ignore            Folder names to exclude, separated by ';'
  --context N         Show surrounding text for each result
  --max N             Max number of results
  --segment           Segment of code to search: code, string or comment
  --kv, --keys        Extract key-value pairs
  --header, --footer  Columns or keys to include as header or footer
  --rule              Rules for actions to perform on found matches
  --script            External script file for custom result handling
  --width N           Width of output

Flags:
  -R                  Search all subfolders (depth 16)
  --match-all         All patterns must match
  --quiet             Hide file positions in output
  -llm                Output adapted for LLM
  --vs                Visual Studio output window format (clickable files)

Global:
  --current-directory Folder to run in
)help";

   constexpr std::string_view stringHelpCountUser_g = R"help(count - Count patterns, lines and segments in selected files.

Usage: cleaner count [filter] [options]

Arguments:
  filter              File name wildcards, separated by ';'. All found text files if omitted
  --pattern           Text to search for, separated by ',' or ';'
  --rpattern          Regular expression to search for
  --source            File(s) or folder(s) to count in
  --ignore            Folder(s) to ignore
  --segment           Segment of code to search in
  --sort              Sort result on column name
  --page N            Page to print
  --page-size N       Max rows per page (default 10)
  --where             SQL-like condition for filtering result
  --where-expression  Internal expression format for filtering result
  --stats             Add statistics to output
  --table             Table name, used when generating for example sql insert queries

Flags:
  -R                  Search all subfolders (depth 16)
  -llm                Output adapted for LLM
  --vs                Visual Studio output window format (clickable files)

Global:
  --current-directory Folder to run in
)help";
}

std::pair<bool, std::string> Help_g(CDocument* pdocument, const gd::cli::options* poptionsHelp, gd::cli::options* poptionsApplication)
{                                                                                                   assert(poptionsHelp != nullptr);
   if(poptionsHelp->exists("command") == true)
   {
      std::string stringDocumentation;
      auto  [bOk, stringError] = HelpCommand_g((*poptionsHelp)["command"].as_string(), poptionsHelp->exists("llm"), stringDocumentation);
      if( bOk == false ) return { false, stringError };

      pdocument->MESSAGE_Display(stringDocumentation);
   }

   return { true, "" };
}


std::pair<bool, std::string> HelpCommand_g( std::string_view stringCommand, bool bLLM, std::string& stringDocumentation)
{
   if( stringCommand == "list" )
   {
      stringDocumentation = bLLM ? std::string(stringHelpListLLM_g) : std::string(stringHelpListUser_g);
   }
   else if(stringCommand == "find")
   {
      stringDocumentation = bLLM ? std::string(stringHelpFindLLM_g) : std::string(stringHelpFindUser_g);
   }
   else if(stringCommand == "count")
   {
      stringDocumentation = bLLM ? std::string(stringHelpCountLLM_g) : std::string(stringHelpCountUser_g);
   }
   else
   {
      return { false, "Unknown command for help: " + std::string(stringCommand) };
   }

   return { true, stringDocumentation };
}

NAMESPACE_CLI_END