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


#include <tinia/model/GUILayout.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/placeholders.hpp>
#include <tinia/model/GUILayout.hpp>
#include <tinia/model/ExposedModel.hpp>
#include <tinia/model/exceptions/KeyNotFoundException.hpp>
#include <tinia/model/impl/validateGUI.hpp>

namespace tinia { namespace model { namespace impl {
namespace {

template<typename ContainerType>
void checkTab(tinia::model::gui::Element* element, ContainerType* parent) {
    if(element->type() == tinia::model::gui::TAB) {
        throw std::invalid_argument("Found a Tab that's not an immediate child of a TabLayout. Key of Tab was "
                                    + (dynamic_cast<tinia::model::gui::Tab*>(element)->key()));
    }
}

void checkTab(tinia::model::gui::Element* element,
              tinia::model::gui::Container1D<tinia::model::gui::Tab>* parent) {
    if(element->type() != tinia::model::gui::TAB) {
        throw std::invalid_argument("Found a non-Tab key as immediate child a TabLayout");
    }
}

void checkNonEmptyRestriction(const std::string& key, ExposedModel& model) {
    if(model.getRestrictionSet(key).empty()) {
        throw std::invalid_argument("Expecting a non-empty restriction set for key " + key);
    }
}

void isAnyType(const std::string& key, ExposedModel& model, const std::vector<std::string>& types) {

    std::string allTypes("");
    for( size_t i = 0; i < types.size(); ++i) {

        if(model.getStateSchemaElement(key).getXSDType() == types[i]) {
            return;
        }

        allTypes +=types[i] + ((i < types.size() -1 ) ? ", " : "");
    }


    throw tinia::model::TypeException(model.getStateSchemaElement(key).getXSDType(), allTypes);
}

void checkBool(const std::string& key, ExposedModel& model) {
    if(!(model.getStateSchemaElement(key).getXSDType() == std::string("xsd:bool"))) {
        throw tinia::model::TypeException(model.getStateSchemaElement(key).getXSDType(),
                                          "xsd:bool");
    }
}

void checkDouble(const std::string& key, ExposedModel& model) {
    if(!(model.getStateSchemaElement(key).getXSDType() == std::string("xsd:double"))) {
        throw tinia::model::TypeException(model.getStateSchemaElement(key).getXSDType(),
                                          "xsd:double");
    }
}

void checkInt(const std::string& key, ExposedModel& model) {
    if(!(model.getStateSchemaElement(key).getXSDType() == std::string("xsd:integer"))) {
        throw tinia::model::TypeException(model.getStateSchemaElement(key).getXSDType(),
                                          "xsd:integer");
    }
}

void checkConstrained(const std::string& key, ExposedModel& model) {
    if(model.getStateSchemaElement(key).emptyConstraints()) {
        throw std::invalid_argument("Expecting constraints for key " + key);
    }
}

template<typename T>
void isValid( tinia::model::gui::Container1D<T>* root,
              tinia::model::ExposedModel& model) {
    for(size_t i = 0; i < root->children(); ++i) {
        if(root->child(i) == NULL) {
            continue;
        }
        tinia::model::impl::validateGUI(root->child(i), model);
        checkTab(root->child(i), root);
    }


}


template<typename T>
void isValid( tinia::model::gui::Container0D<T>* root,
             tinia::model::ExposedModel& model) {
    tinia::model::impl::validateGUI(root->child(), model);
    checkTab(root->child(), root);


}


void isValid( tinia::model::gui::Tab* root, tinia::model::ExposedModel& model)  {
    tinia::model::gui::Container0D<tinia::model::gui::Element>* container =  dynamic_cast<tinia::model::gui::Container0D<tinia::model::gui::Element>*>(root);
    if(container != NULL) {
        isValid(container, model);
    }
}

template<typename T>
void isValid( tinia::model::gui::Container2D<T>* root,
              tinia::model::ExposedModel& model) {
    for(size_t i = 0; i < root->height(); ++i) {
        for(size_t j = 0; j <root->width(); ++j) {
            if(root->child(i, j) == NULL) {
                continue;
            }
            tinia::model::impl::validateGUI(root->child(i, j), model);
            checkTab(root->child(i, j), root);

        }
    }


}

void isValid(tinia::model::gui::KeyValue *root, ExposedModel &model) {
    if(!model.hasElement(root->key())) {
        throw tinia::model::KeyNotFoundException(root->key());
    }
}

void isValid(tinia::model::gui::Button* root, ExposedModel& model) {
    checkBool(root->key(), model);
}

void isValid(tinia::model::gui::CheckBox* root, ExposedModel& model) {
    checkBool(root->key(), model);
}

void isValid(tinia::model::gui::ComboBox* root, ExposedModel& model) {
    checkNonEmptyRestriction(root->key(), model);
}

void isValid(tinia::model::gui::RadioButtons* root, ExposedModel& model) {
    checkNonEmptyRestriction(root->key(), model);

}

void isValid(tinia::model::gui::SpinBox* root, ExposedModel& model) {
    checkInt(root->key(), model);
    checkConstrained(root->key(), model);
}

void isValid(tinia::model::gui::DoubleSpinBox* root, ExposedModel& model) {
    checkDouble(root->key(), model);
    checkConstrained(root->key(), model);
}

void isValid(tinia::model::gui::HorizontalSlider* root, ExposedModel& model) {
    checkInt(root->key(), model);
    checkConstrained(root->key(), model);
}

void isValid(tinia::model::gui::Canvas* root, ExposedModel& model) {

    // Check if we can get it out
    tinia::model::Viewer v;
    model.getElementValue(root->key(), v);
}

void isValid(tinia::model::gui::TextInput* root, ExposedModel& model) {
    std::vector<std::string> allowedTypes;
    allowedTypes.push_back("xsd:bool");
    allowedTypes.push_back("xsd:integer");
    allowedTypes.push_back("xsd:double");
    allowedTypes.push_back("xsd:string");
    isAnyType(root->key(), model, allowedTypes);
}



template<class T>
struct Wrap {

};

struct CheckGUI {
    CheckGUI(tinia::model::gui::Element* root,
             tinia::model::ExposedModel& model)
        : m_root(root), m_model(model)
    {
        ;
    }

    template<typename T>
    void operator()(Wrap<T>) {
        T* element = dynamic_cast<T*>(m_root);
        if(element != NULL) {
            isValid(element, m_model);
        }
    }

    tinia::model::gui::Element* m_root;
    tinia::model::ExposedModel& m_model;
};



}

void validateGUI(tinia::model::gui::Element *root,
                 tinia::model::ExposedModel& model) {
    using namespace tinia::model::gui;
    typedef boost::mpl::vector<TabLayout, HorizontalLayout, VerticalLayout,
            Grid, Tab, ElementGroup,
            KeyValue, Button,
            CheckBox, RadioButtons, ComboBox, SpinBox, DoubleSpinBox,
            HorizontalSlider, Canvas, TextInput> guiTypes;
    CheckGUI f(root, model);
    boost::mpl::for_each<guiTypes, Wrap<boost::mpl::placeholders::_1> >(f);

}
}}}

