/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

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
