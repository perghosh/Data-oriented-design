#include <string>
#include <fstream>

#include "gd_utf8.hpp"

#include "gd_csv.h"

_GD_CSV_BEGIN

std::pair<gd::argument::arguments, std::vector<gd::variant>> read( const std::string_view& stringFileName, no_quote )
{
   //unsigned uColumnCount;
   gd::argument::arguments argumentsStats;
   std::vector< gd::variant > vectorValues;

   std::ifstream ifstreamCsv( stringFileName.data() );
   if( ifstreamCsv.fail() == false ) { return std::pair<gd::argument::arguments, std::vector<gd::variant>>( { {"error", true}, {"information", "Failed to open file"}, {"file", stringFileName.data()}}, {}); }

   std::string stringLine;

   if( std::getline( ifstreamCsv, stringLine ) )
   {
      //gd::utf8::split( stringLine, ',', vectorValues );
      //argumentsStats << { "column_count", (uint32_t)vectorValues.size() };
   }

   while( std::getline( ifstreamCsv, stringLine ) )
   {
      //gd::utf8::split( stringLine, ',', vectorValues );
   }



   return std::pair<gd::argument::arguments, std::vector<gd::variant>>();
}


_GD_CSV_END