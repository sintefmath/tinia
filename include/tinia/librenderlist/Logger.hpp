#pragma once
#include <string>
#include <iostream>

namespace tinia {
namespace librenderlist {

typedef std::string Logger;

static inline Logger getLogger( const std::string name )
{
    return name;
}

#define RL_LOG_TRACE(a,b) do { std::cerr << "[T] " << a << ": " << b << std::endl; } while(0)
#define RL_LOG_INFO(a,b)  do { std::cerr << "[I] " << a << ": " << b << std::endl; } while(0)
#define RL_LOG_DEBUG(a,b) do { std::cerr << "[D] " << a << ": " << b << std::endl; } while(0)
#define RL_LOG_WARN(a,b)  do { std::cerr << "[W] " << a << ": " << b << std::endl; } while(0)
#define RL_LOG_ERROR(a,b) do { std::cerr << "[E] " << a << ": " << b << std::endl; } while(0)
#define RL_LOG_FATAL(a,b) do { std::cerr << "[F] " << a << ": " << b << std::endl; } while(0)


} // of namespace librenderlist
} // of namespace tinia

