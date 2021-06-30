/**
 * CSS Class
 * 
 * The content of this class is an internal representation of a css file once
 * it has been parsed using a CSSParser instance.
 * 
 * It supplies the tools to search for a pattern matching of rules and the proper 
 * sequencing of search through the specificity sorted list of rules.
 */

#pragma once

#include <cinttypes>

#include <string>
#include <map>
#include <list>
#include <forward_list>
#include <iterator>
#include <iostream>
#include <fstream>

#include "memory_pool.hpp"
#include "dom.hpp"
#include "fonts.hpp"

/**
 * List of supported CSS properties
 * 
 * This is the list of all css properties. The ones
 * in use are marked with a star.
 *      
 *      background
 *      background-attachment
 *      background-color
 *      background-image
 *      background-position
 *      background-repeat
 *      border
 *      border-bottom
 *      border-bottom-color
 *      border-bottom-style
 *      border-bottom-width
 *      border-collapse
 *      border-color
 *      border-left
 *      border-left-color
 *      border-left-style
 *      border-left-width
 *      border-right
 *      border-right-color
 *      border-right-style
 *      border-right-width
 *      border-spacing
 *      border-style
 *      border-top
 *      border-top-color
 *      border-top-style
 *      border-top-width
 *      border-width
 *      bottom
 *      caption-side
 *      clear
 *      clip
 *      color
 *      content
 *      counter-increment
 *      counter-reset
 *      cursor
 *      direction
 *   *  display
 *      empty-cells
 *      float
 *      font
 *   *  font-family
 *   *  font-size
 *   *  font-style
 *      font-variant
 *   *  font-weight
 *   *  height
 *      left
 *      letter-spacing
 *   *  line-height
 *      list-style
 *      list-style-image
 *      list-style-position
 *      list-style-type
 *   *  margin
 *   *  margin-bottom
 *   *  margin-left
 *   *  margin-right
 *   *  margin-top
 *      max-height
 *      max-width
 *      min-height
 *      min-width
 *      outline
 *      outline-color
 *      outline-style
 *      outline-width
 *      overflow
 *      padding
 *      padding-bottom
 *      padding-left
 *      padding-right
 *      padding-top
 *      page-break-after
 *      page-break-before
 *      page-break-inside
 *      position
 *      quotes
 *      right
 *   *  src
 *      table-layout
 *   *  text-align
 *      text-decoration
 *   *  text-indent
 *   *  text-transform
 *      top
 *      vertical-align
 *      visibility
 *      white-space
 *   *  width
 *      word-spacing
 *      z-index
 */

class CSS
{
  private:
    const std::string id;           // Unique identifier (filename) for this CSS instance
    const std::string folder_path;  // Path used for all other files access (relative)
    bool  ghost;                    // True if this instance rules content came from other instances

  public:
    CSS(const std::string & css_id, 
        const std::string & file_folder_path, 
        const char *        buffer, 
        int32_t             size);

    CSS(const std::string & css_id, 
        const std::string & file_folder_path) : id(css_id), folder_path(file_folder_path), ghost(true) {}

   ~CSS();

    const std::string &          get_id() const { return id;          }
    const std::string & get_folder_path() const { return folder_path; }


    enum class     ValueType : uint8_t { NO_TYPE, EM,  EX, PERCENT, STR, PX,   CM,   MM,  IN,  PT, 
                                         PC,      VH,  VW, REM,     CH,  VMIN, VMAX, DEG, RAD, GRAD, 
                                         MSEC,    SEC, HZ, KHZ,     URL };

    enum class         Align : uint8_t { LEFT, CENTER, RIGHT, JUSTIFY };
    enum class TextTransform : uint8_t { NONE, UPPERCASE, LOWERCASE, CAPITALIZE };
    enum class       Display : uint8_t { NONE, INLINE, BLOCK, INLINE_BLOCK };
    enum class    PropertyId : uint8_t { NOT_USED,   FONT_FAMILY, FONT_SIZE,      FONT_STYLE,  FONT_WEIGHT,
                                         TEXT_ALIGN, TEXT_INDENT, TEXT_TRANSFORM, LINE_HEIGHT, SRC,
                                         MARGIN,     MARGIN_LEFT, MARGIN_RIGHT,   MARGIN_TOP,  MARGIN_BOTTOM,
                                         WIDTH,      HEIGHT,      DISPLAY };

    static const char * value_type_str[25];

    typedef std::map<std::string, PropertyId> PropertyMap;
    typedef std::map<std::string, int16_t>    FontSizeMap;

    static PropertyMap property_map;
    static FontSizeMap font_size_map;

    // ---- Selector definition ----

    enum class     SelOp : uint8_t { NONE, DESCENDANT, CHILD, ADJACENT };
    enum class Qualifier : uint8_t { NONE, FIRST_CHILD                 };

    typedef std::forward_list<std::string> ClassList;

    #pragma pack(push, 1)
      // The following is OK in a little endian context.
      union Specificity {
        uint32_t value;
        struct {
          uint8_t order, tag_count, class_count, id_count;
        } spec;
        void show() const {
          std::cout << "[" << +spec.id_count    << ","
                           << +spec.class_count << ","
                           << +spec.tag_count   << ","
                           << +spec.order 
                           <<"]("
                           << value << ") ";

        }
      };

      struct SelectorNode {
        std::string id;
        ClassList   class_list;
        Qualifier   qualifier;
        uint8_t     class_count, id_count;
        SelOp       op;
        DOM::Tag    tag;
        SelectorNode() {
          op          = SelOp::NONE;
          tag         = DOM::Tag::NONE;
          qualifier   = Qualifier::NONE;
          class_count = 0;
          id_count    = 0;
        }
        ~SelectorNode() {
          class_list.clear();
        }
        void add_class(std::string class_name) {
          class_list.push_front(class_name);
          class_count += 1;
        }
        void add_id(std::string the_id) {
          id = the_id;
          id_count += 1;
        }
        void set_tag(DOM::Tag the_tag) {
          tag = the_tag;
        }
        void set_qualifier(Qualifier q) {
          qualifier = q;
        }
        void show() const {
          if (op == SelOp::CHILD)      std::cout << " > ";
          if (op == SelOp::ADJACENT)   std::cout << " + ";
          if (op == SelOp::DESCENDANT) std::cout <<   " ";
          if (tag != DOM::Tag::NONE) {
            for (const auto & [key, value] : DOM::tags) {
              if (value == tag) {
                std::cout << key;
                break;
              }
            }         
          }       
          if (id_count > 0) std::cout << "#" << id;
          for (auto & cl : class_list) std::cout << "." << cl;
          if (qualifier == Qualifier::FIRST_CHILD) std::cout << ":first_child";
        }
      };

      typedef std::forward_list<SelectorNode *> SelectorNodeList;

      struct Selector {
        Specificity specificity;
        SelectorNodeList selector_node_list;
        Selector() {
          specificity.value = 0;
        }
        ~Selector() {
          for (auto * selector_node : selector_node_list) {
              selector_node_pool.deleteElement(selector_node);
          }
          selector_node_list.clear();
        }
        void add_selector_node(SelectorNode * node) {
          selector_node_list.push_front(node);
        }
        void compute_specificity(uint8_t order) {
          specificity.spec.order = order;
          for (auto * node : selector_node_list) {
            specificity.spec.tag_count   += (((node->tag == DOM::Tag::NONE) || (node->tag == DOM::Tag::ANY)) ? 0 : 1);
            specificity.spec.class_count += node->class_count;
            specificity.spec.id_count    += node->id_count;
          }
        }
        bool is_empty() {
          return selector_node_list.empty();
        }
        void show_selector(SelectorNodeList::const_iterator node_it, int8_t lev) const {
          if (node_it != selector_node_list.end()) {
            SelectorNodeList::const_iterator next_node_it = node_it;
            show_selector(++next_node_it, lev + 1);
            (*node_it)->show();
          }
        }
        void show() const {
          specificity.show();
          show_selector(selector_node_list.cbegin(), 0); 
        }
      };

      struct Value {
        float       num;
        std::string str;
        ValueType   value_type;
        union {
          Display          display;
          Align            align;
          TextTransform    text_transform;
          Fonts::FaceStyle face_style;
        } choice;
        Value() {
          value_type = ValueType::NO_TYPE;
          num = 0.0;
        }
        void show() {
          if (value_type == ValueType::STR) {
            std::cout << str;
          }
          else if (value_type == ValueType::URL) {
            std::cout << "url(" << str << ')';
          }
          else {
            std::cout << num << value_type_str[(uint8_t)value_type];
          }         
        }
      };

      typedef std::forward_list<Value *> Values;

      struct Property {
        PropertyId id;
        Values     values;
        ~Property() {
          for (auto * value : values) {
            value_pool.deleteElement(value);
          }
          values.clear();
        }
        void add_value(Value * v) {
          values.push_front(v);
        }
        void completed() {
          values.reverse();
        }
        void show() {
          std::cout << "  ";
          for (const auto & [key, value] : property_map) {
            if (value == id) {
              std::cout << key;
              break;
            }
          }         
          std::cout << ": ";
          bool first = true;
          for (auto * v : values) {
            if (!first) std::cout << ", ";
            v->show();
            first = false;
          }
          std::cout << ';' << std::endl;
        }
      };
    #pragma pack(pop)

    // Sorted from the less specific to the most specific
    struct rule_compare {
      bool operator() (const Selector * s1, const Selector * s2) const { 
        return s1->specificity.value < s2->specificity.value; 
      }
    };

    typedef std::list<Selector *>              Selectors;
    typedef std::forward_list<Property *>      Properties;

    // typedef std::list<Properties *>            PropertySuite;
    typedef std::forward_list<Properties *>    PropertySuiteList;

    typedef std::map<Selector *, Properties *, rule_compare>  RulesMap;

    RulesMap          rules_map;
    PropertySuiteList suites;     // Linear list of suites to be deleted when the instance will be destroyed.

    static MemoryPool<Value>        value_pool;
    static MemoryPool<Property>     property_pool;
    static MemoryPool<Properties>   properties_pool;
    static MemoryPool<SelectorNode> selector_node_pool;
    static MemoryPool<Selector>     selector_pool;

    void match(DOM::Node * node, RulesMap & to_rules);
    void show(RulesMap & the_rules_map);

  private:
    bool match_simple_selector(DOM::Node & node, SelectorNode & simple_sel);
    bool match_selector(DOM::Node * node, Selector & sel);
};