#include "stdafx.h"

/*
* Example of a Windows OpenGL program.
* The OpenGL code is the same as that used in
* the X Window System sample
*/
#include <rgdk/math.hpp>
#include <rgdk/scope_exit.hpp>

#include <windows.h> 
#include <GL/gl.h> 
#include <GL/glu.h> 

#include <cmath>
#include <iostream>

/* Windows globals, defines, and prototypes */
CHAR szAppName[] = "Win OpenGL";
HDC   ghDC;
HGLRC ghRC;

#define SWAPBUFFERS SwapBuffers(ghDC) 
#define BLACK_INDEX     0
#define RED_INDEX       13
#define GREEN_INDEX     14
#define BLUE_INDEX      16
#define WIDTH           800
#define HEIGHT          600

LRESULT WINAPI MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL bSetupPixelFormat(HDC);

/* OpenGL globals, defines, and prototypes */
GLfloat latitude, longitude, latinc, longinc;
GLdouble radius;

#define GLOBE    1 
#define CYLINDER 2 
#define CONE     3 

GLvoid resize(GLsizei, GLsizei);
GLvoid initializeGL(GLsizei, GLsizei);
GLvoid drawScene(DWORD);
void polarView(GLdouble, GLdouble, GLdouble, GLdouble);

class OpenGLTestWindow
{
   HWND m_hWnd;
public:

   OpenGLTestWindow() : m_hWnd() { }

   void Create()
   {
      static WindowClass windowClass;

      CreateWindow(
         szAppName,
         "Generic OpenGL Sample",
         WS_OVERLAPPED,
         CW_USEDEFAULT,
         CW_USEDEFAULT,
         WIDTH,
         HEIGHT,
         NULL,
         NULL,
         NULL,
         this);

      ShowWindow(m_hWnd, SW_SHOWNORMAL);
      UpdateWindow(m_hWnd);
   }

private:

   class WindowClass
   {
   public:
      WindowClass()
      {
         WNDCLASSEX wndclass{};

         wndclass.cbSize = sizeof(WNDCLASSEX);
         wndclass.style = 0;
         wndclass.lpfnWndProc = MainWndProc;
         wndclass.cbClsExtra = 0;
         wndclass.cbWndExtra = 0;
         wndclass.hInstance = NULL;
         wndclass.hIcon = LoadIcon(NULL, szAppName);
         wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
         wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
         wndclass.lpszMenuName = szAppName;
         wndclass.lpszClassName = szAppName;

         if (!RegisterClassEx(&wndclass))
            throw std::exception();
      }

      ~WindowClass()
      {
         std::cout << "Unregistering class: " << szAppName << std::endl;
         UnregisterClass(szAppName, NULL);
      }
   };

   static LRESULT WINAPI MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
   {
      auto onCreate = [](HWND hWnd, const CREATESTRUCT& createStruct)
      {
         auto window = reinterpret_cast<OpenGLTestWindow*>(createStruct.lpCreateParams);
         window->m_hWnd = hWnd;
         SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
         return window;
      };
      auto window = uMsg == WM_CREATE ?
         onCreate(hWnd, *reinterpret_cast<const CREATESTRUCT*>(lParam)) :
         reinterpret_cast<OpenGLTestWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

      std::cout << "msg: " << uMsg << " window: " << window << std::endl;

      auto result = window ?
         window->WindowProc(uMsg, wParam, lParam) :
         DefWindowProc(hWnd, uMsg, wParam, lParam);

      if (uMsg == WM_DESTROY)
      {
         SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
      }
      return result;
   }

   virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
   {
      LRESULT result = true;
      RECT rect;

      switch (uMsg)
      {
      case WM_CREATE:
      {
         ghDC = GetDC(m_hWnd);
         if (!bSetupPixelFormat(ghDC))
            PostQuitMessage(0);

         ghRC = wglCreateContext(ghDC);
         wglMakeCurrent(ghDC, ghRC);
         GetClientRect(m_hWnd, &rect);
         initializeGL(rect.right, rect.bottom);
      }
      break;

      case WM_PAINT:
      {
         PAINTSTRUCT ps;
         BeginPaint(m_hWnd, &ps);
         EndPaint(m_hWnd, &ps);
      }
      break;

      case WM_SIZE:
         GetClientRect(m_hWnd, &rect);
         resize(rect.right, rect.bottom);
         break;

      case WM_CLOSE:
         if (ghRC)
            wglDeleteContext(ghRC);
         if (ghDC)
            ReleaseDC(m_hWnd, ghDC);
         ghRC = 0;
         ghDC = 0;

         DestroyWindow(m_hWnd);
         break;

      case WM_DESTROY:
         if (ghRC)
            wglDeleteContext(ghRC);
         if (ghDC)
            ReleaseDC(m_hWnd, ghDC);

         PostQuitMessage(0);
         break;

      case WM_KEYDOWN:
         switch (wParam)
         {
         case VK_ESCAPE:
            DestroyWindow(m_hWnd);
            break;
         }
         break;

      default:
         result = DefWindowProc(m_hWnd, uMsg, wParam, lParam);
         break;
      }
      return result;
   }

};

int main()
{
   OpenGLTestWindow window;
   window.Create();

   for (;;)
   {
      MSG msg { };
      while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
      {
         if (GetMessage(&msg, NULL, 0, 0))
         {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
         }
         else
         {
            return TRUE;
         }
      }
      drawScene(GetTickCount());
      Sleep(5);
   }
}

BOOL bSetupPixelFormat(HDC hdc)
{
   PIXELFORMATDESCRIPTOR pfd;
   pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
   pfd.nVersion = 1;
   pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.dwLayerMask = PFD_MAIN_PLANE;
   pfd.iPixelType = PFD_TYPE_COLORINDEX;
   pfd.cColorBits = 8;
   pfd.cDepthBits = 16;
   pfd.cAccumBits = 0;
   pfd.cStencilBits = 0;

   const int pixelformat = ChoosePixelFormat(hdc, &pfd);

   return pixelformat && SetPixelFormat(hdc, pixelformat, &pfd);
}

/* OpenGL code */

GLvoid resize(GLsizei width, GLsizei height)
{
   GLfloat aspect;

   glViewport(0, 0, width, height);

   aspect = (GLfloat)width / height;

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0, aspect, 3.0, 7.0);
   glMatrixMode(GL_MODELVIEW);
}

GLvoid createObjects()
{
   GLUquadricObj *quadObj;

   glNewList(GLOBE, GL_COMPILE);
   quadObj = gluNewQuadric();
   gluQuadricDrawStyle(quadObj, GLU_LINE);
   gluSphere(quadObj, 1.5, 16, 16);
   glEndList();

   glNewList(CONE, GL_COMPILE);
   quadObj = gluNewQuadric();
   gluQuadricDrawStyle(quadObj, GLU_LINE);
   gluQuadricNormals(quadObj, GLU_SMOOTH);
   gluCylinder(quadObj, 0.3, 0.0, 0.6, 15, 10);
   glEndList();

   glNewList(CYLINDER, GL_COMPILE);
   glPushMatrix();
   glRotatef((GLfloat)90.0, (GLfloat)1.0, (GLfloat)0.0, (GLfloat)0.0);
   glTranslatef((GLfloat)0.0, (GLfloat)0.0, (GLfloat)-1.0);
   quadObj = gluNewQuadric();
   gluQuadricDrawStyle(quadObj, GLU_LINE);
   gluQuadricNormals(quadObj, GLU_SMOOTH);
   gluCylinder(quadObj, 0.3, 0.3, 0.6, 12, 4);
   glPopMatrix();
   glEndList();
}

GLvoid initializeGL(GLsizei width, GLsizei height)
{
   GLfloat     maxObjectSize, aspect;
   GLdouble    near_plane, far_plane;

   glClearIndex((GLfloat)BLACK_INDEX);
   glClearDepth(1.0);

   glEnable(GL_DEPTH_TEST);

   glMatrixMode(GL_PROJECTION);
   aspect = (GLfloat)width / height;
   gluPerspective(45.0, aspect, 3.0, 7.0);
   glMatrixMode(GL_MODELVIEW);

   near_plane = 3.0;
   far_plane = 7.0;
   maxObjectSize = 3.0F;
   radius = near_plane + maxObjectSize / 2.0;

   latitude = 0.0F;
   longitude = 0.0F;
   latinc = 6.0F;
   longinc = 2.5F;

   createObjects();
}

void polarView(GLdouble radius, GLdouble twist, GLdouble latitude, GLdouble longitude)
{
   glTranslated(0.0, 0.0, -radius);
   glRotated(-90.f, 1.0, 0.0, 0.0);
   glRotated(twist, 0.0, 0.0, 1.0);
   glRotated(latitude, 1.0, 0.0, 0.0);
   glRotated(longitude, 0.0, 0.0, 1.0);
}

GLvoid drawScene(DWORD msek)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glPushMatrix();
   rgdk::scope_exit restoreMatrix = glPopMatrix;

   //auto twist = 0; //360.f * (msek % 3000) / 3000; //15.f + 30.f * std::sin(2 * rgdk::math::pi<float> * (msek % 7000) / 7000);
   auto twist = 360.f * (msek % 10000) / 10000;
   auto latitude = 15.f + 30.f * std::sin(2 * rgdk::math::pi<float> * (msek % 7000) / 7000);
   auto longitude = -360.f * (msek % 3000) / 3000;

   polarView(radius, twist, latitude, longitude);

   //glIndexi(RED_INDEX);
   glColor3ub(255, 127, 0);
   glCallList(CONE);

   //glIndexi(BLUE_INDEX);
   glColor3ub(0, 255, 127);
   glCallList(GLOBE);

   //glIndexi(GREEN_INDEX);
   glColor3ub(255, 0, 127);
   glTranslatef(0.8F, -0.65F, 0.0F);
   glRotatef(30.0F, 1.0F, 0.5F, 1.0F);
   glCallList(CYLINDER);

   SWAPBUFFERS;
}