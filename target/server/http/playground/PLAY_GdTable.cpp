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

std::string GetCode(unsigned uIndex)
{
   switch(uIndex)
   {
      // Case 0: Simple single-line comment at start
   case 0:
      return "-- This is a single line comment at the start\nlocal x = 5\nprint(x)\n-- Final comment";

      // Case 1: Multiple comments scattered throughout
   case 1:
      return "local a = 1\n-- First comment\nlocal b = 2\n-- Second comment\nlocal c = a + b\n-- Third comment\nprint(c)\n-- Fourth";

      // Case 2: Long multiline comment (tests pack boundaries)
   case 2:
      return "-- This is a very long comment that will definitely span across pack boundaries in the table because we need to test the cross-pack handling logic properly here\nlocal y = 10\nprint(y)";

      // Case 3: Code with no comments (control test)
   case 3:
      return "local val1 = 100\nlocal val2 = 200\nlocal sum = val1 + val2\nlocal diff = val2 - val1\nlocal prod = val1 * val2\nlocal quot = val2 / val1\nprint(sum, diff, prod, quot)";

      // Case 4: Comments only (edge case)
   case 4:
      return "-- Comment one\n-- Comment two\n-- Comment three\n-- Comment four\n-- Comment five\n-- All comments no code";

      // Case 5: Cross-pack boundary comment (critical test)
      // Pack 0 ends with '-', Pack 1 starts with '-' to form "--"
   case 5:
      return "local x = 1\nlocal y = 2\nlocal z = 3\nlocal w = 4\nlocal v = 5\nlocal u = 6\nlocal t = 7\n-- ";

      // Case 6: Complex nested structure with mixed comments
   case 6:
      return "local config = {\n  name = \"test\",\n  -- debug mode enabled\n  enabled = true,\n  -- version tracking\n  version = 1,\n  settings = {\n    -- timeout in seconds\n    timeout = 30,\n    retries = 3\n  }\n}\n-- End of configuration\nreturn config";

      // Case 7: Very long code with many comments (stress test)
   case 7:
      return "-- Module initialization\nlocal module = {}\n-- Constants section\nmodule.MAX_VALUE = 1000\nmodule.MIN_VALUE = 0\nmodule.DEFAULT = 500\n-- Function definitions\nfunction module.process(data)\n  -- Validate input first\n  if not data then return nil end\n  -- Process the data\n  local result = data * module.MAX_VALUE\n  -- Apply constraints\n  if result > module.MAX_VALUE then\n    result = module.MAX_VALUE\n  end\n  return result\nend\n-- Export module\nreturn module";

      // Case 8: Real-world Lua pattern (medium length)
   case 8:
      return "local Class = require(\"Class\")\n-- Player class extends base Class\nlocal Player = Class:extends()\n-- Constructor with default values\nfunction Player:init(name, health)\n  self.name = name\n  self.health = health or 100\n  self.maxHealth = 100\nend\n-- Take damage method\nfunction Player:takeDamage(amount)\n  -- Clamp damage to prevent negative health\n  self.health = math.max(0, self.health - amount)\n  -- Check for death\n  if self.health == 0 then\n    self:onDeath()\n  end\nend\n-- Return player instance\nreturn Player";

      // Case 9: Dense code with minimal spacing
   case 9:
      return "local t={a=1,b=2,c=3}\n-- table comment\nfor k,v in pairs(t)do print(k,v)end\n-- end loop\nlocal sum=0\nfor i=1,#t do sum=sum+t[i]end\nprint(sum)";

      // Case 10: Extremely long (400+ chars, tests 6+ packs)
   case 10:
      return "-- Configuration loader module with extensive documentation and multiple comment blocks\nlocal ConfigLoader = {}\n\n-- Private helper functions section\nlocal function parseFile(filename)\n  -- Open file for reading with error handling\n  local file = io.open(filename, \"r\")\n  if not file then return nil, \"Could not open file\" end\n  -- Read all content into memory\n  local content = file:read(\"*all\")\n  file:close()\n  return content\nend\n\nlocal function validateConfig(config)\n  -- Check required fields exist\n  if not config.name then return false, \"Missing name field\" end\n  if not config.version then return false, \"Missing version field\" end\n  return true, nil\nend\n\n-- Public API functions\nfunction ConfigLoader.load(filepath)\n  -- Parse the file contents\n  local rawContent, err = parseFile(filepath)\n  if err then return nil, err end\n  -- TODO: Actually parse the config\n  return {name=\"default\", version=1}, nil\nend\n\nreturn ConfigLoader";

      // Case 11: Cross-pack at exact boundary (64th byte is '-')
   case 11:
      return "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA-";  //64 'A' + '-' at position 63, next char needs to be '-'

         // Case 12: Mixed comment styles and edge cases
   case 12:
      return "local x = 1 -- inline comment\nlocal y = 2\n--- triple dash comment\nlocal z = 3\n--\n-- double empty line comment\n--\nprint(x,y,z)";

      // Case 13: Unicode/UTF-8 in strings (non-comment characters)
   case 13:
      return "local greeting = \"Hello World!\"\nlocal unicode = \"Üñíçödé Tëst\"  -- test unicode support\nlocal emoji = \"👨‍💻\"  -- emoji in comment\nprint(greeting, unicode, emoji)";

      // Case 14: Deep nesting with comments at each level
   case 14:
      return "local level1 = {\n  -- Level 1 comment\n  level2 = {\n    -- Level 2 comment\n    level3 = {\n      -- Level 3 comment\n      data = \"deeply nested\",\n      -- More data\n      value = 42\n    }\n  }\n}\n-- End of nesting\nreturn level1";

      // Case 15: Large dataset generation (stress memory)
   case 15:
      return "-- Array initialization\nlocal array = {}--comment\nfor i = 1, 10 do--loop comment\n  array[i] = i * 2--calculation\n  -- print(array[i])\nend\n-- Processing\nlocal total = 0\nfor _,v in ipairs(array)do total = total + v end--sum calculation\nprint(total)--output result\n-- Done";
   }

   return "";
}

TEST_CASE("[gd-table] iterate with position to remove comments", "[gd-table]")
{
   using namespace gd::table::simd;

   {
      table_8_8 tableCode(100u, gd::table::tag_repare_to_add_column{});
      tableCode.column_add("uint64", 0, "code");
      tableCode.prepare();

      // Sample Lua-like source code

      for(unsigned uIndex = 0; uIndex < 16; ++uIndex)
      {
         tableCode.row_clear();
         std::string stringCodeCore = GetCode(uIndex);
         std::string_view stringCode = stringCodeCore;
         std::string stringCodeCleaned;
         stringCodeCleaned.reserve(stringCode.size());

         // ## Fill table with source bytes, pad with 0x00
         std::span<const char> span_(stringCode.data(), stringCode.size());
         tableCode.pack_plant_span<char>(span_, 0, '\0');

         // ## `position` walks row/pack boundaries internally (via get_uint8), so the scan below
         //    never has to know a pack even exists - that's the whole point of the type.
         table_8_8::position positionEnd;
         positionEnd.advance((unsigned)stringCode.size());

         // ### Pass 1: find every "-- ... \n" comment as a [begin, end) range
         std::vector<table_8_8::range> vectorComment;
         {
            table_8_8::position position_;
            while(position_ < positionEnd)
            {
               uint8_t uByte = tableCode.get_uint8(position_);

               table_8_8::position positionNext = position_;
               positionNext.advance(1);

               if(uByte == '-' && positionNext < positionEnd && tableCode.get_uint8(positionNext) == '-')
               {
                  table_8_8::position positionStart = position_;
                  table_8_8::position positionScan = position_;
                  positionScan.advance(2);                                     // skip past the opening "--"

                  while(positionScan < positionEnd && tableCode.get_uint8(positionScan) != '\n') { positionScan.advance(1); }
                  if(positionScan < positionEnd) { positionScan.advance(1); }   // include the trailing newline in the stripped range

                  vectorComment.push_back(table_8_8::range{ positionStart, positionScan });
                  position_ = positionScan;
                  continue;
               }

               position_.advance(1);
            }
         }

         // ### Pass 2: copy everything that isn't covered by a comment range
         {
            table_8_8::position position_;
            auto itComment = vectorComment.begin();
            while(position_ < positionEnd)
            {
               if(itComment != vectorComment.end() && position_ == itComment->first)
               {
                  position_ = itComment->second;                               // skip the whole comment in one jump
                  ++itComment;
                  continue;
               }

               stringCodeCleaned.push_back((char)tableCode.get_uint8(position_));
               position_.advance(1);
            }
         }

         std::cout << "-------------------------------------------------\n";
         std::cout << "\n--- Test Case " << uIndex << " ---\n";
         std::cout << "Original Length: " << stringCode.size() << " bytes\n";
         std::cout << "Comment Ranges Found: " << vectorComment.size() << "\n";
         std::cout << "Original Code:\n" << stringCode << "\n";
         std::cout << "\nCleaned Code:\n" << stringCodeCleaned << "\n";
         std::cout << "Cleaned Length: " << stringCodeCleaned.size() << " bytes\n";
      }
   }
}

TEST_CASE("[gd-table] iterate with position to remove comments, hybrid", "[gd-table]")
{
   using namespace gd::table::simd;

   for(unsigned uIndex = 0; uIndex < 16; ++uIndex)
   {
      table_8_8 tableCode(100u, gd::table::tag_repare_to_add_column{});
      tableCode.column_add("uint64", 0, "code");
      tableCode.prepare();

      std::string stringCodeCore = GetCode(uIndex);
      std::string_view stringCode = stringCodeCore;

      tableCode.row_clear();
      std::span<const char> span_(stringCode.data(), stringCode.size());
      tableCode.pack_plant_span<char>(span_, 0, '\0');

      // ## HYBRID VERSION - fast happy path, simple sad path
      std::string stringCodeCleaned;
      stringCodeCleaned.reserve(stringCode.size());

      bool bInComment = false;
      uint64_t uTotalBytes = stringCode.size();
      table_8_8::position positionEnd;
      positionEnd.advance((unsigned)uTotalBytes);

      constexpr unsigned uPackBytes = table_8_8::size_pack_s();                 // PACKCOUNT * VALUESIZE = 64 for table_8_8

      // ## `pack_plant_span` only ever pads the LAST pack it writes - every pack before that is
      //    guaranteed to be 100% real data. So the main loop below never needs to ask "am I near
      //    the end", "how many bytes are valid here", or mask anything - that question is answered
      //    once, up front, and the trailing partial pack is handled on its own after the loop.
      uint64_t uFullPackCount = uTotalBytes / uPackBytes;
      unsigned uTailByteCount = (unsigned)(uTotalBytes % uPackBytes);

      // ## SAD PATH: deliberately simple, same shape as the plain/non-hybrid test - a single byte
      //    at a time via `position`, which absorbs pack-boundary crossing for free. This only runs
      //    on the (rare) packs where the fast check below found something worth disambiguating,
      //    plus the trailing partial pack.
      auto ScanSimple = [&](table_8_8::position position_, table_8_8::position positionRangeEnd)
         {
            while(position_ < positionRangeEnd)
            {
               uint8_t uByte = tableCode.get_uint8(position_);

               if(bInComment)
               {
                  position_.advance(1);
                  if(uByte == '\n') { bInComment = false; }
                  continue;
               }

               if(uByte == '-')
               {
                  table_8_8::position positionNext = position_;
                  positionNext.advance(1);
                  if(positionNext < positionEnd && tableCode.get_uint8(positionNext) == '-')
                  {
                     bInComment = true;
                     position_ = positionNext;
                     position_.advance(1);                                        // skip the "--"
                     continue;
                  }
               }

               stringCodeCleaned.push_back((char)uByte);
               position_.advance(1);
            }
         };

      // ## FAST PATH: one mask over the whole (guaranteed-full) pack decides everything.
      for(uint64_t uPack = 0; uPack < uFullPackCount; ++uPack)
      {
         if(bInComment)
         {
            uint64_t uMaskNewline = tableCode.pack_find_value<char>(uPack, 0, '\n');
            if(uMaskNewline == 0) { continue; }                                // whole pack still inside comment, nothing to copy
         }
         else
         {
            uint64_t uMaskDash = tableCode.pack_find_value<char>(uPack, 0, '-');
            if(uMaskDash == 0)                                                 // no '-' anywhere in this pack
            {
               auto spanPack = tableCode.pack_harvest_span<char>(uPack, 0);
               stringCodeCleaned.append(spanPack.data(), spanPack.size());     // bulk copy, no per-byte scanning at all
               continue;
            }
         }

         // something needs disambiguating in this pack (comment ends here, or a real '-' shows up)
         table_8_8::position positionPackStart;
         positionPackStart.m_uRow = uPack * table_8_8::count_pack_s();
         table_8_8::position positionPackEnd = positionPackStart;
         positionPackEnd.advance(uPackBytes);

         ScanSimple(positionPackStart, positionPackEnd);
      }

      // ## TAIL: the one possibly-partial pack, if any. Rare (at most once per string) and small
      //    (< uPackBytes), so it always goes through the simple scan - no fast-path check needed here.
      if(uTailByteCount > 0)
      {
         table_8_8::position positionTailStart;
         positionTailStart.m_uRow = uFullPackCount * table_8_8::count_pack_s();
         table_8_8::position positionTailEnd = positionTailStart;
         positionTailEnd.advance(uTailByteCount);

         ScanSimple(positionTailStart, positionTailEnd);
      }

      // ## Verification
      std::cout << "-------------------------------------------------\n";
      std::cout << "\n--- Test Case " << uIndex << " (HYBRID) ---\n";
      std::cout << "Original Length: " << stringCode.size() << " bytes\n";
      std::cout << "Cleaned Length: " << stringCodeCleaned.size() << " bytes\n";
      std::cout << "Cleaned Code:\n[" << stringCodeCleaned << "]\n";
   }
}
/*

TEST_CASE("[gd-table] strip comments from code", "[gd-table]")
{
   enum CodeState { eCode, eComment };
   using namespace gd::table::simd;

   {
      CodeState eCodeState = eCode;
      table_8_8 tableCode(100u, gd::table::tag_repare_to_add_column{});
      tableCode.column_add("uint64", 0, "code");
      tableCode.prepare();

      // Sample Lua-like source code

      for(unsigned uIndex = 0; uIndex < 16; ++uIndex)
      {
         tableCode.row_clear();
         std::string stringCodeCore = GetCode(uIndex);
         std::string_view stringCode = stringCodeCore;
         std::string stringCodeCleaned;
         stringCodeCleaned.reserve(stringCode.size());
         bool bIsComment = false;

         // ## Fill table with source bytes, pad with 0x00
         std::span<const char> span_(stringCode.data(), stringCode.size());
         tableCode.pack_plant_span<char>(span_, 0, '\0');

         // ## Iterate over each row pack to find comments starting with '--' and ending with newline
         for(uint64_t uPack = 0; uPack < tableCode.get_row_pack_count(); ++uPack) {
            auto spanPack = tableCode.pack_harvest_span<char>(uPack, 0); // span will be needed to add or check for comments

            // ### Find '--' sequences (0x2D 0x2D)
            unsigned  uPositionSave = 0;
            uint64_t uMask = tableCode.pack_find_value<char>(uPack, 0, '-');
            if(bIsComment == false)
            {
               if(uMask == 0)                                                  // No comments found in this pack, copy entire span to cleaned code
               {
                  stringCodeCleaned.append(spanPack.data(), spanPack.size());
                  continue;
               }
            }
            else
            {
               auto iCommentEnd = gd::buffer_find_g((const uint8_t*)spanPack.data(), spanPack.size(), '\n', gd::types::tag_size8{});
               if(iCommentEnd != -1)
               {
                  uPositionSave = static_cast<unsigned>(iCommentEnd + 1);   // Move position to after the newline
                  bIsComment = false;
               }
               else { continue; }                                             // Entire pack is comment, skip to next pack
            }

            // ### Comment character for comment found, now check if next character is also '-' to confirm comment start

            // check if position of '-' is the last character in the span, if so, extend spand to next pack to check for comment start
            if(uMask == 0x01 && uPack < tableCode.get_row_pack_count())
            {
               spanPack = std::span<char>(spanPack.data(), spanPack.size() + 1); // extend span to include next character from next pack
            }

            // Process mask to identify comment positions
            while(uMask) {
               unsigned uPosition = static_cast<unsigned>(std::countr_zero(uMask));
               uMask &= (uMask - 1);

               if(uPosition < uPositionSave) continue;                         // Already in (comment) zone ?

               stringCodeCleaned.append(spanPack.data() + uPositionSave, uPosition - uPositionSave); // Copy characters from last position to current position (excluding comment start)
               uPositionSave = uPosition + 1;

               if(uPosition + 1 < spanPack.size() && spanPack[uPosition + 1] == '-') // Check if next byte is also '-' (check within span bounds)
               {

                  bIsComment = true;
                  uMask &= (uMask - 1);
               }

               if(bIsComment == true)
               {
                  //## Found comment start at uPosition, scan to end of comment within the current pack
                  auto iCommentEnd = gd::buffer_find_g((const uint8_t*)spanPack.data(), spanPack.size(), '\n', gd::types::tag_size8{}, uPosition);
                  if(iCommentEnd != -1) 
                  { 
                     uPositionSave = static_cast<unsigned>(iCommentEnd + 1);   // Move position to after the newline
                  }
                  else { uPositionSave = static_cast<unsigned>(spanPack.size()); }// Move position to end of span, comment continues in next pack
               }
               else
               {
                  stringCodeCleaned.push_back(spanPack[uPosition]);              // Not a comment start, copy character to cleaned code
                  uPositionSave = uPosition + 1;                                 // Move position to next character after found '-' character
               }
            }
         }

         std::cout << "-------------------------------------------------\n";
         std::cout << "\n--- Test Case " << uIndex << " ---\n";
         std::cout << "Original Length: " << stringCode.size() << " bytes\n";
         std::cout << "Pack Count: " << tableCode.get_row_pack_count() << "\n";
         std::cout << "Original Code:\n" << stringCode << "\n";
         std::cout << "\nCleaned Code:\n" << stringCodeCleaned << "\n";
         std::cout << "Cleaned Length: " << stringCodeCleaned.size() << " bytes\n";
         std::cout << "Comment Count: " << std::count(stringCodeCleaned.begin(), stringCodeCleaned.end(), '-') << " dashes remain\n";
      }
   }
}
*/

