#ifndef POLICY_FILE_HPP
#define POLICY_FILE_HPP

#include <iostream>
#include <sstream>

namespace tinia {
namespace policy {

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
#endif // POLICY_FILE_HPP
