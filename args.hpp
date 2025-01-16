#pragma once

#include "read.hpp"

#include <cxxabi.h>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct base_argument {
    std::string long_name;
    std::string alias;
    std::string description;

    base_argument(std::string long_name, std::string alias, std::string description)
        : long_name(long_name), alias(alias), description(description) {}
    virtual ~base_argument() = default;

    virtual void parse(const std::vector<std::string>& argv, size_t& index) = 0;
    virtual std::string help() const = 0;
};

struct flag_argument : base_argument {
    bool& value;

    flag_argument(std::string long_name, std::string alias, std::string description, bool& value)
        : base_argument(long_name, alias, description), value(value) {}

    void parse(const std::vector<std::string>& argv, size_t& index) override {
        const std::string& arg = argv[index];

        value = true;
        size_t equals_pos = arg.find('=');
        if (equals_pos != std::string::npos) {
            read_stream read_s(arg.substr(equals_pos + 1));
            read_s >> value;
            if (read_s.fail()) throw std::runtime_error("Failed to parse value for " + long_name);
        }
    }
    std::string help() const override {
        return "--" + long_name + (alias == "" ? ": " : " (alias -" + alias + "): ") + description;
    }
};


template<typename T>
struct value_argument : base_argument {
    T& value;

    value_argument(std::string long_name, std::string alias, std::string description, T& value)
        : base_argument(long_name, alias, description), value(value) {}

    void parse(const std::vector<std::string>& argv, size_t& index) override {
        const std::string& arg = argv[index];

        std::string val;
        size_t equals_pos = arg.find('=');
        if (equals_pos != std::string::npos) {
            val = arg.substr(equals_pos + 1);
        } else {
            ++index;
            if (index >= argv.size()) throw std::runtime_error("No value for last value argument");
            val = argv[index];
        }
        read_stream read_s(val);
        set_delim(read_s, ' ', false);
        read_s >> value;
        if (read_s.fail()) throw std::runtime_error("Failed to parse value for " + long_name);
    }
    std::string help() const override {
        return "--" + long_name + "=<" + type_sig<T> + ">" + (alias == "" ? ": " : " (alias -" + alias + "): ") + description;
    }
};

inline std::shared_ptr<base_argument> make_argument(std::string long_name, std::string alias, std::string description, bool& value) {
    return std::make_shared<flag_argument>(long_name, alias, description, value);
}

template<typename T>
inline std::shared_ptr<base_argument> make_argument(std::string long_name, std::string alias, std::string description, T& value) {
    return std::make_shared<value_argument<T>>(long_name, alias, description, value);
}

inline void parse_arguments(const std::vector<std::shared_ptr<base_argument>>& args, int argc, char* argv[]) {
    std::vector<std::string> argv_str(argv, argv + argc);
    std::map<std::string, std::shared_ptr<base_argument>> arg_map;
    for (const std::shared_ptr<base_argument>& a : args) {
        arg_map[a->long_name] = a;
        if (a->alias != "") {
            arg_map[a->alias] = a;
        }
    }
    bool errored = false, do_help = false;
    std::string errors;
    for (size_t i = 1; i < argv_str.size(); ++i) {
        std::string arg = argv_str[i];
        if (arg.size() < 2 || arg[0] != '-') {
            errors += "Bad argument: " + arg + '\n';
            errored = true;
            continue;
        }
        arg = arg.substr(1);
        if (arg[0] == '-') arg = arg.substr(1);

        if (arg == "help" || arg == "h") {
            do_help = true;
            continue;
        }

        size_t equals_pos = arg.find('=');
        if (equals_pos != std::string::npos) {
            arg = arg.substr(0, equals_pos);
        }
        if (arg_map.count(arg) != 0) {
            arg_map[arg]->parse(argv_str, i);
        } else {
            errors += "Unknown argument: " + arg + '\n';
            errored = true;
        }
    }
    if (do_help) {
        for (const std::shared_ptr<base_argument>& a : args) {
            std::cout << a->help() << std::endl;
        }
    }
    if (errored) {
        std::cerr << "Couldn't parse arguments:\n" << errors;
        exit(1);
    }
    if (do_help) exit(0);
}

