#include <boost/test/unit_test.hpp>

#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <array>
#include <vector>
#include <ctime>
#include <cstring>


#include "policylib/PolicyLib.hpp"
#include "policylib/ElementData.hpp"
#include "policylib/StateListener.hpp"
#include "policylib/StateSchemaListener.hpp"
#include "policylib/GUILayout.hpp"
#include "policylib/File.hpp"

#include "testutils.hpp"

#include <boost/algorithm/string.hpp>


//BOOST_AUTO_TEST_SUITE( PolicyLib )

struct PolicyLibFixture {
   PolicyLibFixture() {}
   ~PolicyLibFixture() {}
   policylib::PolicyLib policylib;
};

// This might be a slight abuse of Fixtures. If someone is offended, please do
// not hestitate to prettify.
struct SchemaListenerFixture {
   SchemaListenerFixture() {
      // Should probably have a add-listener-test as well
      policyLib.addStateSchemaListener(&schemaListener);
   }
   ~SchemaListenerFixture() {}

   policylib::PolicyLib policyLib;
   class SchemaListener : public policylib::StateSchemaListener {
   public:
      SchemaListener() : added_registered(false), removed_registered(false),
         modified_registered(false), added_called_times(0) {}
      virtual void stateSchemaElementAdded(policylib::StateSchemaElement* element)
      {
         added_called_times++;
         added_registered = true;
         added_key = element->getKey();
      }
      virtual void stateSchemaElementRemoved(policylib::StateSchemaElement *stateSchemaElement)
      {
         removed_registered = true;
      }
      virtual void stateSchemaElementModified(policylib::StateSchemaElement *stateSchemaElement)
      {
         modified_registered = true;
      }
      bool added_registered;
      std::string added_key;
      bool removed_registered;
      bool modified_registered;
      int added_called_times;
   } schemaListener;

};




BOOST_FIXTURE_TEST_CASE(stateSchemaElementAdded, SchemaListenerFixture)
{
   BOOST_CHECK(!schemaListener.added_registered);
   BOOST_CHECK_EQUAL(schemaListener.added_called_times, 0);

   policyLib.addElement("myValue", 5);
   BOOST_CHECK(schemaListener.added_registered);
   BOOST_CHECK_EQUAL(schemaListener.added_called_times, 1);

   BOOST_CHECK_EQUAL(schemaListener.added_key, "myValue");

}


BOOST_FIXTURE_TEST_CASE(stateSchemaElementRemoved, SchemaListenerFixture)
{

   BOOST_CHECK(!schemaListener.removed_registered);

   policyLib.addElement("myValue", 5);
   policyLib.removeElement("myValue");
   BOOST_CHECK_EQUAL(schemaListener.added_called_times, 1);

   BOOST_CHECK(schemaListener.removed_registered);
}

BOOST_FIXTURE_TEST_CASE(stateSchemaElementModified, SchemaListenerFixture)
{
   BOOST_CHECK(!schemaListener.modified_registered);

   policyLib.addElement("myValue", 5);
   policyLib.addAnnotation("myValue", "My very own value");
   BOOST_CHECK_EQUAL(schemaListener.added_called_times, 1);

   BOOST_CHECK(schemaListener.modified_registered);
}

BOOST_FIXTURE_TEST_CASE(stateSchemaElementAddedWithConstraints, SchemaListenerFixture)
{
   BOOST_CHECK_EQUAL(schemaListener.added_called_times, 0);

   BOOST_CHECK(!schemaListener.added_registered);
   policyLib.addConstrainedElement("myConstrainedValue", 5, -1, 10);
   BOOST_CHECK_EQUAL(schemaListener.added_called_times, 1);

   BOOST_CHECK(schemaListener.added_registered);
}

struct StateListenerFixture {
   policylib::PolicyLib policyLib;
   StateListenerFixture()
   {
      policyLib.addStateListener(&stateListener);
      policyLib.addElement("myValue", 5);
   }
   ~StateListenerFixture()
   {
   }

   class StateListener : public policylib::StateListener
   {
   public:
      StateListener()
         : modified_registered(false)
      {

      }

      virtual void stateElementModified(policylib::StateElement *stateElement)
      {
         modified_registered = true;
      }
      bool modified_registered;
   } stateListener;
};

BOOST_FIXTURE_TEST_CASE(stateElementNotModified, StateListenerFixture)
{
   BOOST_CHECK(!stateListener.modified_registered);
   policyLib.addElement("value_not_to_be_changed", 10);
   BOOST_CHECK(!stateListener.modified_registered);
   policyLib.updateElement("value_not_to_be_changed", 10);
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
   policyLib.addMatrixElement("myMatrix", matrix);
   BOOST_CHECK(!stateListener.modified_registered);
   policyLib.updateMatrixValue("myMatrix", copy_of_matrix);
   BOOST_CHECK(!stateListener.modified_registered);
}

BOOST_FIXTURE_TEST_CASE(stateElementNotModifed_Viewer, StateListenerFixture)
{
   policylib::Viewer v;
   policyLib.addElement("viewer", v);
   policylib::Viewer w;
   policyLib.updateElement("viewer", w);

   // For complex elements we want an update for now
   BOOST_CHECK(stateListener.modified_registered);
}

BOOST_FIXTURE_TEST_CASE(stateElementModified, StateListenerFixture)
{
   BOOST_CHECK(!stateListener.modified_registered);
   policyLib.updateElement("myValue", 10);
   BOOST_CHECK(stateListener.modified_registered);
}

BOOST_FIXTURE_TEST_CASE(restrictionSetCopy, PolicyLibFixture)
{
   std::vector<std::string> restrictions;
   restrictions.push_back("legal1");
   restrictions.push_back("legal2");
   policylib.addElementWithRestriction<std::string>("myVal", "legal1", restrictions.begin(),
                                       restrictions.end());
   auto restrictions2 = policylib.getRestrictionSet("myVal");
   BOOST_CHECK_EQUAL_COLLECTIONS(restrictions.begin(), restrictions.end(),
                                 restrictions2.begin(), restrictions2.end());

   // Make sure this is a real copy
   restrictions2.erase("legal1");
   auto restrictions3 = policylib.getRestrictionSet("myVal");
   BOOST_CHECK(restrictions3.find("legal1")!=restrictions3.end());

   BOOST_CHECK(!policylib.emptyRestrictionSet("myVal"));
}

BOOST_FIXTURE_TEST_CASE(intAddElement, PolicyLibFixture)
{
   policylib.addElement<int>("myValInt", 12);
   int policylibReturnValue;
   policylib.getElementValue<int>("myValInt", policylibReturnValue);
   BOOST_CHECK_EQUAL(12, policylibReturnValue);
}
BOOST_FIXTURE_TEST_CASE(getFullStateSchema, PolicyLibFixture)
{
      policylib.addConstrainedElement("Element1", 1, 0, 100);
      policylib.addConstrainedElement("Element2", 2, 0, 100);

      policylib.addConstrainedElement("Element3", 3, 0, 100);

      policylib.addConstrainedElement("Element4", 4, 0, 100);

      policylib.addConstrainedElement("Element5", 5, 0, 100);

      policylib.addConstrainedElement("Element6", 6, 0, 100);

      policylib.addConstrainedElement("Element7", 7, 0, 100);

      policylib.addConstrainedElement("Element8", 8, 0, 100);
      policylib.addConstrainedElement("Element9", 9, 0, 100);

      std::vector<policylib::StateSchemaElement> elements;
      policylib.getFullStateSchema(elements);

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

      policylib.getGUILayout(policylib::gui::DESKTOP);

      std::vector<bool>foundCheck;
      for(int i = 0; i < 9; i++) foundCheck.push_back(true);
      BOOST_CHECK_EQUAL_COLLECTIONS(foundCheck.begin(), foundCheck.end(),
                                    found.begin(), found.end());



}

BOOST_FIXTURE_TEST_CASE(PolicyLibGUITestCase, PolicyLibFixture)
{
   policylib::Viewer viewer;
   viewer.height = 500;
   viewer.width = 500;
   policylib.addElement("viewer", viewer);
   policylib.addElement<bool>("myTab", false);
      policylib.addElement<bool>("myBool", false);
   policylib.addElement<std::string>("myVal", "THIS WORKS!");

   const char* restrictions[] = {"select1", "select2", "select3", "select4"};
   policylib.addElementWithRestriction<std::string>("myVal2", "select1",
                                                       &restrictions[0], &restrictions[4]);

   policylib.addConstrainedElement<int>("myIntBA", 5,0, 900);
   policylib.addConstrainedElement<double>("myDouble", 10., 0., 11.);
   policylib.addElement<bool>("myButton", false);


   using namespace policylib::gui;
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
   verticalLayout->addChild(elemGroup);
   verticalLayout->addChild(elemGroup);
   verticalLayout->addChild(elemGroup);
   canvasGrid->setChild(1,0, verticalLayout);
   superLayout->addChild(canvasGrid);


   policylib.setGUILayout(root, DESKTOP);

   std::vector<policylib::StateSchemaElement> elements;
   policylib.getFullStateSchema(elements);
}

BOOST_FIXTURE_TEST_CASE(DefaultLengthCheck, PolicyLibFixture)
{
   policylib.addElement("myKey", "myValue");
   BOOST_CHECK_EQUAL(policylib.getStateSchemaElement("myKey").getLength(),
                     policylib::StateSchemaElement::LENGTH_NOT_SET);
}



BOOST_FIXTURE_TEST_CASE(FileCheck, PolicyLibFixture)
{
   policylib::File myFile;
   policylib.addElement("FileKey", myFile);
   BOOST_CHECK_EQUAL(myFile.fullPath(), std::string(""));
   policylib::File fileFromPolicylib;
   policylib.getElementValue("FileKey", fileFromPolicylib);
   BOOST_CHECK_EQUAL(myFile.fullPath(), fileFromPolicylib.fullPath());

}

BOOST_FIXTURE_TEST_CASE(FileCheckWithPath, PolicyLibFixture)
{
   policylib::File myFile;
   myFile.fullPath("/some/path/file.ext");
   policylib.addElement("FileKey", myFile);
    policylib::File fileFromPolicylib;
   policylib.getElementValue("FileKey", fileFromPolicylib);
   BOOST_CHECK_EQUAL(myFile.fullPath(), fileFromPolicylib.fullPath());
   BOOST_CHECK_EQUAL(myFile.fullPath(), "/some/path/file.ext");

}

BOOST_FIXTURE_TEST_CASE(FileCheckChangePath, PolicyLibFixture)
{
   policylib::File myFile;
   myFile.fullPath("/some/path/file.ext");
   policylib.addElement("FileKey", myFile);
    policylib::File fileFromPolicylib;
   policylib.getElementValue("FileKey", fileFromPolicylib);
   BOOST_CHECK_EQUAL(myFile.fullPath(), fileFromPolicylib.fullPath());
   BOOST_CHECK_EQUAL(myFile.fullPath(), "/some/path/file.ext");

   policylib::File newFile;
   newFile.fullPath("/some/other/path/file.ext");
   policylib.updateElement("FileKey", newFile);

   policylib::File otherFile;
   policylib.getElementValue("FileKey", otherFile);

   BOOST_CHECK_EQUAL(otherFile.fullPath(), newFile.fullPath());

}
