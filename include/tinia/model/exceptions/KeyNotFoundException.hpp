#pragma once
#include <stdexcept>
namespace tinia { namespace model {
class KeyNotFoundException : public std::invalid_argument {
public:
    KeyNotFoundException(const std::string& key)
        : std::invalid_argument(makeWhat(key)), m_key(key)
    {

    }

    std::string key() {
        return m_key;
    }

private:
    std::string makeWhat(const std::string& key) {
        std::stringstream ss;
        ss << "Could not find the key \"" << key << "\".";
        return ss.str();
    }

    std::string m_key;
};
}}
