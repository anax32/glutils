namespace gl
{
  /*
  * Manage a gl context on a platform
  */
  namespace context
  {
    bool create (HWND hwnd)
    {
      PIXELFORMATDESCRIPTOR pfd =
      {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,
        32,              // colour depth
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,              // depthbuffer
        8,              // stencilbuffer
        0,              // aux buffers
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
      };

      int    npfd;
      HDC    hDC;
      HGLRC  htglrc, hglrc;  // temp rc, actual rc

      if (hwnd == NULL)
      {
        hwnd = window::create ();
      }

      if ((hDC = GetDC (hwnd)) == NULL)
      {
        window::clean (hwnd);
        logr::err ("Could not get initial window device context");
        return false;
      }

      npfd = ChoosePixelFormat (hDC, &pfd);
      SetPixelFormat (hDC, npfd, &pfd);

      if ((htglrc = wglCreateContext (hDC)) == NULL)
      {
        window::clean (hwnd);
        logr::err ("Could not create gl context");
        return false;
      }

      if (wglMakeCurrent (hDC, htglrc) == FALSE)
      {
        wglDeleteContext (htglrc);
        window::clean (hwnd);
        logr::err ("Could not make gl context current on device");
        return false;
      }

      // setup glew
#ifdef __glew_h__
      GLenum glewInitVal = glewInit ();
      if (glewInitVal != GLEW_OK)        // Enable GLEW
      {
        wglDeleteContext (htglrc);
        return false;
      }
#else
      wglDeleteContext (htglrc);
      return false;
#endif

      // create the better gl context
      int attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, REQUEST_GL_MAJOR_VERSION, // Set the MAJOR version of OpenGL to 3  
        WGL_CONTEXT_MINOR_VERSION_ARB, REQUEST_GL_MINOR_VERSION, // Set the MINOR version of OpenGL to 2  
        WGL_CONTEXT_FLAGS_ARB,
        //  WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB |
#ifdef OPENGL_DEBUGGING
        WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
        0
      };

      if (wglewIsSupported ("WGL_ARB_create_context") == 1)
      {
        hglrc = wglCreateContextAttribsARB (hDC, NULL, attributes);
        wglMakeCurrent (NULL, NULL);
        wglDeleteContext (htglrc);
        wglMakeCurrent (hDC, hglrc);
      }
      else
      {
        hglrc = htglrc;
      }

#ifdef OPENGL_DEBUGGING
      glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glDebugMessageCallback ((GLDEBUGPROC)openglDebugCallback, NULL);
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
    void clean (HWND hwnd)
    {
      HGLRC hglrc = wglGetCurrentContext ();
      wglMakeCurrent (GetDC (hwnd), NULL);
      wglDeleteContext (hglrc);
    }
  };
}
