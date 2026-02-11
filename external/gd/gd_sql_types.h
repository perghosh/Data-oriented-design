// @FILE [tag: sql, type] [description: Type values for SQL queries] [type: header] [name: gd_sql_types.h]

#pragma once

#ifndef _GD_SQL_QUERY_BEGIN
   #define _GD_SQL_QUERY_BEGIN namespace gd { namespace sql {
   #define _GD_SQL_QUERY_END } }
#endif

_GD_SQL_QUERY_BEGIN

/// tag dispatcher used for table operations
struct tag_table {};
/// tag dispatcher used for field operations
struct tag_field {};
/// tag dispatcher used for condition operations
struct tag_condition {};
/// tag dispatcher used name
struct tag_name {};


/// tag dispatcher for values that is owned
struct tag_value {};
/// tag dispatcher for values that is viewed (not owned)
struct tag_value_view {};


/*----------------------------------------------------------------------------
 * \brief SQL dialect used to generate SQL code
 *
 * Values are assigned so that closely related dialects (by syntax family / compatibility)
 * have nearby numbers. This makes switch/case blocks or range-based logic easier.
 *
 * Groups (approximate ranges):
 *   1–19  →  High ANSI compliance (modern standard-like)
 *   20–39 →  MySQL-family + embedded/test DBs (medium compliance, widespread)
 *   40–59 →  Enterprise traditional RDBMS (proprietary-heavy)
 *   60–79 →  Cloud data warehouses / analytics (ANSI base + extensions)
 *   80+   →  Extreme outliers (OLAP columnar / very non-standard)
 *
 * Within each group: ordered roughly most → least similar to core ANSI SQL.
 */
enum enumSqlDialect
{
   // Optional fallback (place at 0 or 99 depending on your preference)
   eSqlDialectUnknown   = 0,

   // ────────────────────────────────────────────────
   // PostgreSQL family — Highest ANSI compliance
   // Modern reference for portable SQL; rich features
   // ────────────────────────────────────────────────
   eSqlDialectPostgreSql = 10,  // PostgreSQL — closest to modern ANSI SQL standard; extensive CTEs, windows, JSONB, arrays, strict typing, extensibility

   eSqlDialectCockroachDB = 11,  // CockroachDB — PostgreSQL wire & dialect compatible; very high compliance + distributed/consistent tweaks

   // ────────────────────────────────────────────────
   // Embedded / in-memory / test DBs — Good ANSI coverage
   // Often emulate PostgreSQL or MySQL modes
   // ────────────────────────────────────────────────
   eSqlDialectH2 = 15,  // H2 Database — strong standards support; PostgreSQL & MySQL compatibility modes; ideal for testing

   eSqlDialectHSQLDB = 16,  // HyperSQL (HSQLDB) — excellent ANSI compliance in strict mode; lightweight, test-friendly

   eSqlDialectDerby = 17,  // Apache Derby / JavaDB — solid compliance; conservative, Java-centric

   // ────────────────────────────────────────────────
   // SQLite family — Medium compliance, lightweight/embedded
   // Loose typing, some missing advanced features
   // ────────────────────────────────────────────────
   eSqlDialectSqlite = 20,  // SQLite — embedded standard; good core SQL but partial windows, no RIGHT/FULL JOIN in older versions, dynamic typing

   // ────────────────────────────────────────────────
   // MySQL family — High adoption, medium compliance
   // Many historical quirks (non-standard GROUP BY, LIMIT, loose casts)
   // MariaDB edges closer to standard in newer releases
   // ────────────────────────────────────────────────
   eSqlDialectMariaDB = 25,  // MariaDB — very close to MySQL but improved standards compliance (CTEs, window functions, etc.)

   eSqlDialectMySql = 26,  // MySQL — dominant in web/apps; persistent non-standard behaviors despite progress

   // ────────────────────────────────────────────────
   // Enterprise traditional — Feature-rich but proprietary syntax heavy
   // Big differences in paging, dates, hierarchical queries, etc.
   // ────────────────────────────────────────────────
   eSqlDialectSqlServer = 40,  // Microsoft SQL Server + Azure SQL — T-SQL; TOP, MERGE, different string/date funcs, CTE quirks

   eSqlDialectDB2 = 41,  // IBM Db2 — enterprise extensions; unique paging, OLAP, schema behavior

   eSqlDialectOracle = 42,  // Oracle Database — highly proprietary (ROWNUM → analytic, CONNECT BY, old (+) joins, strict)

   // ────────────────────────────────────────────────
   // Cloud data warehouses — ANSI-inspired + analytics extensions
   // Good standard base but diverge on semi-structured, time-travel, clustering
   // ────────────────────────────────────────────────
   eSqlDialectSnowflake = 60,  // Snowflake — PostgreSQL-like base + cloud extensions (time-travel, VARIANT semi-structured, clustering)

   eSqlDialectBigQuery = 61,  // Google BigQuery — Standard SQL + Google extensions (ARRAY/STRUCT, UNNEST, scripting, geography)

   eSqlDialectRedshift = 62,  // Amazon Redshift — old PostgreSQL base + AWS-specific (DIST/SORTKEY, COPY/UNLOAD, columnar tweaks)

   // ────────────────────────────────────────────────
   // Extreme outlier — Columnar OLAP-first, massive non-standard extensions
   // ────────────────────────────────────────────────
   eSqlDialectClickHouse = 80   // ClickHouse — columnar OLAP; ARRAY JOIN, WITH FILL, specialized aggregations, weak classic transactions
};


/*-----------------------------------------*/ /**
 * \brief how to format sql
 *
 *
 */
enum enumFormat
{
   eFormatUseQuotes        = (1 << 0),
   eFormatAddASKeyword     = (1 << 1),
   eFormatAddINNERKeyword  = (1 << 2),

};


/*-----------------------------------------*/ /**
 * \brief constant values for describing what type of join to use
 *
 *
 */
enum enumJoin
{
   eJoinUnknown = 0,
   eJoinInner = 1,
   eJoinLeft,
   eJoinRight,
   eJoinFull,
};

enum enumOperatorTypeNumber
{
   eOperatorTypeNumberEqual = 0,        // =
   eOperatorTypeNumberNotEqual = 1,     // !=
   eOperatorTypeNumberLess = 2,         // <
   eOperatorTypeNumberLessEqual = 3,    // <=
   eOperatorTypeNumberGreater = 4,      // >
   eOperatorTypeNumberGreaterEqual = 5, // >=
   eOperatorTypeNumberLike = 6,         // ..=..
   eOperatorTypeNumberLikeBegin = 7,    // ..=
   eOperatorTypeNumberLikeEnd = 8,      // =..
   eOperatorTypeNumberNull = 9,         // IS NULL
   eOperatorTypeNumberNotNull = 10,     // IS NOT NULL
   eOperatorTypeNumberIn = 11,          // IN
   eOperatorTypeNumberNotIn = 12,       // NOT IN
   eOperatorTypeNumberEND,              // Used to check for max valid operator number
};

enum enumOperatorGroupType
{
   eOperatorGroupTypeBoolean    = 0x00000100,   // boolean value
   eOperatorGroupTypeNumber     = 0x00000200,   // number value
   eOperatorGroupTypeDate       = 0x00000400,   // date value
   eOperatorGroupTypeString     = 0x00000800,   // text value
   eOperatorGroupTypeBinary     = 0x00001000,   // binary
};

enum enumOperator
{
   eOperatorEqual =                 eOperatorTypeNumberEqual | eOperatorGroupTypeBoolean | eOperatorGroupTypeNumber | eOperatorGroupTypeDate | eOperatorGroupTypeString | eOperatorGroupTypeBinary,
   eOperatorNotEqual =              eOperatorTypeNumberNotEqual | eOperatorGroupTypeBoolean | eOperatorGroupTypeNumber | eOperatorGroupTypeDate | eOperatorGroupTypeString | eOperatorGroupTypeBinary,
   eOperatorLess =                  eOperatorTypeNumberLess | eOperatorGroupTypeNumber | eOperatorGroupTypeDate | eOperatorGroupTypeString,
   eOperatorLessEqual =             eOperatorTypeNumberLessEqual | eOperatorGroupTypeNumber | eOperatorGroupTypeDate | eOperatorGroupTypeString,
   eOperatorGreater =               eOperatorTypeNumberGreater | eOperatorGroupTypeNumber | eOperatorGroupTypeDate | eOperatorGroupTypeString,
   eOperatorGreaterEqual =          eOperatorTypeNumberGreaterEqual | eOperatorGroupTypeNumber | eOperatorGroupTypeDate | eOperatorGroupTypeString,
   eOperatorLike =                  eOperatorTypeNumberLike | eOperatorGroupTypeString,
   eOperatorLikeBegin =             eOperatorTypeNumberLikeBegin | eOperatorGroupTypeString,
   eOperatorLikeEnd =               eOperatorTypeNumberLikeEnd | eOperatorGroupTypeString,
   eOperatorNull =                  eOperatorTypeNumberNull | eOperatorGroupTypeBoolean | eOperatorGroupTypeNumber | eOperatorGroupTypeDate | eOperatorGroupTypeString | eOperatorGroupTypeBinary,
   eOperatorNotNull =               eOperatorTypeNumberNotNull | eOperatorGroupTypeBoolean | eOperatorGroupTypeNumber | eOperatorGroupTypeDate | eOperatorGroupTypeString | eOperatorGroupTypeBinary,
   eOperatorIn =                    eOperatorTypeNumberIn | eOperatorGroupTypeBoolean | eOperatorGroupTypeNumber | eOperatorGroupTypeDate | eOperatorGroupTypeString | eOperatorGroupTypeBinary,
   eOperatorNotIn =                 eOperatorTypeNumberNotIn | eOperatorGroupTypeBoolean | eOperatorGroupTypeNumber | eOperatorGroupTypeDate | eOperatorGroupTypeString | eOperatorGroupTypeBinary,

   eOperatorError =                 0xffffffff,
};

enum enumOperatorMask
{
   eOperatorMaskNumber = 0x000000ff,
};

/**
 * \brief Important sql parts to build sql queries.
 *
 * `query` are able to generate sql queries, different parts can be selected
 * for generation. To combine parts you can sett flags for wich parts that
 * is generated. Flags from `enumSqlPart` are used for that.
 *
 */
enum enumSqlPart
{
   eSqlPartUnknown =       0b0000'0000'0000'0000'0000'0000'0000'0000,
   //                        3       2 2       1 1
   //                        1       4 3       6 5       8 7       0
   eSqlPartSelect =        0b0000'0000'0000'0001'0000'0000'0000'0000,
   eSqlPartInsert =        0b0000'0000'0000'0010'0000'0000'0000'0000,
   eSqlPartUpdate =        0b0000'0000'0000'0100'0000'0000'0000'0000,
   eSqlPartDelete =        0b0000'0000'0000'1000'0000'0000'0000'0000,
   eSqlPartFrom =          0b0000'0000'0001'0000'0000'0000'0000'0000,
   eSqlPartWhere =         0b0000'0000'0010'0000'0000'0000'0000'0000,
   eSqlPartLimit =         0b0000'0000'0100'0000'0000'0000'0000'0000,
   eSqlPartOrderBy =       0b0000'0000'1000'0000'0000'0000'0000'0000,
   eSqlPartGroupBy =       0b0000'0001'0000'0000'0000'0000'0000'0000,
   eSqlPartWith =          0b0000'0010'0000'0000'0000'0000'0000'0000,
   eSqlPartHaving =        0b0000'0100'0000'0000'0000'0000'0000'0000,
   eSqlPartValues =        0b0000'1000'0000'0000'0000'0000'0000'0000,
   eSqlPartSet =           0b0001'0000'0000'0000'0000'0000'0000'0000,
   eSqlPartReturning =     0b0010'0000'0000'0000'0000'0000'0000'0000,
};

enum enumSql
{
   eSqlSelect =            eSqlPartSelect | eSqlPartFrom | eSqlPartWhere | eSqlPartOrderBy | eSqlPartGroupBy | eSqlPartWith | eSqlPartLimit,
   eSqlInsert =            eSqlPartInsert,
   eSqlUpdate =            eSqlPartUpdate | eSqlPartWhere,
   eSqlDelete =            eSqlPartDelete | eSqlPartFrom | eSqlPartWhere,
};


/** ---------------------------------------------------------------------------
 * @brief Return  part number for part name
 * Converts sql part name to part number and are able to do this at commpile time.
 * Valid part names are:
 * DELETE, FROM, GROUPBY, HAVING, INSERT, LIMIT, ORDERBY, SELECT, UPDATE, WHERE, WITH
 * @param stringPartName Part as name that is converted to number
 * @return {enumSqlPart} number for part name
*/
constexpr enumSqlPart sql_get_part_type_g(const std::string_view& stringPartName)
{                                                                              assert(stringPartName.empty() == false);
   // ## convert character to uppercase if lowercase is found
   constexpr uint8_t LOWER_A = 'a';
   uint8_t uFirst = (uint8_t)stringPartName[0];                                // only check first character
   if( uFirst >= LOWER_A ) uFirst -= ('a' - 'A');                              // convert to lowercase subtracting to capital letter

   switch( uFirst )
   {
   case 'D': return enumSqlPart::eSqlPartDelete;
   case 'F': return enumSqlPart::eSqlPartFrom;
   case 'G': return enumSqlPart::eSqlPartGroupBy;
   case 'H': return enumSqlPart::eSqlPartHaving;
   case 'I': return enumSqlPart::eSqlPartInsert;
   case 'L': return enumSqlPart::eSqlPartLimit;
   case 'O': return enumSqlPart::eSqlPartOrderBy;
   case 'S': return enumSqlPart::eSqlPartSelect;
   case 'U': return enumSqlPart::eSqlPartUpdate;
   case 'W': {
      if( stringPartName[1] == 'I' || stringPartName[1] == 'i' ) return enumSqlPart::eSqlPartWith;
      return enumSqlPart::eSqlPartWhere;
      }
   }
   return eSqlPartUnknown;
}

/** ---------------------------------------------------------------------------
 * @brief Return dialect enum value from dialect name (case-insensitive)
 *
 * Converts dialect name (SQL server, PostgreSql, mysql, etc.) to enumSqlDialect.
 * Designed to be very fast and constexpr-evaluable.
 * Accepts mixed/upper/lower case.
 *
 * Most common dialects are resolved with 1–2 char checks.
 * Longer/rarer names fall through to slightly more comparisons.
 *
 * @param stringDialect Dialect name (e.g. "PostgreSQL", "mysql", "SQLSERVER")
 * @return enumSqlDialect value, or eSqlDialectUnknown if no match
 */
constexpr enumSqlDialect sql_get_dialect_g( std::string_view stringDialect )
{
   if( stringDialect.empty() ) { return eSqlDialectUnknown; }

   // Fast uppercase-first-letter trick (like in sql_get_part_type_g)
   constexpr uint8_t LOWER_A = 'a';
   uint8_t uFirst = static_cast<uint8_t>( stringDialect[0] );
   if( uFirst >= LOWER_A ) { uFirst -= ( 'a' - 'A' ); }

   switch( uFirst )
   {
   case 'B':   // BigQuery
   if( stringDialect.size() >= 7 && ( stringDialect[1] == 'i' || stringDialect[1] == 'I' ) && ( stringDialect[2] == 'g' || stringDialect[2] == 'G' ) ) { return eSqlDialectBigQuery; }
   break;

   case 'C':   // ClickHouse, CockroachDB
   if( stringDialect.size() >= 10 ) 
   {
      char iSecond = ( stringDialect[1] == 'o' || stringDialect[1] == 'O' ) ? 'o' : 0;
      if( iSecond ) 
      {
         char iThird = ( stringDialect[2] == 'c' || stringDialect[2] == 'C' ) ? 'c' : 0;
         if( iThird ) { return eSqlDialectCockroachDB; }
      }
   }
   if( stringDialect.size() >= 10 && ( stringDialect[1] == 'l' || stringDialect[1] == 'L' ) && ( stringDialect[2] == 'i' || stringDialect[2] == 'I' ) ) { return eSqlDialectClickHouse; }
   break;

   case 'D':   // DB2, Derby
   if( stringDialect.size() >= 3 && ( stringDialect[1] == 'B' || stringDialect[1] == 'b' ) && ( stringDialect[2] == '2' ) ) { return eSqlDialectDB2; }
   if( stringDialect.size() >= 5 && ( stringDialect[1] == 'e' || stringDialect[1] == 'E' ) && ( stringDialect[2] == 'r' || stringDialect[2] == 'R' ) ) { return eSqlDialectDerby; }
   break;

   case 'H':   // H2, HSQLDB
   if( stringDialect.size() >= 2 ) {
      char iSecond = ( stringDialect[1] == '2' ) ? '2' : 0;
      if( iSecond ) return eSqlDialectH2;

      if( ( stringDialect[1] == 's' || stringDialect[1] == 'S' ) && ( stringDialect[2] == 'q' || stringDialect[2] == 'Q' ) ) { return eSqlDialectHSQLDB; }
   }
   break;

   case 'M':   // MariaDB, MySQL
   if( stringDialect.size() >= 6 && ( stringDialect[1] == 'a' || stringDialect[1] == 'A' ) && ( stringDialect[2] == 'r' || stringDialect[2] == 'R' ) ) { return eSqlDialectMariaDB; }
   if( stringDialect.size() >= 5 && ( stringDialect[1] == 'y' || stringDialect[1] == 'Y' ) && ( stringDialect[2] == 'S' || stringDialect[2] == 's' ) ) { return eSqlDialectMySql; }
   break;

   case 'O':   // Oracle
   if( stringDialect.size() >= 6 && ( stringDialect[1] == 'r' || stringDialect[1] == 'R' ) && ( stringDialect[2] == 'a' || stringDialect[2] == 'A' ) ) { return eSqlDialectOracle; }
   break;

   case 'P':   // PostgreSQL
   if( stringDialect.size() >= 10 && ( stringDialect[1] == 'o' || stringDialect[1] == 'O' ) && ( stringDialect[2] == 's' || stringDialect[2] == 'S' ) ) { return eSqlDialectPostgreSql; }
   break;

   case 'R':   // Redshift
   if( stringDialect.size() >= 8 && ( stringDialect[1] == 'e' || stringDialect[1] == 'E' ) && ( stringDialect[2] == 'd' || stringDialect[2] == 'D' ) ) { return eSqlDialectRedshift; }
   break;

   case 'S':   // SQL Server, Snowflake, SQLite
   if( stringDialect.size() >= 6 ) {
      char iSecond = ( stringDialect[1] == 'Q' || stringDialect[1] == 'q' ) ? 'Q' : 0;
      if( iSecond ) {
         if( ( stringDialect[2] == 'L' || stringDialect[2] == 'l' ) ) {
            // SQL...
            if( ( stringDialect[3] == 'S' || stringDialect[3] == 's' ) ||
                ( stringDialect[4] == 'S' || stringDialect[4] == 's' ) ) {  // SQLServer or MSSQL etc.
               return eSqlDialectSqlServer;
            }
            // SQLite
            if( ( stringDialect[3] == 'i' || stringDialect[3] == 'I' ) ) {  return eSqlDialectSqlite; }
         }
      }
   }
   // Snowflake
   if( stringDialect.size() >= 8 && ( stringDialect[1] == 'n' || stringDialect[1] == 'N' ) && ( stringDialect[2] == 'o' || stringDialect[2] == 'O' ) ) { return eSqlDialectSnowflake; }
   break;
   }
                                                                                                   assert( false );
   return eSqlDialectUnknown;
}


_GD_SQL_QUERY_END
