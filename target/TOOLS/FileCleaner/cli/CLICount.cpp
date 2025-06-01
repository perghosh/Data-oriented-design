/**
 * @file CLIHistory.cpp
 * @brief Command line interface for counting lines in files.
 */

 // @TAG #cli


#include "../Command.h"
#include "../Application.h"

#include "CLICount.h"




NAMESPACE_CLI_BEGIN

// ## History operations

std::pair<bool, std::string> Count_g( const gd::cli::options* poptionsCount, CDocument* pdocument )
{                                                                                                  assert( poptionsCount != nullptr );
   const gd::cli::options& options_ = *poptionsCount;

   std::string stringCommandName = options_.name();
   if( stringCommandName == "count" )
   {
      if( options_.exists("explain") == true )
      {
         std::string stringExplain = CountGetExplain_g(( *poptionsCount )["explain"].as_string());
         pdocument->MESSAGE_Display(stringExplain);
      }
      else
      {
         auto result_ = CountLine_g(poptionsCount, pdocument);
         if( result_.first == false ) return result_;
      }

   }

   return { true, "" };
}

/** --------------------------------------------------------------------------- @TAG #print
 * @brief Executes the "count" command based on the provided options.
 *
 * This method processes the "count" command, which involves harvesting files,
 * applying filters, counting rows, and optionally saving or printing results.
 *
 * ### Arguments in optionsApplication:
 * - `source` (string, required): Specifies the file or folder to count lines in.
 * - `recursive` (integer, optional): Specifies the depth for recursive operations.
 * - `R` hardcoded recursive and sets depth to 16 (all).
 * - `sort` (string, optional): Specifies the column to sort the results by.
 * - `stats` (string, optional): Specifies the statistics to calculate (sum, count, relation).
 * - `max` (integer, optional): Specifies the maximum number of lines to process.
 * - `segment` (string, optional): Specifies the type of segment to search in (code, comment, string).
 * - `filter` (string, optional): A filter to apply to the files. If empty, all files are counted.
 * - `pattern` (string, optional): Patterns to search for, separated by `,` or `;`.
 * - `print` (flag, optional): Indicates whether to print the results to the console.
 * - `output` (string, optional): Specifies the file to save the output. Defaults to stdout if not set.
 * - `table` (string, optional): Specifies the table name for generating SQL insert queries.
 *
 * @param poptionsActive The active command-line options that should be on 'count'.
 * @return std::pair<bool, std::string> A pair indicating success or failure and an error message if applicable.
 */
std::pair<bool, std::string> CountLine_g(const gd::cli::options* poptionsCount, CDocument* pdocument)
{                                                                                                  assert( poptionsCount != nullptr );
   const gd::cli::options& options_ = *poptionsCount;

   enum { linecount_report_, patterncount_report_ };
   enum stats { stats_none_ = 0, stats_sum_ = 0x01, stats_count_ = 0x02, stats_relation_ = 0x04 };
   int iReportType = linecount_report_; // default to line report
   unsigned uStatistics = stats_none_; // default to no statistics

   // Harvest files based on the "source" option
   std::string stringSource = options_["source"].as_string();
   bool bExplain = options_["explain"].is_true();
   
   CApplication::PathPrepare_s(stringSource);
   int iRecursive = options_["recursive"].as_int();
   if( iRecursive == 0 && options_.exists("R") == true ) iRecursive = 16;// set to 16 if D is set, find all files

   std::string stringFilter = options_["filter"].as_string();
   if( stringFilter == "*" ) 
   { 
      stringFilter.clear();                                                   // if filter is set to * then clear it, we want all files
      if( iRecursive == 0 ) iRecursive = 16;                                  // if recursive is not set, set it to 16, find all files
   }

   gd::argument::shared::arguments argumentsPath({ {"source", stringSource}, {"recursive", iRecursive} });
   auto result_ = pdocument->FILE_Harvest(argumentsPath, stringFilter);                            if( !result_.first ) { return result_; }

   // Count rows in the harvested files
   result_ = pdocument->FILE_UpdateRowCounters();                                                  if( !result_.first ) { return result_; }

   if( options_["pattern"].is_true() )                              // Handle pattern matching if specified
   {
      iReportType = patterncount_report_;                                           // set report type to pattern report
      std::string stringPattern = options_["pattern"].as_string();
      auto vectorPattern = CApplication::Split_s(stringPattern);
      result_ = pdocument->FILE_UpdatePatternCounters(vectorPattern);                              if( !result_.first ) { return result_; }
   }

   // ## Determine sorting options
   if( options_["sort"].is_true() == true )
   {
      std::string stringSortColumn = options_["sort"].as_string();
      result_ = pdocument->CACHE_Sort( "file-count", stringSortColumn );                           if( !result_.first ) { return result_; }
   }

   // ## Determine statistics options
   if( options_["stats"].is_true() == true )
   {
      std::string stringStats = options_["stats"].as_string();
      if( stringStats.find("sum") != std::string::npos ) uStatistics |= stats_sum_;
      if( stringStats.find("count") != std::string::npos ) uStatistics |= stats_count_;
      if( stringStats.find("relation") != std::string::npos ) uStatistics |= stats_relation_;
      if( uStatistics == 0 ) uStatistics = stats_sum_;                         // default if no other is sum
   }


   // Determine output options
   bool bPrint = options_.exists("print");
   std::string stringOutput = options_["output"].as_string();
   bool bOutput = options_["output"].is_true();

   if( !bPrint && !bOutput && stringOutput.empty() ) { bPrint = true; }        // Default to printing if no output options are specified
   if( bPrint == true && uStatistics == 0 ) { uStatistics = stats_sum_; }      // set output to stdout if print is set

   // ## prepare statistics

   gd::table::dto::table tableResult;
   if( iReportType == linecount_report_ ) { tableResult = pdocument->RESULT_RowCount(); }
   else                                   { tableResult = pdocument->RESULT_PatternCount(); }

   unsigned uFooterRowCount = 0;
   if( uStatistics != 0 )
   {
      if( uStatistics & stats_sum_ )
      {
         if( iReportType == linecount_report_ )
         {
            if( bExplain == true ) { pdocument->MESSAGE_Display(CLI::CountGetExplain_g("count-lines")); }
            result_ = TABLE_AddSumRow(&tableResult, { 2, 3, 4, 5, 6 });                            if( !result_.first ) { return result_; }
            tableResult.cell_set(tableResult.get_row_count() - 1, "folder", "Total:");
            uFooterRowCount = 1;
         }
         else if( iReportType == patterncount_report_ )
         {                                                                                         assert(options_["pattern"].is_true());
            std::vector<unsigned> vectorColumn;
            for( auto u = 2u; u < tableResult.get_column_count(); u++ ) vectorColumn.push_back(u);// add sum columns

            result_ = TABLE_AddSumRow(&tableResult, vectorColumn);                                 if( !result_.first ) { return result_; }
            uFooterRowCount = 1;
         }
      }
   }

   // ## Paging and limiting results to display
   std::string stringHeader;

   if( options_.exists("page") == true )
   {
      uint64_t uPageSize = options_["page-size"].as_uint64();
      if( uPageSize == 0 ) uPageSize = 10;                                    // default page size
      int64_t iPage = options_["page"].as_int64();                            // default page number
      if( iPage > 0 ) iPage--;                                                // internal page is 0 based


      gd::table::page page_( (uint64_t)iPage, uPageSize, 0, uFooterRowCount, tableResult.size() );
      if( iPage > (int64_t)page_.get_page_count() || iPage < 0 ) 
      { 
         page_.set_page(page_.get_page_count() - 1);                          // set to last page if out of range
         uint64_t uPage = page_.get_page();
         page_.set_flags(gd::table::page::eFlagAll, 0);                       // copy all rows from page index to end
         stringHeader += "From row: " + std::to_string(page_.first() + 1) + " in page " + std::to_string(uPage + 1) +  " to row: " + std::to_string(page_.get_row_count() + 1) + "\n";
      }
      else
      {
         stringHeader += "Page: " + std::to_string(iPage + 1) + " of " + std::to_string(page_.get_page_count() + 1) + "\n";
      }

      gd::table::dto::table tableResultPage(tableResult, page_);              // create a new table with the page size
      tableResult = std::move(tableResultPage);                               // move the page result to the original table
   }  

   
   if( bPrint || bOutput || !stringOutput.empty() )                            // Generate and handle results ?
   {
      if( bPrint == true ) 
      {
         
         std::string stringCliTable = gd::table::to_string(tableResult, { {"verbose", true} }, gd::table::tag_io_cli{});
         if( options_.exists("vs") == false )
         {
            if( stringHeader.empty() == false  ) pdocument->MESSAGE_Display( stringHeader );
            pdocument->MESSAGE_Display( stringCliTable );
         }
         else
         {
            // generate string to prepend to output
            pdocument->MESSAGE_Display(stringCliTable, { {"ui","vs"}});
         }
      }

      // ## Save result if output is specified 

      if( stringOutput.empty() == false ) 
      {
         gd::argument::shared::arguments argumentsResult({ {"type", "COUNT"}, {"output", stringOutput}, {"table", options_["table"].as_string()} });
         result_ = pdocument->RESULT_Save(argumentsResult, &tableResult);                          if( !result_.first ) { return result_; }
      }
   }

   return { true, "" };
}


/**
* @brief 
*/
std::string CountGetExplain_g( const std::string_view& stringType )
{
   std::string stringHelp;
   if( stringType == "count-lines" )
   {
      stringHelp = 
R"(
Count lines in file/files.
   columns:
      - folder - The name of the folder containing the file.
      - filename - The name of the file being analyzed.
      - count - Total number of lines in the file.
      - code - Number of lines containing actual code (excluding comments and whitespace).
      - characters - Total count of code characters (excluding comments and strings).
      - comment - The number of comment segments (not lines—counts).
      - string - The number of string segments (not lines—counts).

)";
   }

   return stringHelp;
}

NAMESPACE_CLI_END