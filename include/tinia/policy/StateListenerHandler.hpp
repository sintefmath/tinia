#ifndef STATELISTENERHANDLER_HPP
#define STATELISTENERHANDLER_HPP
#include "tinia/policy/StateListener.hpp"
#include <vector>
#include <string>
#include <map>

namespace tinia {
namespace policy {
class StateListenerHandler
{
public:
   StateListenerHandler() : m_isBuffering(false) {}
    void addStateListener(StateListener* listener);
    void addStateListener(std::string key, StateListener* listener);
    void removeStateListener(StateListener* listener);
    void removeStateListener(std::string key, StateListener* listener);
    void fireStateElementModified(StateElement* stateElement);
    void holdEvents();
    void releaseEvents();
private:
    std::list<StateListener*> m_listeners;
    std::map<std::string, std::list<StateListener*> > m_keylisteners;
    std::vector<StateElement> m_buffer;
    bool m_isBuffering;

};
}
}
#endif // STATELISTENERHANDLER_HPP
