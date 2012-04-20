#pragma once
#include "jobobserver/Job.hpp"
#include "jobobserver/Observer.hpp"

namespace tinia {
namespace Trell
{

class InteractiveJob : public jobobserver::Job
{
public:
    InteractiveJob( jobobserver::Observer* observer );


};

} // of namespace Trell
} // of namespace tinia
