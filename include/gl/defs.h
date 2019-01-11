#define OPENGL_DEBUGGING
#define GLEW_STATIC

#ifdef _WIN32
#include "GL\glew.h"
#include "GL\wglew.h"
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

  using callback_t = std::function<void(const resource_set_t&)>;
  typedef std::map<std::string, callback_t, cmp_str> callback_set_t;

  /*
  * get a resource from a const map
  * NB: [] operator is not valid on const maps
  */
  template<typename T>
  typename T::value_type get_resource(const T& res, const typename T::key_type& name)
  {
    typename T::const_iterator it = res.find(name);

    if (std::distance(it, std::end(res)) == 0)
    {
      return std::make_pair<T::value_type::first_type, T::value_type::second_type>("none", 0);
    }

    return *it;
  }
  
  template<typename T>
  bool has_resource (const T& res, const typename T::key_type& name)
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