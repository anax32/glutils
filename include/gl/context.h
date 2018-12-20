namespace gl
{
  /*
  * Manage a gl context on a platform
  */
  namespace context
  {
    const unsigned int  REQUEST_GL_MAJOR_VERSION = 3;
    const unsigned int  REQUEST_GL_MINOR_VERSION = 0;

    /*
    * debug callback from opengl
    */
    void openglDebugCallback (GLenum source,
                              GLenum type,
                              GLuint id,
                              GLenum severity,
                              GLsizei length,
                              const GLchar* message,
                              const void* userParam)
    {
      std::string  err;

      switch (type)
      {
        case GL_DEBUG_TYPE_ERROR:
        {
          err += "ERROR\0";
          break;
        }
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        {
          err += "DEPRECATED_BEHAVIOR\0";
          break;
        }
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        {
          err += "UNDEFINED_BEHAVIOR\0";
          break;
        }
        case GL_DEBUG_TYPE_PORTABILITY:
        {
          err += "PORTABILITY\0";
          break;
        }
        case GL_DEBUG_TYPE_PERFORMANCE:
        {
          err += "PERFORMANCE\0";
          break;
        }
        case GL_DEBUG_TYPE_OTHER:
        {
          err += "OTHER\0";
          break;
        }
      }

      err += " [\0";

      switch (severity)
      {
        case GL_DEBUG_SEVERITY_LOW:
        {
          err += "LOW\0";
          break;
        }
        case GL_DEBUG_SEVERITY_MEDIUM:
        {
          err += "MEDIUM\0";
          break;
        }
        case GL_DEBUG_SEVERITY_HIGH:
        {
          err += "HIGH\0";
          break;
        }
      }

      err += "] \0";

      std::cerr << (err + message);
    }

    std::pair<int, int> get_version()
    {
      int gl_version[2] = { -1, -1 };
      glGetIntegerv (GL_MAJOR_VERSION, &gl_version[0]);
      glGetIntegerv (GL_MINOR_VERSION, &gl_version[1]);

      return std::make_pair (gl_version[0], gl_version[1]);
    }
  };
}

#ifdef _WIN32
#include "context_win32.h"
#elif __linux__
#include "context_x.h"
#endif
