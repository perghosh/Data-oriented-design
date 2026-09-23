#pragma once

#include <vector>

#include "window.h"

enum enumShape
{
   eLine,
   eRectangle,
   eEllipse
};

struct CShape
{
   POINT pointFirst;
   POINT pointSecond;
   enumShape eShape;
};

class CDrawWindow : public CWindow
{
public:
   CDrawWindow();

private:
   void OnKeyDown(UINT uKey, UINT);
   void OnLButtonDown(UINT, POINT point);

   void OnPaint();
   
   void OnDestroy() { PostQuitMessage(0); }

public:
   enumShape m_eShape = eLine;
   bool m_bHasFirst = false;
   POINT m_pointFirst = {};
   std::vector<CShape> m_vectorShape;
};
