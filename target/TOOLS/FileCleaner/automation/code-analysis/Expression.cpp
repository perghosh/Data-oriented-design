#include <filesystem>

#include "gd/gd_variant_view.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_aggregate.h"

#include "gd/math/gd_math_string.h"

#include "gd/expression/gd_expression_value.h"
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_runtime.h"

#include "gd/expression/gd_expression_glue_to_gd.h"

#include "../../Application.h"

#include "Expression.h"

NAMESPACE_AUTOMATION_BEGIN

using namespace gd::expression;


static std::pair<bool, std::string> CountLines_s( const std::vector<value>& vectorArgument )
{                                                                                                  assert(vectorArgument.size() > 0);

   return { true, "" };
}


static std::pair<bool, std::string> GetArgument_s( const std::vector<value>& vectorArgument, value* pvalueReturn )
{                                                                                                  assert(vectorArgument.size() > 1);
   auto object_ = vectorArgument[1];                                                               assert(object_.is_pointer() == true);
   gd::argument::shared::arguments* parguments_ = (gd::argument::shared::arguments*)object_.get_pointer();

   auto& name_ = vectorArgument[0];
   if( name_.is_string() == true )
   {
      std::string stringName( name_.as_string() );
      if( stringName.empty() == true ) { return { false, "Argument name cannot be empty." }; }
      auto variantview_ = ( *parguments_ )[stringName].as_variant_view();
      *pvalueReturn = to_value_g( variantview_ );

      return { true, "" };
   }

   return { false, "Invalid argument name type, expected string." };
}

static std::pair<bool, std::string> SelectAll_s( const std::vector<value>& vectorArgument, value* pvalueReturn )
{                                                                                                  assert(vectorArgument.size() > 0);
   auto source_ = vectorArgument[0];                                                               assert(source_.is_pointer() == true);

   ExpressionSource* psource = (ExpressionSource*)source_.get_pointer();                           assert( psource->file().empty() == false );
   auto result_ = psource->GotoLine();
   if( result_.first == false ) { return result_; } // error in goto line

   if( psource->source().empty() == false )
   {
      *pvalueReturn = psource->source();
   }
   else
   {
      *pvalueReturn = value(); // return empty value if source is empty
   }

   return { true, "" };
}

static std::pair<bool, std::string> SelectLine_s( const std::vector<value>& vectorArgument, value* pvalueReturn )
{                                                                                                  assert(vectorArgument.size() > 1);
   auto source_ = vectorArgument[1];                                                               assert(source_.is_pointer() == true);

   ExpressionSource* psource = (ExpressionSource*)source_.get_pointer();                           assert( psource->file().empty() == false );
   auto result_ = psource->GotoLine();
   if( result_.first == false ) { return result_; } // error in goto line

   

   auto from_ = vectorArgument[0];
   if( from_.is_string() == true )
   {
      std::string stringFind( from_.as_string() );

      auto uPosition = psource->source().find( stringFind );
      if(uPosition != std::string::npos) 
      {
         uPosition += stringFind.length();
         auto uEndPosition = psource->source().find('\n', uPosition); // find end of line
         if( uEndPosition == std::string::npos ) uEndPosition = psource->source().length(); // if no end of line found, use the end of source code

         std::string stringLine = psource->source().substr(uPosition, uEndPosition - uPosition); // get the line of text
         *pvalueReturn = value(stringLine);
      } 
      else 
      {
         *pvalueReturn = value(std::string()); // if the string is not found, return empty string
      }
   }
   else
   {
      std::string stringLine = psource->GetGotoLineText();
      *pvalueReturn = value(stringLine); // assign stringLine to pvalueReturn
   }

   return { true, "" };
}



static std::pair<bool, std::string> SelectLines_s( const std::vector<value>& vectorArgument )
{                                                                                                  assert(vectorArgument.size() > 0);

   return { true, "" };
}

/// sample: `select_between(source, start, end)`
static std::pair<bool, std::string> SelectBetween_s( const std::vector<value>& vectorArgument, std::vector<value>* pvectorReturn )
{                                                                                                  assert(vectorArgument.size() > 2);
   auto source_ = vectorArgument[2];                                                               assert(source_.is_pointer() == true);

   ExpressionSource* psource = (ExpressionSource*)source_.get_pointer();                           assert( psource->file().empty() == false );
   auto result_ = psource->GotoLine();
   if( result_.first == false ) { return result_; } // error in goto line

   if( psource->source().empty() == false )
   {
      std::string from_ = vectorArgument[1].get_string();                                          assert(from_.empty() == false);
      std::string to_ = vectorArgument[0].get_string();                                            assert(from_.empty() == false);

      std::vector<std::string> vector_ = gd::math::string::select_between_all(psource->source(), from_, to_);  // select text between from_ and to_

      for(auto it = vector_.rbegin(); it != vector_.rend(); ++it) // iterate over results but do it backwards
      {
         pvectorReturn->push_back(value(*it)); // add result to the return vector
      }
   }

   return { true, "" };
}


// Array of MethodInfo for visual studio operations
const method pmethodSelect_g[] = {
   { (void*)&CountLines_s, "count_lines", 1, 0 },
   { (void*)&GetArgument_s, "get_argument", 2, 1 },
   { (void*)&SelectAll_s, "select_all", 1, 1 },
   { (void*)&SelectBetween_s, "select_between", 3, 2 },
   { (void*)&SelectLine_s, "select_line", 2, 1 },
   { (void*)&SelectLines_s, "select_lines", 3, 1 },
   //{ (void*)&SetArgument_s, "set_argument", 3, 0 },
};

const size_t uMethodSelectSize_g = sizeof(pmethodSelect_g) / sizeof(gd::expression::method);

std::pair<bool, std::string> ExpressionSource::GotoLine()
{
   std::string stringBuffer;
   std::string stringInStateBuffer;

   if( m_uCurrentLine == 0 || m_uCurrentLine > m_uGotoLine )
   {
      if( m_uCurrentLine > 0 )
      { 
         m_ifstream.clear();
         m_ifstream.seekg(0);
         Reset();
      }
   }

   auto uAvailable = m_line.available();                                                           assert(uAvailable > 0);
   m_ifstream.read((char*)m_line.buffer(), uAvailable);
   auto uReadSize = m_ifstream.gcount();                                       // get number of valid bytes read
   m_line.update(uReadSize);                                                   // Update valid size in line buffer

   while( m_line.eof() == false )
   {
      auto [first_, last_] = m_line.range(gd::types::tag_pair{});             // get range of valid data in buffer
      for( auto it = first_; it < last_; it++ ) 
      {
         if( m_state.in_state() == false )                                    // not in a state? that means we are reading source code
         {
            if( m_state[*it] != 0 && m_state.exists(it) == true )             // switch to state ?
            {
               unsigned uLength = (unsigned)m_state.activate(it);             // activate state
               if( uLength > 1 ) it += ( uLength - 1 );                       // skip to end of state marker and if it is more than 1 character, skip to end of state
               if( m_uCurrentLine >= m_uGotoLine )
               {
                  set_source( std::move(stringBuffer) );                      // set source code to the buffer
                  return { true, "" };                                        // return success
               }
               stringBuffer.clear();
            }
            else
            {
               stringBuffer.push_back(*it);                                   // add character to buffer
            }
         }
         else
         {
            // ## check if we have found end of state
            unsigned uLength;
            if( m_state.deactivate( it, &uLength ) == true ) 
            {
               // ## Special case: if we are at the goto line and the next character is a new line, 
               //    then we can return the buffer because we have found the end of state.
               //    This means that state ends with newline character and we are at the line after the goto line.
               if( ( m_uGotoLine - m_uCurrentLine ) == 1 && *it == '\n' )
               {
                  m_uCurrentLine++;
                  set_source( std::move(stringBuffer) );                      // set source code to the buffer
                  return { true, "" };                                        // return success
               }

               if( uLength > 1 ) it += (uLength - 1);                         // skip to end of state marker and if it is more than 1 character, skip to end of state
               if( m_uCurrentLine >= m_uGotoLine )
               {
                  set_source( std::move(stringBuffer) );                      // set source code to the buffer
                  return { true, "" };                                        // return success
               }

               // check for ending linebreak 
               stringBuffer.clear();
            }
            else
            {
               stringBuffer.push_back(*it);                                   // add character to buffer
            }
         }

         if( *it == '\n' ) m_uCurrentLine++;                                  // increment current line number if we have found a line break
      }

      m_line.rotate();                                                        // rotate buffer

      auto uAvailable = m_line.available();                                   // get available space in buffer to be filled
      m_ifstream.read((char*)m_line.buffer(), m_line.available());            // read more data into available space in buffer
      auto uSize = m_ifstream.gcount();
      m_line.update(uSize);                                                   // update valid size in line buffer
   }
  
   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Attempts to open the file specified by m_stringFile, preparing internal state and returning the result.
 * @return A std::pair where the first element is true if the file was opened successfully, or false if an error occurred; the second element is an error message if opening failed, or an empty string on success.
 */
std::pair<bool, std::string> ExpressionSource::OpenFile()
{                                                                                                  assert( m_stringFile.empty() == false ); assert( std::filesystem::exists(m_stringFile) == true );
// ## prepare state
   m_state.clear();
   auto result_ = CApplication::PrepareState_s( {{"source",m_stringFile}}, m_state);
   if( result_.first == false ) return result_;                                // error in state preparation

   // ## open file
   CloseFile();
   m_ifstream.open(m_stringFile);
   if( !m_ifstream.is_open() ) return { false, "Failed to open the file" };

   // ## initialize line window
   if( m_line.capacity() == 0 )
   {
      m_line.create(8096 - 1024, 8096);
   }
   else { m_line.reset(); } // reset line window if it was already created

    
   // This function is a placeholder for opening a file.
   // The actual implementation would depend on the context and requirements.
   // For now, we return true indicating success and an empty string for the message.
   return { true, "" };
}

void ExpressionSource::CloseFile()
{
   if(m_ifstream.is_open()) { m_ifstream.close(); }
}

std::string ExpressionSource::GetGotoLineText() const
{                                                                                                  assert( m_uCurrentLine >= m_uGotoLine);
   std::string stringLine;
   if( m_uCurrentLine == m_uGotoLine )
   {
      stringLine = m_stringSource; // return the whole source code if we are at the goto line
   }
   else
   {
      auto uLineCount = gd::math::string::count_character(m_stringSource, '\n'); 
      decltype( uLineCount ) uSelectLine = uLineCount - (m_uCurrentLine - m_uGotoLine);            assert(uSelectLine < 0x00F000000); // realistic? 
      uSelectLine--;                                                           // change to zero based
      stringLine = gd::math::string::select_line(m_stringSource, uSelectLine); // select line from source code
   }

   return stringLine;
}


NAMESPACE_AUTOMATION_END