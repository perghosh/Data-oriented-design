#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

class Position
{
public:

   // Constructors

   Position() 
   {
      m_uX = 0;
      m_uY = 0;
   }

   Position(unsigned int uX, unsigned int uY, std::string stringWord)
   {
      m_uX = uX;
      m_uY = uY;
      m_stringWord = stringWord;
   }

   //Functions

   void Build()
   {
      const unsigned int uSize = 5;
      std::string stringSpace[uSize][uSize];

      for (int i = 0; i < uSize; i++)
      {
         for (int j = 0; j < uSize; j++)
         {
            stringSpace[i][j] = " ";
         }
      }

      if (m_uX < uSize && m_uY < uSize)
      {
         stringSpace[m_uX][m_uY] = m_stringWord;
      }

      for (int i = 0; i < uSize; i++)
      {
         for (int j = 0; j < uSize; j++)
         {
            std::cout << stringSpace[i][j];
         }

         std::cout << std::endl;
      }

   }

   // Member variables

   std::string m_stringWord;
   unsigned int m_uX;
   unsigned int m_uY;
};

TEST_CASE( "[kevin] 01", "[kevin]" ) {
   
   Position position(2, 4, "Hello");

   position.Build();
   
   std::cout << "Hello";

   std::cout << std::endl;
}