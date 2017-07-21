#include "gl/defs.h"
#include "gl/context.h"
#include "gl/shader.h"
#include <fstream>

#include "maketest.h"

void shader_type_enum_test ()
{
  assert_are_equal (gl::shader::shader_type_enum ("program"), gl::shader::SHADER_PROGRAM);
  assert_are_equal (gl::shader::shader_type_enum ("vertex"), gl::shader::VERTEX_SHADER);
  assert_are_equal (gl::shader::shader_type_enum ("geometry"), gl::shader::GEOMETRY_SHADER);
  assert_are_equal (gl::shader::shader_type_enum ("fragment"), gl::shader::FRAGMENT_SHADER);
  assert_are_equal (gl::shader::shader_type_enum ("compute"), gl::shader::COMPUTE_SHADER);
  assert_are_equal (gl::shader::shader_type_enum ("tesselation_control"), gl::shader::TESS_CTRL_SHADER);
  assert_are_equal (gl::shader::shader_type_enum ("tesselation_eval"), gl::shader::TESS_EVAL_SHADER);
}

#define EMPTY_VERTEX_SHADER  "#version 450\n void main () {gl_Position = vec4 (1.0);}"
#define EMPTY_FRGMNT_SHADER  "#version 450\n out vec4 colour; void main () {colour = vec4 (1.0);}"

void create_test ()
{
  gl::shader::definition_t  def = {{"vertex", EMPTY_VERTEX_SHADER},
                     {"fragment", EMPTY_FRGMNT_SHADER}};
  gl::shader::shader_t    shader;
  gl::shader::pipeline_t    pipe = {def, shader};

  gl::shader::create (pipe);

  assert_are_not_equal (pipe.second["vertex"], gl::shader::invalid_shader_id);
  assert_are_not_equal (pipe.second["fragment"], gl::shader::invalid_shader_id);
  assert_are_not_equal (pipe.second["program"], gl::shader::invalid_shader_id);

  assert (gl::shader::link (pipe.second) == true);
  assert_is_true (gl::shader::link_status (pipe.second));
  assert_is_true (gl::shader::is_valid (pipe.second));

  gl::shader::clean (pipe);
}
void create_inline_def_test ()
{
  gl::shader::pipeline_t    pipe = 
  {
    {{"vertex", EMPTY_VERTEX_SHADER},
     {"fragment", EMPTY_FRGMNT_SHADER}},
     {}
  };

  gl::shader::create (pipe);

  assert_are_not_equal (pipe.second["vertex"], gl::shader::invalid_shader_id);
  assert_are_not_equal (pipe.second["fragment"], gl::shader::invalid_shader_id);
  assert_are_not_equal (pipe.second["program"], gl::shader::invalid_shader_id);

  assert (gl::shader::link (pipe.second) == true);
  assert_is_true (gl::shader::link_status (pipe.second));
  assert_is_true (gl::shader::is_valid (pipe.second));

  gl::shader::clean (pipe);
}
void create_pipeline_set_def_test ()
{
  gl::shader::pipeline_set_t    pipelineset = 
  {
    {"basic_1",
      {
        {{"vertex", EMPTY_VERTEX_SHADER},
         {"fragment", EMPTY_FRGMNT_SHADER}},
         {}
      }
    },
    {"basic_2",
      {
        {{"vertex", EMPTY_VERTEX_SHADER},
         {"fragment", EMPTY_FRGMNT_SHADER}},
         {}
      }
    },
    {"basic_3",
      {
        {{"vertex", EMPTY_VERTEX_SHADER},
         {"fragment", EMPTY_FRGMNT_SHADER}},
         {}
      }
    }
  };

  gl::shader::create (pipelineset);

  assert_are_not_equal (pipelineset["basic_1"].second["vertex"], gl::shader::invalid_shader_id);
  assert_are_not_equal (pipelineset["basic_1"].second["fragment"], gl::shader::invalid_shader_id);
  assert_are_not_equal (pipelineset["basic_1"].second["program"], gl::shader::invalid_shader_id);

  assert_are_not_equal (pipelineset["basic_2"].second["vertex"], gl::shader::invalid_shader_id);
  assert_are_not_equal (pipelineset["basic_2"].second["fragment"], gl::shader::invalid_shader_id);
  assert_are_not_equal (pipelineset["basic_2"].second["program"], gl::shader::invalid_shader_id);

  assert_are_not_equal (pipelineset["basic_3"].second["vertex"], gl::shader::invalid_shader_id);
  assert_are_not_equal (pipelineset["basic_3"].second["fragment"], gl::shader::invalid_shader_id);
  assert_are_not_equal (pipelineset["basic_3"].second["program"], gl::shader::invalid_shader_id);

  assert (gl::shader::link (pipelineset["basic_1"].second) == true);
  assert (gl::shader::link (pipelineset["basic_2"].second) == true);
  assert (gl::shader::link (pipelineset["basic_3"].second) == true);

  assert_is_true (gl::shader::link_status (pipelineset["basic_1"].second));
  assert_is_true (gl::shader::link_status (pipelineset["basic_2"].second));
  assert_is_true (gl::shader::link_status (pipelineset["basic_3"].second));

  assert_is_true (gl::shader::is_valid (pipelineset["basic_1"].second));
  assert_is_true (gl::shader::is_valid (pipelineset["basic_2"].second));
  assert_is_true (gl::shader::is_valid (pipelineset["basic_3"].second));

  gl::shader::clean (pipelineset);
}
void bind_test ()
{
  gl::shader::pipeline_t    pipe = 
  {
    {{"vertex", EMPTY_VERTEX_SHADER},
     {"fragment", EMPTY_FRGMNT_SHADER}},
     {}
  };

  gl::shader::create (pipe);

  assert (gl::shader::link (pipe.second) == true);
  assert_is_true (gl::shader::link_status (pipe.second));
  assert_is_true (gl::shader::is_valid (pipe.second));

  gl::shader::bind (pipe);
  gl::shader::unbind ();

  gl::shader::clean (pipe);
}

void gl_shader_tests ()
{
  gl::context::create ();
  TEST(shader_type_enum_test)
  TEST(create_test);
  TEST(create_inline_def_test);
  TEST(create_pipeline_set_def_test);
  TEST(bind_test);
  gl::context::clean ();
}

int main (int argc, char** argv)
{  
  TEST_GROUP(gl_shader_tests);
}
