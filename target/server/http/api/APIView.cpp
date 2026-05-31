// @FILE [tag: api, view] [summary: API View renders pages, like server side rendering] [type: source] [name: APIView.cpp]

#include <array>
#include <cctype>
#include <fstream>
#include <filesystem>

#include "gd/gd_arguments.h"
#include "gd/gd_binary.h"
#include "gd/parse/gd_parse_window_line.h"
#include "gd/expression/gd_expression_parse_state.h"


#include "../Router.h"
#include "../Document.h"
#include "../Application.h"


#include "APIView.h"

std::pair<bool, std::string> CAPIView::Execute()
{

   std::pair<bool, std::string> result_(true, "");

   for(std::size_t uIndex = 0; uIndex < m_vectorCommand.size(); ++uIndex)
   {
      m_uCommandIndex = static_cast<unsigned>(uIndex);
      std::string_view stringCommand = m_vectorCommand[uIndex];

      if(stringCommand == "view") continue;

      if(stringCommand == "ssr")
      {
         std::string stringPage;
         result_ = Execute_RenderPage( stringPage );
      }
      else
      {
         return { false, "unknown SQL command: " + std::string(stringCommand) };
      }

      if(result_.first == false) { return result_; }
   }

   return { true, "" };
}

std::pair<bool, std::string> CAPIView::Execute_RenderPage( std::string& stringRendered )
{                                                                                                  assert(m_stringPath.empty() == false );
   enum enumLanguage { eLanguageNone = 0, eLanguageLua, eLanguageGD, eLanguageExpression };
   gd::parse::window::line lineBuffer(48 * 64, 64 * 64, gd::types::tag_create{});  // create line buffer 64 * 64 = 4096 bytes = 64 cache lines
   gd::expression::parse::state state_; // state is used to check what type of code part we are in
   const gd::expression::parse::state::rule* pruleActive = nullptr; // pointer to active rule, this is used to check what type of code part we are in and to get end marker for code part

   std::string stringPage;
   std::string stringCode; // string to hold code found in page, this will be used to render page

   stringPage.reserve(1024 * 8);                                              // reserve space for page
   stringCode.reserve(1024);                                                  // reserve space for code

   std::ifstream file_(m_stringPath.data(), std::ios::binary);
   if(file_.is_open() == false) return { false, "Failed to open file: " + std::string(m_stringPath) };

   auto uAvailable = lineBuffer.available();
   file_.read((char*)lineBuffer.buffer(), uAvailable);
   auto uReadSize = file_.gcount();                                           // get number of valid bytes read
   lineBuffer.update(uReadSize);                                              // Update valid size in line buffer

   state_.add(std::string_view("SCRIPTCODE"), "[[lua", "]]");
   state_.add(std::string_view("SCRIPTCODE"), "[[gd", "]]");
   state_.add(std::string_view("EXPRESSION"), "[[=", "]]");

   uint8_t uCharacter = 0;                                                    // character to hold current character


   // ## Scan file and read lines found in table
   while(lineBuffer.eof() == false)
   {
      auto [first_, last_] = lineBuffer.range(gd::types::tag_pair{});         // get range of valid data in buffer
      for(auto it = first_; it < last_; it++)
      {
         uCharacter = *it;                                                    // get current character

         if(state_.in_state() == false)                                       // not in a state? that means we are reading page code
         {
            // ## check if we have found state ...............................
            if(state_[uCharacter] != 0 && state_.exists(it) == true)
            {
               stringCode.clear();                                            // clear code string to start reading new code
               auto uLength = state_.activate(it);                            // activate state
               if(uLength > 1) it += (uLength - 1);                           // skip to end of state marker and if it is more than 1 character, skip to end of state
               pruleActive = state_.get_active_rule_pointer();                // get pointer to active rule, this will be used to check for end of state
               continue;                                                      // continue to next character, we will start reading code in next iteration
            }

            stringPage += uCharacter;                                         // add character to page string
         }
         else
         {
            // ## check if we have found end of state
            unsigned uLength;
            if(state_.deactivate(it, &uLength) == true)
            {
               if(uLength > 1) it += (uLength - 1);                           // skip to end of state marker and if it is more than 1 character, skip to end of state

               if(stringCode.empty() == false)
               {
                  assert(pruleActive != nullptr);
                  auto stringType = std::string_view(pruleActive->get_start()).substr(2);// get code type from start marker, this will be used to determine how to render code
                  //auto reult_ = Run(stringType, stringCode, &stringPage);      // run code and get result, this will be used to render page
               }
               pruleActive = nullptr;
               continue;
            }

            stringCode += uCharacter;                                         // add character to code string
         }
      }

      lineBuffer.rotate();                                                    // rotate buffer

      if(uReadSize > 0)                                                       // was it possible to read data last read, then more data is available
      {
         auto uAvailable = lineBuffer.available();                            // get available space in buffer to be filled
         file_.read((char*)lineBuffer.buffer(), lineBuffer.available());      // read more data into available space in buffer
         uReadSize = file_.gcount();
         lineBuffer.update(uReadSize);                                        // update valid size in line buffer
      }
   }

   m_stringSSRPage = std::move(stringPage);                                    // for now we just return the page

   return { true, "" };
}
