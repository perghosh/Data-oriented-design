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
   string() : m_piData(nullptr), m_uLength(0), m_uBufferSize(0) {}
   string(const char* piData): m_piData(nullptr), m_uLength(0), m_uBufferSize(0) { append(piData); }
   string(const char* piData, const size_t uLength) : m_piData(nullptr), m_uLength(0), m_uBufferSize(0) { append(piData, uLength); }
   // copy
   string(const string& o) { common_construct(o); }
   string(string&& o) noexcept { common_construct(std::move(o)); }
   // assign
   string& operator=(const string& o) { common_construct(o); return *this; }
   string& operator=(string&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~string() { delete[] m_piData; }
private:
   // common copy
   void common_construct(const string& o) {}
   void common_construct(string&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:
   string& operator+=( const char* piData ) { return append( piData ); }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{

   void allocate(const uint64_t uLength);

   string& append(const char* piData);
   string& append(const char* piData, size_t uLength);

   string& insert(size_t uPosition, const char* piData, size_t uLength);
   string& insert(size_t uPosition, const char* piData);

   size_t size() const { return strlen(m_piData); };

   string substr(size_t uPosition = 0, size_t uLength = m_uNoPosition);

   const char* c_str() const;

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
   uint64_t m_uLength;
   uint64_t m_uBufferSize;

   static const size_t m_uNoPosition = -1;


// ## free functions ------------------------------------------------------------
public:



};
