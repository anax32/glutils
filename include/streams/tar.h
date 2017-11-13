#include <sys/stat.h>

#include <iomanip>
#include <sstream>
#include <fstream>

#include <numeric>

namespace files
{
	namespace tar
	{
		std::string to_oct_string(const unsigned int number, const unsigned int padding)
		{
			std::stringstream ss;
			ss << std::setfill('0') << std::setw(padding) << std::oct << number;
			return ss.str();
		}
		std::string to_string(const unsigned int number, const unsigned int padding)
		{
			std::stringstream ss;
			ss << std::setfill('0') << std::setw(padding) << number;
			return ss.str();
		}
		unsigned int from_oct_string(const std::string& number)
		{
			unsigned int num = 0;
			std::stringstream ss(number);
			ss >> std::oct >> num;
			return num;
		}
		unsigned int from_string(const std::string& number)
		{
			unsigned int num = 0;
			std::stringstream ss(number);
			ss >> num;
			return num;
		}

		/*
		* append the contents of a file to the tar archive
		*/
		void append(std::iostream& strm, std::istream& file, const std::string filename, const unsigned int file_size = -1, const unsigned int file_time = -1)
		{
			char buf[512] = { 0 };
			auto file_mode = 0100777;
			auto file_user = 0x0000000;
			auto file_group = 0x0000000;
#if _WIN32
			struct _stat file_info = { 0 };
			_stat(filename.c_str(), &file_info);
#elif __linux__
			struct stat file_info = { 0 };
			stat(filename, &file_info);
#endif
			if (file_size != -1)			{ file_info.st_size = file_size; }
			if (file_time != -1)			{ file_info.st_mtime = file_time; }

			// create a header
			strm << std::left;
			strm << std::setfill('\0');

			// file info
			strm << std::setw(100) << filename;								// filename,		0,100
			strm << std::setw(8) << to_string(file_mode, 7);				// mode,			100,8
			strm << std::setw(8) << to_oct_string(file_user, 7);			// uid,				108,8
			strm << std::setw(8) << to_oct_string(file_group, 7);			// gid,				116,8
			strm << std::setw(12) << to_oct_string(file_info.st_size, 11);	// size,			124,12
			strm << std::setw(12) << to_oct_string(file_info.st_mtime, 6);	// mtime,			136,12

			// checksum, entered as 7 ' ' + ' ', encoded as SIX chars + '\0' + ' '
			strm << std::setw(7) << "       ";		// checksum,		148, 8
			strm << std::setw(1) << " ";			// post checksum space
			strm << std::setw(1) << "0";			// type,			156, 1
			strm << std::setw(100) << '\0';			// link name,		157, 100
			strm << std::setw(6) << "ustar";		// magic,			257, 6
			strm << std::setw(2) << "00";			// version,			263, 2
			strm << std::setw(32) << '\0';			// user name,		265, 32
			strm << std::setw(32) << '\0';			// group name,		297, 32
			strm << std::setw(8) << '\0';			// device maj,		329, 8
			strm << std::setw(8) << '\0';			// device min,		337, 8
			strm << std::setw(155) << '\0';			// filename prefix, 345, 155
			strm << std::setw(12) << '\0';			// padd to 512

			// compute the checksum
			strm.seekg(-512, std::ios::end);
			strm.get(buf, 512);

			// FIXME: use stream iterators somehow, this one invalidates the stream, probably by reading to eof
			//chksm = std::accumulate(std::istream_iterator<char>(strm), std::istream_iterator<char>(), 0);
			auto chksm = std::accumulate(std::begin(buf), std::end(buf), 0);

			// skip back, write the checksum, skip forward
			strm.seekp(-(512 - 148), std::ios::end);
			strm.write(to_oct_string(chksm, 6).c_str(), 6);
			strm.write("\0", 1);
			strm.seekp(0, std::ios::end);

			// append the file in 512 byte chunks
			while (file.eof() == false)
			{
				file.read(buf, 512);
				strm.write(buf, 512);
			}
		}
		void append(std::iostream& strm, const char *filename)
		{
			std::ifstream	ifs(filename, std::ios::in | std::ios::binary);

			if (ifs.good() == false)
				return;

			auto file_mode = 0100777;
			auto file_user = 0x0000000;
			auto file_group = 0x0000000;
#if _WIN32
			struct _stat file_info = { 0 };
			_stat(filename, &file_info);
#elif __linux__
			struct stat file_info = { 0 };
			stat(filename, &file_info);
#endif
			append(strm, ifs, filename, file_info.st_size, file_info.st_mtime);

			ifs.close();
		}
		/*
		* remove a single file entry, if there are any contents, write them to output
		*/
		void extract(std::istream& input, std::ostream& output)
		{
			std::string		filename;
			unsigned int	file_mode = 0;
			unsigned int	file_user = 0;
			unsigned int	file_group = 0;
			unsigned int	file_size = 0;
			unsigned int	file_mtime = 0;
			unsigned int	chksm;
			char			buf[512] = { 0 };

			// read the block
#if 0
			input.read(buf, 100);	filename.assign(buf);
			input.read(buf, 8);		file_mode = from_string(buf);
			input.read(buf, 8);		file_user = from_oct_string(buf);
			input.read(buf, 8);		file_group = from_oct_string(buf);
			input.read(buf, 12);	file_size = from_oct_string(buf);
			input.read(buf, 12);	file_mtime = from_oct_string(buf);

			// skip over the remaining header, we don't use it
			input.seekg(512 - 148, std::ios::cur);
#else
			input.read(buf, 512);

			if (input.gcount() != 512)
			{
			//	std::cerr << "ERR: could not read tar block" << std::endl;
				return;
			}

			// check the block is non-zero
			if (std::accumulate(buf, buf + 512, 0) == 0)
			{
			//	std::cerr << "ERR: read empty tar block" << std::endl;
				return;
			}

			filename.assign(buf, buf + 100);
			file_mode = from_string(std::string(buf + 100, buf + 108));
			file_user = from_oct_string(std::string(buf + 108, buf + 116));
			file_group = from_oct_string(std::string(buf + 116, buf + 124));
			file_size = from_oct_string(std::string(buf + 124, buf + 136));
			file_mtime = from_oct_string(std::string(buf + 136, buf + 148));
#endif
			// calculate the number of 512 byte blocks
			unsigned int	blk_cnt = ((file_size / 512) + 1);

			// output the filename and filesize
			output << filename << std::endl;
			output << file_size << std::endl;

			// output the file contents
			while (blk_cnt-- > 0)
			{
				std::fill(std::begin(buf), std::end(buf), 0);
				auto c = (blk_cnt == 0) ? (file_size % 512) : (512);
				input.read(buf, 512);
				output.write(buf, c);
			}

			output << std::endl;

			return;
		}
	}
}