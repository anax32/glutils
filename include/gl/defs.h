#define OPENGL_DEBUGGING
#define GLEW_STATIC

#ifdef _WIN32
#include "include\GL\glew.h"
#include "include\GL\wglew.h"
#include <gl/GL.h>
#elif __linux__
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

typedef GLXContext(*GLXCREATECONTEXTATTRIBSARBPROC)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
#endif

#include <iostream>
#include <iterator>
#include <string>
#include <map>
#include <algorithm>
#include <functional>

#include <string.h>    // FIXME: move strlen stuff to stl

namespace gl
{
  // types
  typedef std::tuple<unsigned int, unsigned int>  size2d;
  typedef std::tuple<unsigned int, unsigned int, unsigned int> size3d;

  // containers
  /*
  * string compare function for std::map override
  */
  struct cmp_str
  {
    bool operator() (const std::string& a, const std::string& b) const
    {
      return (a.compare(b) > 0);
    }
  };

  /*
  * typedef to allow gl resources to managed with a dictionary type object
  */
  typedef std::map<std::string, unsigned int, cmp_str>  resource_set_t;

  /*
  * get a resource from a const map
  * NB: [] operator is not valid on const maps
  */
  resource_set_t::value_type get_resource(const resource_set_t& res, const resource_set_t::key_type& name)
  {
    resource_set_t::const_iterator it = res.find(name);

    if (std::distance(it, std::end(res)) == 0)
    {
      return std::make_pair<resource_set_t::value_type::first_type, resource_set_t::value_type::second_type>("none", 0);
    }

    return *it;
  }
  
  bool has_resource (const resource_set_t& res, const resource_set_t::key_type& name)
  {
    return (res.find (name) != res.end ());
  }

  /*
  * misc utility functions
  */
  namespace utils
  {
    /*
    * return true if none of the gl resources loaded in the container have invalid_values as handles
    */
    template<typename T>
    bool any_invalid_entries(const T& container, const typename T::value_type::second_type invalid_value)
    {
      return (std::any_of(std::begin(container),
              std::end(container),
              [&invalid_value](const typename T::value_type v)
                              {return v.second == invalid_value; }));
    }
  }
};

/*
#include <context.h>
#include <texture.h>
#include <framebuffer.h>
#include <shader.h>
*/