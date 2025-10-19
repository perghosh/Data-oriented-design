#include "../gd_table_aggregate.h"

#include "gd_table_formater.h"

_GD_TABLE_BEGIN

namespace format
{

/** ---------------------------------------------------------------------------
 * @brief Print table data as cards/boxes arranged horizontally, with optional bordering and customizable characters.
 * 
 * Each row is printed as a vertical card containing the values from the specified columns,
 * stacked top-down. Cards are arranged in horizontal rows based on the total width.
 * The box content width is provided as uBoxWidth (precomputed max value length).
 * The full box width (including borders or padding) is used to calculate the number of
 * cards per horizontal row. A callback can be provided to modify the string values for
 * each card before printing.
 * 
 * @tparam TABLE The table type being printed.
 * @tparam CALLBACK The type of the optional callback function, defaults to std::nullptr_t.
 *                  If provided, it should have signature: void(uint64_t row, std::vector<std::string>& values).
 * @param table The table to print.
 * @param uBegin The starting row index.
 * @param uCount The number of rows to print (may be overridden by "count" argument).
 * @param vectorColumn The indices of columns to include in each card.
 * @param uBoxWidth The content width for each box (max value length across selected columns).
 * @param uTotalWidth The total available width for arranging cards horizontally.
 * @param callback Optional callback to modify values per card.
 * @param argumentOption Options for printing:
 *                       - "count": uint64_t, max rows to print.
 *                       - "border": bool, whether to draw borders (default: true).
 *                       - "tl", "tr", "bl", "br": string (length 1), border corner characters (default: "+").
 *                       - "horizontal": string (length 1), horizontal line character (default: "-").
 *                       - "vertical": string (length 1), vertical line character (default: "|").
 *                       - "row-space": unsigned, number of blank lines between rows (default: 1).
 *                       - "max-value-width": unsigned, truncate content width if exceeded.
 *                       - "prepend": string, text to prepend to output.
 * @param stringOut The output string to append the formatted cards to.
 * @param tag_card Tag to distinguish this overload for card printing.
 */
template <typename TABLE, typename CALLBACK = std::nullptr_t>
void to_string_s( const TABLE& table, uint64_t uBegin, uint64_t uCount, const std::vector<unsigned>& vectorColumn, unsigned uBoxWidth, unsigned uTotalWidth, CALLBACK&& callback_, const gd::argument::arguments& argumentOption, std::string& stringOut, gd::types::tag_card )
{
   if( argumentOption.exists("count") == true ) { uCount = argumentOption["count"].as_uint64(); }  // set max count if argument for count is found
   uint64_t uEndRow = uBegin + uCount;                                    // last row to print
   if( uEndRow > table.get_row_count() ) { uEndRow = table.get_row_count(); }
   if( vectorColumn.empty() == true || uEndRow <= uBegin ) { return; }    // nothing to print
   bool bHasNull = table.is_null();                                       // check if table supports null values

   unsigned uEffectiveWidth = uBoxWidth;                                  // effective content width after limits
   unsigned uMaxValueWidth = (unsigned)-1;                                // max content length before padding
   if( argumentOption.exists("max-value-width") == true )
   {
      uMaxValueWidth = argumentOption["max-value-width"].as_uint();
      if( uEffectiveWidth > uMaxValueWidth ) { uEffectiveWidth = uMaxValueWidth; }
   }

   unsigned uRowSpace = 1;                                                // space between rows (not used in card format)
   if( argumentOption.exists("row-space") == true ) { uRowSpace = argumentOption["row-space"].as_uint(); }


   bool bBorder = true;                                                   // draw borders by default
   if( argumentOption.exists("border") == true ) { bBorder = argumentOption["border"].is_true(); }

   // ## extract border characters from arguments, with defaults
   char piBorder[] = { '+', '+', '+', '+', '-', '|' };                    // tl, tr, bl, br, horizontal, vertical
   const std::string_view stringBorderKeys[] = { "tl", "tr", "bl", "br", "horizontal", "vertical" };
   
   for( unsigned uIdx = 0; uIdx < 6; uIdx++ )
   {
      if( argumentOption.exists(stringBorderKeys[uIdx]) == true )
      {
         std::string stringChar = argumentOption[stringBorderKeys[uIdx]].as_string();
         if( stringChar.length() > 0 ) { piBorder[uIdx] = stringChar[0]; }
      }
   }
   
   char iTL = piBorder[0]; // top-left
   char iTR = piBorder[1]; // top-right
   char iBL = piBorder[2]; // bottom-left
   char iBR = piBorder[3]; // bottom-right
   char iH =  piBorder[4]; // horizontal
   char iV =  piBorder[5]; // vertical

   unsigned uFullBoxWidth = uEffectiveWidth + 2;                          // width including left/right borders
   unsigned uSeparation = 1;                                              // spaces between boxes
   unsigned uCardPerRow = 1;                                              // max cards per horizontal row
   // ## calculate how many cards fit in total width
   for( unsigned uAttempt = 1; ; uAttempt++ )
   {
      unsigned uNeededWidth = uAttempt * uFullBoxWidth + std::max( 0u, uAttempt - 1 ) * uSeparation;
      if( uNeededWidth > uTotalWidth ) { break; }
      uCardPerRow = uAttempt;
   }

   std::string stringResult;                                              // result string with card data
   std::vector<gd::variant_view> vectorRowValue;                          // row values as variant views
   std::string stringValue;                                               // temporary value string

   // ## check for prepend string
   if( argumentOption.exists("prepend") == true ) { stringResult += argumentOption["prepend"].as_string(); }

   uint64_t uNumRows = uEndRow - uBegin;
   uint64_t uNumPrintRows = ( uNumRows + uCardPerRow - 1 ) / uCardPerRow;   // ceil division

   // ## iterate print rows (each print row contains uCardPerRow cards)
   for( uint64_t uPrintRow = 0; uPrintRow < uNumPrintRows; uPrintRow++ )
   {
      uint64_t uStartRow = uBegin + uPrintRow * uCardPerRow;
      uint64_t uEndThisRow = std::min( uStartRow + uCardPerRow, uEndRow );
      unsigned uNumCardsThisRow = static_cast<unsigned>( uEndThisRow - uStartRow );

      std::vector< std::vector<std::string> > vectorCardLines;            // lines for each card in this print row
      vectorCardLines.reserve( uNumCardsThisRow );

      // ## build card lines for each card in this print row
      for( unsigned uCard = 0; uCard < uNumCardsThisRow; uCard++ )
      {
         vectorRowValue.clear();
         uint64_t uRow = uStartRow + uCard;
         table.row_get_variant_view( uRow, vectorRowValue );                  // fetch row values

         std::vector<std::string> vectorBoxValue;                             // strings for this card (one per row)
         vectorBoxValue.reserve( vectorColumn.size() );

         // ## extract and convert selected column values to strings .........
         for( unsigned uColumn : vectorColumn )
         {
            const auto& value_ = vectorRowValue[uColumn];
            if( value_.is_null() == true ) { stringValue = ""; }
            else if( value_.is_string() == true ) { stringValue = value_.as_string_view(); }
            else { stringValue = value_.as_string(); }

            vectorBoxValue.push_back( std::move( stringValue ) );
         }

         // ## apply callback to modify box values if provided
         if constexpr( !std::is_same_v<std::decay_t<decltype( callback_ )>, std::nullptr_t> ) // check if callback is provided (not nullptr)
         {
            callback_( uRow, vectorBoxValue );
         }

         // ## truncate and pad values to effective width ....................
         for( auto& stringValue : vectorBoxValue )
         {
            if( stringValue.length() > uMaxValueWidth )
            {
               stringValue = stringValue.substr( 0, uMaxValueWidth - 3 );
               stringValue += std::string_view{ "..." };
            }
            if( static_cast<unsigned>( stringValue.length() ) > uEffectiveWidth )
            {
               stringValue = stringValue.substr( 0, uEffectiveWidth - 3 );
               stringValue += std::string_view{ "..." };
            }
            stringValue.append( uEffectiveWidth - static_cast<unsigned>( stringValue.length() ), ' ' );
         }

         // ## build lines for this card
         std::vector<std::string> vectorLines;
         if( bBorder == true )
         {
            // ### top line
            std::string stringTopLine;
            stringTopLine += iTL;
            stringTopLine.append( uEffectiveWidth, iH );
            stringTopLine += iTR;
            vectorLines.push_back( std::move( stringTopLine ) );

            // ### inner lines (values stacked vertically)
            for( const auto& stringContent : vectorBoxValue )
            {
               std::string stringInnerLine;
               stringInnerLine += iV;
               stringInnerLine += stringContent;
               stringInnerLine += iV;
               vectorLines.push_back( std::move( stringInnerLine ) );
            }

            // ### bottom line
            std::string stringBottomLine;
            stringBottomLine += iBL;
            stringBottomLine.append( uEffectiveWidth, iH );
            stringBottomLine += iBR;
            vectorLines.push_back( std::move( stringBottomLine ) );
         }
         else
         {
            // ### no border: padded lines with indent
            for( const auto& stringContent : vectorBoxValue )
            {
               std::string stringInnerLine( 1, ' ' );
               stringInnerLine += stringContent;
               vectorLines.push_back( std::move( stringInnerLine ) );
            }
         }

         vectorCardLines.push_back( std::move( vectorLines ) );
      }

      // ## concatenate lines across cards for this print row
      if( uNumCardsThisRow > 0 )
      {
         size_t uHeight = vectorCardLines[0].size();
         for( size_t uLine = 0; uLine < uHeight; uLine++ )
         {
            std::string thisPrintLine;
            for( size_t uCard = 0; uCard < uNumCardsThisRow; uCard++ )
            {
               if( uCard > 0 ) { thisPrintLine += std::string( uSeparation, ' ' ); }
               thisPrintLine += vectorCardLines[uCard][uLine];
            }
            stringResult += thisPrintLine;
            stringResult += '\n';
         }

         if( uRowSpace == 1 ) stringResult += '\n';                           // blank line after each print row
         else
         {
            for( unsigned uSpace = 0; uSpace < uRowSpace; uSpace++ ) { stringResult += '\n'; }  
         }
      }
   }

   stringOut += stringResult;
}

/** ---------------------------------------------------------------------------
 * @brief Print table data as cards without callback
 * @param table The table to print.
 * @param uBegin The starting row index.
 * @param uCount The number of rows to print.
 * @param vectorColumn The indices of columns to include in each card.
 * @param uBoxWidth The content width for each box.
 * @param uTotalWidth The total available width for arranging cards horizontally.
 * @param argumentOption Options for printing.
 * @param stringOut The output string to append the formatted cards to.
 * @param tag_card Tag to distinguish this overload for card printing.
 */
template <typename TABLE>
void to_string_s( const TABLE& table, uint64_t uBegin, uint64_t uCount, const std::vector<unsigned>& vectorColumn, unsigned uBoxWidth, unsigned uTotalWidth, const gd::argument::arguments& argumentOption, std::string& stringOut, gd::types::tag_card )
{
   to_string_s(table, uBegin, uCount, vectorColumn, uBoxWidth, uTotalWidth, nullptr, argumentOption, stringOut, gd::types::tag_card{});
}

/// @brief Print table data as cards/boxes arranged horizontally, with optional bordering and customizable characters.
void to_string( const gd::table::dto::table& table_, uint64_t uBegin, uint64_t uCount, const std::vector<unsigned>& vectorColumn, unsigned uBoxWidth, unsigned uTotalWidth, const gd::argument::arguments& argumentOption, std::string& stringOut, gd::types::tag_card )
{
   to_string_s(table_, uBegin, uCount, vectorColumn, uBoxWidth, uTotalWidth, argumentOption, stringOut, gd::types::tag_card{});
}


/* ---------------------------------------------------------------------------
 * @brief Print table data as cards/boxes arranged horizontally, with optional bordering and customizable characters.
 * 
 * Each row is printed as a vertical card containing the values from the specified columns,
 * stacked top-down. Cards are arranged in horizontal rows based on the total width.
 * The box content width is computed as the maximum value length across selected columns.
 * The full box width (including borders or padding) is used to calculate the number of
 * cards per horizontal row.
 * 
 * @param table_ The table to print.
 * @param uBegin The starting row index.
 * @param uCount The number of rows to print.
 * @param vectorColumn The indices of columns to include in each card.
 * @param uTotalWidth The total available width for arranging cards horizontally.
 * @param argumentOption Options for printing:
 *                       - "count": uint64_t, max rows to print.
 *                       - "border": bool, whether to draw borders (default: true).
 *                       - "tl", "tr", "bl", "br": string (length 1), border corner characters (default: "+").
 *                       - "horizontal": string (length 1), horizontal line character (default: "-").
 *                       - "vertical": string (length 1), vertical line character (default: "|").
 *                       - "row-space": unsigned, number of blank lines between rows (default: 1).
 *                       - "max-value-width": unsigned, truncate content width if exceeded.
 *                       - "prepend": string, text to prepend to output.
 * @param tag_card Tag to distinguish this overload for card printing.
 * @return Formatted string representing the table as cards.
 */
std::string to_string(const gd::table::dto::table& table_, uint64_t uBegin, uint64_t uCount, const std::vector<unsigned>& vectorColumn, unsigned uTotalWidth, const gd::argument::arguments& argumentOption, gd::types::tag_card)
{
   std::string stringOut;
   std::vector<unsigned> vectorWidth;
   gd::table::aggregate aggregateWidth( &table_ );
   aggregateWidth.max(vectorWidth, uBegin, uCount, vectorColumn, tag_length{});

   // ## Find out max box width across selected columns ......................
   unsigned uBoxWidth = 0;
   for(const auto& uWidth : vectorWidth) { uBoxWidth = std::max(uBoxWidth, uWidth); }

   unsigned uBorder = 2;
   if( argumentOption.exists("border") == true ) { bool bBorder = argumentOption["border"].is_true() ? 2 : 0; }
   to_string_s( table_, uBegin, uCount, vectorColumn, uBoxWidth, uTotalWidth, argumentOption, stringOut, gd::types::tag_card{} );
   return stringOut;
}

} // namespace format

_GD_TABLE_END