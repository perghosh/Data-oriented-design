#include "Application.h";

void Application::Initialize()
{
   //auto rowcolumn = m_pairPoint;

   //m_deviceGame(250, 100);
   m_deviceGame.create(20, 35);
}

void Application::Draw()
{
   auto pairSize = m_deviceGame.size();
   
   m_deviceGame.print(m_pairPoint.first, m_pairPoint.second, '#');

   if (m_iMove <= 5)
   {
      m_iMove++;
      m_pairPoint.second++;
   }
   

   std::cout << m_deviceGame.render(gd::console::tag_format_cli{});
}