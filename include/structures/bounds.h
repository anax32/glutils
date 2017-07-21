#ifndef BOUNDS_H
#define BOUNDS_H

#ifndef _LIST_
#include <list>
#endif

#ifndef _ALGORITHM_
#include <algorithm>
#endif

#include <cmath>

namespace UI
{
typedef enum LocationEnum
{
  TOP = 0x01,
  RIGHT = 0x02,
  BOTTOM = 0x04,
  LEFT = 0x10
} Location;
typedef enum ComponentEnum
{
  NOSEL,
  FRAME,
  HEADER,
  HOTSPOT,
  ELEMENT
} Component;
typedef enum AlignmentEnum
{
  DEFAULT,
  MIN,
  MAX,
  CENTER
} Alignment;
typedef enum RotationEnum
{
  ZERO,
  RIGHT90,
  LEFT90,
  INVERT
} Rotation;

class Bounds //: public Patterns::Persistable
{
protected:
  // called after setting the region
  // NB: This is essentially a callback, but without the functional object decoration.
  virtual void updateRegion ()  {}
public:
  /**
   * get a bound as an interleaved gl vertex array.
   * only verts and texcoords at the moment.
   * 16 floats required (4 * [x,y,u,v])
   */
  virtual void glRegion (float *buffer, const unsigned int buflen = 32) const
  {
    if (buflen != (4+4)*4)
    {
      return;
    }

    float  *gld = buffer;

    // x                      // y                       // z                  // w                  // u                  // v                  // s                  //t
    gld[(0*8)+0] = left ();   gld[(0*8)+1] = bottom ();  gld[(0*8)+2] = 0.0f;  gld[(0*8)+3] = 1.0f;  gld[(0*8)+4] = 0.0f;  gld[(0*8)+5] = 0.0f;  gld[(0*8)+6] = 0.0f;  gld[(0*8)+7] = 0.0f;
    gld[(1*8)+0] = right ();  gld[(1*8)+1] = bottom ();  gld[(1*8)+2] = 0.0f;  gld[(1*8)+3] = 1.0f;  gld[(1*8)+4] = 1.0f;  gld[(1*8)+5] = 0.0f;  gld[(1*8)+6] = 0.0f;  gld[(1*8)+7] = 0.0f;
    gld[(2*8)+0] = right ();  gld[(2*8)+1] = top ();     gld[(2*8)+2] = 0.0f;  gld[(2*8)+3] = 1.0f;  gld[(2*8)+4] = 1.0f;  gld[(2*8)+5] = 1.0f;  gld[(2*8)+6] = 0.0f;  gld[(2*8)+7] = 0.0f;
    gld[(3*8)+0] = left ();   gld[(3*8)+1] = top ();     gld[(3*8)+2] = 0.0f;  gld[(3*8)+3] = 1.0f;  gld[(3*8)+4] = 0.0f;  gld[(3*8)+5] = 1.0f;  gld[(3*8)+6] = 0.0f;  gld[(3*8)+7] = 0.0f;
  }

#ifdef __GL_H__
  static GLUtils::BufferDescription  vao;
#endif

private:
  float  pos[4];  //{mn[2], mx[2]}
public:
// STRUCTORS
  Bounds ()
  {
    clear ();
  }
  Bounds (int minx, int miny, int maxx, int maxy)
  {
    pos[0] = static_cast<float>(minx);
    pos[1] = static_cast<float>(miny);
    pos[2] = static_cast<float>(maxx);
    pos[3] = static_cast<float>(maxy);
  }
  Bounds (float minx, float miny, float maxx, float maxy)
  {
    pos[0] = minx;
    pos[1] = miny;
    pos[2] = maxx;
    pos[3] = maxy;
  }
  Bounds (const Bounds& b)
  {
    pos[0] = b.pos[0];
    pos[1] = b.pos[1];
    pos[2] = b.pos[2];
    pos[3] = b.pos[3];
  }
  virtual ~Bounds ()
  {}
// ACCESSORS
#if 1
  /**
   * @returns the horizontal minimum
   */
  float left () const                                 {return pos[0];}
  /**
   * @returns the horizontal maximum
   */
  float right () const                                {return pos[2];}
  /**
   * @returns the vertical minimum
   */
  float top () const                                  {return pos[1];}
  /**
   * @returns the vertical maximum
   */
  float bottom () const                               {return pos[3];}
#else
  float left () const                                 {return std::min (pos[0], pos[2]);}
  float right () const                                {return std::max (pos[0], pos[2]);}
  float top () const                                  {return std::min (pos[1], pos[3]);}
  float bottom () const                               {return std::max (pos[1], pos[3]);}
#endif
  float horzMin () const                               {return std::min (left (), right ());}
  float vertMin () const                               {return std::max (top (), bottom ());}
  float horzMax () const                               {return std::max (left (), right ());}
  float vertMax () const                               {return std::min (top (), bottom ());}
  /**
   * @returns the horizontal distance of the bound region
   */
  float width () const                                 {return (right () - left ());}
  /**
   * @returns the vertical distance of the bound region
   */
  float height () const                                {return (bottom () - top ());}
  /**
   * return the area of the bounds as width () * height ()
   */
  float area () const                                  {return (width () * height ());}
  /**
   * @returns the aspect ratio of the bound region (width ()/height ())
   */
  float aspectRatio () const                           {return (width ()/height ());}
  /**
   * gets the two midpoints of the bound region
   * @param x output to hold the horizontal midpoint
   * @param y output to hold the vertical midpoint
   */
  void center (float& x, float &y) const               {x = left ()+(width ()/2.0f); y = top ()+(height ()/2.0f);}
#ifdef _WIN32_
  /**
   * fills a win32 rect struct with the bound dimensions.
   * @param r win32 rect to fill, if null nothing happens.
   */
  void getRegion (RECT& r) const
  {
    r.left   = static_cast<LONG>(left ());
    r.right  = static_cast<LONG>(right ());
    r.top    = static_cast<LONG>(top ());
    r.bottom = static_cast<LONG>(bottom ());
  }
#endif
  void getRegion (Bounds& b) const
  {
    b.pos[0] = left ();
    b.pos[1] = top ();
    b.pos[2] = right ();
    b.pos[3] = bottom ();
  }
  Rotation orientation () const
  {
    if (aspectRatio () > 1.0f)    // width () > height
    {
      return ZERO;
    }
    else              // height () > width ()
    {
      float  tl[2] = {top (), left ()};
      float  br[2] = {bottom (), right ()};

      if (tl[1] > br[1])
      {
        return LEFT90;
      }
      else
      {
        return RIGHT90;
      }
    }
  //  return ((aspectRatio () > 1.0f)?ZERO:RIGHT90);
  }
  // FIXME: take orientation as a param?
  const float orientedLeft (const Rotation orient) const
  {
    switch (orient)
    {
    case ZERO:    return left ();
    case RIGHT90: return bottom ();
    case LEFT90:  return top ();
    case INVERT:  return right ();
    default:      return left ();
    }
  }
  const float orientedRight (const Rotation orient)  const
  {
    switch (orient)
    {
    case ZERO:    return right ();
    case RIGHT90: return top ();
    case LEFT90:  return bottom ();
    case INVERT:  return left ();
    default:      return right ();
    }
  }
  const float orientedTop (const Rotation orient) const
  {
    switch (orient)
    {
    case ZERO:    return top ();
    case RIGHT90: return right  ();
    case LEFT90:  return left ();
    case INVERT:  return bottom ();
    default:      return top ();
    }
  }
  const float orientedBottom (const Rotation orient) const
  {
    switch (orient)
    {
    case ZERO:    return bottom ();
    case RIGHT90: return left  ();
    case LEFT90:  return right();
    case INVERT:  return top ();
    default:      return bottom ();
    }
  }
  const float orientedWidth (const Rotation orient) const     {return std::abs ((orientedRight (orient) - orientedLeft (orient)));}
  const float orientedHeight (const Rotation orient) const    {return std::abs ((orientedBottom (orient) - orientedTop (orient)));}
  const float longEdgeLength () const                         {return std::max (std::abs (width ()), std::abs (height ()));}
  const float shortEdgeLength () const                        {return std::min (std::abs (width ()), std::abs (height ()));}
  void getRegionOriented (Bounds& b) const
  {
    switch (orientation ())
    {
    case UI::ZERO:    b.setRegion (left (),  right (), bottom (), top ());    break;
    case UI::RIGHT90: b.setRegion (right (), left (),  top (),    bottom ()); break;
    case UI::LEFT90:  b.setRegion (left (),  right (), bottom (), top ());    break;
    case UI::INVERT:  b.setRegion (left (),  right (), top (),    bottom ()); break;
    default:          b.setRegion (0.0f, 1.0f, 1.0f, 1.0f);                   break;
    }
  }
// QUERIES
  /**
   * tests if a point is inside the bound region
   * @param x horizontal location of the point
   * @param y vertical location
   * @returns true if (x,y) is inside the bound region, false otherwise
   */
  const bool pointInside (const float x, const float y) const
  {
    return ((horzMin () <= x) &&
            (x <= horzMax ()) &&
            (vertMax () <= y) &&
            (y <= vertMin ()));
  }
  /**
   * integer overload of pointInside (float).
   * Simply casts the integer argument to float.
   * @returns return value of pointInside (float)
   */
  const bool pointInside (const IntegerSetType x, const IntegerSetType y) const
  {
    return pointInside (static_cast<float>(x),
                        static_cast<float>(y));
  }
  /**
   * maps two co-ordinates to the unit square defined by the bounds.
   * Co-ordinate values should be initialised.
   * @param x horizontal location
   * @param y vertical location
   * @returns nothing, x and y values are modified
   */
  void toUnit (float& x, float& y) const
  {
    x -= left ();
    y -= top ();
    x /= width ();
    y /= height ();
  }
// MUTATORS
  /**
   * @param region bound to copy the parameters from onto this object
   */
  void setRegion (const Bounds& region)
  {
    pos[0] = region.pos[0];
    pos[1] = region.pos[1];
    pos[2] = region.pos[2];
    pos[3] = region.pos[3];

    updateRegion ();
  }
  /**
   * sets this bound to the parameter values
   * @param minx new horizontal minimum
   * @param miny new vertical minimum
   * @param maxx new horizontal maximum
   * @param maxy new vertical maximum
   */
  void setRegion (float minx, float miny, float maxx, float maxy)
  {
    pos[0] = minx;
    pos[1] = miny;
    pos[2] = maxx;
    pos[3] = maxy;

    updateRegion ();  // causes a crash cos this is called in the constructor if gl hasn't been initialised
              // FIXED: by putting ifs all around the gl commands
  }
  /**
   * integer overload of setRegion (float)
   * simply calls setRegion (float) with casts
   */
  void setRegion (int minx, int miny, int maxx, int maxy)
  {
    setRegion (static_cast<float>(minx),
               static_cast<float>(miny),
               static_cast<float>(maxx),
               static_cast<float>(maxy));
  }
  void setRegion (unsigned int minx, unsigned int miny, unsigned int maxx, unsigned int maxy)
  {
    setRegion (static_cast<float>(minx),
               static_cast<float>(maxx),
               static_cast<float>(miny),
               static_cast<float>(maxy));
  }
#ifdef _WIN32_
  /**
   * @param r win32 rect object to fill this bound from, if null, the bound is set to zero
   */
  void setRegion (RECT& r)
  {
    setRegion (r.left, r.top, r.right, r.bottom);
  }
#endif
  /**
   * move the window to an absolute position
   * @param xpos new horizontal position
   * @param ypos new vertical position
   */
  void position (const float xpos, const float ypos)
  {
    const float w = width ();
    const float h = height ();

    pos[0] = xpos;
    pos[1] = ypos;
    pos[2] = left () + w;
    pos[3] = bottom () + h;

    updateRegion ();
  }
  /**
   * move the max corner of the bound (i.e., keep it at the current location,
   * but change the width and height, but specify the values in abs coords).
   * @param xpos new maximum horizontal position
   * @param ypos new maximum vertical position
   */
  void corner (const float xpos, const float ypos)
  {
    pos[2] = xpos;
    pos[3] = ypos;

    updateRegion ();
  }
  /**
   * Keep the bound and the current location, but change the width and height
   * @param width new width
   * @param height new height
   */
  void size (const float width, const float height)
  {
    pos[2] = left () + width;
    pos[3] = top () + height;

    updateRegion ();
  }
  /**
   * move the window by xdist and ydist
   * @param xdist horizontal offset
   * @param ydist vertical offset
   */
  void offset (const float xdist, const float ydist)
  {
    pos[0] += xdist;
    pos[2] += xdist;
    pos[1] += ydist;
    pos[3] += ydist;

    updateRegion ();
  }
  void offsetOriented (const float shortEdgeOffset, const float longEdgeOffset)
  {
    switch (orientation ())
    {
      case UI::ZERO:
      case UI::INVERT:
      {
        offset (shortEdgeOffset, longEdgeOffset);
        break;
      }
      case UI::RIGHT90:
      case UI::LEFT90:
      {
        offset (longEdgeOffset, shortEdgeOffset);
        break;
      }
    }
  }
  /**
   * increase the maximum values by x and y
   * @param x horizontal increase
   * @param y vertical increase
   */
  void enlarge (const float x, const float y)
  {
    pos[2] += x;
    pos[3] += y;
    
    updateRegion ();
  }
  void enlarge (const unsigned int x, const unsigned int y)
  {
    enlarge (static_cast<float>(x), static_cast<float>(y));
  }
  /**
   * scale the bounds by a value (centered about the origin)
   * @param x horizontal scale
   * @param y vertical scale
   */
  void scale (const float x, const float y)
  {
    float       c[2];
    const float w = width ();
    const float h = height ();

    center (c[0], c[1]);

    pos[0] = c[0] - ((w/2.0f)*x);
    pos[2] = c[0] + ((w/2.0f)*x);
    pos[1] = c[1] - ((h/2.0f)*y);
    pos[3] = c[1] + ((h/2.0f)*y);

    updateRegion ();
  }
  /**
   * clamp the bound to another bound.
   * @param rng minimum and maximum range this bound may occupy.
   */
  void clamp (const Bounds& rng)
  {
    pos[0] = std::max (left (), rng.left ());
    pos[1] = std::max (top (), rng.top ());
    pos[2] = std::min (right (), rng.right ());
    pos[3] = std::min (bottom (), rng.bottom ());

    updateRegion ();
  }
  /**
   * zero all the values
   */
  void clear ()
  {
    std::fill (pos, pos+4, 0.0f);
  }
// DRAWING
#ifdef TEST_DRAWABLE
  virtual void draw () const
  {
    const unsigned int x = 0;
    const unsigned int y = 0;
    const unsigned int w = 1;
    const unsigned int h = 1;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin (GL_QUADS);
    glColor3f (1,0,0);    glVertex2i (x+0,y+0);
    glColor3f (0,1,0);    glVertex2i (x+w,y+0);
    glColor3f (0,0,1);    glVertex2i (x+w,y+h);
    glColor3f (1,1,1);    glVertex2i (x+0,y+h);
    glEnd();
  }
#else
  // draw the area under the bounds
#ifdef __GL_H__
  static void draw (const GLenum style, const Bounds& bound)
  {
#if 1
    float  gld[(4+4)*4];
  //  GLUtils::bindBuffer (bound.vao (), GL_ARRAY_BUFFER);
  //  GLUtils::bufferData (4, bound.pos, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
  //  bound.updateRegion ();
    bound.glRegion (gld, (4+4)*4);
  //  GLUtils::bindBuffer (bound.vao.index (), GL_ARRAY_BUFFER);
    bound.vao.bind (GL_ARRAY_BUFFER);
    GLUtils::bufferData ((4+4)*4, gld, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);

  //  GLUtils::bindBuffer (vao (), GL_ARRAY_BUFFER);

    glEnableVertexAttribArray (0);
    glEnableVertexAttribArray (1);

    glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 8*sizeof (float), 0);
    glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, 8*sizeof (float), (void*)(4*sizeof (float)));

    if (style == GL_PATCHES)
    {
      glPatchParameteri (GL_PATCH_VERTICES, 4);

      glPolygonMode (GL_FRONT, GL_LINE);
      glPolygonMode (GL_BACK, GL_LINE);
    }

    glDrawArrays (style, 0, 4);  // GL_PATCHES
  //  glDrawArrays (GL_QUADS, 0, 4);

    glDisableVertexAttribArray (1);
    glDisableVertexAttribArray (0);

    GLUtils::unbindBuffer (GL_ARRAY_BUFFER);
  //  glBindBuffer (GL_ARRAY_BUFFER, 0);
#else
    glBegin (style);
    glTexCoord2i (0, 0);  glVertex2f (bound.left (),  bound.bottom ());
    glTexCoord2i (1, 0);  glVertex2f (bound.right (), bound.bottom ());
    glTexCoord2i (1, 1);  glVertex2f (bound.right (), bound.top ());
    glTexCoord2i (0, 1);  glVertex2f (bound.left (),  bound.top ());
    glEnd ();
/*
    glBegin (GL_POINTS);
    glColor4f (1.0f, 0.0f, 0.0f, 1.0f);  glVertex2f (bound.left (), bound.bottom ());
    glColor4f (0.0f, 0.0f, 1.0f, 1.0f);  glVertex2f (bound.right (), bound.top ());
    glEnd ();*/
#endif
  }
  static void drawBorder (const GLenum style, const Bounds& bound, const Location edge)
  {
    glBegin (style);

    if (edge & TOP)
    {
      glVertex2f (bound.left (), bound.top ());
      glVertex2f (bound.right (), bound.top ());
    }
    if (edge & BOTTOM)
    {
      glVertex2f (bound.left (), bound.bottom ());
      glVertex2f (bound.right (), bound.bottom ());
    }
    if (edge & LEFT)
    {
      glVertex2f (bound.left (), bound.top ());
      glVertex2f (bound.left (), bound.bottom ());
    }
    if (edge & RIGHT)
    {
      glVertex2f (bound.right (), bound.bottom ());
      glVertex2f (bound.right (), bound. top ());
    }

    glEnd ();
  }
  static void drawShortEdges (const GLenum style, const Bounds& bound)
  {
    if (std::abs (bound.width ()) > std::abs (bound.height ()))
    {
      drawBorder (GL_LINES, bound, UI::LEFT);
      drawBorder (GL_LINES, bound, UI::RIGHT);
    }
    else
    {
      drawBorder (GL_LINES, bound, UI::TOP);
      drawBorder (GL_LINES, bound, UI::BOTTOM);
    }
  }
  // FIXME: drawlongedges?
  static void drawNotches (const GLenum style, const Bounds& bound, const Location edge, const unsigned int steps)
  {
    const unsigned int  N = steps;
    float               D = 1.0f/static_cast<float>(N-1);
    float               L = 0.2f;
    unsigned int        i;

    glBegin (style);

    if (edge & TOP)
    {
      for (i=0, D*=bound.width (), L*=bound.height (); i<N; i++)
      {
        glVertex2f (bound.left () + i * D, bound.top () + L);
        glVertex2f (bound.left () + i * D, bound.top ());
      }
    }
    if (edge & BOTTOM)
    {
      for (i=0, D*=bound.width (), L*=bound.height (); i<N; i++)
      {
        glVertex2f (bound.left () + i * D, bound.bottom ());
        glVertex2f (bound.left () + i * D, bound.bottom () - L);
      }
    }
    if (edge & RIGHT)
    {
      for (i=0, D*=bound.height (), L*=bound.width (); i<N; i++)
      {
        glVertex2f (bound.right () - L, bound.top () + i * D);
        glVertex2f (bound.right (), bound.top () + i * D);
      }
    }
    if (edge & LEFT)
    {
      for (i=0, D*=bound.height (), L*=bound.width (); i<N; i++)
      {
        glVertex2f (bound.left () + L, bound.top () + i * D);
        glVertex2f (bound.left (), bound.top () + i * D);
      }
    }

    glEnd ();
  }
  static void drawHighlighted (const Bounds& bound)
  {
    // FIXME: use UI namespace highlight colour
    glColor4f (1.0f, 0.0f, 0.0f, 0.4f);
    glLineWidth (4.0f);
    draw (GL_LINE_LOOP, bound);
    glLineWidth (1.0f);

    glPointSize (6.0f);
    draw (GL_POINTS, bound);
    glPointSize (1.0f);
  }
#else
  static void draw (const int style, const Bounds& bound)
  {}
  static void drawBorder (const int style, const Bounds& bound, const Location edge)
  {}
#endif
#endif
// PERSISTENCE
#if 0
  const char* persistableNodeLabelID ()                        {return "label\0";}
  const char* persistableNodeValueID ()                        {return "value\0";}
  virtual void write (std::ostream& os) const
  {
    Patterns::Persistable::pushParameter<const float> (os, "minx\0", pos[0]);
    Patterns::Persistable::pushParameter<const float> (os, "miny\0", pos[1]);
    Patterns::Persistable::pushParameter<const float> (os, "maxx\0", pos[2]);
    Patterns::Persistable::pushParameter<const float> (os, "maxy\0", pos[3]);
  }
  virtual void read (std::istream& is)
  {
    Patterns::Persistable::pullParameter<float> (is, "minx\0", pos[0]);
    Patterns::Persistable::pullParameter<float> (is, "miny\0", pos[1]);
    Patterns::Persistable::pullParameter<float> (is, "maxx\0", pos[2]);
    Patterns::Persistable::pullParameter<float> (is, "maxy\0", pos[3]);
  }
  virtual void read (std::istream& is, const XMLNode& doc)
  {
    XMLNode::ConstChildIterator  it;
    std::string          n, l, lv, v, vv;
    float            x;

    for (it = pullObject<float> (is, doc.begin (), doc.end ());
       it!= doc.end ();
       it = pullObject<float> (is, ++it, doc.end ()))
    {
      is.seekg (it->offset ()+1);
      n = Persistable::fetchNextToken (is);
      Persistable::fetchNextAttributeValuePair (is, l, lv);
      Persistable::fetchNextAttributeValuePair (is, v, vv);

      std::stringstream (vv) >> x;

      if (lv.compare ("minx\0") == 0)      {pos[0] = x;}
      else if (lv.compare ("miny\0") == 0)  {pos[1] = x;}
      else if (lv.compare ("maxx\0") == 0)  {pos[2] = x;}
      else if (lv.compare ("maxy\0") == 0)  {pos[3] = x;}
    }

    updateRegion ();
  }
  friend std::ostream& operator<< (std::ostream& os, const Bounds& b)          {b.write (os); return os;}
  friend std::ostream& operator<< (std::ostream& os, const Bounds* b)          {if (b != NULL)  {b->write (os);} return os;}
  friend std::istream& operator>> (std::istream& is, Bounds& b)            {b.read (is); return is;}
  friend std::istream& operator>> (std::istream& is, Bounds* b)            {if (b != NULL) {b->read (is);} return is;}
#endif
};

typedef ParameterSet<Bounds>  BoundsSet;


/**
 * Static class which contains size definitions for controls, fonts etc.
 */
class Display
{
public:
// UNITS
  static const unsigned int  PX = 1;   ///< base unit (pixel)
  static const unsigned int  SU = 5;   ///< minor-unit (5*px)
  static const unsigned int  MU = 40;  ///< major-unit (20*px)
// CONTROL SIZES
  // FIXME: Use GetSystemMetrics on windows
  static unsigned int headerHeight ()         {return MU+(PX*2);} // SM_CYCAPTION
  static unsigned int borderSize ()           {return PX*2;}      // SM_CXEDGE
  static unsigned int minmaxButtonSize ()     {return MU;}        // SM_CXSIZE, SM_CYSMSIZE
  static unsigned int spacing ()              {return SU;}
  static unsigned int defaultItemWidth ()     {return MU*8;}
  static unsigned int defaultItemHeight ()    {return MU*6;}
  static unsigned int axisSize ()             {return MU;}
  static unsigned int hotspotSize ()          {return SU;}

  static const unsigned int sideBarWidth ()   {return 320;}
  static const unsigned int dataPaneWidth ()  {return 240;}
// FONT SIZES

// COLOURS
//  static float[4] bright ()          {return {1.0f, 0.0f, 0.0f, 1.0f};}

  /**
   * compute a title area for a bound
   * @param input the source bound region
   * @param output bound region describing the title area
   */
  static void titleArea (const UI::Bounds& input, UI::Bounds& output)
  {
    float minx, miny, maxx, maxy;
#if 0
    miny = input.top () - UI::Display::headerHeight () / 2.0f;
    maxy = input.top () + UI::Display::headerHeight () / 2.0f;
    minx = input.left ();
    maxx = input.right ();
#else
    miny = input.top ();
    maxy = input.bottom ();
    minx = input.left () - UI::Display::headerHeight ();
    maxx = input.left ();
#endif
    output.setRegion (minx, miny, maxx, maxy);
  }
  /**
   * compute the hotspot for a bound
   * @param input, the source bound region
   * @param output, the hotspot region
   */
  static void hotspotArea (const UI::Bounds& input, UI::Bounds& output)
  {
    const float  hs = static_cast<float>(UI::Display::hotspotSize ());
    float minx, miny, maxx, maxy;

    miny = input.bottom () - hs;
    maxy = input.bottom () + hs;
    minx = input.right () - hs;
    maxx = input.right () + hs;

    output.setRegion (minx, miny, maxx, maxy);
  }
// BOUND DRAWING
#ifdef __GL_H__
  static void drawText (const Bounds& region, const char* text, GLUtils::Text::HorizontalAlignment hanchor, GLUtils::Text::VerticalAlignment vanchor, const float scale = 20.0f)
  {
    Rotation   rot = region.orientation ();
    float      px[2], w, h, dir[2];
    float      bbox[4];
    GLUtils::Text::HorizontalAlignment  halign = hanchor;
    GLUtils::Text::VerticalAlignment    valign = vanchor;

    glPushMatrix ();
  //  glTranslatef (region.left (), region.top (), 0.0f);
    glTranslatef (std::min (region.left (), region.right ()),
            std::min (region.top (), region.bottom ()),
            0.0f);

    // get the width and height of the bounds
    w = region.orientedWidth (rot);
    h = region.orientedHeight (rot);

    // get the width of the string
    GLUtils::Text::glStringDimensions (text, bbox);
    bbox[0] *= scale;
    bbox[1] *= scale;
    bbox[2] *= scale;
    bbox[3] *= scale;

#if 0
    // set the text anchor points
    switch (hanchor)
    {
      case GLUtils::Text::LEFT:
      {
        if (w > bbox[2]-bbox[0])  {halign = GLUtils::Text::LEFT;}
        else            {halign = GLUtils::Text::RIGHT;}
        break;
      }
      case GLUtils::Text::RIGHT:
      {
        if (w > bbox[2]-bbox[0])  {halign = GLUtils::Text::RIGHT;}
        else            {halign = GLUtils::Text::LEFT;}
        break;
      }
    }
#endif

    // get the "up" direction for writing
    switch (rot)
    {
    case UI::ZERO:      {dir[0] =  1; dir[1] =  1; break;}
    case UI::INVERT:    {dir[0] = -1; dir[1] = -1; break;}
    case UI::RIGHT90:   {dir[0] =  1; dir[1] = -1; break;}
    case UI::LEFT90:    {dir[0] = -1; dir[1] = -1; break;}
    }

#if 0
    // FIXME: this is correct BUT when the text size returned by glStringDimensions
    // is too big for the box this incorrectly flips the text.
    if (hanchor != halign)  {dir[0] *= -1;}
#endif

    // get the starting point for aligned text
    {
    switch (hanchor)
    {
      case GLUtils::Text::LEFT:   px[0] = 0.0f + dir[0] * UI::Display::spacing (); break;
      case GLUtils::Text::CENTER: px[0] = 0.0f + dir[0] * w * 0.5f; break;
      case GLUtils::Text::RIGHT:  px[0] = w    - dir[0] * UI::Display::spacing (); break;
      default:                    px[0] = region.left (); break;
    }

    switch (vanchor)
    {
      case GLUtils::Text::BOTTOM: px[1] = 0.0f + dir[1] * (UI::Display::spacing ()/* + bbox[3]*/); break;
      case GLUtils::Text::MIDDLE: px[1] = 0.0f + dir[1] * h * 0.5f; break;
      case GLUtils::Text::TOP:    px[1] = h    - dir[1] * (UI::Display::spacing ()/* + bbox[3]*/); break;
      default:                    region.bottom (); break;
    }
    }

    // rotate
    switch (rot)
    {
      case UI::ZERO:    {break;}
      case UI::INVERT:  {break;}
      case UI::RIGHT90: {glRotatef (90.0f, 0.0f, 0.0f, 1.0f); break;}
      case UI::LEFT90:  {glRotatef (270.0f, 0.0f, 0.0f, 1.0f); break;}
    }

    // translate
    switch (rot)
    {
      case UI::ZERO:    {glTranslatef (  px[0], px[1], 0.0f); break;}
      case UI::INVERT:  {glTranslatef (  px[0], px[1], 0.0f); break;}
      case UI::RIGHT90: {glTranslatef (w-px[0], px[1], 0.0f); break;}
      case UI::LEFT90:  {glTranslatef (  px[0], px[1], 0.0f); break;}
    }

    // scale
    switch (rot)
    {
      case UI::ZERO:    {glScalef ( scale, -scale, 1.0f); break;}
      case UI::INVERT:  {glScalef ( scale,  scale, 1.0f); break;}
      case UI::RIGHT90: {glScalef (-scale,  scale, 1.0f); break;}
      case UI::LEFT90:  {glScalef (-scale,  scale, 1.0f); break;}
    }

    // FIXME: just make a matrix!!

    // draw the text
    GLUtils::Text::setHorizontalAlignment (halign);
    GLUtils::Text::setVerticalAlignment (valign);
    GLUtils::Text::glPrintf (text);
    GLUtils::Text::setHorizontalAlignment (GLUtils::Text::LEFT);
    GLUtils::Text::setVerticalAlignment (GLUtils::Text::BOTTOM);

    glPopMatrix ();
  }
#endif
  static void drawSparkline (const Bounds& region, const std::list<float>& hs, const bool fill = true)
  {
    std::list<float>::const_iterator  it;
    float         mn, mx, rng;
    size_t        i;
    float         col[4];
    const size_t  N = hs.size ();
    const float   D = 1.0f/static_cast<float>(hs.size ()-1);

    if (hs.size () == 0)        {return;}

    mn = *std::min_element (hs.begin (), hs.end ());
    mx = *std::max_element (hs.begin (), hs.end ());
    rng = mx-mn;

#ifdef __GL_H__
    glLineWidth (2.0f);
    glGetFloatv (GL_CURRENT_COLOR, col);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    switch (region.orientation ())
    {
      case UI::LEFT90:
      case UI::RIGHT90:
      {
        if (fill == true)
        {
          glColor4f (col[0], col[1], col[2], 0.5f);

          glBegin (GL_QUAD_STRIP);
          for (i=N-1,it=hs.begin (); (it!=hs.end()); --i,++it)
          {
            glVertex2f (region.right () - region.width () * (((*it)-mn)/(rng)),
                        region.top () + (i*D*region.height ()));

            glVertex2f (region.right (),
                        region.top () + (i*D*region.height ()));
          }
          glEnd ();
        }

      //  glColor4f (col[0], col[1], col[2], 0.8f);
        glColor4f (0.0f, 0.0f, 0.0f, 1.0f);

        glBegin (GL_LINE_STRIP);
        for (i=N-1,it=hs.begin (); (it!=hs.end()); --i,++it)
        {
          glVertex2f (region.right () - region.width () * (((*it)-mn)/(rng)),
                      region.top () + (((float)i)*D*((float)region.height ())));
        }
        glEnd ();
        break;
      }
      case UI::ZERO:
      case UI::INVERT:
      {
        if (fill == true)
        {
          glColor4f (col[0], col[1], col[2], 0.5f);

          glBegin (GL_QUAD_STRIP);
          for (i=N-1,it=hs.begin (); (it!=hs.end()); --i,it++)
          {
            glVertex2f (region.left () + i * D * region.width (),
                        region.top () + region.height () * (((*it)-mn)/(rng)));
            glVertex2f (region.left () + i * D * region.width (),
                        region.top ());
          }
          glEnd ();
        }

        glColor4f (col[0], col[1], col[2], 0.8f);

        glBegin (GL_LINE_STRIP);
        for (i=N-1,it=hs.begin (); (it!=hs.end()); --i,it++)
        {
          glVertex2f (region.left () + i * D * region.width (),
                      region.top () + region.height () * (((*it)-mn)/(rng)));
        }
        glEnd ();
        break;
      }
    }

    glDisable (GL_BLEND);
    glColor4fv (col);
    glLineWidth (1.0f);
#endif
  }
};

#ifdef __GL_H__
GLUtils::BufferDescription  Bounds::vao;
#endif

/**
 * FIXME: make this a mutable set
 */
class DrawStyle
{
protected:
//  float  hdr[4];    ///< header
  float  frm[4];    ///< frame
  float  bkg[4];    ///< background
  float  txc[4];    ///< text
  float  htt[4];    ///< hot component (selected or hovering)

public:
  DrawStyle ()
  {
    std::fill (frm, frm+4, 0);
    std::fill (bkg, bkg+4, 0);
    std::fill (txc, txc+4, 0);
  }
//  void setHeader (const float *vals)    {memcpy (hdr, vals, 4*sizeof (float));}
  void setFrame (const float *vals)
  {
    std::copy (vals, vals+4, frm);
  }
  void setBackground (const float *vals)
  {
    std::copy (vals, vals+4, bkg);
  }
  void setText (const float *vals)
  {
    std::copy (vals, vals+4, txc);
  }
  void setHot (const float *vals)
  {
    std::copy (vals, vals+4, htt);
  }
//  const float* header () const      {return hdr;}
  const float* frame () const        {return frm;}
  const float* background () const    {return bkg;}
  const float* text () const        {return txc;}
  const float* hot () const        {return htt;}

  void write (std::ostream& os) const
  {
    os << "[DRAWSTYLE]\0";
  }

  friend std::ostream& operator<< (std::ostream& os, const DrawStyle& ds)
  {
    ds.write (os);
    return os;
  }
};

typedef ParameterSet<DrawStyle>  DrawStyleSet;
static DrawStyleSet  drawStyleSet;
//static DrawStyle  Focus;
//static DrawStyle  Background;
//static DrawStyle  Hot;
#if 0
/**
 * Acts as a visual indicator of where an interface element is by drawing a
 * border and optional caption around the element.
 * FIXME: this should extend the Drawable class, but the draw function requires
 * that the caption label is passed as a string -- which violates the signature
 * of the function. NB: the caption should not be duplicated within this class.
 */
class Container : public Bounds
{
protected:
  Location  captionLocation;
  Component  selComp;

  virtual void updateRegion ()
  {
    Bounds::updateRegion ();

    switch (captionLocation)
    {
    case TOP:
      const_cast<Bounds&>(displayRegion).setRegion (left () + UI::Display::borderSize (), top () + UI::Display::headerHeight (),
                   right () - UI::Display::borderSize (), bottom () - UI::Display::borderSize ());
      const_cast<Bounds&>(headerRegion).setRegion (left (), top (), right (), top () + UI::Display::headerHeight ());
      break;
    case LEFT:
      const_cast<Bounds&>(displayRegion).setRegion (left () + UI::Display::headerHeight (), top () + UI::Display::borderSize (),
                   right () - UI::Display::borderSize (), bottom () - UI::Display::borderSize ());
      const_cast<Bounds&>(headerRegion).setRegion (left (), top (), left () + UI::Display::headerHeight (), bottom ());
      break;
    }
  }
public:
  Bounds  displayRegion;
  Bounds  headerRegion;  ///< where to draw the caption

  Container (Location captionDrawLocation = Location::LEFT)
    : captionLocation (captionDrawLocation), selComp (NOSEL)
  {
#if 0
    setRegion (static_cast<unsigned int>(0), static_cast<unsigned int>(0), UI::Display::defaultItemWidth (), UI::Display::defaultItemHeight ());
#else
    clear ();
    enlarge (UI::Display::defaultItemWidth (), UI::Display::defaultItemHeight ());
#endif
  }
  virtual ~Container ()
    {}
  void draw (const char *caption, const DrawStyle& drawStyle)
  {
#ifdef __GL_H__
    // draw a solid background
    {
    glColor4fv (drawStyle.background ());
    Bounds::draw (GL_QUADS, *this);
    }
    // draw an outline
    {
    glLineWidth (1);
    glColor4fv (drawStyle.frame ());
    Bounds::draw (GL_LINE_LOOP, *this);
    glLineWidth (1);
    }
    /// draw the header
    {
      glColor4fv (drawStyle.frame ());
      Bounds::draw (GL_QUADS, headerRegion);
      glColor4fv (drawStyle.text ());
      // draw the caption text
#if 1
      UI::Bounds::drawText (headerRegion, caption, GLUtils::Text::LEFT, GLUtils::Text::MIDDLE, 30.0f);
#else
    switch (captionLocation)
    {
      case TOP:
      {
        glTranslatef (headerRegion.left ()+6.0f, headerRegion.bottom ()-9.0f, -0.5f);
        glScalef (30.0f, -30.0f, 1.0f);
        GLUtils::Text::glPrintf (caption);
        break;
      }
      case LEFT:
      {
        glTranslatef (headerRegion.right ()-9.0f,
                headerRegion.bottom ()-6.0f,
                -0.5f);
        glRotatef (270.0f, 0.0f, 0.0f, 1.0f);
        glScalef (30.0f, -30.0f, 1.0f);
        GLUtils::Text::glPrintf (caption);
        break;
      }
    }
#endif
    }

    // hotspot
    glColor4fv (drawStyle.frame ());
    glPointSize (hotspotSize ());
  //  glEnable (GL_POINT_SMOOTH);
    glBegin (GL_POINTS);
  //  glVertex2f (right (), bottom ());
    glVertex2f (horzMax (), vertMin ());
    glEnd ();
  //  glDisable (GL_POINT_SMOOTH);
    glPointSize (1.0f);

//    sourceRegion.viewport ();
    glColor4fv (drawStyle.background ());
#endif
  }
};

typedef ParameterSet<Container>  ContainerSet;
#endif

#if 0
/**
 * Layout modifiers
 * FIXME: change to use BoundSet (might be nothing but a find+replace)
 */
class Layout : public Bounds
{
protected:
  virtual void beg (const unsigned int n) = 0;
  virtual void stp (Bounds& b) = 0;
  virtual void end () = 0;
public:
  void operator() (BoundsSet& boundSet)
  {
    BoundsSet::ParamList  pl;

    beg (static_cast<unsigned int>(boundSet.parameterCount ())-1);

    for (pl=boundSet.begin (); pl!=boundSet.end (); pl++)
    {
      stp (pl->second);
    }

    end ();
  }
  void operator() (ContainerSet& containerSet)
  {
    ContainerSet::ParamList  pl;

    beg (static_cast<unsigned int>(containerSet.parameterCount ())-1);

    for (pl=containerSet.begin (); pl!=containerSet.end (); pl++)
    {
      stp (dynamic_cast<Bounds&>(pl->second));
    }

    end ();
  }
#ifdef _WIN32_
  void operator() (HWND hwnd, BoundsSet& boundSet)
  {
    RECT  r;

    GetClientRect (hwnd, &r);

    setRegion (r);
    operator() (boundSet);
  }
  void operator() (HWND hwnd, ContainerSet& containerSet)
  {
    RECT  r;

    GetClientRect (hwnd, &r);

    setRegion (r);
    operator() (containerSet);
  }
#endif
};
class Cascade : public Layout
{
protected:
  float  px, py;

  void beg (const unsigned int n)
  {
    px = 0.0f;
    py = 0.0f;
  }
  void stp (Bounds& b)
  {
    const float  w = b.width ();
    const float h = b.height ();

    //  b.offset (px, py);
    b.setRegion (0.0f+px,
           0.0f+py,
           0.0f+px+w,
           0.0f+py+h);

    px += Display::headerHeight ();
    py += Display::headerHeight ();
  }
  void end ()  {}
};
class Tile : public Layout
{
protected:
  int    ri, ci;      // row index, column index
  int    rc, cc;      // row count, column count

  void beg (const unsigned int count)
  {
    ri = ci = 1 + static_cast<int>(floor (sqrt (static_cast<float>(count))));
    rc = cc = 0;
  }
  void stp (Bounds& b)
  {
    float  px, py, pw, ph;  // item bounding box

    // get the current row we're doing
    if (rc == ri-1)  {rc = 0; cc += 1;}

    pw = width ()/static_cast<float>(ri+1);
    ph = height ()/static_cast<float>(ci+1);
    px = pw * (float)rc;
    py = ph * (float)cc;

    b.setRegion (px, py, px + pw, py + ph);

    rc++;
  }
  void end ()
  {}
};
class Rows : public Layout
{
protected:
  int    ri;        // row index, column index
  int    rc;        // row count, column count

  void beg (const unsigned int n)
  {
    ri = std::max<unsigned int> (1, n);
    rc = 0;
  }
  void stp (Bounds& b)
  {
    float  px, py, pw, ph;  // item bounding box

    pw = width ();
    ph = height ()/static_cast<float>(ri+1);
    px = left ();
    py = ph * (float)rc;

    b.setRegion (px, py, px + pw, py + ph);

    rc++;
  }
  void end ()
  {}
};
class Columns : public Layout
{
protected:
  int    ri;        // row index, column index
  int    rc;        // row count, column count

  void beg (const unsigned int n)
  {
    ri = std::max<unsigned int> (1, n);
    rc = 0;
  }
  void stp (Bounds& b)
  {
    float  px, py, pw, ph;  // item bounding box

    pw = width ()/static_cast<float>(ri+1);
    ph = height ();
    px = pw * (float)rc;
    py = top ();

    b.setRegion (px, py, px + pw, py + ph);

    rc++;
  }
  void end ()
  {}
};
#endif
class Grid //: public Layout
{
public:
  static float clamp (const float a, const float c)
  {
    float  r = std::fmod (a, c);
    float  d = (a-r)/c;

    if (std::abs (a - (d*c)) < std::abs (a - ((d+1)*c)))
    {
      return (d*c);
    }
    else
    {
      return ((d+1)*c);
    }
  }
  static void clamp (Bounds *b, const float grx, const float gry)
  {
    float  minx, maxx, miny, maxy;

    if (b == NULL)
    {
      return;
    }

    minx = clamp (b->left (), grx);
    maxx = clamp (b->right (), grx);
    miny = clamp (b->top (), gry);
    maxy = clamp (b->bottom (), gry);

    b->setRegion (minx, miny, maxx, maxy);
  }
};
};

#endif
