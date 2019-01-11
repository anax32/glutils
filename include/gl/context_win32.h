#include <Windows.h>

namespace gl
{
  /*
  * Manage a gl context on a platform
  */
  namespace context
  {
	  LRESULT CALLBACK dummy_window_proc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	  {
        if (message == WM_NCCREATE)
        {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
            return TRUE;
        }

        auto callbacks = reinterpret_cast<callback_set_t*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        if (callbacks != nullptr)
        {
            // if we have a handler, call it and return 0,
            // otherwise fall through to DefWindowProc
            switch (message)
            {
            case WM_CREATE:
            {
                if (gl::has_resource(*callbacks, "on_create") == true)
                {
                    auto p = resource_set_t({
                        {"hwnd", reinterpret_cast<typename gl::resource_set_t::value_type::second_type>(hwnd)}
                        });
                    gl::get_resource(*callbacks, "on_create").second(p);
                    return 0;
                }
                break;
            }
            case WM_SIZE:
            {
                if (gl::has_resource(*callbacks, "on_resize") == true)
                {
                    RECT r;
                    GetWindowRect(hwnd, &r);
                    auto p = resource_set_t({
                        {"left", r.left},
                        {"top",r.top},
                        {"right",r.right},
                        {"bottom",r.bottom}
                    });
                    gl::get_resource(*callbacks, "on_resize").second(p);
                    return 0;
                }
                break;
            }
            case WM_TIMER:
            {
                if (gl::has_resource(*callbacks, "on_timer") == true)
                {
                    auto p = resource_set_t({
                        {"timer-id", static_cast<typename gl::resource_set_t::value_type::second_type>(wParam)}
                    });
                    gl::get_resource(*callbacks, "on_timer").second(p);
                    return 0;
                }
                break;
            }
            case WM_PAINT:
            {
                if (gl::has_resource(*callbacks, "on_paint") == true)
                {
                    RECT    rc;    // RECT structure  
                    auto hdc = GetDC(hwnd);
                    PAINTSTRUCT ps;

                    // FIXME: only get the ps if get update rect is not empty
                    GetUpdateRect(hwnd, &rc, FALSE);
                    BeginPaint(hwnd, &ps);

                    GetClientRect(hwnd, &rc);

                    auto p = resource_set_t();
                    gl::get_resource(*callbacks, "on_paint").second(p);

                    SwapBuffers(hdc);
                    ReleaseDC(hwnd, hdc);

                    EndPaint(hwnd, &ps);

                    return 0;
                }
                break;
            }
            case WM_KEYDOWN:
            {
                // param = static_cast<char>(wParam)
                if (gl::has_resource(*callbacks, "on_keydown") == true)
                {
                    auto p = resource_set_t({
                        {"keycode", static_cast<typename gl::resource_set_t::value_type::second_type>(wParam)},
                    });
                    gl::get_resource(*callbacks, "on_keydown").second(p);
                    return 0;
                }
                break;
            }
            case WM_DESTROY:
            {
                if (gl::has_resource(*callbacks, "on_destroy") == true)
                {
                    auto p = resource_set_t();
                    gl::get_resource(*callbacks, "on_destroy").second(p);
                    return 0;
                }
                break;
            }
            }
        }

        return DefWindowProc(hwnd, message, wParam, lParam);
	  }

	  HWND dummy_window(const callback_set_t* callback_ptr = nullptr)
	  {
		  WNDCLASSEX wcex;
		  auto dummy_class_name = "dummy_class\0";

		  wcex.cbSize = sizeof(WNDCLASSEX);
		  wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		  wcex.lpfnWndProc = dummy_window_proc;
		  wcex.cbClsExtra = 0;
		  wcex.cbWndExtra = sizeof(callback_set_t*);
		  wcex.hInstance = GetModuleHandle (NULL);
		  wcex.hIcon = NULL;
		  wcex.hCursor = NULL;
		  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		  wcex.lpszMenuName = NULL;
		  wcex.lpszClassName = dummy_class_name;
		  wcex.hIconSm = NULL;

		  if (!RegisterClassEx(&wcex))
		  {
			  return (HWND)INVALID_HANDLE_VALUE;
		  }

		  return CreateWindow(
			  dummy_class_name,
			  NULL,
			  WS_OVERLAPPEDWINDOW,
			  CW_USEDEFAULT, CW_USEDEFAULT,
			  CW_USEDEFAULT, CW_USEDEFAULT,
			  NULL,
			  NULL,
			  GetModuleHandle(NULL),
        (LPVOID)(callback_ptr));
	  }

    bool create (const callback_set_t* callbacks = nullptr, HWND hwnd = nullptr)
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

      if (hwnd == nullptr)
      {
        hwnd = dummy_window (callbacks);
      }

      if ((hDC = GetDC (hwnd)) == nullptr)
      {
        //window::clean (hwnd);
        //logr::err ("Could not get initial window device context");
        return false;
      }

      npfd = ChoosePixelFormat (hDC, &pfd);
      SetPixelFormat (hDC, npfd, &pfd);

      if ((htglrc = wglCreateContext (hDC)) == nullptr)
      {
        //window::clean (hwnd);
        //logr::err ("Could not create gl context");
        return false;
      }

      if (wglMakeCurrent (hDC, htglrc) == FALSE)
      {
        wglDeleteContext (htglrc);
        //window::clean (hwnd);
        //logr::err ("Could not make gl context current on device");
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
    * + make a null context current
    * + delete the obtained context
    */
    void clean ()
    {
      HGLRC hglrc = wglGetCurrentContext ();

      // If hglrc is NULL, the function makes the calling thread's current
      // rendering context no longer current, and releases the device context
      // that is used by the rendering context. In this case, hdc is ignored.
      // https://docs.microsoft.com/en-gb/windows/desktop/api/wingdi/nf-wingdi-wglmakecurrent
      wglMakeCurrent (NULL, NULL);

      wglDeleteContext (hglrc);
    }

    void fullscreen(HWND hwnd)
    {
        auto hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        if (!GetMonitorInfo(hmon, &mi))
            return;

        SetWindowLong(
            hwnd,
            GWL_STYLE,
            WS_POPUP | WS_VISIBLE);

        SetWindowPos(
            hwnd,
            HWND_TOPMOST,
            mi.rcMonitor.left,
            mi.rcMonitor.top,
            mi.rcMonitor.right - mi.rcMonitor.left,
            mi.rcMonitor.bottom - mi.rcMonitor.top,
            SWP_SHOWWINDOW);
    }
    void windowed(HWND hwnd, RECT size)
    {
        SetWindowLong(
            hwnd,
            GWL_STYLE,
            WS_OVERLAPPEDWINDOW
        );

        SetWindowPos(
            hwnd,
            HWND_DESKTOP,
            size.left,
            size.top,
            size.right - size.left,
            size.bottom - size.top,
            SWP_SHOWWINDOW
        );
    }
    void windowed(HWND hwnd)
    {
        windowed(
            hwnd,
            { CW_USEDEFAULT,
              CW_USEDEFAULT,
              CW_USEDEFAULT,
              CW_USEDEFAULT }
        );
    }
  };
}
