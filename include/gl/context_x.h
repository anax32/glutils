namespace gl
{
  /*
  * Manage a gl context on a platform
  */
  namespace context
  {
    bool create (Display *display = NULL)
    {
      int visual_attrs[] = { GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
                             GLX_RENDER_TYPE,   GLX_RGBA_BIT,
                             GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
                             GLX_RED_SIZE,      8,
                             GLX_GREEN_SIZE,    8,
                             GLX_BLUE_SIZE,     8,
                             GLX_ALPHA_SIZE,    8,
                             GLX_DEPTH_SIZE,    24,
                             GLX_STENCIL_SIZE,  8,
                             GLX_DOUBLEBUFFER,  True,
                             None };

      int win_attrs[] = { GLX_RGBA,
                          GLX_DEPTH_SIZE, 24,
                          GLX_DOUBLEBUFFER,
                          None};

      XVisualInfo           *vi = NULL;
      GLXFBConfig           *fbc = NULL;
      int                   fbcCount = 0;
      Window                win;
      XSetWindowAttributes  swa;
      GLXContext            glc;

      if ((display == NULL) && ((display = XOpenDisplay (NULL)) == NULL))
      {
        std::cerr << "ERR: cannot connect to X server\0" << std::endl;
        return false;
      }

      if ((fbc = glXChooseFBConfig (
                   display,
                   DefaultScreen (display),
                   visual_attrs,
                   &fbcCount))
                == NULL)
      {
        std::cerr << "ERR: Could not retrieve framebuffer config\0" << std::endl;
        XFree (fbc);
        XCloseDisplay (display);
        return false;
      }

      // get the visual from the framebuffer config
      vi = glXGetVisualFromFBConfig (display, fbc[0]);

      swa.colormap = XCreateColormap (display, DefaultRootWindow(display), vi->visual, AllocNone);
      swa.event_mask = ExposureMask | KeyPressMask;

      win = XCreateWindow (
              display,
              DefaultRootWindow(display),
              0, 0, 600, 600,
              0,
              vi->depth,
              InputOutput,
              vi->visual,
              CWColormap|CWEventMask,
              &swa);

      if (win == BadWindow)
      {
        std::cerr << "ERR: Could not create window\0" << std::endl;
        XFree (vi);
        XFree (fbc);
        return false;
      }

      XFree (vi);

      XMapWindow (display, win);

      int attributes[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, REQUEST_GL_MAJOR_VERSION, // Set the MAJOR version of OpenGL to 3  
        GLX_CONTEXT_MINOR_VERSION_ARB, REQUEST_GL_MINOR_VERSION, // Set the MINOR version of OpenGL to 2  
        GLX_CONTEXT_FLAGS_ARB,
        //  WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB |
#ifdef OPENGL_DEBUGGING
        GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
        None
      };

      auto glXCreateContextAttribsARB = (GLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");

      if (glXCreateContextAttribsARB == NULL)
      {
        XFree (fbc);
        return false;
      }

      if ((glc = glXCreateContextAttribsARB (display, fbc[0], 0, True, attributes)) == NULL)
      {
        std::cerr << "ERR: Could not create context" << std::endl;
        XFree (fbc);
        return false;
      }

      XFree (fbc);

      if (glXMakeCurrent (display, win, glc) == False)
      {
        std::cerr << "ERR: Could not make new context current" << std::endl;
        return false;
      }

#ifdef __glew_h__
      GLenum glewInitVal = glewInit ();
      if (glewInitVal != GLEW_OK)        // Enable GLEW
      {
        std::cerr << "ERR: Could not initialise glew" << std::endl;
        glXDestroyContext (display, glc);
        return false;
      }
#endif

#ifdef OPENGL_DEBUGGING
      glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glDebugMessageCallback (openglDebugCallback, NULL);
      //glDebugMessageControl (GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
#endif
      // set some initial gl state
      {
        glClearColor (0.4f, 0.6f, 0.9f, 0.0f); // clear color based on Microsofts CornflowerBlue (default in XNA)
        glClearColor (0.0f, 1.0f, 1.0f, 0.0f);
      }

      return true;
    }

    /*
    * clean a context on a windows system.
    * + get the current context
    * + make a null context current on hwnd
    * + delete the obtained context
    */
    void clean()
    {
      auto display = XOpenDisplay (NULL);
      auto glrc = glXGetCurrentContext ();
      glXMakeCurrent (display, None, NULL);
      glXDestroyContext (display, glrc);
      XCloseDisplay (display);
    }
  };
}
