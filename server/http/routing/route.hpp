#pragma once
// std
#include <sstream>
#include <types>

// lib
#include <utils/net>
#include "../http_method.hpp"
#include "../http_types.hpp"

namespace net::http {

class route {
  public:
    struct _segment {
        string value;
        bool is_param;

        _segment(string val) : value(std::move(val)), is_param(!value.empty() && value[0] == ':') {}

        bool is_match(const string& path_segment) const {
            return is_param || value == path_segment;
        }

        string param_name() const { return is_param ? value.substr(1) : ""; }
    };

    struct _pattern {
        string original_path;
        list<_segment> segments;

        static _pattern from_string(const string& path) {
            _pattern p;
            p.original_path = path;

            std::stringstream ss(path);
            string segment;

            while (std::getline(ss, segment, '/')) {
                if (segment.empty())
                    continue;
                p.segments.push_back(segment);
            }

            return p;
        }

        bool match(const string& input_path, string_map& out_params) const {
            std::stringstream ss(input_path);
            string part;
            list<string> input_segments;

            while (std::getline(ss, part, '/')) {
                if (!part.empty()) {
                    input_segments.push_back(part);
                }
            }

            if (input_segments.size() != segments.size())
                return false;

            for (size_t i = 0; i < segments.size(); ++i) {
                const auto& seg   = segments[i];
                const auto& input = input_segments[i];

                if (!seg.is_match(input))
                    return false;

                if (seg.is_param)
                    out_params[seg.param_name()] = input;
            }

            return true;
        }
    } pattern;

    method method;
    route_handler handler;
};

using route_segment = route::_segment;
using route_pattern = route::_pattern;

} // namespace net::http