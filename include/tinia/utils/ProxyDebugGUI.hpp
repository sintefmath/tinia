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

#pragma once

#include <string>
#include <iostream>

// QT's moc doesn't like BOOST_JOIN ( can be removed in QT 5.0 we think)
#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#endif

#include "tinia/model/GUILayout.hpp"
#include "tinia/model/ExposedModel.hpp"




namespace tinia {
namespace utils {

class ProxyDebugGUI: public tinia::model::StateListener
{

public:

    ProxyDebugGUI( boost::shared_ptr<model::ExposedModel> model,
                   const bool with_ap, const bool with_ap_debugging, const bool with_jpg, const bool with_auto_select,
                   const bool with_depth_buffer_manipulation = false );
    ~ProxyDebugGUI();
    void stateElementModified(tinia::model::StateElement *stateElement);
    tinia::model::gui::Grid *getGrid();
    tinia::model::gui::Grid *getCompactGrid();

private:

    void resetSettingsConnection();
    void resetSettingsClient();
    void lowBandwidthHighLatency();
    void mediumBandwidthMediumLatency();
    void highBandwidthLowLatency();
    void clientPhablet();
    void clientLaptop();
    void clientDesktop();

    bool m_w_ap, m_w_apd, m_w_jpg, m_w_as, m_with_depth_buffer_manipulation;
    boost::shared_ptr<model::ExposedModel> m_model;
};




} // of namespace utils
} // of namespace tinia
