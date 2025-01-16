#include "args.hpp"

#include <string>

int main(int argc, char** argv) {
    bool debug = false;
    std::string filename = "test";
    std::vector<std::string> values;
    std::tuple<std::string, int, bool> tup;
    // std::vector<std::tuple<std::string, int, bool>> tupvec; // unsupported

    auto args = {
        make_argument("debug", "d", "12345", debug),
        make_argument("filename", "f", "test", filename),
        make_argument("values", "v", "vector test", values),
        make_argument("tup", "t", "tuple test", tup),
        // make_argument("tupvec", "tv", "tuple vector test", tupvec)
    };
    parse_arguments(args, argc, argv);

    std::cout << "debug " << debug << " default " << 0 << std::endl;

    std::cout << "filename " << filename << " default " << "test" << std::endl;

    std::cout << "values ";
    for (const std::string& s : values) std::cout << s << ",";
    std::cout << std::endl;

    std::cout << "tup " << std::get<0>(tup) << " " << std::get<1>(tup) << " " << std::get<2>(tup) << std::endl;

    /* unsupported
    for (const auto& t : tupvec) {
        std::cout << "t " << std::get<0>(t) << " " << std::get<1>(t) << " " << std::get<2>(t) << std::endl;
    } */

    return 0;
}
