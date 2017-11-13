#include <zlib.h>
#include <iostream>	// only for cerr
#include <sstream>
#include <functional>

namespace gz
{
	typedef enum mode_s
	{
		zlib,
		gz,
		any
	} mode_t;

	unsigned int _adjust_window_size_for_mode(const mode_t mode, const unsigned int initial_size)
	{
		switch (mode)
		{
			case zlib:
			{
				return initial_size;
			}
			case gz:
			{
				return initial_size + 16;
			}
			case any:
			{
				return initial_size + 32;
			}
			default:
			{
				std::cerr << "ERR: unknown mode value ["
						  << mode
						  << "]"
						  << std::endl
						  << std::flush;
				return initial_size;
			}
		}
	}

	bool _compress_init(z_stream& zs, unsigned int window)
	{
		auto ret_val = deflateInit2(
			&zs,
			Z_BEST_COMPRESSION,
			Z_DEFLATED,
			window,
			8,
			Z_DEFAULT_STRATEGY);

		switch (ret_val)
		{
			case Z_OK:
			{
				break;
			}
			case Z_MEM_ERROR:
			case Z_STREAM_ERROR:
			case Z_VERSION_ERROR:
			default:
			{
				std::cerr << "ERR: could note deflateInit ["
						  << ((zs.msg == NULL) ? "unknown" : zs.msg)
						  << "]"
					      << std::endl
						  << std::flush;
				return false;
			}
		}

		return true;
	}
	bool _compress_end(z_stream& zs)
	{
		// flush the compressor
		switch (deflate(&zs, Z_FINISH))
		{
			case Z_OK:
			{
				break;
			}
			case Z_STREAM_END:
			case Z_STREAM_ERROR:
			case Z_BUF_ERROR:
			{
				// some kind of error happened...
				break;
				// we don't return false here,
				// we try to call deflateEnd, which
				// might result in "return true"
			}
		}

		// end the compression stream
		switch (deflateEnd(&zs))
		{
			case Z_OK:
			{
				break;
			}
			case Z_STREAM_ERROR:
			case Z_DATA_ERROR:
			default:
			{
				std::cerr << "ERR: deflateEnd ["
						  << ((zs.msg == NULL) ? "unknown" : zs.msg)
						  << "]"
						  << std::endl
						  << std::flush;
				return false;
			}
		}

		return true;
	}
	bool _compress_step(z_stream& zs, int flush)
	{
		auto ret_val = deflate (&zs, flush);

		// check what happend
		switch (ret_val)
		{
			case Z_OK:
			{
				return true;
			}
			case Z_STREAM_END:
			{
#if 1
				// why would this happen?
				std::cerr << "ERR: unexpected Z_STREAM_END"
							<< std::endl
							<< std::flush;
				return false;
#else
				// we should not finish the compression stream
				// because we terminate it with an explicit call
				// to _compress_end.
				return _compress_end(zs);
#endif
			}
			case Z_STREAM_ERROR:
			case Z_BUF_ERROR:	// could recover from here, but treat as error
			default:
			{
				std::cerr << "ERR: deflate ["
							<< ((zs.msg == NULL) ? "unknown" : zs.msg)
							<< "]"
							<< std::endl
							<< std::flush;
				return false;
			}
		}
	}

	bool _decompress_init(z_stream& zs, unsigned int window)
	{
		auto ret_val = inflateInit2(&zs, window);

		switch (ret_val)
		{
			case Z_OK:
			{
				break;
			}
			case Z_MEM_ERROR:
			case Z_VERSION_ERROR:
			case Z_STREAM_ERROR:
			default:
			{
				std::cerr << "ERR: could note inflateInit ["
						  << zs.msg
						  << "]"
						  << std::endl
						  << std::flush;
				return false;
			}
		}

		return true;
	}
	bool _decompress_end(z_stream& zs)
	{
		switch (inflate(&zs, Z_FINISH))
		{
			case Z_OK:
			{
				break;
			}
			default:
			{
				break;
			}
		}

		switch (inflateEnd(&zs))
		{
			case Z_OK:
			{
				break;
			}
			case Z_STREAM_ERROR:
			case Z_DATA_ERROR:
			default:
			{
				std::cerr << "ERR: inflateEnd ["
					<< ((zs.msg == NULL) ? "unknown" : zs.msg)
					<< "]"
					<< std::endl;
				return false;
			}
		}

		return true;
	}
	bool _decompress_step(z_stream& zs, int flush)
	{
		auto ret_val = inflate(&zs, flush);

		switch (ret_val)
		{
			case Z_OK:
			{
				break;
			}
			case Z_STREAM_END:
			{
				return _decompress_end(zs);
			}
			case Z_STREAM_ERROR:
			case Z_BUF_ERROR:	// could continue from here, but treat as error
			case Z_DATA_ERROR:
			default:
			{
				std::cerr << "ERR: inflate ["
						  << ((zs.msg == NULL) ? "unknown" : zs.msg)
						  << "]"
						  << std::endl;
				return false;
			}
		}

		return true;
	}

	class zstream_buffer
	{
	protected:
		void reset_input()
		{
			std::fill(
				std::begin(in_buf),
				std::end(in_buf),
				0);
			zs.next_in = (Bytef*)&in_buf.front();
			zs.avail_in = 0;
		}
		void reset_output()
		{
			std::fill(
				std::begin(ot_buf),
				std::end(ot_buf),
				0);
			zs.next_out = (Bytef*)&ot_buf.front();
			zs.avail_out = sz;
		}
		auto in_buf_freespace()
		{
			return (in_buf.size() - zs.avail_in);
		}
		auto ot_buf_freespace()
		{
			return (zs.avail_out);
		}
		auto ot_buf_used_space()
		{
			return (sz - zs.avail_out);
		}
	public:
		static const size_t		sz = 256;
		std::array<char, sz>	in_buf;
		std::array<char, sz>	ot_buf;
		z_stream				zs;

		zstream_buffer()
		{
			memset(&zs, 0, sizeof(z_stream));
			reset();
		}
		void reset()
		{
			reset_input();
			reset_output();
		}
		void put(std::istream& input)
		{
			input.read((char*)&in_buf.front(), sz);
			zs.avail_in += input.gcount();
		}
		void get(std::ostream& output)
		{
			auto cb = sz - zs.avail_out;
			auto p = output.tellp();
			output.write((char*)&ot_buf.front(), cb);
			// check we wrote
			auto put_cb = output.tellp() - p;

			// move the buffer down
			if (put_cb == cb)
			{
				reset_output();
			}
			else
			{
				// copy part of the buffer down
				std::copy(
					std::next(std::begin(ot_buf), put_cb),
					std::end(ot_buf),
					std::begin(ot_buf));
				// clear the end part of the buffer
				std::fill(
					std::next(std::begin(ot_buf), sz - put_cb),
					std::end(ot_buf),
					0);
				zs.avail_out += put_cb;
			}
		}
	};

	typedef std::array<std::function<bool(z_stream&, int)>, 3> compression_process_t;

	// abstraction of the compression/decompression process
	// input data is passed in via an input stream (compressed/decompressed)
	// output data is written to the output stream (decompressed/compressed)
	// process_fns are the functions which do the work
	bool _process(std::istream& input, std::ostream& output, compression_process_t process_fn, mode_t mode)
	{
		zstream_buffer		buffer;
		unsigned int		window = 15;

		window = _adjust_window_size_for_mode(mode, window);
		process_fn[0](buffer.zs, window);

		auto flush = input.eof() ? Z_FINISH : Z_NO_FLUSH;

		// get the de/compressed data
		while (input.eof() == false)
		{
			// get some input if the input buffer has been entirely consumed
			if (buffer.zs.avail_in == 0)
			{
				buffer.put(input);
			}

			// perform the de/compression
			if (process_fn[1](buffer.zs, flush) == false)
			{
				break;
			}

			// copy the data to the output stream
			buffer.get(output);
			// reset the buffers
			buffer.reset();
		}

		// ensure the last few bytes are read/written
		process_fn[2](buffer.zs, 0);
		buffer.get(output);
		return true;
	}
	// read the data in the input stream, compress it and write it to the output stream
	bool compress(std::istream& input, std::ostream& output, mode_t mode = zlib)
	{
		// the _compress_end function doesn't fit the method
		// signature for the function array, so stick it in 
		// lambda.
		return _process(
			input,
			output,
			compression_process_t
			{
				_compress_init,
				_compress_step,
				[](z_stream& zs, int dummy) {return _compress_end(zs); }
			},
			mode);
	}
	// read the compressed data in the input, decompress it and write it to the output stream
	bool decompress(std::istream& input, std::ostream& output, mode_t mode = any)
	{
		// the eof of the compression stream is encountered and
		// handled in _decompress_step, so we do not explicitly
		// call _decompress_end. Just pass a dummy function to
		// the streaming process.
		return _process(
			input,
			output,
			compression_process_t
			{
				_decompress_init,
				_decompress_step,
				[](z_stream& zs, int dummy) {return true; }
			},
			mode);
	}

#if 0
	// custom stream manipulators
	inline int getcompressed()
	{
		static int i = std::ios_base::xalloc();
		return i;
	}

	// set a custom int on the stream to indicate compression state
	std::ostream& compress(std::ostream& os)		{ os.iword(getcompressed()) = 1; return os; }
	std::ostream& uncompressed(std::ostream& os)	{ os.iword(getcompressed()) = 0; return os; }
#endif

	class compressor : public zstream_buffer
	{
	protected:
		std::ostream_iterator<char>	output;

	public:
		// creates a compressor object which will compress data and
		// write the output to the output_stream_iterator
		compressor(std::ostream_iterator<char> output_stream_iterator)
			: output (output_stream_iterator)
		{
			auto w_sz = 15;
			auto mode = zlib;
			
			if (_compress_init(zs, _adjust_window_size_for_mode(mode, w_sz)) == false)
				std::cerr << "fail in constructor LOL" << std::endl;

			reset();
		}
		// finishes the compression stream and destroys the compression object
		virtual ~compressor()
		{
			// write the end of compression stream stuff
			_compress_end(zs);

			// copy to the output stream
			std::copy(
				std::begin(ot_buf),
				std::next(std::begin(ot_buf), ot_buf_used_space()),
				output);

			reset_output();
		}
		// flush the compression buffer to the output stream
		// does NOT finish the compression process, to complete
		// the process call the destructor
		void flush()
		{
			// consume the buffer
			_compress_step(zs, Z_NO_FLUSH);
			reset_input();

			// copy the out buffer to the ostream
			std::copy(
				std::begin(ot_buf),
				std::next(std::begin(ot_buf), ot_buf_used_space()),
				output);

			// reset the output
			reset_output();
		}
		// writes a byte buffer to the compression stream
		void put(const char* a, size_t input_length)
		{
			auto s = a;
			auto n = a + input_length;

			while (std::distance(s, n) > 0)
			{
				// if everything fits in the buffer...
				if (std::distance(s, n) < in_buf_freespace () + 1)
				{
					auto b = std::next(std::begin(in_buf), zs.avail_in);

					std::copy(s, n, b);
					zs.avail_in += std::distance(s, n);
					s = n;
				}

				// if it does not...
				if (std::distance(s, n) > in_buf_freespace())
				{
					flush();
					
					// reset
					zs.avail_in = 0;

					// take as much as we can
					auto n2 = std::next(s, in_buf_freespace());
					auto b = std::next(std::begin(in_buf), zs.avail_in);
					std::copy(s, n2, b);

					// update the buffers
					zs.avail_in += std::distance(s, n2);
					s = n2;
				}
			}
		}

		// converts the object to a byte buffer and writes to the compression stream
		template<typename T>
		void put (const T& obj)
		{
			auto s = (char*)&obj;
			auto n = ((char*)&obj) + sizeof(T);

			put(s, sizeof(T));
		}
	};
	class decompressor : public zstream_buffer
	{
	protected:
		std::ostream_iterator<char> output;
	public:
		decompressor(std::ostream_iterator<char> output_stream_iterator)
			: output(output_stream_iterator)
		{
			auto w_sz = 15;
			auto mode = zlib;

			if (_decompress_init(zs, _adjust_window_size_for_mode(mode, w_sz)) == false)
				std::cerr << "fail in constructor LOL!" << std::endl;

			reset();
		}
		// the caller should encounter Z_FINISH by pulling data out of the stream,
		// it does not need to be finished explicitly.
		virtual ~decompressor()
		{}
		void flush()
		{
			_decompress_step(zs, Z_NO_FLUSH);
			reset_input();

			std::copy(
				std::begin(ot_buf),
				std::next(std::begin(ot_buf), ot_buf_used_space()),
				output);

			reset_output();
		}
		// writes a byte buffer to the compression stream
		void put(const char* a, size_t input_length)
		{
			auto s = a;
			auto n = a + input_length;

			while (std::distance(s, n) > 0)
			{
				// if everything fits in the buffer...
				if (std::distance(s, n) < in_buf_freespace() + 1)
				{
					auto b = std::next(std::begin(in_buf), zs.avail_in);

					std::copy(s, n, b);
					zs.avail_in += std::distance(s, n);
					s = n;
				}

				// if it does not...
				if (std::distance(s, n) > in_buf_freespace())
				{
					flush();

					// reset
					zs.avail_in = 0;

					// take as much as we can
					auto n2 = std::next(s, in_buf_freespace());
					auto b = std::next(std::begin(in_buf), zs.avail_in);
					std::copy(s, n2, b);

					// update the buffers
					zs.avail_in += std::distance(s, n2);
					s = n2;
				}
			}
		}

		// converts the object to a byte buffer and writes to the compression stream
		template<typename T>
		void put(const T& obj)
		{
			auto s = (char*)&obj;
			auto n = ((char*)&obj) + sizeof(T);

			put(s, sizeof(T));
		}
	};
}