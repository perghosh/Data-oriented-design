/**
 * @file gd_expression.cpp
 * 
 * @brief 
 * 
 */

#include <cstdint>

#include "gd_expression.h"

_GD_EXPRESSION_BEGIN


/*----------------------------------------------------------------------------- to_precedence
 * Returns precedence level for an operator to be used in shunting yard algorithm
 * Higher values indicate higher precedence
 * @param cOperator the operator character to check
 * @return int precedence level (0 for non-operators)
 */
int to_precedence_g(const char iOperator)
{ 
    switch(iOperator)
    {
        case '^':                                                              // exponentiation
            return 4;
        
        case '*':                                                              // multiplication
        case '/':                                                              // division
        case '%':                                                              // modulo
            return 3;
        
        case '+':                                                              // addition
        case '-':                                                              // subtraction
            return 2;
        
        case '<':                                                              // less than
        case '>':                                                              // greater than
        case '=':                                                              // equality
            return 1;
        
        case '&':                                                              // logical AND
        case '|':                                                              // logical OR
            return 0;
        
        default:                                                               // not an operator
            return 0;
    }
}

/*----------------------------------------------------------------------------- to_precedence
 * Returns precedence level for an operator using lookup table for maximum speed
 * Higher values indicate higher precedence
 * @param cOperator the operator character to check
 * @return int precedence level (0 for non-operators)
 */
int to_precedence_g(const char iOperator, tag_optimize )
{ 
    // Static lookup table initialized once
    static const uint8_t puPrecedenceLookup[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0-15
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 16-31
        0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 3, 2, 0, 2, 0, 3,  // 32-47  (%, *, +, -, /)
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,  // 48-63  (<, =, >)
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 64-79
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0,  // 80-95  (^)
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 96-111
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 112-127
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 128-143
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 144-159
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 160-175
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 176-191
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 192-207
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 208-223
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 224-239
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // 240-255
    };
    
    // Maps: ^ -> 4, *, /, % -> 3, +, - -> 2, <, >, = -> 1, & | -> 0
    // Special case for logical operators that could be double characters
    /*
    if( iOperator == '&' || iOperator == '|')                                     // handle logical operators
    {
        return 0;
    }
    */
    
    return puPrecedenceLookup[static_cast<uint8_t>(iOperator)];                // direct lookup - O(1) time complexity
}

_GD_EXPRESSION_END