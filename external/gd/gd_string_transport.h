#pragma once

/** @CLASS [name: string_transport] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class string_transport
{
   // @API [tag: construction]
public:
   string_transport() : m_uStateEndLength(0), m_piText(nullptr) {}

   string_transport( const char* piText ) : m_uStateEndLength(0) 
   {
      allocate(piText);
   }

   // copy
   string_transport(const string_transport& o) { common_construct(o); }
   string_transport(string_transport&& o) noexcept { common_construct(std::move(o)); }
   // assign
   string_transport& operator=(const string_transport& o) { common_construct(o); return *this; }
   string_transport& operator=(string_transport&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~string_transport() {}
private:
// common copy
void common_construct(const string_transport& o) {}
void common_construct(string_transport&& o) noexcept {}

// @API [tag: operator]
public:


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]

// @API [tag: operation]

void allocate(const char* piText)
{
   allocate(piText, std::strlen(piText));
}

void allocate(const char* piText, const uint64_t uLength)
{
   if ( m_piText != nullptr )
   {
      delete[] m_piText;
      m_piText = nullptr;
   }

   m_piText = new char[uLength + 1];
   for (uint64_t i = 0; i < uLength; ++i)
   {
      m_piText[i] = piText[i];
   }
   m_piText[uLength] = '\0';

   m_uStateEndLength = uLength + 1;
}

void append(const char* piText, const uint64_t uLength);

protected:
// @API [tag: internal]

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   uint64_t m_uStateEndLength;
   char* m_piText;


// @API [tag: free-functions]
public:



};

/*
inline void string_transport::append(const char* piText)
{
   auto uLength = std::strlen(piText);
   append(piText, uLength);
}

inline void string_transport::append(int32_t iValue)
{
   std::string stringNumber = std::to_string(iValue); // Convert integer to string using std::to_string  
   
   uint64_t uLength = static_cast<uint64_t>(stringNumber.size());
   append(stringNumber.c_str(), uLength);
}

inline void string_transport::append(double dValue)
{
   // allocate new memory for the combined string
   auto uOldLlength = m_uStateEndLength;
   uint64_t uNewLength = m_uStateEndLength + uLength;
   allocate_add(uNewLength);

   std::memcpy(m_piText + uOldLlength, piText, static_cast<size_t>(uLength));
}
*/


inline void string_transport::append(const char* piText, const uint64_t uLength)
{
   // allocate new memory for the combined string
   auto uOldLlength = m_uStateEndLength;
   uint64_t uNewLength = m_uStateEndLength + uLength;
   allocate_add(uNewLength);

   std::memcpy(m_piText + uOldLlength, piText, static_cast<size_t>(uLength));
}

inline void string_transport::allocate( uint64_t uLength )
{
   if( m_piText != nullptr ) { delete[] m_piText; }
   m_piText = new char[uLength + 1];
   m_piText[uLength] = '\0';

   m_uStateEndLength = uLength;
}

inline void string_transport::allocate_add(uint64_t uLength)
{
   // ## allocate new memory and compy existing text to new memory and the delete the old memory and set to the new memory
   uint64_t uNewLength = m_uStateEndLength + uLength;
   char* piText_ = new char[uNewLength + 1];

   if( m_piText != nullptr && m_uStateEndLength > 0 )
   {
      std::memcpy(piText_, m_piText, static_cast<size_t>(m_uStateEndLength));
   }

   piText_[m_uStateEndLength] = '\0';

   delete[] m_piText;
   m_piText = piText_;
   m_uStateEndLength = uNewLength;
}
