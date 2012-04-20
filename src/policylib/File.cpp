#include "tinia/policylib/File.hpp"

namespace policylib {

File::File() : m_fullPath("")
{
}



std::string File::fullPath() const
{
   return m_fullPath;
}

void File::fullPath(const std::string& fullPath)
{
   m_fullPath = fullPath;
}

std::string File::name() const
{
   // broken, for now
   return m_fullPath;
}


std::ostream& operator<<(std::ostream& stream, const File& file)
{
   stream << file.fullPath();
   return stream;
}

std::istream& operator>>(std::istream& stream, File& file)
{
   std::string fullPath;

   stream >> fullPath;
   file.fullPath(fullPath);
   stream.clear();
   return stream;

}

}
