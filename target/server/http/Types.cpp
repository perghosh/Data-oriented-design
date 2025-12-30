
#include "gd/gd_arguments.h"
#include "gd/gd_table_column-buffer.h"

#include "Types.h"

namespace Types {

void Clear_g( unsigned uType, void* pobject_ )
{
   if( pobject_ == nullptr ) { return; }

   switch( uType & 0x000000ff )
   {
   case eTypeTextPlain:
   case eTypeTextXml:
   case eTypeTextJson:
   case eTypeTextCsv:
      {
         auto pstring = static_cast<std::string*>( pobject_ );
         delete pstring;
      }
      break;
   case eTypeTableDto:
      {
         auto ptable = static_cast<gd::table::table_column_buffer*>( pobject_ );
         delete ptable;
      }
      break;
   case eTypeArgumentsDto:
      {
         auto parguments = static_cast<gd::argument::arguments*>( pobject_ );
         delete parguments;
      }
   default:
      // unknown type, do nothing
      break;
   }

}

}