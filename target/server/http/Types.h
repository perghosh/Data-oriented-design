// @FILE [tag: types] [description: http types] [type: header] [name: Types.h]

#pragma once

namespace Types {

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

void Clear_g( unsigned uType, void* pobject_ );

} // namespace Types