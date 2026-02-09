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
};

enum enumSql
{
   eSqlSelect =            eSqlPartSelect | eSqlPartFrom | eSqlPartWhere | eSqlPartOrderBy | eSqlPartGroupBy | eSqlPartWith | eSqlPartLimit,
   eSqlInsert =            eSqlPartInsert,
   eSqlUpdate =            eSqlPartUpdate | eSqlPartWhere,
   eSqlDelete =            eSqlPartDelete | eSqlPartFrom | eSqlPartWhere,
};




_GD_SQL_QUERY_END
