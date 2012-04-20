#ifndef POLICYLIB_FILE_HPP
#define POLICYLIB_FILE_HPP

#include <iostream>
#include <sstream>

namespace tinia {
namespace policylib {

class File
{
public:
    File();

    std::string fullPath() const;

    void fullPath(const std::string& fullPath);

    std::string name() const;

private:
   std::string m_fullPath;
};

std::ostream& operator<<(std::ostream& stream, const File& file);

std::istream& operator>>(std::istream& stream, File& file);
}
}
#endif // POLICYLIB_FILE_HPP
