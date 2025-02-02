#pragma once
#include <cassert>
#include <cstdint>
#include <string_view>


#ifndef _GD_CONSOLE_BEGIN
   #define _GD_CONSOLE_BEGIN namespace gd { namespace console {
   #define _GD_CONSOLE_END } }
   _GD_CONSOLE_BEGIN
#else
   _GD_CONSOLE_BEGIN
#endif

/// color names for colors from 16 - 255
enum enumColor : uint8_t 
{
   eColorAquamarine1        = 122,
   eColorAquamarine1Bis     = 86,
   eColorAquamarine3        = 79,
   eColorBlue1              = 21,
   eColorBlue3              = 19,
   eColorBlue3Bis           = 20,
   eColorBlueViolet         = 57,
   eColorCadetBlue          = 72,
   eColorCadetBlueBis       = 73,
   eColorChartreuse1        = 118,
   eColorChartreuse2        = 112,
   eColorChartreuse2Bis     = 82,
   eColorChartreuse3        = 70,
   eColorChartreuse3Bis     = 76,
   eColorChartreuse4        = 64,
   eColorCornflowerBlue     = 69,
   eColorCornsilk1          = 230,
   eColorCyan1              = 51,
   eColorCyan2              = 50,
   eColorCyan3              = 43,
   eColorDarkBlue           = 18,
   eColorDarkCyan           = 36,
   eColorDarkGoldenrod      = 136,
   eColorDarkGreen          = 22,
   eColorDarkKhaki          = 143,
   eColorDarkMagenta        = 90,
   eColorDarkMagentaBis     = 91,
   eColorDarkOliveGreen1    = 191,
   eColorDarkOliveGreen1Bis = 192,
   eColorDarkOliveGreen2    = 155,
   eColorDarkOliveGreen3    = 107,
   eColorDarkOliveGreen3Bis = 113,
   eColorDarkOliveGreen3Ter = 149,
   eColorDarkOrange         = 208,
   eColorDarkOrange3        = 130,
   eColorDarkOrange3Bis     = 166,
   eColorDarkRed            = 52,
   eColorDarkRedBis         = 88,
   eColorDarkSeaGreen       = 108,
   eColorDarkSeaGreen1      = 158,
   eColorDarkSeaGreen1Bis   = 193,
   eColorDarkSeaGreen2      = 151,
   eColorDarkSeaGreen2Bis   = 157,
   eColorDarkSeaGreen3      = 115,
   eColorDarkSeaGreen3Bis   = 150,
   eColorDarkSeaGreen4      = 65,
   eColorDarkSeaGreen4Bis   = 71,
   eColorDarkSlateGray1     = 123,
   eColorDarkSlateGray2     = 87,
   eColorDarkSlateGray3     = 116,
   eColorDarkTurquoise      = 44,
   eColorDarkViolet         = 128,
   eColorDarkVioletBis      = 92,
   eColorDeepPink1          = 198,
   eColorDeepPink1Bis       = 199,
   eColorDeepPink2          = 197,
   eColorDeepPink3          = 161,
   eColorDeepPink3Bis       = 162,
   eColorDeepPink4          = 125,
   eColorDeepPink4Bis       = 89,
   eColorDeepPink4Ter       = 53,
   eColorDeepSkyBlue1       = 39,
   eColorDeepSkyBlue2       = 38,
   eColorDeepSkyBlue3       = 31,
   eColorDeepSkyBlue3Bis    = 32,
   eColorDeepSkyBlue4       = 23,
   eColorDeepSkyBlue4Bis    = 24,
   eColorDeepSkyBlue4Ter    = 25,
   eColorDodgerBlue1        = 33,
   eColorDodgerBlue2        = 27,
   eColorDodgerBlue3        = 26,
   eColorGold1              = 220,
   eColorGold3              = 142,
   eColorGold3Bis           = 178,
   eColorGreen1             = 46,
   eColorGreen3             = 34,
   eColorGreen3Bis          = 40,
   eColorGreen4             = 28,
   eColorGreenYellow        = 154,
   eColorGrey0              = 16,
   eColorGrey100            = 231,
   eColorGrey11             = 234,
   eColorGrey15             = 235,
   eColorGrey19             = 236,
   eColorGrey23             = 237,
   eColorGrey27             = 238,
   eColorGrey3              = 232,
   eColorGrey30             = 239,
   eColorGrey35             = 240,
   eColorGrey37             = 59,
   eColorGrey39             = 241,
   eColorGrey42             = 242,
   eColorGrey46             = 243,
   eColorGrey50             = 244,
   eColorGrey53             = 102,
   eColorGrey54             = 245,
   eColorGrey58             = 246,
   eColorGrey62             = 247,
   eColorGrey63             = 139,
   eColorGrey66             = 248,
   eColorGrey69             = 145,
   eColorGrey7              = 233,
   eColorGrey70             = 249,
   eColorGrey74             = 250,
   eColorGrey78             = 251,
   eColorGrey82             = 252,
   eColorGrey84             = 188,
   eColorGrey85             = 253,
   eColorGrey89             = 254,
   eColorGrey93             = 255,
   eColorHoneydew2          = 194,
   eColorHotPink            = 205,
   eColorHotPink2           = 169,
   eColorHotPink3           = 132,
   eColorHotPink3Bis        = 168,
   eColorHotPinkBis         = 206,
   eColorIndianRed          = 131,
   eColorIndianRed1         = 203,
   eColorIndianRed1Bis      = 204,
   eColorIndianRedBis       = 167,
   eColorKhaki1             = 228,
   eColorKhaki3             = 185,
   eColorLightCoral         = 210,
   eColorLightCyan1Bis      = 195,
   eColorLightCyan3         = 152,
   eColorLightGoldenrod1    = 227,
   eColorLightGoldenrod2    = 186,
   eColorLightGoldenrod2Bis = 221,
   eColorLightGoldenrod2Ter = 222,
   eColorLightGoldenrod3    = 179,
   eColorLightGreen         = 119,
   eColorLightGreenBis      = 120,
   eColorLightPink1         = 217,
   eColorLightPink3         = 174,
   eColorLightPink4         = 95,
   eColorLightSalmon1       = 216,
   eColorLightSalmon3       = 137,
   eColorLightSalmon3Bis    = 173,
   eColorLightSeaGreen      = 37,
   eColorLightSkyBlue1      = 153,
   eColorLightSkyBlue3      = 109,
   eColorLightSkyBlue3Bis   = 110,
   eColorLightSlateBlue     = 105,
   eColorLightSlateGrey     = 103,
   eColorLightSteelBlue     = 147,
   eColorLightSteelBlue1    = 189,
   eColorLightSteelBlue3    = 146,
   eColorLightYellow3       = 187,
   eColorMagenta1           = 201,
   eColorMagenta2           = 165,
   eColorMagenta2Bis        = 200,
   eColorMagenta3           = 127,
   eColorMagenta3Bis        = 163,
   eColorMagenta3Ter        = 164,
   eColorMediumOrchid       = 134,
   eColorMediumOrchid1      = 171,
   eColorMediumOrchid1Bis   = 207,
   eColorMediumOrchid3      = 133,
   eColorMediumPurple       = 104,
   eColorMediumPurple1      = 141,
   eColorMediumPurple2      = 135,
   eColorMediumPurple2Bis   = 140,
   eColorMediumPurple3      = 97,
   eColorMediumPurple3Bis   = 98,
   eColorMediumPurple4      = 60,
   eColorMediumSpringGreen  = 49,
   eColorMediumTurquoise    = 80,
   eColorMediumVioletRed    = 126,
   eColorMistyRose1         = 224,
   eColorMistyRose3         = 181,
   eColorNavajoWhite1       = 223,
   eColorNavajoWhite3       = 144,
   eColorNavyBlue           = 17,
   eColorOrange1            = 214,
   eColorOrange3            = 172,
   eColorOrange4            = 58,
   eColorOrange4Bis         = 94,
   eColorOrangeRed1         = 202,
   eColorOrchid             = 170,
   eColorOrchid1            = 213,
   eColorOrchid2            = 212,
   eColorPaleGreen1         = 121,
   eColorPaleGreen1Bis      = 156,
   eColorPaleGreen3         = 114,
   eColorPaleGreen3Bis      = 77,
   eColorPaleTurquoise1     = 159,
   eColorPaleTurquoise4     = 66,
   eColorPaleVioletRed1     = 211,
   eColorPink1              = 218,
   eColorPink3              = 175,
   eColorPlum1              = 219,
   eColorPlum2              = 183,
   eColorPlum3              = 176,
   eColorPlum4              = 96,
   eColorPurple             = 129,
   eColorPurple3            = 56,
   eColorPurple4            = 54,
   eColorPurple4Bis         = 55,
   eColorPurpleBis          = 93,
   eColorRed1               = 196,
   eColorRed3               = 124,
   eColorRed3Bis            = 160,
   eColorRosyBrown          = 138,
   eColorRoyalBlue1         = 63,
   eColorSalmon1            = 209,
   eColorSandyBrown         = 215,
   eColorSeaGreen1          = 84,
   eColorSeaGreen1Bis       = 85,
   eColorSeaGreen2          = 83,
   eColorSeaGreen3          = 78,
   eColorSkyBlue1           = 117,
   eColorSkyBlue2           = 111,
   eColorSkyBlue3           = 74,
   eColorSlateBlue1         = 99,
   eColorSlateBlue3         = 61,
   eColorSlateBlue3Bis      = 62,
   eColorSpringGreen1       = 48,
   eColorSpringGreen2       = 42,
   eColorSpringGreen2Bis    = 47,
   eColorSpringGreen3       = 35,
   eColorSpringGreen3Bis    = 41,
   eColorSpringGreen4       = 29,
   eColorSteelBlue          = 67,
   eColorSteelBlue1         = 75,
   eColorSteelBlue1Bis      = 81,
   eColorSteelBlue3         = 68,
   eColorTan                = 180,
   eColorThistle1           = 225,
   eColorThistle3           = 182,
   eColorTurquoise2         = 45,
   eColorTurquoise4         = 30,
   eColorViolet             = 177,
   eColorWheat1             = 229,
   eColorWheat4             = 101,
   eColorYellow1            = 226,
   eColorYellow2            = 190,
   eColorYellow3            = 148,
   eColorYellow3Bis         = 184,
   eColorYellow4            = 100,
   eColorYellow4Bis         = 106,
};

consteval uint8_t color_g( const std::string_view& stringColor )
{
   if( stringColor == "aquamarine1")               return enumColor::eColorAquamarine1;
   else if( stringColor == "aquamarine1Bis")       return enumColor::eColorAquamarine1Bis;
   else if( stringColor == "aquamarine1")          return enumColor::eColorAquamarine3;
   else if( stringColor == "blue3")                return enumColor::eColorBlue1;
   else if( stringColor == "blue3")                return enumColor::eColorBlue3;
   else if( stringColor == "blue3bis")             return enumColor::eColorBlue3Bis;
   else if( stringColor == "blueviolet")           return enumColor::eColorBlueViolet;
   else if( stringColor == "cadetbluebis")         return enumColor::eColorCadetBlueBis;
   else if( stringColor == "chartreuse1")          return enumColor::eColorChartreuse1;
   else if( stringColor == "chartreuse2")          return enumColor::eColorChartreuse2;
   else if( stringColor == "chartreuse2bis")       return enumColor::eColorChartreuse2Bis;
   else if( stringColor == "chartreuse3")          return enumColor::eColorChartreuse3;
   else if( stringColor == "chartreuse3bis")       return enumColor::eColorChartreuse3Bis;
   else if( stringColor == "chartreuse4")          return enumColor::eColorChartreuse4;
   else if( stringColor == "cornflowerblue")       return enumColor::eColorCornflowerBlue;
   else if( stringColor == "cornsilk1")            return enumColor::eColorCornsilk1;
   else if( stringColor == "cyan1")                return enumColor::eColorCyan1;
   else if( stringColor == "cyan2")                return enumColor::eColorCyan2;
   else if( stringColor == "cyan3")                return enumColor::eColorCyan3;
   else if( stringColor == "darkblue")             return enumColor::eColorDarkBlue;
   else if( stringColor == "darkcyan")             return enumColor::eColorDarkCyan;
   else if( stringColor == "darkgoldenrod")        return enumColor::eColorDarkGoldenrod;
   else if( stringColor == "darkgreen")            return enumColor::eColorDarkGreen;
   else if( stringColor == "darkkhaki")            return enumColor::eColorDarkKhaki;
   else if( stringColor == "darkmagenta")          return enumColor::eColorDarkMagenta;
   else if( stringColor == "darkmagentabis")       return enumColor::eColorDarkMagentaBis;
   else if( stringColor == "darkolivegreen1")      return enumColor::eColorDarkOliveGreen1;
   else if( stringColor == "darkolivegreen1bis")   return enumColor::eColorDarkOliveGreen1Bis;
   else if( stringColor == "darkolivegreen2")      return enumColor::eColorDarkOliveGreen2;
   else if( stringColor == "darkolivegreen3")      return enumColor::eColorDarkOliveGreen3;
   else if( stringColor == "darkolivegreen3bis")   return enumColor::eColorDarkOliveGreen3Bis;
   else if( stringColor == "darkolivegreen3ter")   return enumColor::eColorDarkOliveGreen3Ter;
   else if( stringColor == "darkorange")           return enumColor::eColorDarkOrange;
   else if( stringColor == "darkorange3")          return enumColor::eColorDarkOrange3;
   else if( stringColor == "darkorange3bis")       return enumColor::eColorDarkOrange3Bis;
   else if( stringColor == "darkred")              return enumColor::eColorDarkRed;
   else if( stringColor == "darkredbis")           return enumColor::eColorDarkRedBis;
   else if( stringColor == "darkseagreen")         return enumColor::eColorDarkSeaGreen;
   else if( stringColor == "darkseagreen1")        return enumColor::eColorDarkSeaGreen1;
   else if( stringColor == "darkseagreen1bis")     return enumColor::eColorDarkSeaGreen1Bis;
   else if( stringColor == "darkseagreen2")        return enumColor::eColorDarkSeaGreen2;
   else if( stringColor == "darkseagreen2bis")     return enumColor::eColorDarkSeaGreen2Bis;
   else if( stringColor == "darkseagreen3")        return enumColor::eColorDarkSeaGreen3;
   else if( stringColor == "darkseagreen3bis" )    return enumColor::eColorDarkSeaGreen3Bis;
   else if( stringColor == "darkseagreen4" )       return enumColor::eColorDarkSeaGreen4;
   else if( stringColor == "darkseagreen4bis" )    return enumColor::eColorDarkSeaGreen4Bis;
   else if( stringColor == "darkslategray1" )      return enumColor::eColorDarkSlateGray1;
   else if( stringColor == "darkslategray2" )      return enumColor::eColorDarkSlateGray2;
   else if( stringColor == "darkslategray3" )      return enumColor::eColorDarkSlateGray3;
   else if( stringColor == "darkturquoise" )       return enumColor::eColorDarkTurquoise;
   else if( stringColor == "darkviolet" )          return enumColor::eColorDarkViolet;
   else if( stringColor == "darkvioletbis" )       return enumColor::eColorDarkVioletBis;
   else if( stringColor == "deeppink1" )           return enumColor::eColorDeepPink1;
   else if( stringColor == "deeppink1bis" )        return enumColor::eColorDeepPink1Bis;
   else if( stringColor == "deeppink2" )           return enumColor::eColorDeepPink2;
   else if( stringColor == "deeppink3" )           return enumColor::eColorDeepPink3;
   else if( stringColor == "deeppink3bis" )        return enumColor::eColorDeepPink3Bis;
   else if( stringColor == "deeppink4" )           return enumColor::eColorDeepPink4;
   else if( stringColor == "deeppink4bis" )        return enumColor::eColorDeepPink4Bis;
   else if( stringColor == "deeppink4ter" )        return enumColor::eColorDeepPink4Ter;
   else if( stringColor == "deepskyblue1" )        return enumColor::eColorDeepSkyBlue1;
   else if( stringColor == "deepskyblue2" )        return enumColor::eColorDeepSkyBlue2;
   else if( stringColor == "deepskyblue3" )        return enumColor::eColorDeepSkyBlue3;
   else if( stringColor == "deepskyblue3bis" )     return enumColor::eColorDeepSkyBlue3Bis;
   else if( stringColor == "deepskyblue4" )        return enumColor::eColorDeepSkyBlue4;
   else if( stringColor == "deepskyblue4bis" )     return enumColor::eColorDeepSkyBlue4Bis;
   else if( stringColor == "deepskyblue4ter" )     return enumColor::eColorDeepSkyBlue4Ter;
   else if( stringColor == "dodgerblue1" )         return enumColor::eColorDodgerBlue1;
   else if( stringColor == "dodgerblue2" )         return enumColor::eColorDodgerBlue2;
   else if( stringColor == "dodgerblue3" )         return enumColor::eColorDodgerBlue3;
   else if( stringColor == "gold1" )               return enumColor::eColorGold1;
   else if( stringColor == "gold3" )               return enumColor::eColorGold3;
   else if( stringColor == "gold3bis" )            return enumColor::eColorGold3Bis;
   else if( stringColor == "green1" )              return enumColor::eColorGreen1;
   else if( stringColor == "green3" )              return enumColor::eColorGreen3;
   else if( stringColor == "green3bis" )           return enumColor::eColorGreen3Bis;
   else if( stringColor == "green4" )              return enumColor::eColorGreen4;
   else if( stringColor == "greenyellow" )         return enumColor::eColorGreenYellow;
   else if( stringColor == "grey0" )               return enumColor::eColorGrey0;
   else if( stringColor == "grey100" )             return enumColor::eColorGrey100;
   else if( stringColor == "grey11" )              return enumColor::eColorGrey11;
   else if( stringColor == "grey15" )              return enumColor::eColorGrey15;
   else if( stringColor == "grey19" )              return enumColor::eColorGrey19;
   else if( stringColor == "grey23" )              return enumColor::eColorGrey23;
   else if( stringColor == "grey27" )              return enumColor::eColorGrey27;
   else if( stringColor == "grey3" )               return enumColor::eColorGrey3;
   else if( stringColor == "grey30" )              return enumColor::eColorGrey30;
   else if( stringColor == "grey35" )              return enumColor::eColorGrey35;
   else if( stringColor == "grey37" )              return enumColor::eColorGrey37;
   else if( stringColor == "grey39" )              return enumColor::eColorGrey39;
   else if( stringColor == "grey42" )              return enumColor::eColorGrey42;
   else if( stringColor == "grey46" )              return enumColor::eColorGrey46;
   else if( stringColor == "grey50" )              return enumColor::eColorGrey50;
   else if( stringColor == "grey53" )              return enumColor::eColorGrey53;
   else if( stringColor == "grey54" )              return enumColor::eColorGrey54;
   else if( stringColor == "grey58" )              return enumColor::eColorGrey58;
   else if( stringColor == "grey62" )              return enumColor::eColorGrey62;
   else if( stringColor == "grey63" )              return enumColor::eColorGrey63;
   else if( stringColor == "grey66" )              return enumColor::eColorGrey66;
   else if( stringColor == "grey69" )              return enumColor::eColorGrey69;
   else if( stringColor == "grey7" )               return enumColor::eColorGrey7;
   else if( stringColor == "grey70" )              return enumColor::eColorGrey70;
   else if( stringColor == "grey74" )              return enumColor::eColorGrey74;
   else if( stringColor == "grey78" )              return enumColor::eColorGrey78;
   else if( stringColor == "grey82" )              return enumColor::eColorGrey82;
   else if( stringColor == "grey84" )              return enumColor::eColorGrey84;
   else if( stringColor == "grey85" )              return enumColor::eColorGrey85;
   else if( stringColor == "grey89" )              return enumColor::eColorGrey89;
   else if( stringColor == "grey93" )              return enumColor::eColorGrey93;
   else if( stringColor == "honeydew2" )           return enumColor::eColorHoneydew2;
   else if( stringColor == "hotpink" )             return enumColor::eColorHotPink;
   else if( stringColor == "hotpink2" )            return enumColor::eColorHotPink2;
   else if( stringColor == "hotpink3" )            return enumColor::eColorHotPink3;
   else if( stringColor == "hotpink3bis" )         return enumColor::eColorHotPink3Bis;
   else if( stringColor == "hotpinkbis" )          return enumColor::eColorHotPinkBis;
   else if( stringColor == "indianred" )           return enumColor::eColorIndianRed;
   else if( stringColor == "indianred1" )          return enumColor::eColorIndianRed1;
   else if( stringColor == "indianred1bis" )       return enumColor::eColorIndianRed1Bis;
   else if( stringColor == "indianredbis" )        return enumColor::eColorIndianRedBis;
   else if( stringColor == "khaki1" )              return enumColor::eColorKhaki1;
   else if( stringColor == "khaki3" )              return enumColor::eColorKhaki3;
   else if( stringColor == "lightcoral" )          return enumColor::eColorLightCoral;
   else if( stringColor == "lightcyan1bis" )       return enumColor::eColorLightCyan1Bis;
   
   if( stringColor == "lightcyan3" )          return enumColor::eColorLightCyan3;
   else if( stringColor == "lightgoldenrod1" )     return enumColor::eColorLightGoldenrod1;
   else if( stringColor == "lightgoldenrod2" )     return enumColor::eColorLightGoldenrod2;
   else if( stringColor == "lightgoldenrod2bis" )  return enumColor::eColorLightGoldenrod2Bis;
   else if( stringColor == "lightgoldenrod2ter" )  return enumColor::eColorLightGoldenrod2Ter;
   else if( stringColor == "lightgoldenrod3" )     return enumColor::eColorLightGoldenrod3;
   else if( stringColor == "lightgreen" )          return enumColor::eColorLightGreen;
   else if( stringColor == "lightgreenbis" )       return enumColor::eColorLightGreenBis;
   else if( stringColor == "lightpink1" )          return enumColor::eColorLightPink1;
   else if( stringColor == "lightpink3" )          return enumColor::eColorLightPink3;
   else if( stringColor == "lightpink4" )          return enumColor::eColorLightPink4;
   else if( stringColor == "lightsalmon1" )        return enumColor::eColorLightSalmon1;
   else if( stringColor == "lightsalmon3" )        return enumColor::eColorLightSalmon3;
   else if( stringColor == "lightsalmon3bis" )     return enumColor::eColorLightSalmon3Bis;
   else if( stringColor == "lightseagreen" )       return enumColor::eColorLightSeaGreen;
   else if( stringColor == "lightskyblue1" )       return enumColor::eColorLightSkyBlue1;
   else if( stringColor == "lightskyblue3" )       return enumColor::eColorLightSkyBlue3;
   else if( stringColor == "lightskyblue3bis" )    return enumColor::eColorLightSkyBlue3Bis;
   else if( stringColor == "lightslateblue" )      return enumColor::eColorLightSlateBlue;
   else if( stringColor == "lightslategrey" )      return enumColor::eColorLightSlateGrey;
   else if( stringColor == "lightsteelblue" )      return enumColor::eColorLightSteelBlue;
   else if( stringColor == "lightsteelblue1" )     return enumColor::eColorLightSteelBlue1;
   else if( stringColor == "lightsteelblue3" )     return enumColor::eColorLightSteelBlue3;
   else if( stringColor == "lightyellow3" )        return enumColor::eColorLightYellow3;
   else if( stringColor == "magenta1" )            return enumColor::eColorMagenta1;
   else if( stringColor == "magenta2" )            return enumColor::eColorMagenta2;
   else if( stringColor == "magenta2bis" )         return enumColor::eColorMagenta2Bis;
   else if( stringColor == "magenta3" )            return enumColor::eColorMagenta3;
   else if( stringColor == "magenta3bis" )         return enumColor::eColorMagenta3Bis;
   else if( stringColor == "magenta3ter" )         return enumColor::eColorMagenta3Ter;
   else if( stringColor == "mediumorchid" )        return enumColor::eColorMediumOrchid;
   else if( stringColor == "mediumorchid1" )       return enumColor::eColorMediumOrchid1;
   else if( stringColor == "mediumorchid1bis" )    return enumColor::eColorMediumOrchid1Bis;
   else if( stringColor == "mediumorchid3" )       return enumColor::eColorMediumOrchid3;
   else if( stringColor == "mediumpurple" )        return enumColor::eColorMediumPurple;
   else if( stringColor == "mediumpurple1" )       return enumColor::eColorMediumPurple1;
   else if( stringColor == "mediumpurple2" )       return enumColor::eColorMediumPurple2;
   else if( stringColor == "mediumpurple2bis" )    return enumColor::eColorMediumPurple2Bis;
   else if( stringColor == "mediumpurple3" )       return enumColor::eColorMediumPurple3;
   else if( stringColor == "mediumpurple3bis" )    return enumColor::eColorMediumPurple3Bis;
   else if( stringColor == "mediumpurple4" )       return enumColor::eColorMediumPurple4;
   else if( stringColor == "mediumspringgreen" )   return enumColor::eColorMediumSpringGreen;
   else if( stringColor == "mediumturquoise" )     return enumColor::eColorMediumTurquoise;
   else if( stringColor == "mediumvioletred" )     return enumColor::eColorMediumVioletRed;
   else if( stringColor == "mistyrose1" )          return enumColor::eColorMistyRose1;
   else if( stringColor == "mistyrose3" )          return enumColor::eColorMistyRose3;
   else if( stringColor == "navajowhite1" )        return enumColor::eColorNavajoWhite1;
   else if( stringColor == "navajowhite3" )        return enumColor::eColorNavajoWhite3;
   else if( stringColor == "navyblue" )            return enumColor::eColorNavyBlue;
   else if( stringColor == "orange1" )             return enumColor::eColorOrange1;
   else if( stringColor == "orange3" )             return enumColor::eColorOrange3;
   else if( stringColor == "orange4" )             return enumColor::eColorOrange4;
   else if( stringColor == "orange4bis" )          return enumColor::eColorOrange4Bis;
   else if( stringColor == "orangered1" )          return enumColor::eColorOrangeRed1;
   else if( stringColor == "orchid" )              return enumColor::eColorOrchid;
   else if( stringColor == "orchid1" )             return enumColor::eColorOrchid1;
   else if( stringColor == "orchid2" )             return enumColor::eColorOrchid2;
   else if( stringColor == "palegreen1" )          return enumColor::eColorPaleGreen1;
   else if( stringColor == "palegreen1bis" )       return enumColor::eColorPaleGreen1Bis;
   else if( stringColor == "palegreen3" )          return enumColor::eColorPaleGreen3;
   else if( stringColor == "palegreen3bis" )       return enumColor::eColorPaleGreen3Bis;
   else if( stringColor == "paleturquoise1" )      return enumColor::eColorPaleTurquoise1;
   else if( stringColor == "paleturquoise4" )      return enumColor::eColorPaleTurquoise4;
   else if( stringColor == "palevioletred1" )      return enumColor::eColorPaleVioletRed1;
   else if( stringColor == "pink1" )               return enumColor::eColorPink1;
   else if( stringColor == "pink3" )               return enumColor::eColorPink3;
   else if( stringColor == "plum1" )               return enumColor::eColorPlum1;
   else if( stringColor == "plum2" )               return enumColor::eColorPlum2;
   else if( stringColor == "plum3" )               return enumColor::eColorPlum3;
   else if( stringColor == "plum4" )               return enumColor::eColorPlum4;
   else if( stringColor == "purple" )              return enumColor::eColorPurple;
   else if( stringColor == "purple3" )             return enumColor::eColorPurple3;
   else if( stringColor == "purple4" )             return enumColor::eColorPurple4;
   else if( stringColor == "purple4bis" )          return enumColor::eColorPurple4Bis;
   else if( stringColor == "purplebis" )           return enumColor::eColorPurpleBis;
   else if( stringColor == "red1" )                return enumColor::eColorRed1;
   else if( stringColor == "red3" )                return enumColor::eColorRed3;
   else if( stringColor == "red3bis" )             return enumColor::eColorRed3Bis;
   else if( stringColor == "rosybrown" )           return enumColor::eColorRosyBrown;
   else if( stringColor == "royalblue1" )          return enumColor::eColorRoyalBlue1;
   else if( stringColor == "salmon1" )             return enumColor::eColorSalmon1;
   else if( stringColor == "sandybrown" )          return enumColor::eColorSandyBrown;
   else if( stringColor == "seagreen1" )           return enumColor::eColorSeaGreen1;
   else if( stringColor == "seagreen1bis" )        return enumColor::eColorSeaGreen1Bis;
   else if( stringColor == "seagreen2" )           return enumColor::eColorSeaGreen2;
   else if( stringColor == "seagreen3" )           return enumColor::eColorSeaGreen3;
   else if( stringColor == "skyblue1" )            return enumColor::eColorSkyBlue1;
   else if( stringColor == "skyblue2" )            return enumColor::eColorSkyBlue2;
   else if( stringColor == "skyblue3" )            return enumColor::eColorSkyBlue3;
   else if( stringColor == "slateblue1" )          return enumColor::eColorSlateBlue1;
   else if( stringColor == "slateblue3" )          return enumColor::eColorSlateBlue3;
   else if( stringColor == "slateblue3bis" )       return enumColor::eColorSlateBlue3Bis;
   else if( stringColor == "springgreen1" )        return enumColor::eColorSpringGreen1;
   else if( stringColor == "springgreen2" )        return enumColor::eColorSpringGreen2;
   else if( stringColor == "springgreen2bis" )     return enumColor::eColorSpringGreen2Bis;
   else if( stringColor == "springgreen3" )        return enumColor::eColorSpringGreen3;
   else if( stringColor == "springgreen3bis" )     return enumColor::eColorSpringGreen3Bis;
   else if( stringColor == "springgreen4" )        return enumColor::eColorSpringGreen4;
   else if( stringColor == "steelblue" )           return enumColor::eColorSteelBlue;
   else if( stringColor == "steelblue1" )          return enumColor::eColorSteelBlue1;
   else if( stringColor == "steelblue1bis" )       return enumColor::eColorSteelBlue1Bis;
   else if( stringColor == "steelblue3" )          return enumColor::eColorSteelBlue3;
   else if( stringColor == "tan" )                 return enumColor::eColorTan;
   else if( stringColor == "thistle1" )            return enumColor::eColorThistle1;
   else if( stringColor == "thistle3" )            return enumColor::eColorThistle3;
   else if( stringColor == "turquoise2" )          return enumColor::eColorTurquoise2;
   else if( stringColor == "turquoise4" )          return enumColor::eColorTurquoise4;
   else if( stringColor == "violet")               return enumColor::eColorViolet;
   else if( stringColor == "wheat1")               return enumColor::eColorWheat1;
   else if( stringColor == "wheat4")               return enumColor::eColorWheat4;
   else if( stringColor == "yellow1")              return enumColor::eColorYellow1;
   else if( stringColor == "yellow2")              return enumColor::eColorYellow2;
   else if( stringColor == "yellow3")              return enumColor::eColorYellow3;
   else if( stringColor == "yellow3Bis")           return enumColor::eColorYellow3Bis;
   else if( stringColor == "yellow4")              return enumColor::eColorYellow4;
   else if( stringColor == "blueviolet")           return enumColor::eColorYellow4Bis;

   return 0;
}


_GD_CONSOLE_END