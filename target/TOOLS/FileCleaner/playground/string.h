#pragma once
#include <cstdint>
#include <utility>
#include <cstring>
#include <iostream>
/**
 * \brief
 *
 *
 *
 \code
 string stringTest;
 stringTest.append("ETT").append("TWÅ");
 \endcode
 */
class string
{
   // ## construction -------------------------------------------------------------
public:
   string() : m_piData(const_cast<char*>(m_piEmpty_s)), m_uLength(0), m_uBufferSize(0) {}
   string(const char* piData): m_piData(nullptr), m_uLength(0), m_uBufferSize(0) { append(piData); }
   string(const char* piData, const size_t uLength) : m_piData(const_cast<char*>(m_piEmpty_s)), m_uLength(0), m_uBufferSize(0) { append(piData, uLength); }
   // copy
   string(const string& o) { common_construct(o); }
   string(string&& o) noexcept { common_construct(std::move(o)); }
   // assign
   string& operator=(const string& o) { common_construct(o); return *this; }
   string& operator=(string&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~string() { if( m_piData != m_piEmpty_s ) delete[] m_piData; }
private:
   // common copy
   void common_construct(const string& o) {}
   void common_construct(string&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:

   string& operator+=( const char* piData ) { return append( piData ); }
   string& operator=(const char* piData) { return assign(piData); }

// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{

   string& append(const char* piData);
   string& append(const char* piData, size_t uLength);

   // ## assing methods

   /// Main implementation of assign, most other assign methods call this one.
   string& assign(const char* piData, size_t ulength);
   string& assign(const char* piData);

   string& insert(size_t uPosition, const char* piData, size_t uLength);
   string& insert(size_t uPosition, const char* piData);

   string& replace(size_t uPosition, const char* piData, size_t uLength);

   size_t size() const { return strlen(m_piData); };

   string substr(size_t uPosition = 0, size_t uLength = m_uNoPosition);

   const char* c_str() const { return m_piData; }

   void clear();


   void allocate(uint64_t uLength);


//@}

protected:
/** \name INTERNAL
*///@{

//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:

   char* m_piData;
   uint64_t m_uLength;     ///< length of the string (not including the terminating null character)
   uint64_t m_uBufferSize; ///< size of the allocated buffer

   inline static const size_t m_uNoPosition = -1;
   inline static const char* m_piEmpty_s = "";


// ## free functions ------------------------------------------------------------
public:



};
