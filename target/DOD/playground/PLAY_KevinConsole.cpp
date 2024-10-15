#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"

#include <chrono>
#include <thread>

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

void MoveCursor(int iRow, int iColumn)
{
   std::cout << "\033[" << iRow << ";" << iColumn << "H";
}

class Device
{
public:

   Device()
   {
      m_uWidth = 0;
      m_uRow = 0;
   }
   
   Device(unsigned int uWidth, unsigned int uRow)
   {
      m_uWidth = uWidth;
      m_uRow = uRow;

      m_vectorDevice.resize(m_uWidth * m_uRow);
   }

   void Fill(char cCharacter)
   {
      for (int i = 0; i < m_vectorDevice.size(); i++)
      {
         m_vectorDevice[i] = cCharacter;
      }
   }

   std::string ToString()
   {
      std::string stringDevice;

      for (int iRow = 0; iRow < m_uRow; iRow++)
      {
         for (int iColumn = 0; iColumn < m_uWidth; iColumn++)
         {
            char cFetch = m_vectorDevice[iRow * m_uWidth + iColumn];
            stringDevice += cFetch;
         }
         stringDevice += "\n";
      }
      return stringDevice;
   }

   unsigned int m_uWidth;
   unsigned int m_uRow;

   std::vector<char> m_vectorDevice;
};


TEST_CASE(" [kevin] 02", "[kevin] ")
{
   Device device(20, 10);

   device.Fill('U');

   std::cout << device.ToString() << std::endl;

   std::cout << "Hello\n";


   

}