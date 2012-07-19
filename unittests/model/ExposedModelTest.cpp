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

#include <boost/test/unit_test.hpp>

#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <array>
#include <vector>
#include <ctime>
#include <cstring>


#include "tinia/model/ExposedModel.hpp"
#include "tinia/model/impl/ElementData.hpp"
#include "tinia/model/StateListener.hpp"
#include "tinia/model/StateSchemaListener.hpp"
#include "tinia/model/GUILayout.hpp"
#include "tinia/model/File.hpp"

#include "testutils.hpp"

#include <boost/algorithm/string.hpp>



using namespace tinia;
namespace  {
struct ExposedModelFixture {
   ExposedModelFixture() {}
   ~ExposedModelFixture() {}
   model::ExposedModel model;
};

// This might be a slight abuse of Fixtures. If someone is offended, please do
// not hestitate to prettify.
struct SchemaListenerFixture {
   SchemaListenerFixture() {
      // Should probably have a add-listener-test as well
      model.addStateSchemaListener(&schemaListener);
   }
   ~SchemaListenerFixture() {}

   model::ExposedModel model;
   class SchemaListener : public model::StateSchemaListener {
   public:
      SchemaListener() : added_registered(false), removed_registered(false),
         modified_registered(false), added_called_times(0) {}
      virtual void stateSchemaElementAdded(model::StateSchemaElement* element)
      {
         added_called_times++;
         added_registered = true;
         added_key = element->getKey();
      }
      virtual void stateSchemaElementRemoved(model::StateSchemaElement *stateSchemaElement)
      {
         removed_registered = true;
      }
      virtual void stateSchemaElementModified(model::StateSchemaElement *stateSchemaElement)
      {
          modified_key = stateSchemaElement->getKey();
          modified_max = stateSchemaElement->getMaxConstraint();
          modified_min = stateSchemaElement->getMinConstraint();
         modified_registered = true;
      }
      bool added_registered;
      std::string added_key;
      bool removed_registered;
      bool modified_registered;
      int added_called_times;
      std::string modified_key;
      std::string modified_max;
      std::string modified_min;
   } schemaListener;

};

}

BOOST_FIXTURE_TEST_CASE(constraintsChange, SchemaListenerFixture)
{
    BOOST_CHECK( !schemaListener.modified_registered );
    BOOST_CHECK( !schemaListener.added_registered );
    model.addConstrainedElement("constrained", 0, 0, 10);
    BOOST_CHECK( schemaListener.added_registered );
    BOOST_CHECK( schemaListener.modified_registered );
    schemaListener.modified_registered = false;
    model.updateConstraints("constrained", 15, 12, 20);
    BOOST_CHECK(schemaListener.modified_registered);
    BOOST_CHECK_EQUAL("constrained", schemaListener.modified_key);
    BOOST_CHECK_EQUAL("12", schemaListener.modified_min);
    BOOST_CHECK_EQUAL("20", schemaListener.modified_max);

    BOOST_CHECK_THROW(model.updateElement("constrained", 0), std::invalid_argument);

}


BOOST_FIXTURE_TEST_CASE(stateSchemaElementAdded, SchemaListenerFixture)
{
   BOOST_CHECK(!schemaListener.added_registered);
   BOOST_CHECK_EQUAL(schemaListener.added_called_times, 0);

   model.addElement("myValue", 5);
   BOOST_CHECK(schemaListener.added_registered);
   BOOST_CHECK_EQUAL(schemaListener.added_called_times, 1);

   BOOST_CHECK_EQUAL(schemaListener.added_key, "myValue");

}


BOOST_FIXTURE_TEST_CASE(stateSchemaElementRemoved, SchemaListenerFixture)
{

   BOOST_CHECK(!schemaListener.removed_registered);

   model.addElement("myValue", 5);
   model.removeElement("myValue");
   BOOST_CHECK_EQUAL(schemaListener.added_called_times, 1);

   BOOST_CHECK(schemaListener.removed_registered);
}

BOOST_FIXTURE_TEST_CASE(stateSchemaElementModified, SchemaListenerFixture)
{
   BOOST_CHECK(!schemaListener.modified_registered);

   model.addElement("myValue", 5);
   model.addAnnotation("myValue", "My very own value");
   BOOST_CHECK_EQUAL(schemaListener.added_called_times, 1);

   BOOST_CHECK(schemaListener.modified_registered);
}

BOOST_FIXTURE_TEST_CASE(stateSchemaElementAddedWithConstraints, SchemaListenerFixture)
{
   BOOST_CHECK_EQUAL(schemaListener.added_called_times, 0);

   BOOST_CHECK(!schemaListener.added_registered);
   model.addConstrainedElement("myConstrainedValue", 5, -1, 10);
   BOOST_CHECK_EQUAL(schemaListener.added_called_times, 1);

   BOOST_CHECK(schemaListener.added_registered);
}

struct StateListenerFixture {
   model::ExposedModel model;
   StateListenerFixture()
   {
      model.addStateListener(&stateListener);
      model.addElement("myValue", 5);
   }
   ~StateListenerFixture()
   {
   }

   class StateListener : public model::StateListener
   {
   public:
      StateListener()
         : modified_registered(false)
      {

      }

      virtual void stateElementModified(model::StateElement *stateElement)
      {
         modified_registered = true;
      }
      bool modified_registered;
   } stateListener;
};

BOOST_FIXTURE_TEST_CASE(stateElementNotModified, StateListenerFixture)
{
   BOOST_CHECK(!stateListener.modified_registered);
   model.addElement("value_not_to_be_changed", 10);
   BOOST_CHECK(!stateListener.modified_registered);
   model.updateElement("value_not_to_be_changed", 10);
   BOOST_CHECK(!stateListener.modified_registered);

}

BOOST_FIXTURE_TEST_CASE(stateElementNotModified_Matrices, StateListenerFixture)
{
   float matrix[16] = {1, 0, 0, 0,
                       0, 1, 0, 0,
                       0, 0, 1, 0,
                       0, 0, 0, 1};
   float copy_of_matrix[16];
   std::copy(&matrix[0], &matrix[16], &copy_of_matrix[0]);

   BOOST_CHECK(!stateListener.modified_registered);
   model.addMatrixElement("myMatrix", matrix);
   BOOST_CHECK(!stateListener.modified_registered);
   model.updateMatrixValue("myMatrix", copy_of_matrix);
   BOOST_CHECK(!stateListener.modified_registered);
}

BOOST_FIXTURE_TEST_CASE(stateElementNotModifed_Viewer, StateListenerFixture)
{
   model::Viewer v;
   model.addElement("viewer", v);
   model::Viewer w;
   model.updateElement("viewer", w);

   // For complex elements we want an update for now
   BOOST_CHECK(stateListener.modified_registered);
}

BOOST_FIXTURE_TEST_CASE(stateElementModified, StateListenerFixture)
{
   BOOST_CHECK(!stateListener.modified_registered);
   model.updateElement("myValue", 10);
   BOOST_CHECK(stateListener.modified_registered);
}

BOOST_FIXTURE_TEST_CASE(restrictionSetCopy, ExposedModelFixture)
{
   std::vector<std::string> restrictions;
   restrictions.push_back("legal1");
   restrictions.push_back("legal2");
   model.addElementWithRestriction<std::string>("myVal", "legal1", restrictions.begin(),
                                       restrictions.end());
   auto restrictions2 = model.getRestrictionSet("myVal");

   BOOST_CHECK_EQUAL_COLLECTIONS(restrictions.begin(), restrictions.end(),
                                 restrictions2.begin(), restrictions2.end());

   // Make sure this is a real copy
   restrictions2.erase("legal1");
   auto restrictions3 = model.getRestrictionSet("myVal");
   BOOST_CHECK(restrictions3.find("legal1")!=restrictions3.end());

   BOOST_CHECK(!model.emptyRestrictionSet("myVal"));
}

BOOST_FIXTURE_TEST_CASE(intAddElement, ExposedModelFixture)
{
   model.addElement<int>("myValInt", 12);
   int modelReturnValue;
   model.getElementValue<int>("myValInt", modelReturnValue);
   BOOST_CHECK_EQUAL(12, modelReturnValue);
}
BOOST_FIXTURE_TEST_CASE(getFullStateSchema, ExposedModelFixture)
{
      model.addConstrainedElement("Element1", 1, 0, 100);
      model.addConstrainedElement("Element2", 2, 0, 100);

      model.addConstrainedElement("Element3", 3, 0, 100);

      model.addConstrainedElement("Element4", 4, 0, 100);

      model.addConstrainedElement("Element5", 5, 0, 100);

      model.addConstrainedElement("Element6", 6, 0, 100);

      model.addConstrainedElement("Element7", 7, 0, 100);

      model.addConstrainedElement("Element8", 8, 0, 100);
      model.addConstrainedElement("Element9", 9, 0, 100);

      std::vector<model::StateSchemaElement> elements;
      model.getFullStateSchema(elements);

      std::vector<bool> found;
      for(int i = 0; i < 9; i++) found.push_back(false);
      for(auto it = elements.begin(); it!=elements.end(); it++)
      {
         std::string key = it->getKey();
         int i = (int)(key[key.size()-1]-'0');
         BOOST_CHECK( i > 0);
         BOOST_CHECK( i <= 9);
         found[i-1] = true;
      }

      model.getGUILayout(model::gui::DESKTOP);

      std::vector<bool>foundCheck;
      for(int i = 0; i < 9; i++) foundCheck.push_back(true);
      BOOST_CHECK_EQUAL_COLLECTIONS(foundCheck.begin(), foundCheck.end(),
                                    found.begin(), found.end());



}

BOOST_FIXTURE_TEST_CASE(ExposedModelGUITestCase, ExposedModelFixture)
{
   model::Viewer viewer;
   viewer.height = 500;
   viewer.width = 500;
   model.addElement("viewer", viewer);
   model.addElement<bool>("myTab", false);
      model.addElement<bool>("myBool", false);
   model.addElement<std::string>("myVal", "THIS WORKS!");

   const char* restrictions[] = {"select1", "select2", "select3", "select4"};
   model.addElementWithRestriction<std::string>("myVal2", "select1",
                                                       &restrictions[0], &restrictions[4]);

   model.addConstrainedElement<int>("myIntBA", 5,0, 900);
   model.addConstrainedElement<double>("myDouble", 10., 0., 11.);
   model.addElement<bool>("myButton", false);


   using namespace model::gui;
   TabLayout* root = new TabLayout();
   HorizontalLayout *superLayout = new HorizontalLayout();

   Grid *mainGrid = new Grid(10, 1);
   superLayout->addChild(mainGrid);
   Tab *mainTab = new Tab("myTab");
   mainTab->setChild(superLayout);
   root->addChild(mainTab);

   // Used to hold our two input fields.


   // Simple Label-input-layout
   Grid *input1Grid = new Grid(3,2);
   mainGrid->setChild(0,0, input1Grid);
   input1Grid->setChild(0, 0, new Label("myVal"));
   input1Grid->setChild(0, 1, new TextInput("myVal"));

   input1Grid->setChild(1,0, new Label("myVal2"));
   input1Grid->setChild(1,1, new ComboBox("myVal2"));

   input1Grid->setChild(2,0, new Label("myVal2"));
   input1Grid->setChild(2,1, new RadioButtons("myVal2"));

   mainGrid->setChild(1, 0, new SpinBox("myIntBA"));
   mainGrid->setChild(2, 0, new TextInput("myIntBA"));
   mainGrid->setChild(3, 0, new CheckBox("myTab"));
   mainGrid->setChild(4, 0, new CheckBox("myTab"));
   mainGrid->setChild(5, 0, new Button("myButton"));
   mainGrid->setChild(6, 0, new HorizontalSlider("myIntBA"));
   mainGrid->setChild(8,0, new DoubleSpinBox("myDouble"));
   mainGrid->setChild(9, 0, new CheckBox("myBool"));

   ElementGroup* elemGroup = new ElementGroup("myTab", false);

   elemGroup->setChild(new TextInput("myVal"));
   mainGrid->setChild(7, 0, elemGroup);

   Grid* canvasGrid = new Grid(2,1);
   Canvas *canvas = new Canvas("viewer");

   canvasGrid->setChild(0,0, canvas);
   VerticalLayout* verticalLayout = new VerticalLayout;
   verticalLayout->setVisibilityKey( "myBool" );

   canvasGrid->setChild(1,0, verticalLayout);
   superLayout->addChild(canvasGrid);


   model.setGUILayout(root, DESKTOP);

   std::vector<model::StateSchemaElement> elements;
   model.getFullStateSchema(elements);
}

BOOST_FIXTURE_TEST_CASE(DefaultLengthCheck, ExposedModelFixture)
{
   model.addElement("myKey", "myValue");
   BOOST_CHECK_EQUAL(model.getStateSchemaElement("myKey").getLength(),
                     model::StateSchemaElement::LENGTH_NOT_SET);
}


BOOST_FIXTURE_TEST_CASE(GetElementByValue, ExposedModelFixture)
{
    model.addElement("one", 1);
    auto one = model.getElementValue<int>("one");
    BOOST_CHECK_EQUAL(1, one);
}


BOOST_FIXTURE_TEST_CASE(FileCheck, ExposedModelFixture)
{
   model::File myFile;
   model.addElement("FileKey", myFile);
   BOOST_CHECK_EQUAL(myFile.fullPath(), std::string(""));
   model::File fileFromExposedModel;
   model.getElementValue("FileKey", fileFromExposedModel);
   BOOST_CHECK_EQUAL(myFile.fullPath(), fileFromExposedModel.fullPath());

}

BOOST_FIXTURE_TEST_CASE(FileCheckWithPath, ExposedModelFixture)
{
   model::File myFile;
   myFile.fullPath("/some/path/file.ext");
   model.addElement("FileKey", myFile);
    model::File fileFromExposedModel;
   model.getElementValue("FileKey", fileFromExposedModel);
   BOOST_CHECK_EQUAL(myFile.fullPath(), fileFromExposedModel.fullPath());
   BOOST_CHECK_EQUAL(myFile.fullPath(), "/some/path/file.ext");

}

BOOST_FIXTURE_TEST_CASE(FileCheckChangePath, ExposedModelFixture)
{
   model::File myFile;
   myFile.fullPath("/some/path/file.ext");
   model.addElement("FileKey", myFile);
    model::File fileFromExposedModel;
   model.getElementValue("FileKey", fileFromExposedModel);
   BOOST_CHECK_EQUAL(myFile.fullPath(), fileFromExposedModel.fullPath());
   BOOST_CHECK_EQUAL(myFile.fullPath(), "/some/path/file.ext");

   model::File newFile;
   newFile.fullPath("/some/other/path/file.ext");
   model.updateElement("FileKey", newFile);

   model::File otherFile;
   model.getElementValue("FileKey", otherFile);

   BOOST_CHECK_EQUAL(otherFile.fullPath(), newFile.fullPath());

}


