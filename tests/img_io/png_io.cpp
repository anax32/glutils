#include <sstream>
#include <cassert>
#include <cstring>

#include "img_io/png_io.h"

#include "maketest.h"

void write_to_stream_test ()
{
  // create an empty image
  const unsigned char  *img_buf = (unsigned char*)"ABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABAB";
  const unsigned int  w = 8;
  const unsigned int  h = 8;
  const unsigned int  c = 1;
  std::stringstream  ss;

  auto start = ss.tellp();
  png_io::write_to_stream(ss, w, h, c, img_buf);
  auto end = ss.tellp();
  auto len = end - start;

  // test
  assert(len > 0);
  assert(len == 68);
  assert(ss.str().substr(1, 3).compare("PNG") == 0);
}
void write_to_file_test ()
{
  std::string fname = "./temp_pngio_write_to_file_test\0";
  const png_io::img_t  image = { 8, 8, 1, 8, (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABABABABABAB" };

  assert(png_io::write(fname, image) == true);

  // attempt to open the file
  std::ifstream ifs(fname);
  assert(ifs.good() == true);
  ifs.close();

  // cleanup
  std::remove(fname.c_str());
}

void png_write_tests ()
{
  TEST(write_to_stream_test);
  TEST(write_to_file_test);
}

void read_from_stream_test()
{
  // create an empty image
  const unsigned char *img_buf = (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABABABABABAB";
  const unsigned int  w = 8;
  const unsigned int  h = 8;
  const unsigned int  c = 1;
  std::stringstream   ss;
  unsigned char       *exp_buf = NULL;
  unsigned int        exp_w = 0;
  unsigned int        exp_h = 0;
  unsigned int        exp_c = 0;
  unsigned int        exp_bd = 0;

  png_io::write_to_stream(ss, w, h, c, img_buf);

  png_io::read_from_stream(ss, exp_w, exp_h, exp_c, exp_bd, &exp_buf);

  assert(exp_w == w);
  assert(exp_h == h);
  assert(exp_c == c);
  assert(exp_bd == 8);

  assert(memcmp(img_buf, exp_buf, 64) == 0);

  delete[] exp_buf;
}
void read_from_stream_operator_test ()
{
  // create an empty image
  const png_io::img_t image = { 8, 8, 1, 8, (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABABABABABAB" };
  png_io::img_t       result = { 0 };
  std::stringstream   ss;

  ss << image;
  ss >> result;

  assert(result.width == image.width);
  assert(result.height == image.height);
  assert(result.channels == image.channels);
  assert(result.bit_depth == 8);

  assert(memcmp(result.buf, image.buf, 64) == 0);

  delete[] result.buf;
}
void read_from_file_test ()
{
  std::string fname = "./temp_pngio_read_file_file_test\0";
  const png_io::img_t   image = { 8, 8, 1, 8, (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABABABABABAB" };
  png_io::img_t         result = { 0 };

  assert (png_io::write (fname, image) == true);

  // attempt to open the file
  std::ifstream ifs (fname);
  assert (ifs.good () == true);
  ifs.close ();

  assert (png_io::read (fname, result) == true);

  assert (result.width == image.width);
  assert (result.height == image.height);
  assert (result.channels == image.channels);
  assert (result.bit_depth == 8);

  assert (memcmp (result.buf, image.buf, 64) == 0);

  // cleanup
  std::remove (fname.c_str ());
  delete[] result.buf;
}

void png_read_tests ()
{
  TEST(read_from_stream_test);
  TEST(read_from_stream_operator_test);
  TEST(read_from_file_test);
}

int main (int argc, char** argv)
{
  TEST_GROUP(png_write_tests);
  TEST_GROUP(png_read_tests);
}