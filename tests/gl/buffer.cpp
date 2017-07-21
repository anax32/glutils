#include "gl/defs.h"
#include "gl/context.h"
#include "gl/buffer.h"
#include <fstream>

#include "maketest.h"

void create_test ()
{
  auto A = gl::buffer::create();

  assert_are_not_equal (A, gl::buffer::invalid_buffer_id);

  gl::buffer::clean(A);
}
void bind_test ()
{
  auto A = gl::buffer::create ();
  assert_are_not_equal (A, gl::buffer::invalid_buffer_id);

  gl::buffer::bind(A);
  gl::buffer::unbind();
  gl::buffer::clean(A);
}

void gl_buffer_tests ()
{
  gl::context::create ();
  TEST(create_test);
  TEST(bind_test);
  gl::context::clean ();
}

int main (int argc, char** argv)
{  
  TEST_GROUP(gl_buffer_tests);
}
