#include "css.hpp"
#include "css_parser.hpp"

MemoryPool<CSS::Value>        CSS::value_pool;
MemoryPool<CSS::Property>     CSS::property_pool;
MemoryPool<CSS::Properties>   CSS::properties_pool;
MemoryPool<CSS::SelectorNode> CSS::selector_node_pool;
MemoryPool<CSS::Selector>     CSS::selector_pool;

CSS::PropertyMap CSS::property_map = {
  { "not-used",       CSS::PropertyId::NOT_USED       },
  { "font-family",    CSS::PropertyId::FONT_FAMILY    }, 
  { "font-size",      CSS::PropertyId::FONT_SIZE      }, 
  { "font-style",     CSS::PropertyId::FONT_STYLE     },
  { "font-weight",    CSS::PropertyId::FONT_WEIGHT    },
  { "text-align",     CSS::PropertyId::TEXT_ALIGN     },
  { "text-indent",    CSS::PropertyId::TEXT_INDENT    },
  { "text-transform", CSS::PropertyId::TEXT_TRANSFORM },
  { "line-height",    CSS::PropertyId::LINE_HEIGHT    },
  { "src",            CSS::PropertyId::SRC            },
  { "margin",         CSS::PropertyId::MARGIN         },
  { "margin-top",     CSS::PropertyId::MARGIN_TOP     },
  { "margin-bottom",  CSS::PropertyId::MARGIN_BOTTOM  },
  { "margin-left",    CSS::PropertyId::MARGIN_LEFT    },
  { "margin-right",   CSS::PropertyId::MARGIN_RIGHT   },
  { "width",          CSS::PropertyId::WIDTH          },
  { "height",         CSS::PropertyId::HEIGHT         },
  { "display",        CSS::PropertyId::DISPLAY        }
};

CSS::FontSizeMap CSS::font_size_map = {
  { "xx-small",  6 },
  { "x-small",   7 },
  { "smaller",   9 },
  { "small",    10 },
  { "medium",   12 },
  { "large",    14 },
  { "larger",   15 },
  { "x-large",  18 },
  { "xx-large", 24 }
};

CSS::Tags CSS::tags
      = {{"p",           Tag::P}, {"div",     Tag::DIV}, {"span", Tag::SPAN}, {"br",  Tag::BREAK}, {"h1",                 Tag::H1},  
         {"h2",         Tag::H2}, {"h3",       Tag::H3}, {"h4",     Tag::H4}, {"h5",     Tag::H5}, {"h6",                 Tag::H6}, 
         {"b",           Tag::B}, {"i",         Tag::I}, {"em",     Tag::EM}, {"body", Tag::BODY}, {"a",                   Tag::A},
         {"img",       Tag::IMG}, {"image", Tag::IMAGE}, {"li",     Tag::LI}, {"pre",   Tag::PRE}, {"blockquote", Tag::BLOCKQUOTE},
         {"strong", Tag::STRONG}, {"none",   Tag::NONE}, {"*",     Tag::ANY}};

const char * CSS::value_type_str[25] = {
  "",     "em",  "ex", "%",   "",   "px",   "cm",   "mm",  "in",  "pt",
  "pc",   "vh",  "vw", "rem", "ch", "vmin", "vmax", "deg", "rad", "grad",
  "msec", "sec", "hz", "khz", "url"
};

CSS::CSS(const std::string & css_id,
         const std::string & file_folder_path, 
         const char *        buffer, 
         int32_t             size)  
  : id(css_id), folder_path(file_folder_path), ghost(false)
{
  CSSParser * parser = new CSSParser(*this, buffer, size);
}

CSS::~CSS()
{
  if (ghost) {
    rules_map.clear();
  }
  else {
    for (auto * props : suites) {
      for (auto * prop : *props) {
        property_pool.deleteElement(prop);
      }
      properties_pool.deleteElement(props);
    }
    suites.clear();

    for (auto & rule : rules_map) {
      selector_pool.deleteElement(rule.first);
    }
    rules_map.clear();
  }
}

void
CSS::show() 
{
  for (auto & rule : rules_map) {
    rule.first->show();
    std::cout << std::endl;
  }
}

