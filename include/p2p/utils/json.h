#pragma once

#include <gsl/string_span>
#include <cctype>
#include <algorithm>

namespace json {

    // Return a string value
    inline gsl::cstring_span<> getstring(const std::string& json, const std::string& key)
    {
        auto first = size_t{ 0 };

        // find the key
        first = json.find("\"" + key + "\"", first);
        if (first == std::string::npos) return {};
        first += key.size() + 1;
        // find the next colon
        first = json.find(':', first);
        if (first == std::string::npos) return {};
        first++;

        // find the beginning quotation mark of the value string
        first = json.find('"', first);
        if (first == std::string::npos) return {};
        first++;

        // find the ending quotation mark of the value string
        auto last = json.find('"', first);
        if (last == std::string::npos) return {};

        return gsl::cstring_span<>{json}.subspan(first, last - first);
    }

    // Remove all whitespaces, to ease comparison
    inline std::string normalize(const std::string& json)
    {
        auto out = json;
        out.erase(std::remove_if(out.begin(), out.end(), std::isspace), out.end());
        return out;
    }

    // Compare two json - caveat: keys must be in the same order in both json
    inline bool is_same(const std::string& left, const std::string& right)
    {
        return normalize(left) == normalize(right);
    }
}

