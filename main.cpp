
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

using std::begin;
using std::end;

namespace po = boost::program_options;
using namespace boost::filesystem;

template<>
class std::hash<path> : private std::hash<std::string> {
public:
    size_t operator()(path p) const {
        return std::hash<std::string>::operator()(p.generic_string());
    }
};

std::vector<path> directories;
std::unordered_map<path, uintmax_t> file_sizes;

void read_dir(path dir) {
    if (is_directory(dir)) {
        directories.push_back(dir);
        for (auto i = recursive_directory_iterator(dir); i != recursive_directory_iterator(); ++i) {
            if (is_regular_file(*i)) {
                file_sizes[*i] = file_size(*i);
            }
            else if (is_directory(*i)) {
                directories.push_back(*i);
            }
        }
    }
    std::sort(begin(directories), end(directories));
}

int main(int argc, char **argv)
{
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "show this message")
        ("directory,d", po::value<std::vector<std::string>>()->required(), "directories to be searched")
        ;
    po::positional_options_description posd;
    posd.add("directory", -1);

    po::variables_map vars;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(posd).run(), vars);
        po::notify(vars);
    }
    catch (po::error const & e) {
        std::cout << desc << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }

    if (vars.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    for (auto dir : vars["directory"].as<std::vector<std::string>>()) {
        read_dir(dir);
    }

    return 0;
}
