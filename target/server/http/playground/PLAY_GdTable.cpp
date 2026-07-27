#include <array>
#include <filesystem>

#include "gd/gd_binary.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_simd.h"
#include "gd/gd_table_arguments.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"
#include "gd/gd_parse.h"
#include "gd/gd_uuid.h"

//#include "gd/gd_sql_query.h"
//#include "gd/gd_sql_query_builder.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

/*

TEST_CASE("[gd-table] create", "[gd-table]")
{
   {
      using namespace gd::table::simd;
      table<8u, 8u> tableFiles(1);
      tableFiles.column_prepare();
      tableFiles.column_add({ { "uint64", 0, "count" }, { "uint64", 0, "size" } }, gd::table::tag_type_name{});
      tableFiles.prepare();
      tableFiles.pack_broadcast_value(0, 0, uint64_t(100));

      std::array<uint64_t, 8> arrayValues;
      tableFiles.pack_harvest<uint64_t>(0u, 0, arrayValues);

      // sum array values
      uint64_t uTotal = 0;
      for(auto u : arrayValues) { uTotal += u; }
      std::cout << "total: " << uTotal << std::endl;

      uint64_t puValues[8];
      tableFiles.pack_harvest<uint64_t>(0u, 0, puValues);
   }
}

TEST_CASE("[gd-table] count characters", "[gd-table]")
{
   {
      using namespace gd::table::simd;
      table_8_8 tableCharacters(100u, gd::table::tag_repare_to_add_column{});  // reserve 100 rows

      tableCharacters.column_add( "uint64", 0, "text" );
      tableCharacters.prepare();

      // generate text with 64 characters
      std::string_view stringData = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      std::array<uint8_t, 64> arrayText{};
      arrayText.fill(0);
      std::size_t uLength = std::min(stringData.size(), arrayText.size());
      std::memcpy(arrayText.data(), stringData.data(), uLength);

      tableCharacters.row_add_pack(3u);                                       // add one row pack (3 * 8 rows)
      bool bSuccess = tableCharacters.size() == 24u;                          // 3 * 8 rows

      bSuccess = tableCharacters.get_row_pack_count() == 3u;                  // 3 row packs

      tableCharacters.pack_plant<uint8_t>(0u, 0, arrayText);
      tableCharacters.pack_plant<uint8_t>(1u, 0, arrayText);
      tableCharacters.pack_plant<uint8_t>(2u, 0, arrayText);


      uint64_t uFindBitMask = tableCharacters.pack_find_value<uint8_t>(0u, 0, uint8_t('s'));
      auto uRemaining = uFindBitMask;
      while(uRemaining) {
         unsigned uPosition = std::countr_zero(uRemaining);
         std::cout << "position: " << uPosition << "\n";
         uRemaining &= (uRemaining - 1);
      }

      // ## Count characters in all row packs
      unsigned uCount = 0;
      for(std::size_t uRowPack = 0; uRowPack < tableCharacters.get_row_pack_count(); ++uRowPack) {
         uint64_t uFindBitMask = tableCharacters.pack_find_value<uint8_t>(uRowPack, 0, uint8_t('s'));
         auto uRemaining = uFindBitMask;
         while(uRemaining) {
            ++uCount;
            unsigned uPosition = std::countr_zero(uRemaining);
            uRemaining &= (uRemaining - 1);
         }
      }

      std::cout << "total count of 's': " << uCount << std::endl;
   }
}

*/

TEST_CASE("[gd-table] strip comments from code", "[gd-table]")
{
   using namespace gd::table::simd;

   {
      table_8_8 tableCode(100u, gd::table::tag_repare_to_add_column{});
      tableCode.column_add("uint64", 0, "code");
      tableCode.prepare();

      // Sample Lua-like source code
      std::string_view stringCode = "-- This is a comment\nlocal x = 5\n-- Another comment\nprint(x)\n-- ready";
      std::string stringCodeCleaned;
      stringCodeCleaned.reserve(stringCode.size());

      // ## Fill table with source bytes, pad with 0x00
      std::span<const char> span_(stringCode.data(), stringCode.size());
      tableCode.pack_plant_span<char>(span_, 0, '\0');

      // ## Iterate over each row pack to find comments starting with '--' and ending with newline
      for(uint64_t uPack = 0; uPack < tableCode.get_row_pack_count(); ++uPack) {
         auto spanPack = tableCode.pack_harvest_span<char>(uPack, 0); // span will be needed to add or check for comments

         // ### Find '--' sequences (0x2D 0x2D)

         uint64_t uMask = tableCode.pack_find_value<char>(uPack, 0, '-');
         if(uMask == 0)                                                       // No comments found in this pack, copy entire span to cleaned code
         {
            stringCodeCleaned.append(spanPack.data(), spanPack.size());
            continue;
         }

         // ### Comment character for comment found, now check if next character is also '-' to confirm comment start

         // check if position of '-' is the last character in the span, if so, extend spand to next pack to check for comment start
         if(uMask == 0x01 && uPack < tableCode.get_row_pack_count())
         {
            spanPack = std::span<char>(spanPack.data(), spanPack.size() + 1); // extend span to include next character from next pack
         }

         // Process mask to identify comment positions
         unsigned  uPositionSave = 0;
         while(uMask) {
            unsigned uPosition = static_cast<unsigned>(std::countr_zero(uMask));
            uMask &= (uMask - 1);

            if(uPosition < uPositionSave) continue;                           // Already in comment zone ?

            // Copy characters from last position to current position (excluding comment start)
            stringCodeCleaned.append(spanPack.data() + uPositionSave, uPosition - uPositionSave);
            uPositionSave = uPosition + 1;

            bool bIsComment = false;
            // Check if next byte is also '-' (check within span bounds)
            if(uPosition + 1 < spanPack.size() && spanPack[uPosition + 1] == '-') { bIsComment = true; }
            else

            if(bIsComment == true)
            {
               //## Found comment start at uPosition, scan to end of comment within the current pack
               auto iCommentEnd = gd::buffer_find_g((const uint8_t*)spanPack.data(), spanPack.size(), '\n', gd::types::tag_size8{}, uPosition);
               if(iCommentEnd != -1) { uPositionSave = static_cast<unsigned>(iCommentEnd + 1); } // Move position to after the newline
               else { uPositionSave = static_cast<unsigned>(spanPack.size()); }// Move position to end of span, comment continues in next pack
            }
            else
            {
               stringCodeCleaned.push_back(spanPack[uPosition]);              // Not a comment start, copy character to cleaned code
               uPositionSave = uPosition + 1;                                 // Move position to next character after found '-' character
            }
         }
      }
   }
}

/*
TEST_CASE("[gd-table] strip comments from code", "[gd-table]")
{
   using namespace gd::table::simd;
   {
      table_8_8 tableCode(100u, gd::table::tag_repare_to_add_column{});  // reserve 100 rows
      tableCode.column_add("uint64", 0, "code");

      // ## Add sample code to table using std::array to transfer data to table

      // ## find all row comments, each row comment starts with "--", lua style comment, and ends with newline
   }
}




TEST_CASE("[gd-table] simd create simple", "[gd-table]")
{
   using namespace gd::table::simd;
   table<8u, 8u> tableFiles(8);
   tableFiles.column_prepare();

   tableFiles.column_add({ { "uint64", 0, "count" }, { "uint64", 0, "size" } }, gd::table::tag_type_name{});
   tableFiles.prepare();

   tableFiles.row_add(16);

   // ## set 16 values on each row
   for(unsigned uRowIndex = 0; uRowIndex < 16; ++uRowIndex)
   {
      tableFiles.cell_set(uRowIndex, 0, uint64_t( uRowIndex ));
      tableFiles.cell_set(uRowIndex, 1, uint64_t( uRowIndex * 10 ));
   }

   for(unsigned uRowIndex = 0; uRowIndex < 16; ++uRowIndex)
   {
      auto uRow = tableFiles.row_add_one();
      tableFiles.cell_set(uRow, 0, uint64_t(uRowIndex));
      tableFiles.cell_set(uRow, 1, uint64_t(uRowIndex * 10));
   }
   
   for(unsigned uRowIndex = 0; uRowIndex < 16; ++uRowIndex)
   {
      auto uRow = tableFiles.row_add_one();
      tableFiles[uRow, 0]= uint64_t(uRowIndex);
      tableFiles[uRow, 1]= uint64_t(uRowIndex * 10);
   }

   for(unsigned uRowIndex = 0; uRowIndex < 16000; ++uRowIndex)
   {
      auto uRow = tableFiles.row_add_one();
      tableFiles[uRow, 0] = uint64_t(uRowIndex);
      tableFiles[uRow, 1] = uint64_t(uRowIndex * 10);
   }
}

TEST_CASE("[gd-table] simd create simple 32 bit", "[gd-table]")
{
   using namespace gd::table::simd;
   table<4u, 4u> tableFiles(4);
   tableFiles.column_prepare();

   tableFiles.column_add("uint32", 0, "count");
   tableFiles.column_add("uint32", 0, "size");

   tableFiles.prepare();

   tableFiles.row_add(8);

   // ## set 8 values on each row
   for(unsigned uRowIndex = 0; uRowIndex < 8; ++uRowIndex)
   {
      tableFiles.cell_set(uRowIndex, 0, uint32_t(uRowIndex));
      tableFiles.cell_set(uRowIndex, 1, uint32_t(uRowIndex * 10));
   }

   for(unsigned uRowIndex = 0; uRowIndex < 8; ++uRowIndex)
   {
      auto uRow = tableFiles.row_add_one();
      tableFiles.cell_set(uRow, 0, uint32_t(uRowIndex));
      tableFiles.cell_set(uRow, 1, uint32_t(uRowIndex * 10));
   }

   for(unsigned uRowIndex = 0; uRowIndex < 8; ++uRowIndex)
   {
      auto uRow = tableFiles.row_add_one();
      tableFiles[uRow, 0] = uint32_t(uRowIndex);
      tableFiles[uRow, 1] = uint32_t(uRowIndex * 10);
   }

   for(unsigned uRowIndex = 0; uRowIndex < 8000; ++uRowIndex)
   {
      auto uRow = tableFiles.row_add_one();
      tableFiles[uRow, 0] = uint32_t(uRowIndex);
      tableFiles[uRow, 1] = uint32_t(uRowIndex * 10);
   }
}

*/
