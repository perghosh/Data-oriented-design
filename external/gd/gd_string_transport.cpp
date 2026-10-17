#include "gd_string_transport.h"

_GD_BEGIN

/// helper function to handle common construction logic for copy and move operations.
void string_transport::common_construct(const string_transport& o)
{
   release();                                                                  // release any existing resources and clear the state of the current string_transport object

   if (o.m_piText == nullptr) { return; }

   const auto uLength = o.length();
   allocate(uLength);
   if (uLength > 0) { std::memcpy(m_piText, o.m_piText, uLength); }
   m_piText[uLength] = '\0';
   set_length(uLength);
}

/// helper function to handle common construction logic for move operations. --
void string_transport::common_construct(string_transport&& o) noexcept
{
   release();

   m_uStateAndSize = o.m_uStateAndSize;
   m_piText = o.m_piText;

   o.m_uStateAndSize = 0;
   o.m_piText = nullptr;
}

void string_transport::append(const char* piText, const std::size_t uLength)
{
   if( uLength == 0 ) { return; }

   // ## allocate new memory for the combined string ..........................
   auto uOldLlength = length();
   auto uNewLength = uOldLlength + uLength;
   allocate(uNewLength);                                                       // allocate
   std::memcpy(m_piText + uOldLlength, piText, static_cast<size_t>(uLength));
   m_piText[uNewLength] = '\0';
   set_length(uNewLength);
}


void string_transport::allocate(std::size_t uSize)
{                                                                                                  assert(uSize >= length());
   // ## Check if borrowed and if the new size is less than the current size, then return without allocating new memory
   auto uCurrentSize = size();                                                 // get the current size of the buffer, simpler to debug
   if( (is_borrowed() == true) && (uSize < uCurrentSize) ) { return; }

   char* piText_ = new char[uSize + 1];                                        // allocate new memory for the string_transport buffer, +1 for null terminator
   if( m_piText != nullptr )
   {
      std::memcpy(piText_, m_piText, static_cast<size_t>(uCurrentSize + 1));
   }
   piText_[uSize] = '\0';                                                      // null terminate the new buffer

   if (is_owned()) { delete[] m_piText; }
   m_piText = piText_;
   m_uStateAndSize = static_cast<uint64_t>(uSize);
}

_GD_END