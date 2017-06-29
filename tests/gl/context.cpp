#include "gl/defs.h"
#include "gl/context.h"

#include "../../test/tests.h"

void create_test ()
{
#ifdef _WIN32
  HWND window_handle = window::create ();
  assert_is_true (gl::context::create (window_handle));
  gl::context::clean (window_handle);
#elif __linux__
  assert_is_true (gl::context::create ());
  gl::context::clean ();
#endif
}

void gl_window_tests ()
{
  TEST(create_test);
}

int main (int argc, char** argv)
{
  TEST_GROUP(gl_window_tests);
}