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
#include <vector>
#include <string>
#include <map>

namespace tinia {
namespace model {
namespace gui {
enum Device {
    WORKSTATION = 1,
    WEB = 2,
    DESKTOP = 3,
    TOUCH_LARGE = 4,
    TOUCH_SMALL = 8,
    TOUCH = 4|8,
    ALL = 16-1
};

enum ElementType {
    TAB_LAYOUT,                 // 0
    TAB,                        // 1
    GRID,                       // 2
    TEXTINPUT,                  // 3
    CANVAS,                     // 4
    LABEL,                      // 5
    COMBOBOX,                   // 6
    RADIOBUTTONS,               // 7
    ELEMENTGROUP,               // 8
    SPINBOX,                    // 9
    CHECKBOX,                   // 10
    BUTTON,                     // 11
    HORIZONTAL_SLIDER,          // 12
    DOUBLE_SPINBOX,             // 13
    HORIZONTAL_LAYOUT,          // 14
    VERTICAL_LAYOUT,            // 15
    VERTICAL_SPACE,             // 16
    HORIZONTAL_SPACE,           // 17
    VERTICAL_EXPANDING_SPACE,   // 18
    HORIZONTAL_EXPANDING_SPACE, // 19
    POPUP_BUTTON,               // 20
    FILE_DIALOG_BUTTON          //21
};


class Element {
public:
    Element()
        : m_visibility_key(""),
          m_visibility_inverted(false),
          m_enabled_key(""),
          m_enabled_inverted(false)
    {
    }
    virtual ~Element() {}
    /** \return The ElementType of the widget. */
    virtual const ElementType type() const = 0;

    Element*
    setVisibilityKey( const std::string& key, const bool inverted = false )
    {
        m_visibility_key = key;
        m_visibility_inverted = inverted;
        return this; // to allow method chaining
    }

    const std::string&
    visibilityKey() const
    {
        return m_visibility_key;
    }

    const bool
    visibilityInverted() const
    {
        return m_visibility_inverted;
    }

    Element*
    setEnabledKey( const std::string& key, const bool inverted = false )
    {
        m_enabled_key = key;
        m_enabled_inverted = inverted;
        return this; // to allow method chaining
    }

    const std::string&
    enabledKey() const
    {
        return m_enabled_key;
    }

    const bool
    enabledInverted() const
    {
        return m_enabled_inverted;
    }


private:
    std::string  m_visibility_key;
    bool         m_visibility_inverted;
    std::string  m_enabled_key;
    bool         m_enabled_inverted;
};

/** Abstract interface class for elements that contains a reference to a model key. */
class KeyValue
{
public:
    /** Constructor.
      *
      * \param key         The model key to show.
      * \param show_value  Flag that indicates that this view should show the
      *                    value of the element and not the key itself.
      */
    KeyValue( const std::string& key, const bool show_value )
        : m_key( key ),
          m_show_value( show_value )
    {}

    const std::string&
    key() const { return m_key; }

    const bool
    showValue() const { return m_show_value; }

private:
    const std::string   m_key;
    const bool          m_show_value;
};


/** Interface for elements that contains a single child element. */
template<typename ChildType>
class Container0D
{
public:
    Container0D()
        : m_child( NULL )
    {}

    ~Container0D()
    {
        if( m_child != NULL ) {
            delete m_child;
        }
    }

    void setChild( ChildType* child ) { if (m_child != NULL ) { delete m_child; } m_child = child; }
    ChildType* child() { return m_child; }
    const ChildType* child() const { return m_child; }

private:
    ChildType*    m_child;
};

/** Interface for elements that contains a sequence of child elements. */
template<typename ChildType>
class Container1D
{
public:
    ~Container1D()
    {
        for( typename std::vector<ChildType*>::iterator it = m_children.begin(); it != m_children.end(); ++it ) {
            delete *it;
        }
    }

    void addChild( ChildType* child) { m_children.push_back( child ); }
    const size_t children() const { return m_children.size(); }
    ChildType* child( size_t index ) { return m_children[ index ]; }
    const ChildType* child( size_t index ) const { return m_children[ index ]; }

private:
    std::vector<ChildType*>   m_children;
};

/** Interface for elements that contains a grid of child elements. */
template<typename ChildType>
class Container2D
{
public:
    Container2D( size_t height, size_t width )
        : m_width( width ),
          m_height( height )
    {
        m_children.resize( m_width*m_height, NULL );
    }

    ~Container2D()
    {
        for(typename std::vector<ChildType*>::iterator it = m_children.begin(); it != m_children.end(); ++it ) {
            if( *it != NULL ) {
                delete *it;
            }
        }
    }

    const size_t width() const { return m_width; }
    const size_t height() const { return m_height; }
    void setChild( size_t row, size_t col, ChildType* child ) { m_children[ index(row,col) ] = child; }
    ChildType* child( size_t row, size_t col ) { return m_children[ index( row, col ) ]; }
    const ChildType* child( size_t row, size_t col ) const { return m_children[ index( row, col ) ]; }

private:
    size_t                  m_width;
    size_t                  m_height;
    std::vector<ChildType*> m_children;

    const size_t index( const size_t row, const size_t col ) const { return row + col*m_height; }
};



class Tab
        : public Element,
        public KeyValue,
        public Container0D<Element>
{
public:
    Tab( const std::string& key, bool show_value = false )
        : KeyValue( key, show_value )
    {}

    virtual const ElementType type() const { return TAB; }
};

class TabLayout
        : public Element,
        public Container1D<Tab>
{
public:
    virtual const ElementType type() const { return TAB_LAYOUT; }
private:
};


class Grid : public Element, public Container2D<Element> {
public:
    Grid( size_t height, size_t width)
        : Element(),
          Container2D( height, width )
    {}

    virtual const ElementType type() const {return GRID; }
};

class PopupButton
        : public Element,
        public Container0D<Element>,
        public KeyValue
{
public:
    PopupButton( const std::string& key, const bool show_value )
        : KeyValue( key, show_value )
    {}

    virtual const ElementType type() const { return POPUP_BUTTON; }
};

/**
  A standard UI-label. This must reference an element in the model (usually
  this will just be the element the Label describes).
  */
class Label
        : public Element,
        public KeyValue
{
public:
    Label( const std::string& key, const bool show_value=false )
        : KeyValue( key, show_value )
    {}

    virtual const ElementType type() const { return LABEL; }
};

/**
  Abstract base class for all widgets visualizing a state-element
  */
class ExposedModelElement
        : public Element {
public:
    /**
     \param key the key in the model this element is supposed to visualize
     */
    ExposedModelElement(std::string key)
        : m_key(key)
    {

    }

    std::string getKey() { return m_key; }
private:
    std::string m_key;
};


/**
  The recommended element used to represent a restricted element in ExposedModel
  */
class ComboBox
        : public Element,
        public KeyValue
{
public:
    ComboBox( const std::string& key )
        : KeyValue( key, true )
    {}

    virtual const ElementType type() const { return COMBOBOX; }
};

/**
  Radio buttons used to represent a restricted element in ExposedModel
  */
class RadioButtons
        : public Element,
        public KeyValue
{
public:
    RadioButtons(std::string key, bool is_horizontal = false )
        : KeyValue( key, true ),
          m_is_horizontal( is_horizontal )
    {}

    virtual const ElementType type() const { return RADIOBUTTONS; }

    const bool horizontal() const { return m_is_horizontal; }
private:
    bool m_is_horizontal;
};
/**
  A simple TextInput class
  */
class TextInput
        : public Element,
        public KeyValue
{
public:
    TextInput( const std::string& key )
        : KeyValue( key, true )
    {}

    virtual const ElementType type() const { return TEXTINPUT; }
};

class CheckBox
        : public Element,
        public KeyValue
{
public:
    CheckBox( const std::string& key)
        : KeyValue( key, true )
    {}

    virtual const ElementType type() const { return CHECKBOX; }
};

class SpinBox
        : public Element,
        public KeyValue
{
public:
    SpinBox( const std::string& key)
        : KeyValue( key, true )
    {}

    virtual const ElementType type() const { return SPINBOX; }
};

class Button :
        public Element,
        public KeyValue
{
public:
    Button( const std::string& key )
        : KeyValue( key, false )
    {}

    virtual const ElementType type() const { return BUTTON; }
};

class HorizontalSlider
        : public Element,
        public KeyValue
{
public:
    HorizontalSlider( const std::string& key, const bool with_buttons = true )
        : KeyValue( key, true ),
          m_with_buttons( with_buttons )
    {}

    virtual const ElementType type() const { return HORIZONTAL_SLIDER; }

    const bool withButtons() const { return m_with_buttons; }
private:
    bool     m_with_buttons;
};


/** A group containing other widgets. This can be used to aid the user (related
  widgets are grouped under a common banner) or to hide/show one or multiple widgets.
  */
class ElementGroup
        : public Element,
        public KeyValue,
        public Container0D<Element>
{
public:
    ElementGroup( const std::string& key,
                  bool               show_label = true,
                  bool               show_value = false )
        : KeyValue(key, show_value ),
          m_show_label( show_label )
    {}

    virtual const ElementType type() const { return ELEMENTGROUP; }

    const bool showLabel() const { return m_show_label; }

private:
    bool m_show_label;
};


class HorizontalLayout
        : public Element,
        public Container1D<Element>
{
public:
    virtual const ElementType type() const { return HORIZONTAL_LAYOUT; }
};


class VerticalLayout
        : public Element,
        public Container1D<Element>
{
public:
    virtual const ElementType type() const { return VERTICAL_LAYOUT; }
};


/**
  Creates a small vertical space
  */
class VerticalSpace: public Element {
    virtual const ElementType type() const { return VERTICAL_SPACE; }
};


/**
  Creates a small horizontal space
  */
class HorizontalSpace: public Element {
    virtual const ElementType type() const { return HORIZONTAL_SPACE; }
};


/** Expanding vertical space. */
class VerticalExpandingSpace: public Element {
    virtual const ElementType type() const { return VERTICAL_EXPANDING_SPACE; }

};


/** Expanding horizontal space. */
class HorizontalExpandingSpace: public Element {
public:
    virtual const ElementType type() const { return HORIZONTAL_EXPANDING_SPACE; }

};

class FileDialogButton: public KeyValue, public Element {
public:
    FileDialogButton(const std::string &key, bool showValue = false) :
        KeyValue(key, showValue) {}
    virtual const ElementType type() const { return FILE_DIALOG_BUTTON; }
};


class DoubleSpinBox
        : public Element,
        public KeyValue
{
public:
    DoubleSpinBox( const std::string& key )
        : KeyValue( key, true )
    {}

    virtual const ElementType type() const { return DOUBLE_SPINBOX; }
};


/** Wrapper object for script arguments
 */
class ScriptArgument {
public:
    ScriptArgument(const ScriptArgument& other) : m_className(other.className()), m_parameters(other.parameters()) {}
    ScriptArgument(const std::string& className) : m_className(className) {}

    ScriptArgument(const std::string &className, const std::map<std::string, std::string> parameters)
        : m_className(className), m_parameters(parameters)
    {
        ;
    }

    const std::string& className() const {
        return m_className;
    }

    const std::map<std::string, std::string>& parameters() const {
        return m_parameters;
    }

    std::map<std::string, std::string>& parameters() {
        return m_parameters;
    }

private:
    std::string m_className;
    std::map<std::string, std::string> m_parameters;
};

/**
  The main tool for viewing OpenGL-context.
  */
class Canvas
        : public Element,
        public KeyValue
{
public:
    Canvas( const std::string& key )
        : KeyValue( key, true ), m_viewerType("DSRV")
    {
        m_boundingbox_key = "boundingbox";
        m_renderlist_key = key + "_renderlist";
        m_reset_view_key = key + "_reset_view";
        addDefaultsKeys(m_viewerType);
    }

    Canvas( const std::string&  key,
            const std::string&  renderlist_key,
            const std::string&  boundingbox_key )
        : KeyValue(key, true ),
          m_renderlist_key( renderlist_key ),
          m_boundingbox_key( boundingbox_key ),
          m_viewerType("DSRV")
    {
        m_reset_view_key = key + "_reset_view";
        addDefaultsKeys(m_viewerType);
    }

    virtual const ElementType type() const { return CANVAS; }

    virtual const std::string& renderlistKey() const { return m_renderlist_key; }
    virtual Canvas* boundingBoxKey(const std::string& key) { m_boundingbox_key = key; return this; }

    virtual const std::string& boundingBoxKey() const { return m_boundingbox_key; }

    virtual Canvas* resetViewKey(const std::string& key) { m_reset_view_key = key; return this; }
    virtual const std::string& resetViewKey() const { return m_reset_view_key; }

    virtual const ScriptArgument& viewerType() const { return m_viewerType; }

    virtual Canvas* setViewerType(const ScriptArgument& argument)
    {
        m_viewerType = argument;
        addDefaultsKeys(m_viewerType);
        return this;
    }

    virtual Canvas* appendScript(const ScriptArgument& argument) {
        m_scripts.push_back(argument);
        addDefaultsKeys(m_scripts.back());
        return this;
    }

    virtual const std::vector<ScriptArgument>& scripts() const { return m_scripts; }

private:
    void addDefaultsKeys(ScriptArgument& argument) {
        argument.parameters()["renderlistKey"] = renderlistKey();
        argument.parameters()["boundingBoxKey"] = boundingBoxKey();
        argument.parameters()["resetViewKey"] = resetViewKey();
        argument.parameters()["key"] = key();
    }

    std::string m_renderlist_key;
    std::string m_boundingbox_key;
    std::string m_reset_view_key;
    ScriptArgument m_viewerType;
    std::vector<ScriptArgument> m_scripts;

};


}
}
}

