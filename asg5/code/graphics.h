// $Id: graphics.h,v 1.2 2019-03-19 16:18:22-07 - - $
// By: Zhuoxuan Wang (zwang437@ucsc.edu)
// and Xiong Lou (xlou2@ucsc.edu)

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <memory>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "rgbcolor.h"
#include "shape.h"

class object {
   private:
      shared_ptr<shape> pshape;
      vertex center;
      rgbcolor color;

   public:
      object (shared_ptr<shape>, vertex, rgbcolor);
      bool selected  = false;
      void draw();
      void draw_border();
      void move (GLfloat delta_x, GLfloat delta_y,
              int window__w, int window_h);
};

class mouse {
      friend class window;
   private:
      int xpos {0};
      int ypos {0};
      int entered {GLUT_LEFT};
      int left_state {GLUT_UP};
      int middle_state {GLUT_UP};
      int right_state {GLUT_UP};
   private:
      void set (int x, int y) { xpos = x; ypos = y; }
      void state (int button, int state);
      void draw();
};


class window {
      friend class mouse;
   private:
      static int width;         // in pixels
      static int height;        // in pixels
      static vector<object> objects;
      static size_t selected_obj;
      static mouse mus;
      static int thickness;
      static rgbcolor border;
      static int moveby;
   private:
      static void close();
      static void entry (int mouse_entered);
      static void display();
      static void reshape (int width, int height);
      static void keyboard (GLubyte key, int, int);
      static void special (int key, int, int);
      static void motion (int x, int y);
      static void passivemotion (int x, int y);
      static void mousefn (int button, int state, int x, int y);
      static void move_selected_object(int x, int y);
      static void select_object(std::size_t obj);
   public:
      static void push_back (const object& obj) {
                  objects.push_back (obj); }
      static void setWidth (int width_) { width = width_; }
      static void setHeight (int height_) { height = height_; }
      static int getWidth () { return width; }
      static int getHeight() { return height; }
      static void setThickness (int thickness_) { thickness = thickness_; };
      static int getThickness () { return thickness; };
      static void setMoveby (int moveby_) { moveby = moveby_; };
      static void main();
};

#endif

