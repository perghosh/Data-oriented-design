#pragma once
#include <cassert>


#ifndef _GD_CONSOLE_BEGIN
   #define _GD_CONSOLE_BEGIN namespace gd { namespace console {
   #define _GD_CONSOLE_END } }
   _GD_CONSOLE_BEGIN
#else
   _GD_CONSOLE_BEGIN
#endif

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


_GD_CONSOLE_END