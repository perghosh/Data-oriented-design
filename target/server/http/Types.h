// @FILE [tag: types] [description: http types] [type: header] [name: Types.h]

#pragma once

#include <memory>
#include <functional>
#include <vector>

#include "gd/gd_arguments.h"
#include "gd/gd_table_column-buffer.h"

#if defined( __clang__ )
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#elif defined( _MSC_VER )
   #pragma warning(push)
#endif

namespace Types {

enum enumRequestItem
{
   eRequestItemIp           = 0x0001,                ///< Use IP address information
   eRequestItemUserAgent    = 0x0002,                ///< Use User-Agent information
   eRequestItemSession      = 0x0004,                ///< Use Session information
};


enum enumTypeNumber
{
   eTypeUnknown         = 0,  // unknown type
   eTypeTextPlain       = 1,  // plain text
   eTypeTextXml         = 2,  // xml text
   eTypeTextJson        = 3,
   eTypeTextCsv         = 4,
   eTypeTableDto        = 5,  // dto::table object 
   eTypeArgumentsDto    = 6,  // gd::table::arguments object
};

enum enumGroupNumber
{
   eGroupNone           = 0x00000000,
   eGroupText           = 0x00000100,
   eGroupBinary         = 0x00000200,
   eGroupTable          = 0x00000300,
   eGroupArguments      = 0x00000400,
};


/// combine types
enum enumType
{
   eTypePlain           = eTypeTextPlain     | eGroupText,
   eTypeXml             = eTypeTextXml       | eGroupText,
   eTypeJson            = eTypeTextJson      | eGroupText,
   eTypeCsv             = eTypeTextCsv       | eGroupText,
   eTypeDtoTable        = eTypeTableDto      | eGroupTable,
   eTypeArguments       = eTypeArgumentsDto  | eGroupArguments,
};

/// clear object based on type, in http there are some common objects used to hold data and these are moved around as void pointers, this method will clear the object based on type
void Clear_g( unsigned uType, void* pobject_ );
inline void Clear_g( std::pair<unsigned, void*> pair_ ) { Clear_g( pair_.first, pair_.second ); }


/// get type number from string type name
constexpr Types::enumTypeNumber TypeNumber_g( std::string_view stringTypeName )
{
   if(  stringTypeName     == "text/plain" )       return Types::eTypeTextPlain;
   else if( stringTypeName == "text/xml" )         return Types::eTypeTextXml;
   else if( stringTypeName == "application/json" ) return Types::eTypeTextJson;
   else if( stringTypeName == "text/csv" )         return Types::eTypeTextCsv;
   else if( stringTypeName == "table" )            return Types::eTypeTableDto;
   else if( stringTypeName == "arguments" )        return Types::eTypeArgumentsDto;
   else return Types::eTypeUnknown;
}

/// compare only type number part of enumType
inline bool operator==(Types::enumTypeNumber eTypeNumber, uint32_t uValue) {
    return (static_cast<uint32_t>(eTypeNumber) & 0xFF) == (uValue & 0xFF);
}

/// compare only type number part of enumType
inline bool operator==(uint32_t uValue, enumTypeNumber eTypeNumber) 
{
    return (uValue & 0xFF) == (static_cast<uint32_t>(eTypeNumber) & 0xFF);
}


/**
 * \brief used to transfer result objects, this only holds a pointer to the object
 *
 *
 */
struct Object
{
   using Delete_ = std::function<void(void*)>;

   // ## construction ------------------------------------------------------------
   Object() = default;
   Object( Types::enumType eType, void* pobject ) 
      : m_eType(eType), m_pobject(pobject, [eType](void* p){ Types::Clear_g(eType, p); }) {}

   // ## methods -----------------------------------------------------------------
   Types::enumType type() const { return m_eType; }
   void* get() const { return m_pobject.get(); }
   void* release() { return m_pobject.release(); } // detach pointer

   Types::enumType m_eType = (Types::enumType)0;  ///< type of result
   std::unique_ptr<void, Delete_> m_pobject; ///< pointer to data
};

struct Objects
{
   // ## construction ------------------------------------------------------------
   Objects() = default;

   void Add( gd::table::dto::table* p_ ) { m_vectorObjects.emplace_back( Object{ eTypeDtoTable, p_ } ); }
   void Add( gd::argument::arguments* p_ ) { m_vectorObjects.emplace_back( Object{ eTypeArguments, p_ } ); }

   size_t Size() const noexcept { return m_vectorObjects.size(); }

   bool Empty() const noexcept { return m_vectorObjects.empty(); }

   // ## attributes --------------------------------------------------------------
   std::vector< Object > m_vectorObjects;  ///< list of objects
};



} // namespace Types

#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
