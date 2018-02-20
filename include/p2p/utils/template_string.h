#pragma once

#include <string>

class template_string : public std::string
{
public:
    explicit template_string(const std::string& s) : std::string(s) {}

    inline template_string& set(const std::string& key, const std::string& value)
    {
        std::string::size_type pos = 0;
        while ((pos = find(key, pos)) != std::string::npos)
        {
            replace(pos, key.size(), value);
            pos++;
        }

        return *this;
    }
};

