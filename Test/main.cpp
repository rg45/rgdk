#include "stdafx.h"

#include <boost/noncopyable.hpp>
#include <functional>
#include <glfw/glfw3.h>
#include <iostream>
#include <memory>

namespace rgdk
{

class ScopeExit : boost::noncopyable
{
public:

   template <typename Action>
   ScopeExit(Action&& action) : m_action(action) { }
   ~ScopeExit() { m_action(); }

private:
   std::function<void()> m_action;
};

} // namespace rgdk

namespace rgdk
{
namespace test
{

class Scene
{
public:

   using Time = double;

   explicit Scene(Time time) : m_time(time) { }

   void Draw(Time)
   {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glPushMatrix();

//      drawBallHow = DRAW_BALL_SHADOW;
//      DrawBoingBall();

      drawGrid();

//      drawBallHow = DRAW_BALL;
//      DrawBoingBall();

      glPopMatrix();
      glFlush();
   }

private:

   void drawGrid()
   {
      static constexpr GLfloat RADIUS = 70.f;
      static constexpr GLfloat DIST_BALL = (RADIUS * 2.f + RADIUS * 0.1f);
      static constexpr GLfloat GRID_SIZE = (RADIUS * 4.5f);

      int              row, col;
      const int        rowTotal = 12;                   /* must be divisible by 2 */
      const int        colTotal = rowTotal;             /* must be same as rowTotal */
      const GLfloat    widthLine = 2.0;                  /* should be divisible by 2 */
      const GLfloat    sizeCell = GRID_SIZE / rowTotal;
      const GLfloat    z_offset = -40.0;
      GLfloat          xl, xr;
      GLfloat          yt, yb;

      glPushMatrix();
      glDisable(GL_CULL_FACE);

      /*
      * Another relative Z translation to separate objects.
      */
      glTranslatef(0.0, 0.0, DIST_BALL);

      /*
      * Draw vertical lines (as skinny 3D rectangles).
      */
      for (col = 0; col <= colTotal; col++)
      {
         /*
         * Compute co-ords of line.
         */
         xl = -GRID_SIZE / 2 + col * sizeCell;
         xr = xl + widthLine;

         yt = GRID_SIZE / 2;
         yb = -GRID_SIZE / 2 - widthLine;

         glBegin(GL_POLYGON);

         glColor3f(0.6f, 0.1f, 0.6f);               /* purple */

         glVertex3f(xr, yt, z_offset);       /* NE */
         glVertex3f(xl, yt, z_offset);       /* NW */
         glVertex3f(xl, yb, z_offset);       /* SW */
         glVertex3f(xr, yb, z_offset);       /* SE */

         glEnd();
      }

      /*
      * Draw horizontal lines (as skinny 3D rectangles).
      */
      for (row = 0; row <= rowTotal; row++)
      {
         /*
         * Compute co-ords of line.
         */
         yt = GRID_SIZE / 2 - row * sizeCell;
         yb = yt - widthLine;

         xl = -GRID_SIZE / 2;
         xr = GRID_SIZE / 2 + widthLine;

         glBegin(GL_POLYGON);

         glColor3f(0.6f, 0.1f, 0.6f);               /* purple */

         glVertex3f(xr, yt, z_offset);       /* NE */
         glVertex3f(xl, yt, z_offset);       /* NW */
         glVertex3f(xl, yb, z_offset);       /* SW */
         glVertex3f(xr, yb, z_offset);       /* SE */

         glEnd();
      }

      glPopMatrix();

      return;
   }

/*
   void reshape(GLFWwindow* window, int w, int h)
   {
      mat4x4 projection, view;

      glViewport(0, 0, (GLsizei)w, (GLsizei)h);

      glMatrixMode(GL_PROJECTION);
      mat4x4_perspective(projection,
         2.f * (float)atan2(RADIUS, 200.f),
         (float)w / (float)h,
         1.f, VIEW_SCENE_DIST);
      glLoadMatrixf((const GLfloat*)projection);

      glMatrixMode(GL_MODELVIEW);
      {
         vec3 eye = { 0.f, 0.f, VIEW_SCENE_DIST };
         vec3 center = { 0.f, 0.f, 0.f };
         vec3 up = { 0.f, -1.f, 0.f };
         mat4x4_look_at(view, eye, center, up);
      }
      glLoadMatrixf((const GLfloat*)view);
   }
*/

private:
   Time m_time;
};

void main()
{
   int returnCode = EXIT_FAILURE;
   ScopeExit exit = [&returnCode]()
   {
      std::cout << "Exit: " << returnCode << std::endl;
      ::glfwTerminate();
      ::exit(returnCode);
   };

   if (glfwInit())
   {
      const auto windowSize = 72;
      auto window = glfwCreateWindow(16 * windowSize, 9 * windowSize,
         "Boing (classic Amiga demo) -- " __FUNCSIG__, NULL, NULL);
      if (window)
      {
         glfwSetWindowAspectRatio(window, 16, 9);

         //glfwSetFramebufferSizeCallback(window, reshape);
         //glfwSetKeyCallback(window, key_callback);
         //glfwSetMouseButtonCallback(window, mouse_button_callback);
         //glfwSetCursorPosCallback(window, cursor_position_callback);

         glfwMakeContextCurrent(window);
         //gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
         glfwSwapInterval(1);

         //glfwGetFramebufferSize(window, &width, &height);
         //reshape(window, width, height);

         glfwSetTime(0.0);

         Scene scene(glfwGetTime());
         while (!glfwWindowShouldClose(window))
         {
            scene.Draw(glfwGetTime());

            /* Swap buffers */
            glfwSwapBuffers(window);
            glfwPollEvents();
         }
         returnCode = EXIT_SUCCESS;
      }
   }
}

} // namespace test
} // namespace 


double f(double x)
{
   return (x*x*x - 14*x*x - 49*x - 36) / 3;
}

int main()
{
   try
   {
      rgdk::test::main();
   }
   catch(...)
   {
      std::cout << "An unhandled exception of unknown type" << std::endl;
   }
}

