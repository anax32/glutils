#include <sstream>
#include <array>
#include <numeric>

#include "streams\png_s.h"

bool write_to_stream_test ()
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
  auto are_sized = len == 68;
  auto has_hdr = ss.str().substr(1, 3).compare("PNG") == 0;

  return ((are_sized == true) && (has_hdr == true));
}
bool write_to_file_test ()
{
  std::string fname = "./temp_pngio_write_to_file_test\0";
  const png_io::img_t  image = { 8, 8, 1, 8, (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABABABABABAB" };

  auto has_written = png_io::write(fname, image);

  // attempt to open the file
  std::ifstream ifs(fname);
  auto can_read = ifs.good();
  ifs.close();

  // cleanup
  std::remove(fname.c_str());

  return ((has_written == true) && (can_read == true));
}

bool readback_from_stream_test()
{
  // create an empty image
  //const unsigned char *img_buf = (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABABABABABAB";
  std::array<unsigned char, 65> img_buf = { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABABABABABAB" };
  const unsigned int  w = 8;
  const unsigned int  h = 8;
  const unsigned int  c = 1;
  std::stringstream   ss;
  unsigned char       *exp_buf = NULL;
  unsigned int        exp_w = 0;
  unsigned int        exp_h = 0;
  unsigned int        exp_c = 0;
  unsigned int        exp_bd = 0;

  auto has_written = png_io::write_to_stream(ss, w, h, c, &img_buf[0]);

  auto can_read = png_io::read_from_stream(ss, exp_w, exp_h, exp_c, exp_bd, &exp_buf);

  auto has_width = exp_w == w;
  auto has_height = exp_h == h;
  auto has_chann = exp_c == c;
  auto has_bidep = exp_bd == 8;

  //auto has_cont = memcmp (img_buf, exp_buf, 64) == 0;
  auto has_cont = std::equal(
	  std::begin(img_buf),
	  std::prev(std::end(img_buf)),	// accomodate the terminating null
	  exp_buf);

  delete[] exp_buf;

  return ((has_written == true) &&
	      (can_read == true) &&
		  (has_width == true) &&
		  (has_height == true) &&
		  (has_chann == true) &&
	      (has_bidep == true) &&
		  (has_cont == true));
}
bool read_from_stream_operator_test ()
{
  // create an empty image
  const png_io::img_t image = { 8, 8, 1, 8, (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABABABABABAB" };
  png_io::img_t       result = { 0 };
  std::stringstream   ss;

  ss << image;
  ss >> result;

  auto has_width = result.width == image.width;
  auto has_height = result.height == image.height;
  auto has_chann = result.channels == image.channels;
  auto has_bidep = result.bit_depth == 8;

  auto has_cont = std::equal(
	  image.buf,
	  image.buf + 64,
	  result.buf);

  delete[] result.buf;

  return ((has_width == true) &&
		  (has_height == true) &&
		  (has_chann == true) &&
	      (has_bidep == true) &&
	      (has_cont == true));
}
bool read_from_file_test ()
{
  std::string fname = "./temp_pngio_read_file_file_test\0";
  const png_io::img_t   image = { 8, 8, 1, 8, (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABABABABABAB" };
  png_io::img_t         result = { 0 };

  auto has_written = png_io::write (fname, image) == true;

  // attempt to open the file
  std::ifstream ifs (fname);
  auto can_read = ifs.good () == true;
  ifs.close ();

  auto can_parse = png_io::read (fname, result) == true;

  auto has_width = result.width == image.width;
  auto has_height = result.height == image.height;
  auto has_chann = result.channels == image.channels;
  auto has_bidep = result.bit_depth == 8;

  auto has_cont = std::equal(
	  image.buf,
	  image.buf + 64,
	  result.buf);

  // cleanup
  std::remove (fname.c_str ());
  delete[] result.buf;

  return ((has_written == true) &&
		  (can_read == true) &&
	      (can_parse == true) &&
	      (has_width == true) &&
	      (has_height == true) &&
	      (has_chann == true) &&
	      (has_bidep == true) &&
	      (has_cont == true));
}

int main (int argc, char** argv)
{
	auto ret_vals = {
		write_to_stream_test(),
		write_to_file_test(),

		readback_from_stream_test(),
		read_from_stream_operator_test(),
		read_from_file_test() };

	auto passes = std::accumulate(
		std::begin(ret_vals),
		std::end(ret_vals),
		0,
		[](const int s, const bool a)
		{
			return (s + ((a == true) ? 1 : 0));
		});

	auto fails = ret_vals.size() - passes;

	return (int)fails;
}