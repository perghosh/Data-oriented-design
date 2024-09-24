/**
 * \file gd_database_types.h
 * 
 * \brief Constants and some core functionality for database code.  
 * 
 */



#pragma once
#include <cassert>
#include <string>
#include <string_view>

#ifndef _GD_DATABASE_BEGIN
#define _GD_DATABASE_BEGIN namespace gd { namespace database {
#define _GD_DATABASE_END } }
_GD_DATABASE_BEGIN
#else
_GD_DATABASE_BEGIN
#endif

#if defined( __clang__ )
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma clang diagnostic ignored "-Wunused-value"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma GCC diagnostic ignored "-Wunused-value"
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning( disable : 26495 26812 )
#endif

/// for convert methods
struct tag_convert {};
/// used to mark that variant values are used
struct tag_variant {};




#define SQL_MAX_NUMERIC_LEN 16
typedef struct tagSQL_NUMERIC_STRUCT
{
   unsigned char precision;
   signed char   scale;
   unsigned char sign;	/* 1 if positive, 0 if negative */
   unsigned char val[SQL_MAX_NUMERIC_LEN];
} SQL_NUMERIC_STRUCT;

typedef struct tagTIMESTAMP_STRUCT
{
   short            year;
   unsigned short   month;
   unsigned short   day;
   unsigned short   hour;
   unsigned short   minute;
   unsigned short   second;
   unsigned long    fraction;
} TIMESTAMP_STRUCT;

typedef struct tagDATE_STRUCT
{
   short            year;
   unsigned short   month;
   unsigned short   day;
} DATE_STRUCT;

typedef struct tagTIME_STRUCT
{
   unsigned short   hour;
   unsigned short   minute;
   unsigned short   second;
} TIME_STRUCT;




/*-----------------------------------------*/ /**
 * \brief Type of database that is connected and script needs to handle
 *
 *
 */
enum enumDatabaseType
{
   eDatabaseTypeNone    = 0,              ///< empty
   eDatabaseTypeODBC    = 1,              ///< ODBC connection
   eDatabaseTypeOLEDB   = 2,              ///< OLEDB connection
   eDatabaseTypeSqlite  = 3,              ///< Sqlite connection
};

/**
 * @brief constant number for different rdbms (relational database management systems)
*/
enum enumServerName
{
   eServerNameUnknown               = 0,
   eServerNameMicrosoftSqlServer    = 1,
   eServerNameOracle                = 2,
   eServerNameMySql                 = 3,
   eServerNamePostgreSQL            = 4,
   eServerNameMicrosoftAccess       = 5,
   eServerNameSqlite                = 6,
};

enum enumDatabaseState
{
   eDatabaseStateOwner          = 0b0000'0000'0000'0001, ///< if database owns the internal connection
   eDatabaseStateConnected      = 0b0000'0000'0000'0010, ///< if object is connected to database
};


/**
 * @brief cursor state marks characteristics for cursor
*/
enum enumCursorState
{
   eCursorStateRow         = 0x01,           ///< cursor is on row
   eCursorStateMemory      = 0x02,           ///< check if all data is retrieved for columns with flexible storage
};

/**
 * @brief marks column value characteristics
*/
enum enumColumnValueState
{
   eColumnValueStateNull   = 0x01,           ///< Marks if value in column is null
   eColumnValueStateFixed  = 0x02,           ///< Value length in column is fixed, all values for column holds need same size in bytes
   eColumnValueStateBlob   = 0x04,           ///< Blob value, this means that buffer storing value need to be checked if it large enough
};

/*
enum enumMakeFieldValue
{
   eMakeFieldValueKey      = 0x01,           ///< generate key value for field
   eMakeFieldValueDefault  = 0x02,           ///< generate default value for field
};
*/


/*-----------------------------------------*/ /**
 * \brief 
 *
 *
 */
enum enumColumnTypeGroup
{
   eColumnTypeGroupNumber  = 0x00000100,
   eColumnTypeGroupInteger = 0x00000200,
   eColumnTypeGroupDecimal = 0x00000400,
   eColumnTypeGroupSigned  = 0x00000800,
   eColumnTypeGroupString  = 0x00001000,
   eColumnTypeGroupDate    = 0x00002000,
   eColumnTypeGroupBinary  = 0x00004000,
   eColumnTypeGroupBoolean = 0x00008000,

   eColumnTypeGroup08      = 0x00010000,
   eColumnTypeGroup16      = 0x00020000,
   eColumnTypeGroup32      = 0x00040000,
   eColumnTypeGroup64      = 0x00080000,
   eColumnTypeGroup128     = 0x00100000,
   eColumnTypeGroup256     = 0x00200000,
   eColumnTypeGroup512     = 0x00300000,

   eColumnTypeGroupNull    = 0x10000000,
   eColumnTypeGroupChar    = 0x20000000,
   eColumnTypeGroupWChar   = 0x30000000,
};

/*-----------------------------------------*/ /**
 * \brief database column types
 *
 *
 */
enum enumColumnTypeNumber
{
   eColumnTypeNumberUnknown      = 0,
   eColumnTypeNumberBool         = 1,
   eColumnTypeNumberInt8         = 2,
   eColumnTypeNumberUInt8        = 3,
   eColumnTypeNumberInt16        = 4,
   eColumnTypeNumberUInt16       = 5,
   eColumnTypeNumberInt32        = 6,
   eColumnTypeNumberUInt32       = 7,
   eColumnTypeNumberInt64        = 8,
   eColumnTypeNumberUInt64       = 9,
   eColumnTypeNumberCFloat       = 10,
   eColumnTypeNumberCDouble      = 11,
   eColumnTypeNumberPointer      = 12,
   eColumnTypeNumberGuid         = 13,
   eColumnTypeNumberString       = 14,
   eColumnTypeNumberUtf8String   = 15,
   eColumnTypeNumberWString      = 16,
   eColumnTypeNumberUtf32String  = 17,
   eColumnTypeNumberBinary       = 18,
   eColumnTypeNumberJson         = 19,
   eColumnTypeNumberXml          = 20,
   eColumnTypeNumberCsv          = 21,
   eColumnTypeNumberVoid         = 22,
   eColumnTypeNumberBit          = 23,
   eColumnTypeNumberInt128       = 24,
   eColumnTypeNumberUInt128      = 25,
   eColumnTypeNumberInt256       = 26,
   eColumnTypeNumberUInt256      = 27,
   eColumnTypeNumberInt512       = 28,
   eColumnTypeNumberUInt512      = 29,
   eColumnTypeNumberBinaryString = 30,
   eColumnTypeNumberNumberBase32 = 31,
   eColumnTypeNumberDateTime     = 32,
   eColumnTypeNumberDate         = 33,
   eColumnTypeNumberTime         = 34,
   eColumnTypeNumberNumeric      = 35,
   eColumnTypeNumberDecimal      = 36,
   eColumnTypeNumberUuidString   = 37,
};



/*-----------------------------------------*/ /**
 * \brief constant number for different value types used in database management
 *
 */
enum enumColumnType
{
   //TODO: Only user of utf-8 string type is sqlite so remove that type
   eColumnTypeUnknown      = eColumnTypeNumberUnknown    | eColumnTypeGroupNull,
   eColumnTypeBit          = eColumnTypeNumberBit        | eColumnTypeGroupNumber,
   eColumnTypeBool         = eColumnTypeNumberBool       | eColumnTypeGroupNumber | eColumnTypeGroupInteger | eColumnTypeGroupBoolean,
   eColumnTypeInt8         = eColumnTypeNumberInt8       | eColumnTypeGroupNumber | eColumnTypeGroupInteger | eColumnTypeGroupSigned,
   eColumnTypeInt16        = eColumnTypeNumberInt16      | eColumnTypeGroupNumber | eColumnTypeGroupInteger | eColumnTypeGroupSigned,
   eColumnTypeInt32        = eColumnTypeNumberInt32      | eColumnTypeGroupNumber | eColumnTypeGroupInteger | eColumnTypeGroupSigned,
   eColumnTypeInt64        = eColumnTypeNumberInt64      | eColumnTypeGroupNumber | eColumnTypeGroupInteger | eColumnTypeGroupSigned,
   eColumnTypeUInt8        = eColumnTypeNumberUInt8      | eColumnTypeGroupNumber | eColumnTypeGroupInteger,
   eColumnTypeUInt16       = eColumnTypeNumberUInt16     | eColumnTypeGroupNumber | eColumnTypeGroupInteger,
   eColumnTypeUInt32       = eColumnTypeNumberUInt32     | eColumnTypeGroupNumber | eColumnTypeGroupInteger,
   eColumnTypeUInt64       = eColumnTypeNumberUInt64     | eColumnTypeGroupNumber | eColumnTypeGroupInteger,
   eColumnTypeCFloat       = eColumnTypeNumberCFloat     | eColumnTypeGroupNumber | eColumnTypeGroupDecimal | eColumnTypeGroupSigned,
   eColumnTypeCDouble      = eColumnTypeNumberCDouble    | eColumnTypeGroupNumber | eColumnTypeGroupDecimal | eColumnTypeGroupSigned,
   eColumnTypeGuid         = eColumnTypeNumberGuid       | eColumnTypeGroupBinary,
   eColumnTypeBinary       = eColumnTypeNumberBinary     | eColumnTypeGroupBinary,
   eColumnTypeString       = eColumnTypeNumberString     | eColumnTypeGroupString,
   eColumnTypeUtf8String   = eColumnTypeNumberUtf8String | eColumnTypeGroupString,
   eColumnTypeWString      = eColumnTypeNumberWString    | eColumnTypeGroupString,
   eColumnTypeUtf32String  = eColumnTypeNumberUtf32String    | eColumnTypeGroupString,
   eColumnTypeNumeric      = eColumnTypeNumberNumeric    | eColumnTypeGroupString,
   eColumnTypeDecimal      = eColumnTypeNumberDecimal    | eColumnTypeGroupString,
   eColumnTypeGuidString   = eColumnTypeNumberUuidString | eColumnTypeGroupString | eColumnTypeGroupBinary,
   eColumnTypeBinaryString = eColumnTypeNumberBinaryString| eColumnTypeGroupString| eColumnTypeGroupBinary,

   eColumnTypeDateTime     = eColumnTypeNumberDateTime   | eColumnTypeGroupDate,
   eColumnTypeDate         = eColumnTypeNumberDate       | eColumnTypeGroupDate,
   eColumnTypeTime         = eColumnTypeNumberTime       | eColumnTypeGroupDate,


   eColumnType_FilterType  = 0x0000ffff,        ///< flags used to get complete type information
   eColumnType_FilterGroup = 0x0000ff00,        ///< flags used to get group information from type
   eColumnType_FilterTypeGroup = 0x0000ff00,    ///< flags used to get group information from type
   eColumnType_FilterTypeNumber = 0x000000ff,   ///< flags for filter out index for type
};

/*-----------------------------------------*/ /**
 * \brief constant number for different value types used in database management
 *
 */
enum enumColumnTypeComplete
{
   eColumnTypeCompleteUnknown      = eColumnTypeUnknown,   
   eColumnTypeCompleteBit          = eColumnTypeBit        | eColumnTypeGroup08,
   eColumnTypeCompleteBool         = eColumnTypeBool       | eColumnTypeGroup08,
   eColumnTypeCompleteInt8         = eColumnTypeInt8       | eColumnTypeGroup08,
   eColumnTypeCompleteInt16        = eColumnTypeInt16      | eColumnTypeGroup16,
   eColumnTypeCompleteInt32        = eColumnTypeInt32      | eColumnTypeGroup32,
   eColumnTypeCompleteInt64        = eColumnTypeInt64      | eColumnTypeGroup64,
   eColumnTypeCompleteUInt8        = eColumnTypeUInt8      | eColumnTypeGroup08,
   eColumnTypeCompleteUInt16       = eColumnTypeUInt16     | eColumnTypeGroup16,
   eColumnTypeCompleteUInt32       = eColumnTypeUInt32     | eColumnTypeGroup32,
   eColumnTypeCompleteUInt64       = eColumnTypeUInt64     | eColumnTypeGroup64,
   eColumnTypeCompleteCFloat       = eColumnTypeCFloat     | eColumnTypeGroup32,
   eColumnTypeCompleteCDouble      = eColumnTypeCDouble    | eColumnTypeGroup64,
   eColumnTypeCompleteGuid         = eColumnTypeGuid,
   eColumnTypeCompleteBinary       = eColumnTypeBinary,
   eColumnTypeCompleteString       = eColumnTypeString     | eColumnTypeGroupChar,
   eColumnTypeCompleteUtf8String   = eColumnTypeUtf8String | eColumnTypeGroupChar,
   eColumnTypeCompleteWString      = eColumnTypeWString    | eColumnTypeGroupWChar,
   eColumnTypeCompleteUtf32String  = eColumnTypeUtf32String| eColumnTypeGroupWChar,
   eColumnTypeCompleteNumeric      = eColumnTypeNumeric    | eColumnTypeGroupChar,
   eColumnTypeCompleteDecimal      = eColumnTypeDecimal    | eColumnTypeGroupChar,
   eColumnTypeCompleteGuidString   = eColumnTypeGuidString | eColumnTypeGroupWChar,
   eColumnTypeCompleteBinaryString = eColumnTypeBinaryString| eColumnTypeGroupWChar,
   eColumnTypeCompleteDateTime     = eColumnTypeDateTime    | eColumnTypeGroup64,
   eColumnTypeCompleteDate         = eColumnTypeDate        | eColumnTypeGroup64,
   eColumnTypeCompleteTime         = eColumnTypeTime        | eColumnTypeGroup64,
};

/*----------------------------------------------------------------------------- get_type */ /**
 * 
 * \param stringTypeName
 * \return constexpr uint32_t
 */
/*
constexpr uint32_t get_type(std::string_view stringTypeName)
{
   if(  )
}
*/

/** ---------------------------------------------------------------------------
 * @brief extract group type from complete type
 * @param uType complete type
 * @return unsigned group part from complete value type
*/
constexpr unsigned value_group_type_g( unsigned uType ) { return (uType & eColumnType_FilterTypeGroup); }

/** ---------------------------------------------------------------------------
 * @brief Return column size for type number
 * This method returns know size for specific types
 * @param uTypeNumber type number size is returned for
 * @return size for type
*/
constexpr unsigned value_size_g(unsigned uTypeNumber)
{
   switch( uTypeNumber & 0x00000000ff )
   {
   case eColumnTypeNumberUnknown : return 0;    
   case eColumnTypeNumberBool : return sizeof(uint8_t);
   case eColumnTypeNumberInt8 : return sizeof(int8_t);       
   case eColumnTypeNumberUInt8 : return sizeof(uint8_t);
   case eColumnTypeNumberInt16 : return sizeof(int16_t);
   case eColumnTypeNumberUInt16 : return sizeof(uint16_t);
   case eColumnTypeNumberInt32 : return sizeof(int32_t);
   case eColumnTypeNumberUInt32 : return sizeof(uint32_t);
   case eColumnTypeNumberInt64 : return sizeof(int64_t);
   case eColumnTypeNumberUInt64 : return sizeof(uint64_t);
   case eColumnTypeNumberCFloat : return sizeof(float);
   case eColumnTypeNumberCDouble : return sizeof(double);
   case eColumnTypeNumberPointer : return sizeof(void*);
   case eColumnTypeNumberGuid : return 16;
   case eColumnTypeNumberString : return 0;     
   case eColumnTypeNumberUtf8String : return 0; 
   case eColumnTypeNumberWString : return 0;    
   case eColumnTypeNumberUtf32String : return 0;
   case eColumnTypeNumberBinary : return 0;     
   case eColumnTypeNumberJson : return 0;       
   case eColumnTypeNumberXml : return 0;        
   case eColumnTypeNumberVoid : return 0;       
   case eColumnTypeNumberBit : return sizeof(uint8_t);        
   case eColumnTypeNumberUInt128 : return sizeof(uint64_t) + sizeof(uint64_t);
   case eColumnTypeNumberDateTime : return sizeof(TIMESTAMP_STRUCT);
   case eColumnTypeNumberDate : return sizeof(DATE_STRUCT);
   case eColumnTypeNumberTime : return sizeof(TIME_STRUCT);    
   case eColumnTypeNumberNumeric : return sizeof(SQL_NUMERIC_STRUCT);
   case eColumnTypeNumberDecimal : return 0; 
   default: return 0;
   }
}

/** ---------------------------------------------------------------------------
 * @brief Return column size for type number
 * This method returns know size for specific types
 * @param uTypeNumber type number size is returned for
 * @return size for type
*/
constexpr unsigned value_size_g(unsigned uTypeNumber, unsigned uCount)
{
   switch( uTypeNumber & 0x00000000ff )
   {
   case eColumnTypeNumberUnknown : return 0;    
   case eColumnTypeNumberBool : return sizeof(uint8_t) * uCount;
   case eColumnTypeNumberInt8 : return sizeof(int8_t) * uCount;       
   case eColumnTypeNumberUInt8 : return sizeof(uint8_t) * uCount;
   case eColumnTypeNumberInt16 : return sizeof(int16_t) * uCount;
   case eColumnTypeNumberUInt16 : return sizeof(uint16_t) * uCount;
   case eColumnTypeNumberInt32 : return sizeof(int32_t) * uCount;
   case eColumnTypeNumberUInt32 : return sizeof(uint32_t) * uCount;
   case eColumnTypeNumberInt64 : return sizeof(int64_t) * uCount;
   case eColumnTypeNumberUInt64 : return sizeof(uint64_t) * uCount;
   case eColumnTypeNumberCFloat : return sizeof(float) * uCount;
   case eColumnTypeNumberCDouble : return sizeof(double) * uCount;
   case eColumnTypeNumberPointer : return sizeof(void*) * uCount;
   case eColumnTypeNumberGuid : return 16 * uCount;
   case eColumnTypeNumberString : return sizeof(int8_t) * uCount;
   case eColumnTypeNumberUtf8String : return sizeof(uint8_t) * uCount; 
   case eColumnTypeNumberWString : return sizeof(uint16_t) * uCount;
   case eColumnTypeNumberUtf32String : return sizeof(uint32_t) * uCount;
   case eColumnTypeNumberBinary : return sizeof(uint8_t) * uCount;     
   case eColumnTypeNumberJson : return sizeof(uint8_t) * uCount;
   case eColumnTypeNumberXml : return sizeof(uint8_t) * uCount;
   case eColumnTypeNumberVoid : return 0;       
   case eColumnTypeNumberBit : return sizeof(uint8_t) * uCount;        
   case eColumnTypeNumberUInt128 : return (sizeof(uint64_t) + sizeof(uint64_t)) * uCount;
   case eColumnTypeNumberDateTime : return sizeof(TIMESTAMP_STRUCT) * uCount;
   case eColumnTypeNumberDate : return sizeof(DATE_STRUCT) * uCount;
   case eColumnTypeNumberTime : return sizeof(TIME_STRUCT) * uCount;    
   case eColumnTypeNumberNumeric : return sizeof(SQL_NUMERIC_STRUCT) * uCount;
   case eColumnTypeNumberDecimal : return 0; 
   default: return 0;
   }
}


/** ---------------------------------------------------------------------------
 * @brief Return number in string
 * If string holds a number that number is extracted and returned as 
 * @param stringGetNumber string where number is searched for and if found it is returned
 * @return number in string as unsigned int if found, otherwise 0
*/
constexpr unsigned value_size_g(const std::string_view& stringGetNumber)
{
   for (auto it = std::begin(stringGetNumber), itEnd = std::end(stringGetNumber); it != itEnd; it++)
   {
      if( *it >= '0' && *it <= '9' )
      {
         unsigned uResult = 0;
         do 
         {
            unsigned uNumber = *it - '0';
            uResult *= 10;
            uResult += uNumber;
            it++;
         } while (it != itEnd && *it >= '0' && *it <= '9');

         return uResult;
      }
   }

   return 0;
}


/** ---------------------------------------------------------------------------
 * @brief Return type number for type name
 * Converts a typename in text to a number and can do this during compile time.
 * It simplifies to read code and you do not need to work with macros or other constants.
 * Valid type names are:
 * BINARY, BOOL, DECIMAL, DAYE, DATETIME, GUID, FLOAT, INT8, INT16, INT32, INT64,
 * NUMERIC, NVARCHAR, STRING, UINT8, UINT16, UINT32, UINT64, UTF8, UTF32, VARCHAR
 * @param stringType type that is converted to number
 * @return unsigned type number
*/
constexpr unsigned value_get_type_number_g( const std::string_view& stringType )
{
   constexpr char LOWER_A = 'a';
   const std::size_t BUFFER_SIZE = 7;
   char pbuffer[BUFFER_SIZE] = {'\0'};
   for( std::size_t u = 0; u < BUFFER_SIZE; u++ )
   {
      if( stringType.length() > u )
      {
         char ch = stringType[u];
         if( ch >= LOWER_A ) ch -= ('a' - 'A');
         pbuffer[u] = ch;
      }
      else
      {
         pbuffer[u] = '\0';
      }
   }

   switch( pbuffer[0] )
   {
      case 'B': {
         if( pbuffer[sizeof "BI" - 1] == 'I' ) return eColumnTypeNumberBinary;
         return eColumnTypeNumberBool;
      }
      break;
      case 'D': {
         if( pbuffer[sizeof "DATET" - 1] == 'T' ) return eColumnTypeNumberDateTime;
         if( pbuffer[sizeof "DEC" - 1] == 'C' ) return eColumnTypeNumberDecimal;
         return eColumnTypeNumberDate;
      }
      break;
      case 'G': {
         return eColumnTypeNumberGuid;
      }
      case 'F': {
         return eColumnTypeNumberCFloat;
      }
      break;
      case 'I': {
         if( pbuffer[sizeof "INT3" - 1] == '3' ) return eColumnTypeNumberInt32;
         else if( pbuffer[sizeof "INT6" - 1] == '6' ) return eColumnTypeNumberInt64;
         else if( pbuffer[sizeof "INT1" - 1] == '1' ) return eColumnTypeNumberInt16;
         else if( pbuffer[sizeof "INT8" - 1] == '8' ) return eColumnTypeNumberInt8;
         return eColumnTypeNumberInt32;
      }
      break;
      case 'N': {
         if( pbuffer[sizeof "NV" - 1] == 'V' ) return eColumnTypeNumberWString;
         return eColumnTypeNumberNumeric;
      }
      break;
      case 'S': {
         return eColumnTypeNumberString;
      }
      break;
      case 'U': {
         if( pbuffer[sizeof "UTF8" - 1] == '8' ) return eColumnTypeNumberUtf8String;
         else if( pbuffer[sizeof "UTF3" - 1] == '3' ) return eColumnTypeNumberUtf32String;
         else if( pbuffer[sizeof "UINT3" - 1] == '3' ) return eColumnTypeNumberUInt32;
         else if( pbuffer[sizeof "UINT6" - 1] == '6' ) return eColumnTypeNumberUInt64;
         else if( pbuffer[sizeof "UINT1" - 1] == '1' ) return eColumnTypeNumberUInt16;
         else if( pbuffer[sizeof "UINT8" - 1] == '8' ) return eColumnTypeNumberUInt8;
         return eColumnTypeNumberUInt32;
      }
      case 'V': {
         return eColumnTypeNumberString;
      }
      break;
   }

   return eColumnTypeNumberUnknown;
}

/** ---------------------------------------------------------------------------
 * @brief Return type number for type name
 * Converts a typename in text to a number and can do this during compile time.
 * It simplifies to read code and you do not need to work with macros or other constants.
 * Valid type names are:
 * BINARY, BOOL, DECIMAL, DAYE, DATETIME, GUID, FLOAT, INT8, INT16, INT32, INT64,
 * NUMERIC, NVARCHAR, STRING, UINT8, UINT16, UINT32, UINT64, UTF8, UTF32, VARCHAR
 * @param stringType type that is converted to number
 * @return unsigned type number
*/
constexpr unsigned value_get_type_complete_g( const std::string_view& stringType )
{
   constexpr char LOWER_A = 'a';
   const std::size_t BUFFER_SIZE = 7;
   char pbuffer[BUFFER_SIZE] = {'\0'};
   for( std::size_t u = 0; u < BUFFER_SIZE; u++ )
   {
      if( stringType.length() > u )
      {
         char ch = stringType[u];
         if( ch >= LOWER_A ) ch -= ('a' - 'A');
         pbuffer[u] = ch;
      }
      else
      {
         pbuffer[u] = '\0';
      }
   }

   switch( pbuffer[0] )
   {
      case 'B': {
         if( pbuffer[sizeof "BI" - 2] == 'I' ) return eColumnTypeCompleteBinary;
         return eColumnTypeCompleteBool;
      }
      break;
      case 'D': {
         if( pbuffer[sizeof "DATET" - 2] == 'T' ) return eColumnTypeCompleteDateTime;
         else if( pbuffer[sizeof "DO" - 2] == 'O' ) return eColumnTypeCompleteCDouble;
         else if( pbuffer[sizeof "DEC" - 2] == 'C' ) return eColumnTypeCompleteDecimal;
         return eColumnTypeCompleteDate;
      }
      break;
      case 'G': {
         return eColumnTypeCompleteGuid;
      }
      case 'F': {
         return eColumnTypeCompleteCFloat;
      }
      break;
      case 'I': {
         if( pbuffer[sizeof "INT3" - 2] == '3' ) return eColumnTypeCompleteInt32;      // INT32
         else if( pbuffer[sizeof "INT6" - 2] == '6' ) return eColumnTypeCompleteInt64; // INT64
         else if( pbuffer[sizeof "INT1" - 2] == '1' ) return eColumnTypeCompleteInt16; // INT16
         else if( pbuffer[sizeof "INT8" - 2] == '8' ) return eColumnTypeCompleteInt8;  // INT8
         return eColumnTypeCompleteInt32;
      }
      break;
      case 'N': {
         if( pbuffer[sizeof "NV" - 2] == 'V' ) return eColumnTypeCompleteWString;
         return eColumnTypeCompleteNumeric;
      }
      break;
      case 'S': {
         return eColumnTypeCompleteString;
      }
      break;
      case 'U': {
         if( pbuffer[sizeof "UTF8" - 2] == '8' ) return eColumnTypeCompleteUtf8String;
         else if( pbuffer[sizeof "UTF3" - 2] == '3' ) return eColumnTypeCompleteUtf32String;
         else if( pbuffer[sizeof "UINT3" - 2] == '3' ) return eColumnTypeCompleteUInt32;
         else if( pbuffer[sizeof "UINT6" - 2] == '6' ) return eColumnTypeCompleteUInt64;
         else if( pbuffer[sizeof "UINT1" - 2] == '1' ) return eColumnTypeCompleteUInt16;
         else if( pbuffer[sizeof "UINT8" - 2] == '8' ) return eColumnTypeCompleteUInt8;
         return eColumnTypeCompleteUInt32;
      }
      case 'V': {
         return eColumnTypeCompleteString;
      }
      break;
   }

   return eColumnTypeCompleteUnknown;
}

// ## helper methods to check group type f�r specific value type
constexpr bool is_boolean_g( enumColumnTypeComplete e ) { return e & eColumnTypeGroupBoolean; }
constexpr bool is_string_g( enumColumnTypeComplete e ) { return e & eColumnTypeGroupString; }
constexpr bool is_integer_g( enumColumnTypeComplete e ) { return e & eColumnTypeGroupInteger; }
constexpr bool is_date_g( enumColumnTypeComplete e ) { return e & eColumnTypeGroupDate; }
constexpr bool is_binary_g( enumColumnTypeComplete e ) { return e & eColumnTypeGroupBinary; }

constexpr bool is_boolean_g( unsigned u ) { return u & eColumnTypeGroupBoolean; }
constexpr bool is_string_g( unsigned u ) { return u & eColumnTypeGroupString; }
constexpr bool is_integer_g( unsigned u ) { return u & eColumnTypeGroupInteger; }
constexpr bool is_date_g( unsigned u ) { return u & eColumnTypeGroupDate; }
constexpr bool is_binary_g( unsigned u ) { return u & eColumnTypeGroupBinary; }


_GD_DATABASE_END

// ## Check for compability with core types found in gd::types
#ifdef _GD_TYPES
static_assert( gd::types::enumTypeNumber::eTypeNumberUInt64 == static_cast<gd::types::enumTypeNumber>(gd::database::enumColumnTypeNumber::eColumnTypeNumberUInt64) );
static_assert( gd::types::enumTypeNumber::eTypeNumberDouble == static_cast<gd::types::enumTypeNumber>(gd::database::enumColumnTypeNumber::eColumnTypeNumberCDouble) );
static_assert( gd::types::enumTypeNumber::eTypeNumberDecimal == static_cast<gd::types::enumTypeNumber>(gd::database::enumColumnTypeNumber::eColumnTypeNumberDecimal) );
#endif

#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif

