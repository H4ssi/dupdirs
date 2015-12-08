
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using std::begin;
using std::end;

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

int main()
{
    read_dir("E:\\b");
    return 0;
}
