#include <filesystem>

#include "gd/gd_variant_view.h"
#include "gd/gd_table_aggregate.h"

#include "gd/math/gd_math_string.h"

#include "gd/expression/gd_expression_value.h"
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_runtime.h"

#include "../../Application.h"

#include "Expression.h"

NAMESPACE_AUTOMATION_BEGIN

using namespace gd::expression;


static std::pair<bool, std::string> CountLines_s( runtime* pruntime, const std::vector<value>& vectorArgument )
{                                                                                                  assert(vectorArgument.size() > 0);

   return { true, "" };
}

static std::pair<bool, std::string> SelectAll_s( runtime* pruntime, const std::vector<value>& vectorArgument )
{                                                                                                  assert(vectorArgument.size() > 0);

   return { true, "" };
}


static std::pair<bool, std::string> SelectLines_s( runtime* pruntime, const std::vector<value>& vectorArgument )
{                                                                                                  assert(vectorArgument.size() > 0);

   return { true, "" };
}

/// sample: `select_between(source, start, end)`
static std::pair<bool, std::string> SelectBetween_s( runtime* pruntime, const std::vector<value>& vectorArgument )
{                                                                                                  assert(vectorArgument.size() > 2);
   auto source_ = vectorArgument[2];                                                               assert(source_.is_pointer() == true);

   ExpressionSource* psource = (ExpressionSource*)source_.get_pointer();                           assert( psource->file().empty() == false );
   auto result_ = psource->GotoLine();
   if( result_.first == false ) { return result_; } // error in goto line

   if( psource->source().empty() == false )
   {
      std::string from_ = vectorArgument[1].get_string();                                          assert(from_.empty() == false);
      std::string to_ = vectorArgument[0].get_string();                                            assert(from_.empty() == false);

      std::string stringResult = gd::math::string::select_between(psource->source(), from_, to_);  // select text between from_ and to_
   }

   return { true, "" };
}


// Array of MethodInfo for visual studio operations
const method pmethodSelect_g[] = {
   { (void*)&CountLines_s, "count_lines", 1, 0, method::eFlagRuntime },
   { (void*)&SelectBetween_s, "select_between", 3, 0, method::eFlagRuntime },
   { (void*)&SelectLines_s, "select_lines", 1, 0, method::eFlagRuntime },
};

const size_t uMethodSelectSize_g = sizeof(pmethodSelect_g) / sizeof(gd::expression::method);

// "args": [ "find", "--source", "target/TOOLS/FileCleaner", "-R", "--pattern", "@brief", "--segment", "comment", "--max", "30", "--context", "10", "-vs", "-verbose", "--rule", "\"select-between:@code,@endcode\"" ],

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

      auto uAvailable = m_line.available();
      m_ifstream.read((char*)m_line.buffer(), uAvailable);
      auto uReadSize = m_ifstream.gcount();                                   // get number of valid bytes read
      m_line.update(uReadSize);                                               // Update valid size in line buffer
   }

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

    
   // This function is a placeholder for opening a file.
   // The actual implementation would depend on the context and requirements.
   // For now, we return true indicating success and an empty string for the message.
   return { true, "" };
}

void ExpressionSource::CloseFile()
{
   if(m_ifstream.is_open()) { m_ifstream.close(); }
}


NAMESPACE_AUTOMATION_END