#pragma once

#include <cxxabi.h>
#include <sstream>
#include <string>
#include <tuple>
#include <iostream>
#include <vector>

template<typename... Ts>
std::istream& operator>>(std::istream& stream, std::tuple<Ts...>& tuple);

template<typename T>
std::istream& operator>>(std::istream& stream, std::vector<T>& vec);

struct read_stream : std::istringstream {
    using std::istringstream::istringstream;

    template<typename T>
    read_stream& operator>>(T& rhs) {
        static_cast<std::istream&>(*this) >> rhs;
        return *this;
    }

    read_stream& operator>>(bool& rhs) {
        std::string read;
        static_cast<std::istream&>(*this) >> read;
        if (read == "true" || (read.size() == 1 && (tolower(read[0]) == 'y' || read[0] == '1'))) {
            rhs = true;
        } else if (read == "false" || (read.size() == 1 && (tolower(read[0]) == 'n' || read[0] == '0'))) {
            rhs = false;
        } else {
            setstate(std::ios::failbit);
        }
        return *this;
    }
};

template<typename T>
inline const std::string type_sig = abi::__cxa_demangle(typeid(T).name(), NULL, NULL, 0);
template<>
inline const std::string type_sig<std::string> = "string";
template<typename K>
inline const std::string type_sig<std::vector<K>> = type_sig<K> + ",...," + type_sig<K>;
template<typename... Ks>
inline const std::string __type_sig_partial = ((type_sig<Ks> + ",") + ...);
template<typename... Ks>
inline const std::string type_sig<std::tuple<Ks...>> = __type_sig_partial<Ks...>.substr(0, __type_sig_partial<Ks...>.size() - 1);

template<typename... Ks>
struct args_tuple {
    std::tuple<Ks...> tup;
    args_tuple<Ks...>(std::tuple<Ks...> in_tup) : tup(in_tup) {}
};

// add or remove a delimeter character to stream
inline void set_delim(std::basic_ios<char>& s, char delim, bool set_to = true) {
    const auto std_ctype = std::ctype<char>::classic_table();
    std::vector<std::ctype<char>::mask> new_ctype(std_ctype, std_ctype + std::ctype<char>::table_size);
    new_ctype[delim] ^= std::ctype_base::space * (((new_ctype[delim] & std::ctype_base::space) != 0) != set_to);
    s.imbue(std::locale(s.getloc(), new std::ctype<char>(data(new_ctype))));
}

// read a stream into a tuple
template<typename... Ts>
inline std::istream& operator>>(std::istream& stream, args_tuple<std::tuple<Ts...>&, char> read_to_w) {
    auto& read_to = read_to_w.tup;
    std::tuple<Ts...>& tuple = std::get<0>(read_to);
    char delim = std::get<1>(read_to);
    std::apply ([&stream, &delim](Ts&... args) {
            std::string line;
            stream >> line;
            read_stream sstream(line);
            set_delim(sstream, delim);

            (sstream >> ... >> args);
            if (sstream.fail()) stream.setstate(std::ios::failbit);
        }, tuple
    );
    return stream;
}
template<typename... Ts>
inline std::istream& operator>>(std::istream& stream, std::tuple<Ts...>& tuple) {
    return stream >> args_tuple(std::tuple<std::tuple<Ts...>&, char>(tuple, ','));
}

// read a stream into a vector
template<typename T>
inline std::istream& operator>>(std::istream& stream, args_tuple<std::vector<T>&, char, bool> read_to_w) {
    auto& read_to = read_to_w.tup;
    std::vector<T>& vec = std::get<0>(read_to);
    char delim = std::get<1>(read_to);
    if (std::get<2>(read_to)) {
        vec.clear(); // clear beforehand if requested
    }
    std::string line;
    stream >> line;
    read_stream sstream(line);
    set_delim(sstream, delim);

    while (sstream && !sstream.eof()) {
        T value;
        sstream >> value;
        vec.push_back(value);
    }
    if (sstream.fail()) stream.setstate(std::ios::failbit);
    return stream;
}
template<typename T>
inline std::istream& operator>>(std::istream& stream, std::vector<T>& vec) {
    return stream >> args_tuple(std::tuple<std::vector<T>&, char, bool>(vec, ',', false));
}
