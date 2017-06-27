#include "gl/defs.h"
#include "gl/context.h"
#include "gl/texture.h"
#include <fstream>

#include "../../test/tests.h"

void create_test ()
{
	auto A = gl::texture::create(gl::size2d(64, 64));
	auto B = gl::texture::create(gl::size2d(64, 64), GL_LUMINANCE);
	auto C = gl::texture::create(gl::size2d(64, 64), GL_RG);
	auto D = gl::texture::create(gl::size3d(64, 64, 64));
	auto E = gl::texture::create(gl::size3d(64, 64, 64), GL_LUMINANCE);
	auto F = gl::texture::create(gl::size2d(64, 64), GL_DEPTH_COMPONENT);
	auto G = gl::texture::create(gl::size2d(64, 64), GL_DEPTH_COMPONENT16);

	assert_are_not_equal (A, gl::texture::invalid_texture_id);
	assert_are_not_equal (B, gl::texture::invalid_texture_id);
	assert_are_not_equal (C, gl::texture::invalid_texture_id);
	assert_are_not_equal (D, gl::texture::invalid_texture_id);
	assert_are_not_equal (E, gl::texture::invalid_texture_id);
	assert_are_not_equal (F, gl::texture::invalid_texture_id);
	assert_are_not_equal (G, gl::texture::invalid_texture_id);

	gl::texture::clean(A);
	gl::texture::clean(B);
	gl::texture::clean(C);
	gl::texture::clean(D);
	gl::texture::clean(E);
	gl::texture::clean(F);
	gl::texture::clean(G);
}

void write_test ()
{
	const unsigned char *img_data = (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABABABABABAB";
	auto A = gl::texture::create (gl::size2d(32, 32));

	// actual image data
	glBindTexture(GL_TEXTURE_2D, A);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);

	// read back image data
	unsigned char	*exp_buf = new unsigned char[32 * 32 * 4];
	auto write_func = [exp_buf](int w, int h, int d, const unsigned char* buf)->bool {memcpy(exp_buf, buf, w*h*d); return true; };

	// write function
	gl::texture::write(A, write_func);
	gl::texture::clean(A);

	// test
	assert_are_equal (0, memcmp (img_data, exp_buf, 32 * 32 * 4));

	// cleanup
	delete[] exp_buf;
}

void write_file_test ()
{
	const unsigned char *img_data = (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABABABABABAB";
	auto A = gl::texture::create (gl::size2d(32, 32));

	// actual image data
	glBindTexture(GL_TEXTURE_2D, A);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);

	// read back image data
	auto tmp_filename = std::string("./temp_texture_write_file_test\0");
	auto write_func_with_filename = [](const std::string& filename, int w, int h, int d, const unsigned char* buf)->bool
	{
		std::ofstream ofs(filename.c_str());
		ofs.write(reinterpret_cast<const char *>(buf), w*h*d);
		ofs.close();
		return true;
	};

	auto write_func = std::bind(write_func_with_filename,
		tmp_filename,
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3,
		std::placeholders::_4);

	// write function
	gl::texture::write(A, write_func);
	gl::texture::clean(A);

	std::ifstream ifs(tmp_filename);
	assert_is_true (ifs.good ());

	unsigned char	*exp_buf = new unsigned char[32 * 32 * 4];
	ifs.read((char*)exp_buf, 32 * 32 * 4);
	ifs.close();

	// test
	assert_are_equal (0, memcmp (img_data, exp_buf, 32 * 32 * 4));

	// cleanup
	delete[] exp_buf;
	std::remove (tmp_filename.c_str());
}

void width_test ()
{
	auto T = gl::texture::create (gl::size2d (64, 128));
	gl::texture::bind (T);
	assert_are_equal (64, gl::texture::width ());
	gl::texture::clean (T);
}
void height_test ()
{
	auto T = gl::texture::create (gl::size2d (64, 128));
	gl::texture::bind (T);
	assert_are_equal (128, gl::texture::height ());
	gl::texture::clean (T);
}
void format_RGBA32F_test ()
{
	auto T = gl::texture::create (gl::size2d (64, 64));
	gl::texture::bind (T);
	assert_are_equal (GL_RGBA32F, gl::texture::format ());
	gl::texture::clean (T);
}
void format_R32F_test ()
{
	auto T = gl::texture::create (gl::size2d (64, 64), GL_R32F);
	gl::texture::bind (T);
	assert_are_equal (GL_R32F, gl::texture::format ());
	gl::texture::clean (T);
}
void format_component_type_test ()
{
	assert_are_equal (1, gl::texture::format_component_count (GL_LUMINANCE));
	assert_are_equal (1, gl::texture::format_component_count (GL_ALPHA));
	assert_are_equal (1, gl::texture::format_component_count (GL_RED));
	assert_are_equal (1, gl::texture::format_component_count (GL_GREEN));
	assert_are_equal (1, gl::texture::format_component_count (GL_BLUE));
	assert_are_equal (1, gl::texture::format_component_count (GL_DEPTH_COMPONENT));
	assert_are_equal (1, gl::texture::format_component_count (GL_R8));
	assert_are_equal (1, gl::texture::format_component_count (GL_R16));
	assert_are_equal (1, gl::texture::format_component_count (GL_R16F));
	assert_are_equal (1, gl::texture::format_component_count (GL_R32F));
	assert_are_equal (1, gl::texture::format_component_count (GL_R8I));
	assert_are_equal (1, gl::texture::format_component_count (GL_R8UI));
	assert_are_equal (1, gl::texture::format_component_count (GL_R16I));
	assert_are_equal (1, gl::texture::format_component_count (GL_R16UI));
	assert_are_equal (1, gl::texture::format_component_count (GL_R32I));
	assert_are_equal (1, gl::texture::format_component_count (GL_R32UI));
	assert_are_equal (2, gl::texture::format_component_count (GL_RG));
	assert_are_equal (2, gl::texture::format_component_count (GL_RG_INTEGER));
	assert_are_equal (2, gl::texture::format_component_count (GL_RG8));
	assert_are_equal (2, gl::texture::format_component_count (GL_RG16));
	assert_are_equal (2, gl::texture::format_component_count (GL_RG16F));
	assert_are_equal (2, gl::texture::format_component_count (GL_RG32F));
	assert_are_equal (2, gl::texture::format_component_count (GL_RG8I));
	assert_are_equal (2, gl::texture::format_component_count (GL_RG8UI));
	assert_are_equal (2, gl::texture::format_component_count (GL_RG16I));
	assert_are_equal (2, gl::texture::format_component_count (GL_RG16UI));
	assert_are_equal (2, gl::texture::format_component_count (GL_RG32I));
	assert_are_equal (2, gl::texture::format_component_count (GL_RG32UI));
	assert_are_equal (2, gl::texture::format_component_count (GL_LUMINANCE_ALPHA));
	assert_are_equal (3, gl::texture::format_component_count (GL_RGB));
	assert_are_equal (4, gl::texture::format_component_count (GL_RGBA));
	assert_are_equal (4, gl::texture::format_component_count (GL_RGBA32F));
}
void target_for_type_GL_TEXTURE_1D_test ()
{
	assert_are_equal (GL_TEXTURE_1D, gl::texture::target_for_type (GL_TEXTURE_1D));
}
void target_for_type_GL_TEXTURE_2D_test ()
{
	assert_are_equal (GL_TEXTURE_2D, gl::texture::target_for_type (GL_TEXTURE_2D));
}
void target_for_type_GL_TEXTURE_3D_test ()
{
	assert_are_equal (GL_TEXTURE_3D, gl::texture::target_for_type (GL_TEXTURE_3D));
}
void target_for_type_GL_DEPTH_COMPONENT_test ()
{
	assert_are_equal (GL_TEXTURE_2D, gl::texture::target_for_type (GL_DEPTH_COMPONENT));
}
void target_for_type_GL_TEXTURE_2D_ARRAY_test ()
{
	assert_are_equal (GL_TEXTURE_2D_ARRAY, gl::texture::target_for_type (GL_TEXTURE_2D_ARRAY));
}
void buffer_size_64_64_3_test ()
{
	assert_are_equal (64*64*3, gl::texture::buffer_size (64, 64, 3));
}
void buffer_size_64_64_4_test ()
{
	assert_are_equal (64*64*4, gl::texture::buffer_size (64, 64, 4));
}
void buffer_size_8_8_3_test ()
{
	assert_are_equal (8*8*3, gl::texture::buffer_size (8, 8, 3));
}
void buffer_size_3_2_3_test ()
{
	assert_are_equal (8*8*3, gl::texture::buffer_size (3, 2, 3));
}
void gl_texture_tests ()
{
	gl::context::create ();
	TEST(create_test);
	TEST(write_test);
	TEST(write_file_test);
	TEST(width_test);
	TEST(height_test);
	TEST(format_RGBA32F_test);
	TEST(format_R32F_test);
	TEST(target_for_type_GL_TEXTURE_1D_test);
	TEST(target_for_type_GL_TEXTURE_2D_test);
	TEST(target_for_type_GL_TEXTURE_3D_test);
	TEST(target_for_type_GL_DEPTH_COMPONENT_test);
	TEST(target_for_type_GL_TEXTURE_2D_ARRAY_test);
	TEST(buffer_size_64_64_3_test);
	TEST(buffer_size_64_64_4_test);
	TEST(buffer_size_8_8_3_test);
	TEST(buffer_size_3_2_3_test);
	gl::context::clean ();
}

int main (int argc, char** argv)
{	
	TEST_GROUP(gl_texture_tests);	
}
