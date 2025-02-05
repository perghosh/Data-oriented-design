#include "gd/gd_script_token.h"


// Create an array with one entry per ASCII code (0–255).
// Each value is the bitwise combination of enumTokenGroup flags that apply to that code.
const uint8_t pbToken_s[256] = 
{
   // 0x00 - 0x0F: Control characters
   0,                // 0x00 NUL
   0,                // 0x01 SOH
   0,                // 0x02 STX
   0,                // 0x03 ETX
   0,                // 0x04 EOT
   0,                // 0x05 ENQ
   0,                // 0x06 ACK
   0,                // 0x07 BEL
   0,                // 0x08 BS
   0x10,             // 0x09 TAB   (whitespace)
   0x10,             // 0x0A LF    (newline)
   0,                // 0x0B VT
   0,                // 0x0C FF
   0x10,             // 0x0D CR    (carriage return)
   0,                // 0x0E SO
   0,                // 0x0F SI

   // 0x10 - 0x1F: More control characters
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,

   // 0x20 - 0x2F:
   0x10,         // 0x20 ' '  : Space (whitespace)
   0x04,         // 0x21 '!'  : Symbol
   0x04,         // 0x22 '"'  : Symbol
   0x04,         // 0x23 '#'  : Symbol
   0x04,         // 0x24 '$'  : Symbol
   0x02,         // 0x25 '%'  : Operator (modulo)
   0x04,         // 0x26 '&'  : Symbol
   0x04,         // 0x27 '''  : Symbol (apostrophe)
   0x04,         // 0x28 '('  : Symbol
   0x04,         // 0x29 ')'  : Symbol
   0x02,         // 0x2A '*'  : Operator (multiplication)
   0x02,         // 0x2B '+'  : Operator (addition)
   0x04,         // 0x2C ','  : Symbol
   0x02,         // 0x2D '-'  : Operator (subtraction)
   0x04,         // 0x2E '.'  : Symbol (period)
   0x02,         // 0x2F '/'  : Operator (division)

   // 0x30 - 0x3F:
   0x01,         // 0x30 '0'  : Number
   0x01,         // 0x31 '1'  : Number
   0x01,         // 0x32 '2'  : Number
   0x01,         // 0x33 '3'  : Number
   0x01,         // 0x34 '4'  : Number
   0x01,         // 0x35 '5'  : Number
   0x01,         // 0x36 '6'  : Number
   0x01,         // 0x37 '7'  : Number
   0x01,         // 0x38 '8'  : Number
   0x01,         // 0x39 '9'  : Number
   0x04,         // 0x3A ':'  : Symbol
   0x04,         // 0x3B ';'  : Symbol
   0x02,         // 0x3C '<'  : Operator (less than)
   0x02,         // 0x3D '='  : Operator (equality)
   0x02,         // 0x3E '>'  : Operator (greater than)
   0x04,         // 0x3F '?'  : Symbol

   // 0x40 - 0x4F:
   0x04,         // 0x40 '@'  : Symbol
   0x08,         // 0x41 'A'  : Letter
   0x08,         // 0x42 'B'  : Letter
   0x08,         // 0x43 'C'  : Letter
   0x08,         // 0x44 'D'  : Letter
   0x08,         // 0x45 'E'  : Letter
   0x08,         // 0x46 'F'  : Letter
   0x08,         // 0x47 'G'  : Letter
   0x08,         // 0x48 'H'  : Letter
   0x08,         // 0x49 'I'  : Letter
   0x08,         // 0x4A 'J'  : Letter
   0x08,         // 0x4B 'K'  : Letter
   0x08,         // 0x4C 'L'  : Letter
   0x08,         // 0x4D 'M'  : Letter
   0x08,         // 0x4E 'N'  : Letter
   0x08,         // 0x4F 'O'  : Letter

   // 0x50 - 0x5F:
   0x08,         // 0x50 'P'  : Letter
   0x08,         // 0x51 'Q'  : Letter
   0x08,         // 0x52 'R'  : Letter
   0x08,         // 0x53 'S'  : Letter
   0x08,         // 0x54 'T'  : Letter
   0x08,         // 0x55 'U'  : Letter
   0x08,         // 0x56 'V'  : Letter
   0x08,         // 0x57 'W'  : Letter
   0x08,         // 0x58 'X'  : Letter
   0x08,         // 0x59 'Y'  : Letter
   0x08,         // 0x5A 'Z'  : Letter
   0x04,         // 0x5B '['  : Symbol
   0x04,         // 0x5C '\'  : Symbol (backslash)
   0x04,         // 0x5D ']'  : Symbol
   0x02,         // 0x5E '^'  : Operator (bitwise XOR)
   0x08,         // 0x5F '_'  : Treated as Letter (common in identifiers)

   // 0x60 - 0x6F:
   0x04,         // 0x60 '`'  : Symbol (grave accent)
   0x08,         // 0x61 'a'  : Letter
   0x08,         // 0x62 'b'  : Letter
   0x08,         // 0x63 'c'  : Letter
   0x08,         // 0x64 'd'  : Letter
   0x08,         // 0x65 'e'  : Letter
   0x08,         // 0x66 'f'  : Letter
   0x08,         // 0x67 'g'  : Letter
   0x08,         // 0x68 'h'  : Letter
   0x08,         // 0x69 'i'  : Letter
   0x08,         // 0x6A 'j'  : Letter
   0x08,         // 0x6B 'k'  : Letter
   0x08,         // 0x6C 'l'  : Letter
   0x08,         // 0x6D 'm'  : Letter
   0x08,         // 0x6E 'n'  : Letter
   0x08,         // 0x6F 'o'  : Letter

   // 0x70 - 0x7F:
   0x08,         // 0x70 'p'  : Letter
   0x08,         // 0x71 'q'  : Letter
   0x08,         // 0x72 'r'  : Letter
   0x08,         // 0x73 's'  : Letter
   0x08,         // 0x74 't'  : Letter
   0x08,         // 0x75 'u'  : Letter
   0x08,         // 0x76 'v'  : Letter
   0x08,         // 0x77 'w'  : Letter
   0x08,         // 0x78 'x'  : Letter
   0x08,         // 0x79 'y'  : Letter
   0x08,         // 0x7A 'z'  : Letter
   0x04,         // 0x7B '{'  : Symbol
   0x04,         // 0x7C '|'  : Symbol
   0x04,         // 0x7D '}'  : Symbol
   0x04,         // 0x7E '~'  : Symbol
   0,            // 0x7F DEL: Control character

   // 0x80 - 0xFF: Extended ASCII (not used, so default to 0)
   0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
   0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
   0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
   0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
   0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
   0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
   0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
   0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};
