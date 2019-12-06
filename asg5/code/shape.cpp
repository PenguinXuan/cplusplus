// $Id: shape.cpp,v 1.2 2019-02-28 15:24:20-08 - - $
// By: Zhuoxuan Wang (zwang437@ucsc.edu)
// and Xiong Lou (xlou2@ucsc.edu)

#include <typeinfo>
#include <unordered_map>
#include <cmath>
using namespace std;

#include "shape.h"
#include "util.h"
#include "graphics.h"

static unordered_map<void*,string> fontname {
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

static unordered_map<string,void*> fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

void* find_fontcode(const string& font) {
    auto f = fontcode.find(font);
    if (f == fontcode.end()) {
        throw runtime_error (font + ": no such font");
    }
    return f->second;
}

ostream& operator<< (ostream& out, const vertex& where) {
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape() {
   DEBUGF ('c', this);
}

text::text (void* glut_bitmap_font_, const string& textdata_):
      glut_bitmap_font(glut_bitmap_font_), textdata(textdata_) {
   DEBUGF ('c', this);
}

ellipse::ellipse (GLfloat width, GLfloat height):
          dimension ({width, height}) {
   DEBUGF ('c', this);
}

circle::circle (GLfloat diameter): ellipse (diameter, diameter) {
   DEBUGF ('c', this);
}


polygon::polygon (const vertex_list& vertices_): vertices(vertices_) {
   DEBUGF ('c', this);
}

rectangle::rectangle (GLfloat width, GLfloat height):
            polygon({{0,0}, {0, height},
                     {width, height}, {width, 0}}) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square (GLfloat width): rectangle (width, width) {
   DEBUGF ('c', this);
}

diamond::diamond (const GLfloat width, const GLfloat height):
            polygon({ {height / 2, 0}, {0, width / 2},
                      {-height / 2, 0}, {0, -width / 2} }) {
    DEBUGF ('c', this);

}
triangle::triangle (const vertex_list &vertices):
            polygon({vertices}){
   DEBUGF ('c', this);

}

equilateral::equilateral(const GLfloat width):
          triangle({{0, 0}, {width, 0}, {width / 2, width}}){
    DEBUGF ('c', this);
}


void text::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   auto text = reinterpret_cast<const GLubyte*> (textdata.c_str());
   glColor3ubv (color.ubvec);
   glRasterPos2f(center.xpos,  center.ypos);
   glutBitmapString(glut_bitmap_font, text);
}

void text::draw_border (const vertex& center, const rgbcolor& color) const {
    DEBUGF ('d', this << "(" << center << "," << color << ")");
}

void ellipse::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   const GLfloat delta = 2.0 * M_PI / 32.0;
   glBegin(GL_POLYGON);
   glColor3ubv(color.ubvec);
   for (GLfloat theta = 0; theta < 2.0 * M_PI; theta += delta) {
       GLfloat xpos = dimension.xpos / 2 * cos (theta) + center.xpos;
       GLfloat ypos = dimension.ypos / 2 * sin (theta) + center.ypos;
       glVertex2f(xpos, ypos);
   }
   glEnd();

}

void ellipse::draw_border (const vertex& center, const rgbcolor& color) const {
    DEBUGF ('d', this << "(" << center << "," << color << ")");
    const GLfloat delta = 2.0 * M_PI / 32.0;
    glLineWidth(window::getThickness());
    glBegin(GL_LINE_LOOP);
    glColor3ubv(color.ubvec);
    for (GLfloat theta = 0; theta < 2.0 * M_PI; theta += delta) {
        GLfloat xpos = dimension.xpos / 2 * cos (theta) + center.xpos;
        GLfloat ypos = dimension.ypos / 2 * sin (theta) + center.ypos;
        glVertex2f(xpos, ypos);
    }
    glEnd();
}

void polygon::draw (const vertex& center, const rgbcolor& color) const {
    DEBUGF ('d', this << "(" << center << "," << color << ")");
    glBegin(GL_POLYGON);
    glColor3ubv(color.ubvec);
    for (const auto &v : vertices) {
        glVertex2f(v.xpos + center.xpos, v.ypos + center.ypos);
    }
    glEnd();
}

void polygon::draw_border (const vertex& center, const rgbcolor& color) const {
    DEBUGF ('d', this << "(" << center << "," << color << ")");
    glBegin(GL_LINE_LOOP);
    glColor3ubv(color.ubvec);
    for (const auto &v : vertices) {
        glVertex2f(v.xpos + center.xpos, v.ypos + center.ypos);
    }
    glEnd();
}

void shape::show (ostream& out) const {
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
   shape::show (out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
   obj.show (out);
   return out;
}

