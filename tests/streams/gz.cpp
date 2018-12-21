#include <sstream>
#include <numeric>
#include <array>
#include <vector>
#include <cassert>

#include "streams\gz.h"

bool compress_default_test()
{
	std::string			data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqurstuvwxyz\0";
	std::stringstream	input(data);
	std::stringstream	output("");
	std::string			exp = { 120,-38, 115, 116, 114, 118,113,117,115, -9,-16,
							    -12,-14, -10, -15, -11, -13, 15,  8, 12, 10, 14,
							      9, 13,  11,-113,-120,-116, 74, 76, 74, 78, 73,
							     77, 75, -49, -56, -52, -54,-50,-55,-51,-53, 47,
							     40, 44,  45,  42,  46,  41, 45, 43,-81,-88,-84,
							      2,  0, -43,  76,  19, 116 };

	// compress the input into output
	if (gz::compress(input, output) == false)
		return false;
#if 0
	auto are_sized = output.str().length() == 61;
	auto result = output.str();
	auto are_equal = std::equal(
		std::begin(exp),
		std::end(exp),
		std::begin(result));
	return ((are_sized == true) && (are_equal == true));
#else

	auto d = output.str();

	std::stringstream	decomp("");
	
	if (gz::decompress(output, decomp) == false)
		return false;

	auto ot = output.str();
	auto de = decomp.str();
	auto are_size = de.size() == data.size();
	auto are_equal = std::equal(
		std::begin(data),
		std::end(data),
		std::begin(de));
	return ((are_size == true) && (are_equal == true));
#endif
}
bool compress_gz_test()
{
	// NB: the expected value exp[9] can be 10 or 11...
	std::string			data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqurstuvwxyz\0";
	std::stringstream	input(data);
	std::stringstream	compressed("");
	std::stringstream	decompressed("");

	if (gz::compress(input, compressed, gz::gz) == false)
		return false;

	if (gz::decompress(compressed, decompressed, gz::gz) == false)
		return false;

	auto decompressed_data = decompressed.str();
	auto are_size = decompressed_data.size() == data.size();
	auto are_equal = std::equal(
		std::begin(data),
		std::end(data),
		std::begin(decompressed_data));
	return ((are_size == true) && (are_equal == true));
}
bool compress_zlib_test()
{
	std::string			data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqurstuvwxyz\0";
	std::stringstream	input(data);
	std::stringstream	output("");
	std::string			exp = { 120, -38, 115, 116, 114, 118, 113, 117, 115, -9, -16,
		-12, -14, -10, -15, -11, -13, 15, 8, 12, 10, 14, 9,
		13, 11, -113, -120, -116, 74, 76, 74, 78, 73, 77, 75,
		-49, -56, -52, -54, -50, -55, -51, -53, 47, 40, 44,
		45, 42, 46, 41, 45, 43, -81, -88, -84, 2, 0, -43, 76,
		19, 116 };

	gz::compress(input, output, gz::zlib);

	auto are_sized = output.str().length() == 61;
	auto are_equal = std::equal(
		std::begin(exp),
		std::end(exp),
		std::begin(output.str()));
	return ((are_sized == true) && (are_equal == true));
}
bool decompress_default_test()
{
	std::string			data = { 120, -38, 115, 116, 114, 118, 113, 117, 115, -9, -16,
		-12, -14, -10, -15, -11, -13, 15, 8, 12, 10, 14, 9,
		13, 11, -113, -120, -116, 74, 76, 74, 78, 73, 77, 75,
		-49, -56, -52, -54, -50, -55, -51, -53, 47, 40, 44,
		45, 42, 46, 41, 45, 43, -81, -88, -84, 2, 0, -43, 76,
		19, 116 };
	std::stringstream	input(data);
	std::stringstream	output("");
	std::string			exp = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqurstuvwxyz\0";

	if (gz::decompress(input, output) == false)
		return false;

	auto are_sized = output.str().length() == 53;
	auto are_equal = std::equal(
		std::begin(exp),
		std::end(exp),
		std::begin(output.str()));
	return ((are_sized == true) && (are_equal == true));
}
bool decompress_gz_test()
{
	// NB: the data value exp[9] can be 10 or 11...
	std::string			data = { 31,-117,   8,  0,  0,  0,  0,  0,  2, 10,115,
				                116, 114, 118,113,117,115, -9,-16,-12,-14,-10,
		                        -15, -11, -13, 15,  8, 12, 10, 14,  9, 13, 11,
		                       -113,-120,-116, 74, 76, 74, 78, 73, 77, 75,-49,
			                    -56, -52, -54,-50,-55,-51,-53, 47, 40, 44, 45,
		                         42,  46,  41, 45, 43,-81,-88,-84,  2,  0, 77,
		                       -128, -76,-101, 53,  0,  0,  0 };
	std::stringstream	input(data);
	std::stringstream	output("");
	std::string			exp = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqurstuvwxyz\0";

	// decompress output into input
	if (gz::decompress(input, output) == false)
		return false;

	auto are_sized = output.str().length() == 53;
	auto are_equal = std::equal(
		std::begin(exp),
		std::end(exp),
		std::begin(output.str()));
	return ((are_sized == true) && (are_equal == true));
}
bool decompress_zlib_test()
{
	std::string			data = { 120, -38, 115, 116, 114, 118, 113, 117, 115, -9, -16,
		-12, -14, -10, -15, -11, -13, 15, 8, 12, 10, 14, 9,
		13, 11, -113, -120, -116, 74, 76, 74, 78, 73, 77, 75,
		-49, -56, -52, -54, -50, -55, -51, -53, 47, 40, 44,
		45, 42, 46, 41, 45, 43, -81, -88, -84, 2, 0, -43, 76,
		19, 116 };
	std::stringstream	input(data);
	std::stringstream	output("");
	std::string			exp = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqurstuvwxyz\0";

	if (gz::decompress(input, output) == false)
		return false;

	auto are_sized = output.str().length() == 53;
	auto are_equal = std::equal(
		std::begin(exp),
		std::end(exp),
		std::begin(output.str()));
	return ((are_sized == true) && (are_equal == true));
}

bool compressor_class_cycle_test()
{
	std::string			data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqurstuvwxyz\0";
	std::stringstream	compressed;
	std::stringstream	decompressed;

	{
		gz::compressor p(compressed);
		p.put(data.c_str(), data.length());
		p.flush();
	}

	if (gz::decompress(compressed, decompressed) == false)
		return false;

	auto d = decompressed.str();

	auto input_lengths_match = data.length() == d.length();
	auto input_contents_match = std::equal(
		std::begin(data),
		std::end(data),
		std::begin(d));

	return ((input_lengths_match == true) &&
			(input_contents_match == true));
}
bool compressor_class_cycle_congruent_with_functions_test()
{
	std::string			data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqurstuvwxyz\0";
	std::stringstream	compressed_fn;
	std::stringstream	compressed_cls;
	std::stringstream	decompressed_fn;
	std::stringstream	decompressed_cls;

	{
		gz::compressor p(compressed_cls);
		p.put(data.c_str(), data.length());
		p.flush();
	}

	if (gz::compress(std::stringstream(data), compressed_fn, gz::zlib) == false)
		return false;

	auto c_fn = compressed_fn.str();
	auto c_cls = compressed_cls.str();

	auto compressed_lengths_match = c_fn.length() == c_cls.length();
	auto compressed_contents_match = std::equal(
		std::begin(c_fn),
		std::end(c_fn),
		std::begin(c_cls));

	// decompress the streams
	if (gz::decompress(compressed_fn, decompressed_fn) == false)
		return false;

	if (gz::decompress(compressed_cls, decompressed_cls) == false)
		return false;

	auto d_fn = decompressed_fn.str();
	auto d_cls = decompressed_cls.str();

	// compare
	auto decompressed_lengths_match = d_fn.length() == d_cls.length();
	auto decompressed_contents_match = std::equal(
		std::begin(d_fn),
		std::end(d_fn),
		std::begin(d_cls));

	return ((compressed_lengths_match == true) && 
		    (compressed_contents_match == true) &&
			(decompressed_lengths_match == true) &&
			(decompressed_contents_match == true));
}

bool decompressor_class_cycle_test()
{
	std::string			data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqurstuvwxyz\0";
	std::stringstream	compressed;
	std::stringstream	decompressed;

	{
		gz::compressor p(compressed);
		p.put(data.c_str(), data.length());
		p.flush();
	}

	{
		auto compressed_data = compressed.str();
		gz::decompressor p(decompressed);
		p.put(compressed_data.c_str(), compressed_data.length());
		p.flush();
	}

	auto d = decompressed.str();

	auto input_lengths_match = data.length() == d.length();
	auto input_contents_match = std::equal(
		std::begin(data),
		std::end(data),
		std::begin(d));

	return ((input_lengths_match == true) &&
			(input_contents_match == true));
}
bool decompressor_class_cycle_congruent_with_functions_test()
{
	std::string			data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqurstuvwxyz\0";
	std::stringstream	compressed_fn;
	std::stringstream	compressed_cls;
	std::stringstream	decompressed_fn;
	std::stringstream	decompressed_cls;

	{
		gz::compressor p(compressed_cls);
		p.put(data.c_str(), data.length());
		p.flush();
	}

	if (gz::compress(std::stringstream(data), compressed_fn, gz::zlib) == false)
		return false;

	auto c_fn = compressed_fn.str();
	auto c_cls = compressed_cls.str();

	auto compressed_lengths_match = c_fn.length() == c_cls.length();
	auto compressed_contents_match = std::equal(
		std::begin(c_fn),
		std::end(c_fn),
		std::begin(c_cls));

	{
		gz::decompressor	p(decompressed_cls);
		p.put(c_cls.c_str(), c_cls.length());
		p.flush();
	}

	// decompress the streams
	if (gz::decompress(compressed_fn, decompressed_fn) == false)
		return false;
	
	auto d_fn = decompressed_fn.str();
	auto d_cls = decompressed_cls.str();

	// compare
	auto decompressed_lengths_match = d_fn.length() == d_cls.length();
	auto decompressed_contents_match = std::equal(
		std::begin(d_fn),
		std::end(d_fn),
		std::begin(d_cls));

	return ((compressed_lengths_match == true) &&
			(compressed_contents_match == true) &&
			(decompressed_lengths_match == true) &&
			(decompressed_contents_match == true));
}


int main (int argc, char** argv)
{
	auto ret_vals = {
		compress_default_test(),
		compress_gz_test(),
		compress_zlib_test(),

		decompress_default_test(),
		decompress_gz_test(),
		decompress_zlib_test(),

		compressor_class_cycle_test(),
		compressor_class_cycle_congruent_with_functions_test(),

		decompressor_class_cycle_test(),
		decompressor_class_cycle_congruent_with_functions_test()
	};

	auto passes = std::accumulate(
		std::begin(ret_vals),
		std::end(ret_vals),
		0,
		[](const int s, const bool a)
		{
			return (s + ((a == true) ? 1 : 0));
		});

	auto fails = ret_vals.size() - passes;

	if (fails != 0)
		throw std::exception();

	return fails;
}