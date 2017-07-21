#include "png.h"

#include <iostream>
#include <fstream>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define PNG_IO_H

namespace png_io
{
  void set_stdout_to_binary ()
  {
    // set cout to binary
#ifdef _WIN32
    if (_setmode(_fileno (stdout), _O_BINARY) == -1)
    {
      std::cerr << "ERR: Cound not set binary mode on stdout" << std::endl;
    }
#endif
  }
  void set_stdout_to_text()
  {
#ifdef _WIN32
    if (_setmode(_fileno (stdout), _O_TEXT) == -1)
    {
      std::cerr << "ERR: Cound not set text mode on stdout" << std::endl;
    }
#endif
  }

  typedef struct img_s
  {
    unsigned int  width;
    unsigned int  height;
    unsigned int  channels;
    unsigned int  bit_depth;
    unsigned char  *buf;
  } img_t;
  typedef struct png_state_s
  {
    png_structp    png;
    png_infop    info;
    png_infop    end_info;
    std::ios*    file_stream;
  } png_state_t;

  void _read_fn (const png_structp png_ptr, png_bytep data, png_size_t len)
  {
    auto state = reinterpret_cast<png_state_t*>(png_get_io_ptr (png_ptr));

    if (state == NULL)          { return; }
    if (state->file_stream == NULL)    { return; }

    dynamic_cast<std::istream*>(state->file_stream)->read (reinterpret_cast<char *>(data), len);
  }
  void _write_fn (const png_structp png_ptr, png_bytep data, png_size_t len)
  {
    auto state = reinterpret_cast<png_state_t*>(png_get_io_ptr (png_ptr));

    if (state == NULL)          { return; }
    if (state->file_stream == NULL)    { return; }

    dynamic_cast<std::ostream*>(state->file_stream)->write (reinterpret_cast<const char *>(data), len);
  }
  void _flush_fn (const png_structp png_ptr)
  {
    auto state = reinterpret_cast<png_state_t*>(png_get_io_ptr(png_ptr));

    if (state == NULL)          { return; }
    if (state->file_stream == NULL)    { return; }

    dynamic_cast<std::ostream*>(state->file_stream)->flush ();
  }

  void set_write_stream (png_state_t& state, std::ostream& output)
  {
    state.file_stream = &output;
    png_set_write_fn (state.png, reinterpret_cast<void*>(&state), _write_fn, _flush_fn);
  }
  void set_read_stream (png_state_t& state, std::istream& input)
  {
    state.file_stream = &input;
    png_set_read_fn (state.png, reinterpret_cast<void*>(&state), _read_fn);
  }

  bool start_write (png_state_t& state)
  {
    state.png = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (state.png == NULL)
    {
      std::cerr << "ERR: could not png_create_write_struct" << std::endl;
      return false;
    }

    state.info = png_create_info_struct (state.png);
    state.end_info = NULL;
    if (state.info == NULL)
    {
      std::cerr << "ERR: could not png_create_info_struct" << std::endl;
      return false;
    }

    if (setjmp (png_jmpbuf (state.png)))
    {
      std::cerr << "ERR: png_init_io" << std::endl;
      return false;
    }
    png_init_io (state.png, NULL);

    return true;
  }
  bool end_write(png_state_t& state)
  {
    if (state.end_info != NULL)
    {
      if (setjmp(png_jmpbuf(state.png)))
      {
        std::cerr << "ERR: png_write_end" << std::endl;
        return false;
      }
      png_write_end(state.png, state.end_info);
    }

    png_destroy_info_struct (state.png, &state.info);
    png_destroy_info_struct (state.png, &state.end_info);
    png_destroy_write_struct (&state.png, &state.info);
    return true;
  }

  bool start_read(png_state_t& state)
  {
    state.png = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (state.png == NULL)
    {
      std::cerr << "ERR: could not png_create_read_struct" << std::endl;
      return false;
    }

    state.info = png_create_info_struct (state.png);
    state.end_info = NULL;
    if (state.info == NULL)
    {
      std::cerr << "ERR: could not png_create_info_struct" << std::endl;
      png_destroy_read_struct (&state.png, &state.info, &state.end_info);
      return false;
    }

    if (setjmp (png_jmpbuf (state.png)))
    {
      std::cerr << "ERR: png_init_io" << std::endl;
      png_destroy_info_struct (state.png, &state.end_info);
      png_destroy_read_struct (&state.png, &state.info, &state.end_info);
      return false;
    }
    png_init_io(state.png, NULL);

    return true;
  }
  bool end_read (png_state_t& state)
  {
    if (state.end_info != NULL)
    {
      if (setjmp (png_jmpbuf (state.png)))
      {
        std::cerr << "ERR: png_read_end" << std::endl;
        return false;
      }
      png_read_end (state.png, state.end_info);
    }

    png_destroy_info_struct (state.png, &state.info);
    png_destroy_info_struct (state.png, &state.end_info);
    png_destroy_read_struct (&state.png, &state.info, &state.end_info);

    return true;
  }

  bool _write_info(png_state_t& state, const unsigned int w, const unsigned int h, const unsigned int c)
  {
    int  ct = 0;

    switch (c)
    {
    case 1: ct = PNG_COLOR_TYPE_GRAY; break;
    case 2: ct = PNG_COLOR_TYPE_GRAY_ALPHA; break;
    case 3: ct = PNG_COLOR_TYPE_RGB; break;
    case 4: ct = PNG_COLOR_TYPE_RGBA; break;
    default: return false;
    }

    if (setjmp(png_jmpbuf (state.png)))
    {
      std::cerr << "ERR: png_set_ihdr, png_write_info" << std::endl;
      return false;
    }
    png_set_IHDR (state.png, state.info, w, h,
      8,
      ct,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_BASE,
      PNG_FILTER_TYPE_BASE);

    png_write_info (state.png, state.info);
    return true;
  }
  bool _write_data (png_state_t& state, const unsigned int w, const unsigned int h, const unsigned int c, const unsigned char* buf)
  {
    // create the rows pointer
    auto rows = new png_bytep[h];

    for (auto i = 0; i < h; i++)
    {
      rows[i] = (png_bytep)(buf + (i*w*c));
    }

    if (setjmp (png_jmpbuf (state.png)))
    {
      std::cerr << "ERR: png_write_image" << std::endl;
      delete[] rows;
      return false;
    }
    png_write_image (state.png, rows);
    delete[] rows;
    return true;
  }

  bool _read_info(png_state_t& state, unsigned int& width, unsigned int& height, unsigned int& channels, unsigned int& bit_depth)
  {
    int  ct;

    png_read_info (state.png, state.info);

    width = png_get_image_width (state.png, state.info);
    height = png_get_image_height (state.png, state.info);
    ct = png_get_color_type (state.png, state.info);
    bit_depth = png_get_bit_depth (state.png, state.info);

    switch (ct)
    {
    case PNG_COLOR_TYPE_GRAY: channels = 1; break;
    case PNG_COLOR_TYPE_GRAY_ALPHA: channels = 2; break;
    case PNG_COLOR_TYPE_RGB: channels = 3; break;
    case PNG_COLOR_TYPE_RGBA: channels = 4; break;
    default: return false;
    }

    return true;
  }
  bool _read_data (png_state_t& state, const unsigned int width, const unsigned int height, const unsigned int channels, const unsigned int bit_depth, unsigned char *buf)
  {
    auto rows = new png_bytep[height];

    for (auto i = 0; i < height; i++)
    {
      rows[i] = (png_bytep)(buf + (i*width*channels*(bit_depth / 8)));
    }

    if (setjmp (png_jmpbuf (state.png)))
    {
      std::cerr << "ERR: png_read_image" << std::endl;
      delete[] rows;
      return false;
    }
    png_read_image (state.png, rows);
    delete[] rows;
    return true;
  }

  /*
  * write a png image to stdout
  * NB: stdout will be set to binary mode for the duration of the function
  * @param w width of the image
  * @param h height of the image
  * @param c channel count of the image (1,2,3, or 4)
  * @param buf block of (w*h*c) bytes describing an image
  */
  bool write_to_stream (std::ostream& stream, const unsigned int w, const unsigned int h, const unsigned int c, const unsigned char *buf)
  {
    png_state_t state;

    if ((w == 0) || (h == 0) || (c == 0))
    {
      std::cerr << "ERR: zero sized image passed to png_io::write_to_stream" << std::endl;
      return false;
    }

    if (buf == NULL)
    {
      std::cerr << "ERR: empty image buffer passed to png_io::write_to_stream" << std::endl;
      return false;
    }

    // write the image to a png
    if (start_write (state) == false)
    {
      std::cerr << "ERR: could not start writing to stream" << std::endl;
      return false;
    }

    set_write_stream (state, stream);

    if (_write_info (state, w, h, c) == false)
    {
      std::cerr << "ERR: could not write image info to stream" << std::endl;
      return false;
    }

    if (_write_data (state, w, h, c, buf) == false)
    {
      std::cerr << "ERR: could not write image data to stream" << std::endl;
      return false;
    }

    if (end_write (state) == false)
    {
      std::cerr << "ERR: could not end_write to stream" << std::endl;
      return false;
    }

    return true;
  }
  /*
  * write a pixel buffer to a file.
  * @param filename name of the file to create
  * @param w width of the image
  * @param h height of the image
  * @param c channel count of the image (1,2,3, or 4)
  * @param buf block of (w*h*c) bytes describing an image
  */
  bool write (const std::string& filename, const unsigned int w, const unsigned int h, const unsigned int c, const unsigned char *buf)
  {
    png_state_t state;

    std::ofstream fpng (filename.c_str(), std::ofstream::out | std::ofstream::binary);
    if (fpng.is_open () == false)
    {
      std::cerr << "ERR: could not write on '" << filename << "'" << std::endl;
      return false;
    }

    if (write_to_stream (fpng, w, h, c, buf) == false)
    {
      std::cerr << "ERR: could not write image to '" << filename << "'" << std::endl;
      fpng.close();
      return false;
    }

    fpng.close();
    return true;
  }
  bool write (const std::string& filename, const img_t& image)
  {
    return write (filename, image.width, image.height, image.channels, image.buf);
  }
  bool write_to_stdout (const unsigned int w, const unsigned int h, const unsigned int c, const unsigned char *buf)
  {
    return write_to_stream (std::cout, w, h, c, buf);
  }

  bool write_buffer (const std::string& filename, const unsigned int w, const unsigned int h, const unsigned int c, const unsigned char *buf)
  {
    return write (filename, w, h, c, buf);
  }

  bool read_from_stream (std::istream& stream, unsigned int& w, unsigned int& h, unsigned int& channels, unsigned int& bit_depth, unsigned char **buf)
  {
    png_state_t    state;

    if (*buf != NULL)
    {
      std::cerr << "ERR: non-zero buffer passed to png_io::read_from_stream" << std::endl;
      return false;
    }

    if (start_read (state) == false)
    {
      std::cerr << "ERR: could not start reading from stream" << std::endl;
      return false;
    }

    //start_read (state);
    set_read_stream(state, stream);

    if (_read_info (state, w, h, channels, bit_depth) == false)
    {
      std::cerr << "ERR: could not read info from stream" << std::endl;
      return false;
    }

    if ((w == 0) || (h == 0) || (channels == 0) || (bit_depth == 0))
    {
      std::cerr << "ERR: zero size read from stream" << std::endl;
      return false;
    }

    // allocate storage for the image
    //*buf = (unsigned char *)malloc(w*h*channels*(bit_depth / 8));
    *buf = new unsigned char[w*h*channels*(bit_depth / 8)];

    if (_read_data (state, w, h, channels, bit_depth, *buf) == false)
    {
      std::cerr << "ERR: could not read data from stream" << std::endl;
      delete[] *buf;
      *buf = NULL;
      return false;
    }

    if (end_read (state) == false)
    {
      std::cerr << "ERR: could not end_read from stream" << std::endl;
      delete[] *buf;
      *buf = NULL;
      return false;
    }

    return true;
  }
  bool read (const std::string& filename, unsigned int& w, unsigned int& h, unsigned int& channels, unsigned int& bit_depth, unsigned char **buf)
  {
    png_state_t  state;

    std::ifstream fpng (filename.c_str(), std::ifstream::in | std::ifstream::binary);
    if (fpng.is_open () == false)
    {
      std::cerr << "ERR: could not read from '" << filename << "'" << std::endl;
      return false;
    }

    if (read_from_stream (fpng, w, h, channels, bit_depth, buf) == false)
    {
      std::cerr << "ERR: could not read image from '" << filename << "'" << std::endl;
      fpng.close ();
      return false;
    }

    fpng.close ();
    return true;
  }
  bool read (const std::string& filename, img_t& image)
  {
    return read (filename, image.width, image.height, image.channels, image.bit_depth, &image.buf);
  }
};

std::ostream& operator<< (std::ostream& strm, const png_io::img_t& img)
{
  png_io::write_to_stream (strm, img.width, img.height, img.channels, img.buf);
  return strm;
}
std::istream& operator>> (std::istream& strm, png_io::img_t& img)
{
  png_io::read_from_stream (strm, img.width, img.height, img.channels, img.bit_depth, &img.buf);
  return strm;
}
