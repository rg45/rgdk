#include "stdafx.h"

#include <rgdk/math.hpp>
#include <rgdk/scope_exit.hpp>
#include <rgdk/type_info.hpp>

#include <windows.h> 
#include <GL/gl.h> 
#include <GL/glu.h> 

#include <cmath>
#include <iostream>

namespace OpenGLTest
{
class OpenGLTestWindow
{
public:

   OpenGLTestWindow()
   : m_hWnd()
   , m_hdc()
   , m_hrc()
   , m_fov(45.0)
   , m_near_plane(3.0)
   , m_far_plane(7.0)
   , m_maxObjectSize(3.0)
   , m_radius(m_near_plane + m_maxObjectSize / 2.0)
   , m_phase(rand())
   {
   }

   void create()
   {
      static WindowClass windowClass;

      CreateWindow(
         m_class_name.c_str(),
         "Generic OpenGL Sample",
         WS_OVERLAPPEDWINDOW,
         CW_USEDEFAULT,
         CW_USEDEFAULT,
         800,
         600,
         nullptr,
         nullptr,
         nullptr,
         this);

      ShowWindow(m_hWnd, SW_SHOWNORMAL);
      UpdateWindow(m_hWnd);
   }

   GLvoid draw(DWORD msek)
   {
      msek += m_phase;

      wglMakeCurrent(m_hdc, m_hrc);
      rgdk::scope_exit atExit = []() { wglMakeCurrent(nullptr, nullptr); };

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glPushMatrix();
      rgdk::scope_exit restoreMatrix = glPopMatrix;

      auto twist = 360.f * (msek % 10000) / 10000;
      auto latitude = 15.f + 30.f * std::sin(2 * rgdk::math::pi<float> * (msek % 7000) / 7000);
      auto longitude = -360.f * (msek % 3000) / 3000;

      polarView(twist, latitude, longitude);

      //glIndexi(RED_INDEX);
      glColor3ub(255, 127, 0);
      glCallList(GLuint(PrimitiveIndex::CONE));

      //glIndexi(BLUE_INDEX);
      glColor3ub(0, 255, 127);
      glCallList(GLuint(PrimitiveIndex::GLOBE));

      //glIndexi(GREEN_INDEX);
      glColor3ub(255, 0, 127);
      glTranslatef(0.8F, -0.65F, 0.0F);
      glRotatef(30.0F, 1.0F, 0.5F, 1.0F);
      glCallList(GLuint(PrimitiveIndex::CYLINDER));

      SwapBuffers(m_hdc);
   }

private:

   enum class PrimitiveIndex : GLuint
   {
      GLOBE = 1,
      CYLINDER = 2,
      CONE = 3
   };

   class WindowClass
   {
   public:
      WindowClass()
      {
         WNDCLASS wndclass { };

         wndclass.lpfnWndProc = MainWndProc;
         wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
         wndclass.lpszClassName = m_class_name.c_str();

         if (!RegisterClass(&wndclass))
            throw std::runtime_error("Could not register window class: '" + m_class_name + "'");
      }

      ~WindowClass()
      {
         std::cout << "Unregistering class: " << m_class_name.c_str() << std::endl;
         UnregisterClass(m_class_name.c_str(), nullptr);
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
         m_hdc = GetDC(m_hWnd);
         if (!bSetupPixelFormat(m_hdc))
            PostQuitMessage(0);

         m_hrc = wglCreateContext(m_hdc);
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
         if (m_hrc)
            wglDeleteContext(m_hrc);
         if (m_hdc)
            ReleaseDC(m_hWnd, m_hdc);
         m_hrc = 0;
         m_hdc = 0;

         DestroyWindow(m_hWnd);
         break;

      case WM_DESTROY:
         if (m_hrc)
            wglDeleteContext(m_hrc);
         if (m_hdc)
            ReleaseDC(m_hWnd, m_hdc);

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

   GLvoid resize(GLsizei width, GLsizei height)
   {
      wglMakeCurrent(m_hdc, m_hrc);
      rgdk::scope_exit atExit = []() { wglMakeCurrent(nullptr, nullptr); };

      glViewport(0, 0, width, height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(m_fov, GLfloat(width) / height, m_near_plane, m_far_plane);
      glMatrixMode(GL_MODELVIEW);
   }

   GLvoid createObjects()
   {
      GLUquadricObj *quadObj;

      glNewList(GLuint(PrimitiveIndex::GLOBE), GL_COMPILE);
      quadObj = gluNewQuadric();
      gluQuadricDrawStyle(quadObj, GLU_LINE);
      gluSphere(quadObj, 1.5, 16, 16);
      glEndList();

      glNewList(GLuint(PrimitiveIndex::CONE), GL_COMPILE);
      quadObj = gluNewQuadric();
      gluQuadricDrawStyle(quadObj, GLU_LINE);
      gluQuadricNormals(quadObj, GLU_SMOOTH);
      gluCylinder(quadObj, 0.3, 0.0, 0.6, 15, 10);
      glEndList();

      glNewList(GLuint(PrimitiveIndex::CYLINDER), GL_COMPILE);
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
      wglMakeCurrent(m_hdc, m_hrc);
      rgdk::scope_exit atExit = []() { wglMakeCurrent(nullptr, nullptr); };

      glClearColor(0.f, 0.f, 0.2f, 0);
      glClearDepth(1.0);
      glEnable(GL_DEPTH_TEST);
      glMatrixMode(GL_PROJECTION);
      gluPerspective(m_fov, GLdouble(width) / height, m_near_plane, m_far_plane);
      glMatrixMode(GL_MODELVIEW);
      createObjects();
   }

   void polarView(GLdouble twist, GLdouble latitude, GLdouble longitude)
   {
      glTranslated(0.0, 0.0, -m_radius);
      glRotated(-90.f, 1.0, 0.0, 0.0);
      glRotated(twist, 0.0, 0.0, 1.0);
      glRotated(latitude, 1.0, 0.0, 0.0);
      glRotated(longitude, 0.0, 0.0, 1.0);
   }

private:
   static const std::string m_class_name;

   HWND m_hWnd;
   HDC   m_hdc;
   HGLRC m_hrc;
   GLdouble m_fov;
   GLdouble m_near_plane;
   GLdouble m_far_plane;
   GLdouble m_maxObjectSize;
   GLdouble m_radius;
   DWORD m_phase;
};

const std::string OpenGLTestWindow::m_class_name = rgdk::type_name<OpenGLTestWindow>();

void main()
{
   PRINT(rgdk::type_name<OpenGLTestWindow>().c_str());

   OpenGLTestWindow windows[5];
   for (auto&& window : windows)
   {
      window.create();
   }

   for (;;)
   {
      MSG msg{};
      while (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE))
      {
         if (GetMessage(&msg, nullptr, 0, 0))
         {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
         }
         else
         {
            return;
         }
      }
      for (auto&& window : windows)
      {
         window.draw(GetTickCount());
      }
      Sleep(2);
   }
}

} // namespace OpenGLTest

int main()
{
   try
   {
      OpenGLTest::main();
   }
   catch (const std::exception& ex)
   {
      std::cout << "[std::exception]: " << ex.what() << std::endl;
   }
   catch (...)
   {
      std::cout << "[ERROR]: An unhandled exception of unknown type." << std::endl;
   }
}