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

//#include <algorithm>
//#include <stdexcept>

#include <tinia/utils/ProxyDebugGUI.hpp>




namespace {

    // NB! This list must match the one in ProxyRenderer.js, exactly!
    const char *allowed_auto_proxy_algos[] = { "0) AngleCoverage-5",
                                               "1) AngleCoverage-2",
                                               "2) OnlyMostRecent",
                                               "3) ReplaceOldestWhenDifferent-5",
                                               "4) ReplaceOldest-5",
                                               NULL };

}




namespace tinia {
namespace utils {




ProxyDebugGUI::ProxyDebugGUI( boost::shared_ptr<model::ExposedModel> model,
                              const bool with_ap, const bool with_ap_debugging, const bool with_jpg, const bool with_auto_select )
    : m_w_ap(with_ap), m_w_apd(with_ap_debugging), m_w_jpg(with_jpg), m_w_as(with_auto_select)
{
    if ( m_w_ap && m_w_jpg && m_w_as ) {
        model->addElement<bool>( "ap_autoSelect", false );            // Selects whatever proxy method works fastest.
        model->addElement<bool>( "ap_autoSelectSampleAll", false );
        model->addAnnotation("ap_autoSelectSampleAll", "Smple all snapsh. types");
        model->addElement( "ap_autoSelectIndicator", "---" );
        model->addConstrainedElement<int>("ap_autoSelectTargetTime", 100, 0, 200);
        model->addAnnotation("ap_autoSelectTargetTime", "Target time:");
        model->addConstrainedElement<int>("ap_autoSelectTargetTimeSlack", 20, 0, 200);
        model->addAnnotation("ap_autoSelectTargetTimeSlack", "Target time slack:");
        model->addConstrainedElement<int>("simulatedAdditionalLatency", 0, 0, 1000);
        model->addAnnotation("simulatedAdditionalLatency", "ms per frame:");
        model->addConstrainedElement<int>("simulatedAdditionalLatencyDecay", 0, -100, 100);
        model->addAnnotation("simulatedAdditionalLatencyDecay", "ms per sec:");
    }

    if (m_w_jpg) {
        model->addElement<bool>( "ap_useJpgProxy", false );            // This turns on the new "proxy mode", for which speedier jpg-snapshots are used
        model->addConstrainedElement<int>("ap_jpgQuality", 0, 0, 99);
        model->addAnnotation("ap_jpgQuality", "Jpg compression q:");
    }

    if (m_w_ap) {
        model->addElement<bool>( "ap_useAutoProxy", false );          // This turns on the new autoProxy. It will override useJpgProxy.
        model->addElement<bool>( "ap_autoProxyDebugging", true );
        model->addAnnotation("ap_autoProxyDebugging", "Debug mode");
    }

    if (m_w_ap && m_w_apd) {
        int algos=0;
        while ( allowed_auto_proxy_algos[algos] != NULL ) {
            algos++;
        }
        model->addElementWithRestriction<std::string>( "ap_autoProxyAlgo", allowed_auto_proxy_algos[2], &allowed_auto_proxy_algos[0], &allowed_auto_proxy_algos[0]+algos );
        model->addAnnotation("ap_autoProxyAlgo", "Proxy model replacement algo");
        model->addElement<bool>( "ap_debugSplatCol", false );
        model->addAnnotation("ap_debugSplatCol", "Index coloring (r, g, b, y, c, m)");
        model->addElement<bool>( "ap_decayMode", false );
        model->addAnnotation("ap_decayMode", "Splats decaying from center");
        model->addElement<bool>( "ap_roundSplats", false );
        model->addAnnotation("ap_roundSplats", "Circular splats");
        model->addElement<bool>( "ap_screenSpaceSized", true );
        model->addAnnotation("ap_screenSpaceSized", "Screen-space-sized splats");
        model->addConstrainedElement<int>("ap_overlap", 200, 1, 300);
        model->addAnnotation("ap_overlap", "Overlap factor)");
        model->addElement<bool>( "ap_alwaysShowMostRecent", true );
        model->addAnnotation("ap_alwaysShowMostRecent", "Most recent model in front");
        model->addConstrainedElement<int>("ap_splats", 64, 2, 512);
        model->addAnnotation("ap_splats", "Number of splats)");
        model->addElement<bool>( "ap_resetAllModels", false );
        model->addAnnotation("ap_resetAllModels", "Remove all models, update now");
        model->addElement<bool>( "ap_useISTC", true );
        model->addAnnotation("ap_useISTC", "Use intra-splat texcoo");
        model->addElement<bool>( "ap_splatOutline", false );
        model->addAnnotation("ap_splatOutline", "Square splat outline");
        model->addElement<bool>( "ap_reloadShader", false );
        model->addAnnotation("ap_reloadShader", "Reload shader");
        model->addElement<bool>( "ap_useFragExt", true );
        model->addAnnotation("ap_useFragExt", "Use FragDepthExt if available");
        model->addElement( "ap_fragExtStatus", "---" );
        model->addElement( "ap_consoleLog", "---" );
        model->addElement<int>( "ap_cntr", 0 );
    }
}




tinia::model::gui::Grid *ProxyDebugGUI::getGrid()
{
    tinia::model::gui::Grid *mainGrid = new tinia::model::gui::Grid(100, 4);
    int row = 0;

    if ( m_w_ap && m_w_jpg && m_w_as ) {
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("ap_autoSelect"));
        mainGrid->setChild(row, 1, new tinia::model::gui::Button("ap_autoSelectSampleAll"));
        //        mainGrid->setChild(row, 0, new tinia::model::gui::Label("ap_autoSelectIndicator", false)); // Showing the name of the label
        mainGrid->setChild(row, 2, new tinia::model::gui::Label("ap_autoSelectIndicator", true)); // Showing the content of the label
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::HorizontalSlider("ap_autoSelectTargetTime"));
        mainGrid->setChild(row, 1, new tinia::model::gui::Label("ap_autoSelectTargetTime", false));
        mainGrid->setChild(row, 2, new tinia::model::gui::Label("ap_autoSelectTargetTime", true));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::HorizontalSlider("ap_autoSelectTargetTimeSlack"));
        mainGrid->setChild(row, 1, new tinia::model::gui::Label("ap_autoSelectTargetTimeSlack", false));
        mainGrid->setChild(row, 2, new tinia::model::gui::Label("ap_autoSelectTargetTimeSlack", true));
        row++;

        mainGrid->setChild(row, 0, new tinia::model::gui::HorizontalSlider("simulatedAdditionalLatency"));
        mainGrid->setChild(row, 1, new tinia::model::gui::Label("simulatedAdditionalLatency", false));
        mainGrid->setChild(row, 2, new tinia::model::gui::Label("simulatedAdditionalLatency", true));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::HorizontalSlider("simulatedAdditionalLatencyDecay"));
        mainGrid->setChild(row, 1, new tinia::model::gui::Label("simulatedAdditionalLatencyDecay", false));
        mainGrid->setChild(row, 2, new tinia::model::gui::Label("simulatedAdditionalLatencyDecay", true));
        row++;
    }

    if (m_w_jpg) {
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("ap_useJpgProxy"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::HorizontalSlider("ap_jpgQuality"));
        mainGrid->setChild(row, 1, new tinia::model::gui::Label("ap_jpgQuality", false));
        mainGrid->setChild(row, 2, new tinia::model::gui::Label("ap_jpgQuality", true));
        row++;
    }

    if (m_w_ap) {
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("ap_useAutoProxy"));
        row++;
    }

    if (m_w_ap && m_w_apd) {
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("ap_autoProxyDebugging"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::Label("ap_autoProxyAlgo"));
        mainGrid->setChild(row, 1, new tinia::model::gui::ComboBox("ap_autoProxyAlgo"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("ap_debugSplatCol"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("ap_decayMode"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("ap_roundSplats"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("ap_screenSpaceSized"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::HorizontalSlider("ap_overlap"));
        mainGrid->setChild(row, 1, new tinia::model::gui::Label("ap_overlap", false));
        mainGrid->setChild(row, 2, new tinia::model::gui::Label("ap_overlap", true));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("ap_alwaysShowMostRecent"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::HorizontalSlider("ap_splats"));
        mainGrid->setChild(row, 1, new tinia::model::gui::Label("ap_splats", false));
        mainGrid->setChild(row, 2, new tinia::model::gui::Label("ap_splats", true));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::Button("ap_resetAllModels"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("ap_useISTC"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("ap_splatOutline"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::Button("ap_reloadShader"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("ap_useFragExt"));
        mainGrid->setChild(row, 1, new tinia::model::gui::Label("ap_fragExtStatus", true)); // true) We get the text string connected to the element, false) name of element
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::Label("ap_consoleLog", false));
        mainGrid->setChild(row, 1, new tinia::model::gui::Label("ap_consoleLog", true));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::VerticalExpandingSpace());
    }

    return mainGrid;
}




} // of namespace utils
} // of namespace tinia
