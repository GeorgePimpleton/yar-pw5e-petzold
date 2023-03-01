/*-------------------------------------------------
   CHECKER4.C -- Mouse Hit-Test Demo Program No. 4
             (c) Charles Petzold, 1998
  -------------------------------------------------*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#define DIVISIONS 5

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ChildWndProc(HWND, UINT, WPARAM, LPARAM);

int    idFocus    = 0;
PCWSTR childClass = L"Checker4_Child";

int WINAPI wWinMain(_In_     HINSTANCE instance,
                    _In_opt_ HINSTANCE prevInstance,
                    _In_     PTSTR     cmdLine,
                    _In_     int       showCmd)
{
   UNREFERENCED_PARAMETER(prevInstance);
   UNREFERENCED_PARAMETER(cmdLine);

   static PCWSTR  appName = L"Checker4";
   HWND           hwnd;
   MSG            msg;
   WNDCLASSW      wc;

   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc   = WndProc;
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = 0;
   wc.hInstance     = instance;
   wc.hIcon         = (HICON)   LoadImageW(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
   wc.hCursor       = (HCURSOR) LoadImageW(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED);
   wc.hbrBackground = (HBRUSH)  (COLOR_WINDOW + 1);
   wc.lpszMenuName  = NULL;
   wc.lpszClassName = appName;

   if ( !RegisterClassW(&wc) )
   {
      MessageBoxW(NULL, L"Program requires Windows NT!",
                  appName, MB_ICONERROR);
      return 0;
   }

   wc.lpfnWndProc   = ChildWndProc;
   wc.cbWndExtra    = sizeof(long);
   wc.hIcon         = NULL;
   wc.lpszClassName = childClass;

   RegisterClassW(&wc);

   hwnd = CreateWindowW(appName, L"Checker4 Mouse Hit-Test Demo",
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        NULL, NULL, instance, NULL);

   ShowWindow(hwnd, showCmd);
   UpdateWindow(hwnd);

   while ( GetMessageW(&msg, NULL, 0, 0) )
   {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
   }
   return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   static HWND hwndChild[ DIVISIONS ][ DIVISIONS ];
   int         cxBlock;
   int         cyBlock;
   int         x;
   int         y;

   switch ( message )
   {
   case WM_CREATE:
      for ( x = 0; x < DIVISIONS; x++ )
         for ( y = 0; y < DIVISIONS; y++ )
            hwndChild[ x ][ y ] = CreateWindowW(childClass, NULL,
                                                WS_CHILDWINDOW | WS_VISIBLE,
                                                0, 0, 0, 0,
                                                hwnd, (HMENU) (y << 8 | x),
                                                (HINSTANCE) GetWindowLongPtrW(hwnd, GWLP_HINSTANCE),
                                                NULL);
      return 0;

   case WM_SIZE:
      cxBlock = LOWORD(lParam) / DIVISIONS;
      cyBlock = HIWORD(lParam) / DIVISIONS;

      for ( x = 0; x < DIVISIONS; x++ )
         for ( y = 0; y < DIVISIONS; y++ )
            MoveWindow(hwndChild[ x ][ y ],
                       x * cxBlock, y * cyBlock,
                       cxBlock, cyBlock, TRUE);
      return 0;

   case WM_LBUTTONDOWN:
      MessageBeep(0);
      return 0;

      // on set-focus message, set focus to child window
   case WM_SETFOCUS:
      SetFocus(GetDlgItem(hwnd, idFocus));
      return 0;

      // on key-down message, possibly change the focus window
   case WM_KEYDOWN:
      x = idFocus & 0xFF;
      y = idFocus >> 8;

      switch ( wParam )
      {
      case VK_UP:    y--;                    break;
      case VK_DOWN:  y++;                    break;
      case VK_LEFT:  x--;                    break;
      case VK_RIGHT: x++;                    break;
      case VK_HOME:  x = y = 0;              break;
      case VK_END:   x = y = DIVISIONS - 1;  break;
      default:       return 0;
      }

      x = (x + DIVISIONS) % DIVISIONS;
      y = (y + DIVISIONS) % DIVISIONS;

      idFocus = y << 8 | x;

      SetFocus(GetDlgItem(hwnd, idFocus));
      return 0;

   case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
   }

   return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK ChildWndProc(HWND hwnd, UINT message,
                              WPARAM wParam, LPARAM lParam)
{
   HDC         hdc;
   PAINTSTRUCT ps;
   RECT        rect;

   switch ( message )
   {
   case WM_CREATE:
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);       // on/off flag
      return 0;

   case WM_KEYDOWN:
      // send most key presses to the parent window
      if ( wParam != VK_RETURN && wParam != VK_SPACE )
      {
         SendMessageW(GetParent(hwnd), message, wParam, lParam);
         return 0;
      }
      // For Return and Space, to toggle the square
      // falls through

   case WM_LBUTTONDOWN:
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, 1 ^ GetWindowLongPtr(hwnd, GWLP_USERDATA));
      SetFocus(hwnd);
      InvalidateRect(hwnd, NULL, FALSE);
      return 0;

      // for focus messages, invalidate the window for repaint

   case WM_SETFOCUS:
      idFocus = (int) GetWindowLongPtrW(hwnd, GWLP_ID);

      // fall through

   case WM_KILLFOCUS:
      InvalidateRect(hwnd, NULL, TRUE);
      return 0;

   case WM_PAINT:
      hdc = BeginPaint(hwnd, &ps);

      GetClientRect(hwnd, &rect);
      Rectangle(hdc, 0, 0, rect.right, rect.bottom);

      // draw the "x" mark
      if ( GetWindowLongPtrW(hwnd, GWLP_USERDATA) )
      {
         MoveToEx(hdc, 0, 0, NULL);
         LineTo(hdc, rect.right, rect.bottom);
         MoveToEx(hdc, 0, rect.bottom, NULL);
         LineTo(hdc, rect.right, 0);
      }

      // draw the "focus" rectangle
      if ( hwnd == GetFocus() )
      {
         rect.left   += rect.right / 10;
         rect.right  -= rect.left;
         rect.top    += rect.bottom / 10;
         rect.bottom -= rect.top;

         SelectObject(hdc, GetStockObject(NULL_BRUSH));
         SelectObject(hdc, CreatePen(PS_DASH, 0, 0));
         Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
         DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
      }

      EndPaint(hwnd, &ps);
      return 0;
   }

   return DefWindowProcW(hwnd, message, wParam, lParam);
}