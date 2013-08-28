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

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>
#include <tinia/model.hpp>
#include <tinia/model/impl/validateGUI.hpp>
#include <tinia/model/exceptions/KeyNotFoundException.hpp>
#include <tinia/model/exceptions/TypeException.hpp>

BOOST_AUTO_TEST_SUITE(ValidateGUI)

BOOST_AUTO_TEST_CASE(Button) {
    tinia::model::ExposedModel model;
    model.addElement<bool>("buttonBool", true);

    tinia::model::gui::Button b1("buttonBool");

    BOOST_CHECK_NO_THROW(tinia::model::impl::validateGUI(&b1, model));


    tinia::model::gui::Button b2("buttonKeyNotFound");
    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(&b2, model),
                      tinia::model::KeyNotFoundException);

    tinia::model::gui::Button b3("buttonWithWrongType");
    model.addElement<double>("buttonWithWrongType", 3.14);
    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(&b3, model),
                      tinia::model::TypeException);

}
BOOST_AUTO_TEST_CASE(CheckBox) {
    tinia::model::ExposedModel model;
    model.addElement<bool>("checkBoxBool", true);

    tinia::model::gui::CheckBox b1("checkBoxBool");

    BOOST_CHECK_NO_THROW(tinia::model::impl::validateGUI(&b1, model));


    tinia::model::gui::CheckBox b2("checkBoxKeyNotFound");
    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(&b2, model),
                      tinia::model::KeyNotFoundException);

    tinia::model::gui::CheckBox b3("checkBoxWithWrongType");
    model.addElement<double>("checkBoxWithWrongType", 3.14);
    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(&b3, model),
                      tinia::model::TypeException);
}

BOOST_AUTO_TEST_CASE(ComboBoxTest) {
    tinia::model::ExposedModel model;
    std::vector<int> restrictions;
    restrictions.push_back(0);
    model.addElementWithRestriction("comboKey", 0, restrictions);

    tinia::model::gui::ComboBox box1("comboKey");

    BOOST_CHECK_NO_THROW(tinia::model::impl::validateGUI(&box1, model));

    model.addElement("nonRestrict", 0);
    tinia::model::gui::ComboBox box2("nonRestrict");

    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(&box2, model),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(RadioButtonsTest) {
    tinia::model::ExposedModel model;
    std::vector<int> restrictions;
    restrictions.push_back(0);
    model.addElementWithRestriction("radioKey", 0, restrictions);

    tinia::model::gui::RadioButtons box1("radioKey");

    BOOST_CHECK_NO_THROW(tinia::model::impl::validateGUI(&box1, model));

    model.addElement("nonRestrict", 0);
    tinia::model::gui::RadioButtons box2("nonRestrict");

    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(&box2, model),
                      std::invalid_argument);
}

typedef boost::mpl::list<boost::mpl::pair<tinia::model::gui::HorizontalSlider, int>,
                        boost::mpl::pair<tinia::model::gui::SpinBox, int>,
                        boost::mpl::pair<tinia::model::gui::DoubleSpinBox, double> >
                        containers;

BOOST_AUTO_TEST_CASE_TEMPLATE(NumberContainers, ContainerPair, containers) {
    typedef typename ContainerPair::first Container;
    typedef typename ContainerPair::second type;
    tinia::model::ExposedModel model;
    model.addConstrainedElement<type>("constrained",0, 0, 10);

    Container c1("constrained");
    BOOST_CHECK_NO_THROW(tinia::model::impl::validateGUI(&c1, model));

    model.addElement<type>("unconstrained", 0);
    Container c2("unconstrained");
    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(&c2, model),
                      std::invalid_argument);

    model.addElement<std::string>("wrongType", "value");
    Container c3("wrongType");
    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(&c3, model),
                      tinia::model::TypeException);
}


BOOST_AUTO_TEST_CASE(Viewer) {
    tinia::model::ExposedModel model;

    model.addElement("viewer", tinia::model::Viewer());

    tinia::model::gui::Canvas c1("viewer");

    BOOST_CHECK_NO_THROW(tinia::model::impl::validateGUI(&c1, model));

    tinia::model::gui::Canvas c2("viewerNotThere");
    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(&c2, model),
                      tinia::model::KeyNotFoundException);

    model.addElement("wrongType", "someValue");
    tinia::model::gui::Canvas c3("wrongType");

    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(&c3, model),
                      tinia::model::TypeException);
}

typedef boost::mpl::list<int, double, bool> LegalTextInputTypes;
BOOST_AUTO_TEST_CASE_TEMPLATE(TextInputNumbers, type, LegalTextInputTypes) {
    tinia::model::ExposedModel model;
    model.addElement<type>("key", type(0));

    tinia::model::gui::TextInput input1("key");
    BOOST_CHECK_NO_THROW(tinia::model::impl::validateGUI(&input1, model));
}

BOOST_AUTO_TEST_CASE(TextInput) {
    tinia::model::ExposedModel model;
    model.addElement<std::string>("key", "value");

    tinia::model::gui::TextInput input1("key");
    BOOST_CHECK_NO_THROW(tinia::model::impl::validateGUI(&input1, model));


    model.addElement("viewer", tinia::model::Viewer());

    tinia::model::gui::TextInput input2("viewer");
    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(&input2, model),
                      tinia::model::TypeException);
}

typedef boost::mpl::list<tinia::model::gui::HorizontalLayout,
                        tinia::model::gui::VerticalLayout> containers1D;
BOOST_AUTO_TEST_CASE_TEMPLATE(Container1D, Container, containers1D) {
    tinia::model::ExposedModel model;

    model.addElement("key", 0);

    tinia::model::gui::TextInput* input = new tinia::model::gui::TextInput("key");

    Container* container = new Container();
    container->addChild(input);

    BOOST_CHECK_NO_THROW(tinia::model::impl::validateGUI(container, model));

    container->addChild(new tinia::model::gui::TextInput("keyNotFound"));
    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(container, model),
                      tinia::model::KeyNotFoundException);

    delete container;
}


BOOST_AUTO_TEST_CASE(TabLayout) {
    tinia::model::ExposedModel model;

    model.addElement("key", 0);

    tinia::model::gui::TextInput* input = new tinia::model::gui::TextInput("key");

    tinia::model::gui::TabLayout* container = new tinia::model::gui::TabLayout();
    tinia::model::gui::Tab* tab = new tinia::model::gui::Tab("key");
    tab->setChild(input);
    container->addChild(tab);

    BOOST_CHECK_NO_THROW(tinia::model::impl::validateGUI(container, model));

    tab->setChild(new tinia::model::gui::TextInput("keyNotFound"));
    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(container, model),
                      tinia::model::KeyNotFoundException);

    container->addChild(new tinia::model::gui::Tab("keynotfound"));
    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(container, model),
                      tinia::model::KeyNotFoundException);

    tinia::model::gui::HorizontalLayout nonTabLayout;
    tinia::model::gui::Tab* tab2 = new tinia::model::gui::Tab("key");
    tab2->setChild(new tinia::model::gui::TextInput("key"));
    nonTabLayout.addChild(tab2);

    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(&nonTabLayout, model),
                      std::invalid_argument);

    delete container;
}

BOOST_AUTO_TEST_CASE(Grid) {
    tinia::model::ExposedModel model;

    model.addElement("key", 0);

    tinia::model::gui::TextInput* input = new tinia::model::gui::TextInput("key");

    tinia::model::gui::Grid* container = new tinia::model::gui::Grid(2,2);
    container->setChild(0, 0, input);

    BOOST_CHECK_NO_THROW(tinia::model::impl::validateGUI(container, model));

    container->setChild(0, 1, new tinia::model::gui::TextInput("keyNotFound"));
    BOOST_CHECK_THROW(tinia::model::impl::validateGUI(container, model),
                      tinia::model::KeyNotFoundException);

    delete container;
}


BOOST_AUTO_TEST_SUITE_END()
