
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

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

std::ofstream out_cache;

std::vector<path> directories;
std::unordered_map<path, uintmax_t> file_sizes;

void read_dir(path dir) {
    if (is_directory(dir)) {
        directories.push_back(dir);
        out_cache << dir.string() << std::endl;
        for (auto i = recursive_directory_iterator(dir); i != recursive_directory_iterator(); ++i) {
            if (is_regular_file(*i)) {
                auto size = file_size(*i);
                file_sizes[*i] = size;
                out_cache << std::string(i.level() + 1, ' ') << size << ' ' << i->path().filename().string() << std::endl;
            }
            else if (is_directory(*i)) {
                directories.push_back(*i);
                out_cache << std::string(i.level() + 1, ' ') << i->path().filename().string() << std::endl;
            }
        }
    }
    std::sort(begin(directories), end(directories));
}

void read_cache() {
    static const boost::regex r("(\\s*)(\\d+\\s)?(.+)");
    int prev_level = 0;
    path p;
    std::string line;
    for (std::ifstream in_cache("cache.txt"); std::getline(in_cache, line); ) {
        boost::smatch m;
        boost::regex_match(line, m, r);

        int cur_level = m.str(1).length();
        auto fragment = m.str(3);

        if (cur_level == 0) {
            p = fragment;
        }
        else if (cur_level == prev_level) {
            p = p.parent_path() / fragment;
        }
        else if (cur_level == prev_level + 1) {
            p /= fragment;
        }
        else for (int i = prev_level; i > cur_level; --i) {
            p = p.parent_path();
        }
        auto size = m.str(2);
        if (size.empty()) {
            directories.push_back(p);
        }
        else {
            uintmax_t file_size = std::stoull(size);
            file_sizes[p] = file_size;
        }
        prev_level = cur_level;
    }
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

    if (is_regular_file("cache.txt")) {
        read_cache();
    }
    else {
        out_cache.open("cache.txt");
        for (auto dir : vars["directory"].as<std::vector<std::string>>()) {
            read_dir(dir);
        }
    }

    return 0;
}
