#include "WindowApplication.h"






CDrawWindow::CDrawWindow()
{
   // message -> method, the method signature is checked against the message at compile time
   Bind<WM_KEYDOWN>(this, &CDrawWindow::OnKeyDown);
   Bind<WM_LBUTTONDOWN>(this, &CDrawWindow::OnLButtonDown);
   Bind<WM_PAINT>(this, &CDrawWindow::OnPaint);
   Bind<WM_DESTROY>(this, &CDrawWindow::OnDestroy);
}

void CDrawWindow::OnKeyDown(UINT uKey, UINT)
{
   switch (uKey)
   {
   case '1': m_eShape = eLine;      break;
   case '2': m_eShape = eRectangle; break;
   case '3': m_eShape = eEllipse;   break;
   default: break;
   }
}

void CDrawWindow::OnLButtonDown(UINT, POINT point)
{
   if (m_bHasFirst == false)
   {
      m_pointFirst = point;
      m_bHasFirst = true;
   }
   else
   {
      m_vectorShape.push_back({ m_pointFirst, point, m_eShape });
      m_bHasFirst = false;
      Invalidate();
   }
}

void CDrawWindow::OnPaint()
{
   win::CPaint paint(GetHandle());

   HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));
   HBRUSH brushOld = (HBRUSH)SelectObject(paint, brush);

   for (const CShape& shape : m_vectorShape)
   {
      switch (shape.eShape)
      {
      case eLine:
         MoveToEx(paint, shape.pointFirst.x, shape.pointFirst.y, nullptr);
         LineTo(paint, shape.pointSecond.x, shape.pointSecond.y);
         break;
      case eRectangle:
         Rectangle(paint, shape.pointFirst.x, shape.pointFirst.y, shape.pointSecond.x, shape.pointSecond.y);
         break;
      case eEllipse:
         Ellipse(paint, shape.pointFirst.x, shape.pointFirst.y, shape.pointSecond.x, shape.pointSecond.y);
         break;
      }
   }

   SelectObject(paint, brushOld);
   DeleteObject(brush);
}

