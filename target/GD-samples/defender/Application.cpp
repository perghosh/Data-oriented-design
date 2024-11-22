#include "Application.h";

std::pair<bool, std::string> Application::Initialize()
{
   m_deviceGame.create(20, 35);
   return application::basic::CApplication::Initialize();
}

void Application::Draw()
{
   m_deviceGame.fill(' ');
   
   auto pairSize = m_deviceGame.size();
   
  

   m_deviceGame.print(m_pairPoint.first, m_pairPoint.second, '#');

   if (m_iMove <= 5)
   {
      m_iMove++;
      m_pairPoint.second++;
   }
   

   std::cout << m_deviceGame.render(gd::console::tag_format_cli{});
}