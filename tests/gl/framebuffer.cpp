#include "gl/defs.h"
#include "gl/context.h"
#include "gl/texture.h"
#include "gl/framebuffer.h"
#include <fstream>

#include "../../test/tests.h"

void create_test ()
{
  auto A = gl::framebuffer::create();

  assert_are_not_equal (A, gl::framebuffer::invalid_framebuffer_id);

  gl::framebuffer::clean(A);
}
void bind_test ()
{
  auto A = gl::framebuffer::create ();
  assert_are_not_equal (A, gl::framebuffer::invalid_framebuffer_id);

  gl::framebuffer::bind(A);
  gl::framebuffer::unbind();
  gl::framebuffer::clean(A);
}
void attach_single_test ()
{
  auto A = gl::framebuffer::create ();
  assert_are_not_equal (A, gl::framebuffer::invalid_framebuffer_id);
  auto T = gl::texture::create (gl::size2d (64,64));
  assert_are_not_equal (T, gl::texture::invalid_texture_id);

  gl::framebuffer::bind(A);
  gl::framebuffer::attach_texture(T);
  
  assert_is_true (gl::framebuffer::verify());

  gl::framebuffer::unbind();
  gl::texture::clean(T);
  gl::framebuffer::clean(A);
}
void attach_bunch_test ()
{
  auto A = gl::framebuffer::create ();
  assert_are_not_equal (A, gl::framebuffer::invalid_framebuffer_id);
  auto T = new unsigned int[4];
  T[0] = gl::texture::create (gl::size2d (64, 64));
  T[1] = gl::texture::create (gl::size2d (64, 64));
  T[2] = gl::texture::create (gl::size2d (64, 64));
  T[3] = gl::texture::create (gl::size2d (64, 64));
  assert_are_not_equal (T[0], gl::texture::invalid_texture_id);
  assert_are_not_equal (T[1], gl::texture::invalid_texture_id);
  assert_are_not_equal (T[2], gl::texture::invalid_texture_id);
  assert_are_not_equal (T[3], gl::texture::invalid_texture_id);

  gl::framebuffer::bind (A);
  gl::framebuffer::attach_texture (T, 4);

  assert_is_true (gl::framebuffer::verify ());

  gl::framebuffer::unbind ();
  gl::texture::clean (T[0]);
  gl::texture::clean (T[1]);
  gl::texture::clean (T[2]);
  gl::texture::clean (T[3]);
  gl::framebuffer::clean (A);
  delete[] T;
}
void attach_textureset_test ()
{
  gl::texture::texture_set_t  texs;
  texs["first"] = gl::texture::create (gl::size2d (64, 64));
  texs["second"] = gl::texture::create (gl::size2d (64, 64));
  texs["third"] = gl::texture::create (gl::size2d (64, 64));
  texs["fourth"] = gl::texture::create (gl::size2d (64, 64));
  assert_is_false (gl::utils::any_invalid_entries (texs, gl::texture::invalid_texture_id));

  auto A = gl::framebuffer::create ();
  assert_are_not_equal (A, gl::framebuffer::invalid_framebuffer_id);

  gl::framebuffer::bind (A);
  gl::framebuffer::attach_texture (texs);

  assert_is_true (gl::framebuffer::verify ());

  gl::framebuffer::unbind ();
  gl::texture::clean (texs);
  gl::framebuffer::clean (A);
}
void render_red_image_test ()
{
  auto A = gl::framebuffer::create ();
  assert_are_not_equal (A, gl::framebuffer::invalid_framebuffer_id);
  auto T = gl::texture::create (gl::size2d (64, 64));
  assert_are_not_equal (T, gl::texture::invalid_texture_id);

  gl::framebuffer::bind (A);
  gl::framebuffer::attach_texture (T);

  glClearColor (1.0f, 0.0f, 0.0f, 1.0f);
  glClear (GL_COLOR_BUFFER_BIT);

  gl::framebuffer::unbind ();
  gl::framebuffer::clean (A);

  gl::texture::write (T, [](const int w, const int h, const int d, const unsigned char *buf)
            {
              auto n = w*h;
              auto *c = buf;
              for (auto i=0;i<n;i++,c+=d)
              {
                assert_are_equal (c[0], 255);
                assert_are_equal (c[1], 0);
                assert_are_equal (c[2], 0);
                assert_are_equal (c[3], 255);
              }
              return true;
            });

  gl::texture::clean (T);
}

void gl_framebuffer_tests ()
{
  gl::context::create ();
  TEST(create_test);
  TEST(bind_test);
  TEST(attach_single_test);
  TEST(attach_bunch_test);
  TEST(attach_textureset_test);
  TEST(render_red_image_test);
  gl::context::clean ();
}

int main (int argc, char** argv)
{  
  TEST_GROUP(gl_framebuffer_tests);  
}
