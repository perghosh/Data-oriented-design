
#include "gd/gd_arguments.h"
#include "gd/gd_table_column-buffer.h"

#include "Types.h"

namespace Types {

/** -------------------------------------------------------------------------- Clear_g
 * @brief Release heap-allocated object stored as `void*` based on low-byte type id.
 *
 * Uses `uType & 0x000000ff` to select the concrete object type and then deletes the
 * corresponding allocation. If `pobject_` is `nullptr`, the function returns directly.
 *
 * **Important:** `pobject_` must have been allocated with `new` using the matching type
 * for the resolved case in `uType`; otherwise behavior is undefined.
 *
 * @param uType Type identifier where the low byte determines the payload category.
 * @param pobject_ Pointer to the heap object to delete; may be `nullptr`.
 * @return void No value is returned.
 */
void Clear_g( unsigned uType, void* pobject_ )
{
   if( pobject_ == nullptr ) { return; }

   unsigned uTypeNumber = uType & 0x000000ff; // extract low byte to determine type
   switch( uTypeNumber )
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
      break;
   default:                                                                                        assert( false && "Unknown type in Clear_g, cannot clear object" );
      // unknown type, do nothing
      break;
   }

}

}