#include <sstream>
#include <numeric>

#include "streams\tar.h"

bool append_test()
{
    std::stringstream	tar("");
    std::string			dummy_filename = "dummy.txt";
    std::string			dummy_file = "this is a dummy file, how do you do? I don't know @ bla\0";
    std::stringstream	buf(dummy_file);
    auto				now = clock();

    files::tar::append(tar, buf, dummy_filename, dummy_file.length(), now);

    // verify the contents of the buffer
    auto tar_file = tar.str();
    auto extract_fname = std::string(std::begin(tar_file), std::next(std::begin(tar_file), 100));
    auto name_correct = extract_fname.compare(0, dummy_filename.length(), dummy_filename) == 0;

    auto extract_start = std::next(std::begin(tar_file), 512);
    auto extract_end = std::next(extract_start, dummy_file.length());
    auto extr = std::string(extract_start, extract_end);
    auto file_contents_correct = dummy_file.compare(extr) == 0;

    return name_correct && file_contents_correct;
}
bool extract_test()
{
    std::stringstream	tar ("");
    std::string			dummy_filename = "dummy.txt";
    std::string			dummy_file = "this is a dummy file, how do you do? I don't know @ bla\0";
    std::stringstream	buf(dummy_file);
    auto				now = clock();

    files::tar::append(tar, buf, dummy_filename, dummy_file.length(), now);

    tar.seekg(0, std::ios::beg);

    //files::tar::extract(tar, std::cout);
    // FIXME: test?!

    return true;    // ?!
}

int main (int argc, char **argv)
{
    auto ret_vals = {
        append_test(),
        extract_test()
    };

    auto passes = std::accumulate(
        std::begin (ret_vals),
        std::end (ret_vals),
        0,
        [](const int s, const bool a)
        {
            return (s + ((a == true) ? 1 : 0));
        });

    auto fails = ret_vals.size() - passes;

    if (fails != 0)
        throw std::exception();

    return (int)fails;
}