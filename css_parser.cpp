
// Simple CSS Parser
//
// Guy Turcotte
// January 2021
//
// This is a bare CSS Parser. Only lexical and syntax analysis.
//
// The parser implements the definition as stated in appendix G of
// the CSS V2.1 definition (https://www.w3.org/TR/CSS21/grammar.html)
// with the following differences:
//
// - No support for unicode
// - Supports numbers starting with '.'
// - Supports @font-style 
//

#define CSS_PARSER_TEST 1

#include <cinttypes>
#include <cstring>

#if CSS_PARSER_TEST
  #include <iostream>
#endif

class CSSParser
{
  public:
    CSSParser() : skip(0), allow_minus(false) { }
   ~CSSParser() { }

  private:
    static constexpr char const * TAG = "CSSParser";
    
    uint8_t skip;

    // ---- Tokenizer ----

    static const int8_t  IDENT_SIZE  =  32;
    static const int8_t  NAME_SIZE   =  32;
    static const int16_t STRING_SIZE = 128;

    enum class Token : uint8_t { 
      ERROR, S, CDO, CDC, INCLUDES, DASHMATCH, STRING, BAD_STRING, IDENT, HASH, 
      IMPORT_SYM, PAGE_SYM, MEDIA_SYM, CHARSET_SYM, FONT_FACE_SYM, IMPORTANT_SYM,
      EMS, EXS, LENGTH, ANGLE, TIME, FREQ, DIMENSION, PERCENTAGE,
      NUMBER, URI, BAD_URI, FUNCTION, SEMICOLON, COLON, COMMA, GT, LT, GE, LE,
      MINUS, PLUS, DOT, STAR, SLASH, EQUAL,
      LBRACK, RBRACK, LBRACE, RBRACE, LPARENT, RPARENT, 
      END_OF_FILE
    };

    enum class LengthType : uint8_t { PX, CM, MM, IN, PT, PC, VH, VW, REM, CH, VMIN, VMAX };
    enum class AngleType  : uint8_t { DEG, RAD, GRAD };
    enum class TimeType   : uint8_t { MS,  S         };
    enum class FreqType   : uint8_t { HZ,  KHZ       };

    int32_t   remains; // number of bytes remaining in the css buffer
    uint8_t * str;     // pointer in the css buffer
    uint8_t   ch;      // next character to be processed

    uint8_t  ident[ IDENT_SIZE];
    uint8_t string[STRING_SIZE];
    uint8_t   name[  NAME_SIZE];

    bool allow_minus;

    float   num;
    
    Token      token;

    LengthType length_type;
    AngleType   angle_type;
    TimeType     time_type;
    FreqType     freq_type;

    void next_ch() { 
      if (remains > 0) { 
        remains--; 
        ch = *str++; 
      } 
      else ch = '\0'; 
    }
    
    inline bool is_space() {
      return (ch == ' ' ) ||
             (ch == '\t') ||
             (ch == '\r') ||
             (ch == '\n') ||
             (ch == '\f'); 
    }

    inline bool is_nmchar() {
      return ((ch >= 'a') && (ch <= 'z')) ||
             ((ch >= 'A') && (ch <= 'Z')) ||
             ((ch >= '0') && (ch <= '9')) ||
             (ch == '_')  || 
             (ch == '-')  ||
             (ch == '\\') ||
             (ch >= 160);
    }
    
    inline bool is_nmstart() {
      return ((ch >= 'a') && (ch <= 'z')) ||
             ((ch >= 'A') && (ch <= 'Z')) ||
             ((ch >= '0') && (ch <= '9')) ||
             (ch == '_' ) || 
             (ch == '\\') ||
             (allow_minus && (ch == '-')) ||
             (ch >= 160);
    }
    
    void skip_spaces() {
      while (is_space()) next_ch();
    }

    bool parse_url() {
      int16_t idx = 0;
      while ((ch > ' ') && (idx < (STRING_SIZE - 1))) {
        if ((ch == '"') || (ch == '\'') || (ch == '(')) {
          return false;
        }
        if (ch == '\\') {
          next_ch();
          if (ch == '\0') return false;
          if (ch == '\r') {
            string[idx++] = '\n';
            next_ch();
            if (ch == '\n') next_ch();
          }
          else {
            string[idx++] = ch;
            next_ch();
          }
        }
        else {
          string[idx++] = ch;
          next_ch();
        }
      }
      string[idx] = 0;
      return true;
    }

    bool parse_string() {
      char    delim = ch;
      int16_t idx   = 0;

      next_ch();
      while ((ch != '\0') && (ch != delim) && (idx < (STRING_SIZE - 1))) {
        if (ch == '\\') {
          next_ch();
          if (ch == '\0') return false;
          if (ch == '\r') {
            string[idx++] = '\n';
            next_ch();
            if (ch == '\n') next_ch();
          }
          else {
            string[idx++] = ch;
            next_ch();
          }
        }
        else if ((ch >= ' ') || (ch == '\t')) {
          string[idx++] = ch;
          next_ch();
        }
        else {
          next_ch();
        }
      }
      string[idx] = 0;
      bool res = ch == delim;
      if (res) next_ch();
      return res;
    }

    void parse_number() {
      num = 0;
      float dec = 0.1;
      while ((ch >= '0') && (ch <= '9')) {
        num = (num * 10) + (ch - '0');
        next_ch();
      }
      if (ch == '.') {
        next_ch();
        while ((ch >= '0') && (ch <= '9')) {
          num = num + (dec * (ch - '0'));
          dec = dec * 0.1;
          next_ch();
        }
      }
    }

    void parse_name() {
      int8_t idx = 0;
      while ((ch != '\0') && is_nmchar() && (idx < (NAME_SIZE - 1))) {
        if (ch == '\\') {
          next_ch();
          if (ch >= ' ') {
            name[idx++] = ch;
            next_ch();
          }
          else break;
        }
        else {
          name[idx++] = ch;
          next_ch();
        }
      }
      if (idx < NAME_SIZE) string[idx] = 0;
    }

    void parse_ident() {
      int8_t idx = 0;
      while ((ch != '\0') && is_nmchar() && (idx < (NAME_SIZE - 1))) {
        if (ch == '\\') {
          next_ch();
          if (ch >= ' ') {
            ident[idx++] = ch;
            next_ch();
          }
          else break;
        }
        else {
          ident[idx++] = ch;
          next_ch();
        }
      }
      if (idx < IDENT_SIZE) ident[idx] = 0;
      #if CSS_PARSER_TEST
        std::cout << "Ident: " << ident << std::endl;
      #endif
    }

    bool skip_comment() {
      for (;;) {
        if ((ch == '*') && (str[0] == '/')) {
          next_ch(); next_ch();
          break;
        }
        else if (ch == '\0') return false;
        next_ch();
      }
      return true;
    }

    void next_token() {
      bool done = false;
      while (!done) {
        if      (ch == '\0') token = Token::END_OF_FILE;
        else if (is_space()) { next_ch(); token = Token::S;         }
        else if (ch == ';')  { next_ch(); token = Token::SEMICOLON; }
        else if (ch == ':')  { next_ch(); token = Token::COLON;     }
        else if (ch == ',')  { next_ch(); token = Token::COMMA;     }
        else if (ch == '{')  { next_ch(); token = Token::LBRACE;    }
        else if (ch == '}')  { next_ch(); token = Token::RBRACE;    }
        else if (ch == '[')  { next_ch(); token = Token::LBRACK;    }
        else if (ch == ']')  { next_ch(); token = Token::RBRACK;    }
        else if (ch == '(')  { next_ch(); token = Token::LPARENT;   }
        else if (ch == ')')  { next_ch(); token = Token::RPARENT;   }
        else if (ch == '=')  { next_ch(); token = Token::EQUAL;     }
        else if (ch == '+')  { next_ch(); token = Token::PLUS;      }
        else if (ch == '*')  { next_ch(); token = Token::STAR;      }
        else if ((ch == '\'') || (ch == '\"')) {
          token = parse_string() ? Token::STRING : Token::BAD_STRING;
        }
        else if (
            ((ch == '-') && ((str[0] == '.') || ((str[0] >= '0') && (str[0] <= '9')))) ||
            ((ch >= '0') && (ch <= '9')) || 
            ((ch == '.') && ((str[0] >= '0') && (str[0] <= '9')))) {
          parse_number();
          token = Token::NUMBER;
          if      ((ch == 'e') && (str[0] == 'm')) { next_ch(); next_ch(); token = Token::EMS       ; }
          else if ((ch == 'e') && (str[0] == 'x')) { next_ch(); next_ch(); token = Token::EXS       ; }
          else if  (ch == '%')                     { next_ch();            token = Token::PERCENTAGE; }
          else if ((ch == 'p') && (str[0] == 'x')) { next_ch(); next_ch(); token = Token::LENGTH; length_type = LengthType::PX; }
          else if ((ch == 'p') && (str[0] == 't')) { next_ch(); next_ch(); token = Token::LENGTH; length_type = LengthType::PT; }
          else if ((ch == 'p') && (str[0] == 'c')) { next_ch(); next_ch(); token = Token::LENGTH; length_type = LengthType::PC; }
          else if ((ch == 'c') && (str[0] == 'm')) { next_ch(); next_ch(); token = Token::LENGTH; length_type = LengthType::CM; }
          else if ((ch == 'm') && (str[0] == 'm')) { next_ch(); next_ch(); token = Token::LENGTH; length_type = LengthType::MM; }
          else if ((ch == 'i') && (str[0] == 'n')) { next_ch(); next_ch(); token = Token::LENGTH; length_type = LengthType::IN; }
          else if ((ch == 'v') && (str[0] == 'h')) { next_ch(); next_ch(); token = Token::LENGTH; length_type = LengthType::VH; }
          else if ((ch == 'v') && (str[0] == 'w')) { next_ch(); next_ch(); token = Token::LENGTH; length_type = LengthType::VW; }
          else if ((ch == 'c') && (str[0] == 'h')) { next_ch(); next_ch(); token = Token::LENGTH; length_type = LengthType::CH; }
          else if ((ch == 'm') && (str[0] == 's')) { next_ch(); next_ch(); token = Token::TIME  ; time_type   =   TimeType::MS; }
          else if  (ch == 's')                     { next_ch();            token = Token::TIME  ; time_type   =   TimeType::S ; }
          else if ((ch == 'h') && (str[0] == 'z')) { next_ch(); next_ch(); token = Token::FREQ  ; freq_type   =   FreqType::HZ; }
          else if ((ch == 'k') && (str[0] == 'h') && (str[1] == 'z')) {
            remains -= 2; str += 2; next_ch(); token = Token::FREQ; freq_type = FreqType::KHZ;
          }
          else if ((ch == 'd') && (str[0] == 'e') && (str[1] == 'g')) {
            remains -= 2; str += 2; next_ch(); token = Token::ANGLE; angle_type = AngleType::DEG;
          }
          else if ((ch == 'r') && (str[0] == 'a') && (str[1] == 'd')) {
            remains -= 2; str += 2; next_ch(); token = Token::ANGLE; angle_type = AngleType::RAD;
          }
          else if ((ch == 'r') && (str[0] == 'e') && (str[1] == 'm')) {
            remains -= 2; str += 2; next_ch(); token = Token::LENGTH; length_type = LengthType::REM;
          }
          else if ((ch == 'g') && (str[0] == 'r') && (str[1] == 'a') && (str[2] == 'd')) {
            remains -= 3; str += 3; next_ch(); token = Token::ANGLE; angle_type = AngleType::GRAD;
          }
          else if ((ch == 'v') && (str[0] == 'm') && (str[1] == 'i') && (str[2] == 'n')) {
            remains -= 3; str += 3; next_ch(); token = Token::LENGTH; length_type = LengthType::VMIN;
          }
          else if ((ch == 'v') && (str[0] == 'm') && (str[1] == 'a') && (str[2] == 'x')) {
            remains -= 3; str += 3; next_ch(); token = Token::LENGTH; length_type = LengthType::VMAX;
          }
          else if (is_nmstart()) {
            parse_ident();
            token = Token::DIMENSION;
          }
        }
        else if ((ch == '-') && (str[0] == '-') && (str[1] == '>')) {
          token = Token::CDC;
          remains -= 2; str += 2; next_ch();
        }        
        else if (is_nmstart()) {
          parse_ident();
          token = Token::IDENT;
          if (ch == '(') {
            next_ch();
            if (strcmp((char *)ident, "url") == 0) {
              skip_spaces();
              if ((ch == '"') || (ch == '\'')) {
                token = parse_string() ? Token::URI : Token::BAD_URI;
              }
              else {
                token = parse_url() ? Token::URI : Token::BAD_URI;
              }
              skip_spaces();
              if (ch == ')') {
                next_ch();
              }
              else {
                token = Token::BAD_URI;
              }
            }
            else {
              token = Token::FUNCTION;
            }
          } 
        }
        else if ((ch == '>') && (str[0] == '=')) { next_ch(); next_ch(); token = Token::GE; }
        else if ((ch == '<') && (str[0] == '=')) { next_ch(); next_ch(); token = Token::LE; }
        else if (ch == '-') { next_ch(); token = Token::MINUS;     }
        else if (ch == '.') { next_ch(); token = Token::DOT;       }
        else if (ch == '>') { next_ch(); token = Token::GT;        }
        else if (ch == '<') { next_ch(); token = Token::LT;        }
        else if (ch == '#') {
          next_ch();
          parse_name();
          token = Token::HASH;
        }
        else if ((ch == '<') && (strncmp((char *)str, "!--", 3) == 0)) {
          token = Token::CDO;
          remains -= 3; str += 3; next_ch();
        }
        else if (ch == '@') {
          if (strncmp((char *) str, "media", 5) == 0) {
            token = Token::MEDIA_SYM;
            remains -= 5; str += 5; next_ch();
          }
          else if (strncmp((char *) str, "page", 4) == 0) {
            token = Token::PAGE_SYM;
            remains -= 4; str += 4; next_ch();
          }
          else if (strncmp((char *) str, "import", 6) == 0) {
            token = Token::IMPORT_SYM;
            remains -= 6; str += 6; next_ch();
          }
          else if (strncmp((char *) str, "charset ", 8) == 0) {
            token = Token::IMPORT_SYM;
            remains -= 8; str += 8; next_ch();
          }
          else if (strncmp((char *) str, "font-face ", 9) == 0) {
            token = Token::FONT_FACE_SYM;
            remains -= 9; str += 9; next_ch();
          }
          else {
            token = Token::ERROR;
          }
        }  
        else if (ch == '~') {
          next_ch();
          if (ch == '=') {
            next_ch();
            token = Token::INCLUDES;
          }
          else {
            token = Token::ERROR;
          }
        }     
        else if (ch == '|') {
          next_ch();
          if (ch == '=') {
            next_ch();
            token = Token::DASHMATCH;
          }
          else {
            token = Token::ERROR;
          }
        }
        else if ((ch == '/') && (str[0] == '*')) {
          next_ch(); next_ch();
          if (!skip_comment()) {
            token = Token::ERROR;
          }
          else continue;
        }
        else if (ch == '/') { next_ch(); token = Token::SLASH; }
        else if (ch == '+') { next_ch(); token = Token::PLUS;  }
        else if (ch == '!') {
          for (;;) {
            skip_spaces();
            if ((ch == '/') && (str[0] == '*')) {
              next_ch(); next_ch();
              if (!skip_comment()) {
                token = Token::ERROR;
                done = true;
                break;
              }
              else continue;
            }
            else break;
          }
          if (!done && (ch == 'i') && (strncmp((char *)str, "mportant", 8) == 0)) {
            remains -= 8; str += 8; next_ch();
            token = Token::IMPORTANT_SYM;
          }
        }
        else {
          token = Token::ERROR;
        }

        done = true;
      }
    }

    // ---- Parser ----

    void skip_blanks() {
      if (token == Token::END_OF_FILE) return;
      do next_token(); while (token == Token::S);
    }

    bool import_statement() {
      skip_blanks();
      if ((token == Token::URI) || (token == Token::STRING)) {
        // Import a css file
      }
      skip_blanks();
      if (token == Token::IDENT) {
        skip_blanks();
        while (token == Token::COMMA) {
          skip_blanks();
          if (token != Token::IDENT) return false;
          skip_blanks();
        }
      }
      if (token == Token::SEMICOLON) {
        skip_blanks();
      }
      else return false;
      return true;
    }

    bool function() {
      skip_blanks();
      expression();
      if (token == Token::RPARENT) skip_blanks();
      else return false;
      return true;
    }

    bool term(bool * none) {
      *none = false;
      if ((token == Token::PLUS) || (token == Token::MINUS)) {
        skip_blanks();
      }
      if ((token == Token::NUMBER    ) ||
          (token == Token::PERCENTAGE) ||
          (token == Token::LENGTH    ) ||
          (token == Token::EMS       ) ||
          (token == Token::EXS       ) ||
          (token == Token::ANGLE     ) ||
          (token == Token::TIME      ) ||
          (token == Token::FREQ)) {
        skip_blanks();
      }
      else if (token == Token::STRING) {
        skip_blanks();
      }
      else if (token == Token::IDENT) {
        skip_blanks();
      }
      else if (token == Token::URI) {
        skip_blanks();
      }
      else if (token == Token::FUNCTION) {
        if (!function()) return false;
      } else if (token == Token::HASH) {
        skip_blanks();
      }
      else *none = true;
      return true;
    }

    bool expression() {
      bool none;
      if (!term(&none)) return false;
      for (;;) {
        if ((token == Token::SLASH) || (token == Token::COMMA)) {
          skip_blanks();
        }
        if (!term(&none)) return false;
        if (none) break;
      }
      return true;
    }

    bool declaration() {
      // process IDENT property
      skip_blanks();
      if (token == Token::COLON) skip_blanks();
      else return false;
      if (!expression()) return false;
      if (token == Token::IMPORTANT_SYM) {
        skip_blanks();
      }
      return true;
    }

    bool page_statement() {
      skip_blanks();
      if (token == Token::COLON) {
        next_token();
        if (token == Token::IDENT) {
          skip_blanks();
        } else return false;
      }
      if (token == Token::LBRACE) {
        allow_minus = true;
        skip_blanks();
        allow_minus = false;
        if (token == Token::IDENT) {
          if (!declaration()) return false;
        }
        while (token == Token::SEMICOLON) {
          allow_minus = true;
          skip_blanks();
          allow_minus = false;
          if (token == Token::IDENT) {
            if (!declaration()) return false;
          }
        }
        if (token == Token::RBRACE) skip_blanks();
        else return false;

      } else return false;
      return true;
    }

    bool font_face_statement() {
      skip_blanks();
      if (token == Token::LBRACE) {
        allow_minus = true;
        skip_blanks();
        allow_minus = false;
        if (token == Token::IDENT) {
          if (!declaration()) return false;
        }
        while (token == Token::SEMICOLON) {
          allow_minus = true;
          skip_blanks();
          allow_minus = false;
          if (token == Token::IDENT) {
            if (!declaration()) return false;
          }
        }
        if (token == Token::RBRACE) skip_blanks();
        else return false;

      } else return false;
      return true;
    }
    
    bool attrib() {
      if (token != Token::IDENT) return false;
      skip_blanks();
      if ((token == Token::EQUAL   ) ||
          (token == Token::INCLUDES) ||
          (token == Token::DASHMATCH)) {
        if (token == Token::EQUAL) {

        }
        else if (token == Token::INCLUDES) {

        }
        else { // Token::DASHMATCH

        }
        skip_blanks();
        if (token == Token::STRING) {

        }
        else if (token == Token::IDENT) {

        }
        else return false;
        skip_blanks();
      }
      if (token != Token::RBRACK) return false;
      next_token();
      return true;
    }

    bool pseudo() {
      if (token == Token::IDENT) {
        next_token();
      }
      else if (token == Token::FUNCTION) {
        skip_blanks();
        if (token == Token::IDENT) {
          skip_blanks();
        }
        if (token != Token::RPARENT) return false;
        next_token();
      }
      return true;
    }

    bool sub_simple_selector() {
      for (;;) {
        if (token == Token::HASH) {
          next_token();
        }
        else if (token == Token::DOT) {
          next_token();
          if (token == Token::IDENT) {
            next_token();
          }
        }
        else if (token == Token::LBRACK) {
          skip_blanks();
          if (!attrib()) return false;
        }
        else if (token == Token::COLON) {
          next_token();
          if (!pseudo()) return false;
        }
        else break;
      }
      return true;
    }
    bool simple_selector() {
      if ((token == Token::IDENT) || (token == Token::STAR)) {
        next_token();
        if (!sub_simple_selector()) return false;
      }
      else if ((token == Token::HASH  ) ||
               (token == Token::DOT   ) ||
               (token == Token::LBRACK) ||
               (token == Token::COLON )) {
        if (!sub_simple_selector()) return false;
      }
      return true;
    }

    bool selector() {
      if (!simple_selector()) return false;
      if ((token == Token::PLUS) || (token == Token::GT)) {
        skip_blanks();
        if (!selector()) return false;
      }
      else if (token == Token::S) {
        skip_blanks();
        if ((token == Token::PLUS) || (token == Token::GT)) {
          skip_blanks();
        }
        if ((token == Token::IDENT ) ||
            (token == Token::STAR  ) ||
            (token == Token::HASH  ) ||
            (token == Token::DOT   ) ||
            (token == Token::LBRACK) ||
            (token == Token::COLON )) {
          if (!selector()) return false;
        }
      }
      return true;
    }

    bool ruleset() {
      if (!selector()) return false;
      while (token == Token::COMMA) {
        skip_blanks();
        if (!selector()) return false;
      }
      if (token == Token::LBRACE) {
        allow_minus = true;
        skip_blanks();
        allow_minus = false;
        if (token == Token::IDENT) {
          if (!declaration()) return false;
        }
        while (token == Token::SEMICOLON) {
          allow_minus = true;
          skip_blanks();
          allow_minus = false;
          if (token == Token::IDENT) {
            if (!declaration()) return false;
          }
        }
        if (token == Token::RBRACE) skip_blanks();
        else return false;
      }
      return true;
    }

    // https://www.w3.org/TR/mediaqueries-4/#typedef-media-query-list
    //
    // @media <media-query-list> {
    //   <group-rule-body>
    // } 
    //                <media-query> = <media-condition>
    //                                | [ not | only ]? <media-type> [ and <media-condition-without-or> ]?
    //                 <media-type> = <ident>
    //            <media-condition> = <media-not> 
    //                                | <media-in-parens> [ <media-and>* | <media-or>* ]
    // <media-condition-without-or> = <media-not> 
    //                                | <media-in-parens> <media-and>*
    //                  <media-not> = not <media-in-parens>
    //                  <media-and> = and <media-in-parens>
    //                   <media-or> = or  <media-in-parens>
    //            <media-in-parens> = ( <media-condition> ) 
    //                                | <media-feature> 
    //                                | <general-enclosed>
    //
    //              <media-feature> = ( [ <mf-plain> | <mf-boolean> | <mf-range> ] )
    //                   <mf-plain> = <mf-name> : <mf-value>
    //                 <mf-boolean> = <mf-name>
    //                   <mf-range> = <mf-name> <mf-comparison> <mf-value>
    //                                | <mf-value> <mf-comparison> <mf-name>
    //                                | <mf-value> <mf-lt> <mf-name> <mf-lt> <mf-value>
    //                                | <mf-value> <mf-gt> <mf-name> <mf-gt> <mf-value>
    //                    <mf-name> = <ident>
    //                   <mf-value> = <number> 
    //                                | <dimension> 
    //                                | <ident> 
    //                                | <ratio>
    //                      <mf-lt> = '<' '='?
    //                      <mf-gt> = '>' '='?
    //                      <mf-eq> = '='
    //              <mf-comparison> = <mf-lt> 
    //                                | <mf-gt> 
    //                                | <mf-eq>
    //
    //           <general-enclosed> = [ <function-token> <any-value> ) ] 
    //                                | ( <ident> <any-value> )

    bool mf_value() {
      if (token == Token::NUMBER) {
        skip_blanks();
        if (token == Token::SLASH) {
          // ratio
          skip_blanks();
          if (token != Token::NUMBER) return false;
          skip_blanks();
        }
      }
      else if (token == Token::DIMENSION) {
        skip_blanks();
      }
      else if (token == Token::IDENT) {
        skip_blanks();
      }
      else return false;

      return true;
    }

    bool is_logical() {
      return 
        (token == Token::GT) ||
        (token == Token::GE) ||
        (token == Token::LT) ||
        (token == Token::LE);
    }

    bool is_lower(Token token) {
      return 
        (token == Token::LT) ||
        (token == Token::LE);
    }

    bool is_greater(Token token) {
      return 
        (token == Token::GT) ||
        (token == Token::GE);
    }

    bool mf_range() {
      if (!mf_value()) return false;
      if (token == Token::EQUAL) {
        skip_blanks();
        if (!mf_value()) return false;
      }
      else if (is_logical()) {
        Token op1 = token;
        skip_blanks();
        if (!mf_value()) return false;
        if (is_logical()) {
          Token op2 = token;
          skip_blanks();
          if (!mf_value()) return false;
          if ((is_lower(op1) && is_greater(op2)) || 
              (is_greater(op1) && is_lower(op2))) {
            return false;
          }
        } 
      }
      else return false;
      return true;
    }

    bool media_in_parens() {
      if (token == Token::LPARENT) {
        skip_blanks();
        bool not_token_present = (token == Token::IDENT) && (strcmp((char *)ident, "not" ) == 0);
        if (not_token_present) skip_blanks();
        if ((token == Token::LPARENT) || (token == Token::FUNCTION)) {
          bool present;
          if (!media_condition(not_token_present, &present, true)) return false;
        }
        else { // media-feature
          if (token == Token::IDENT) {
            skip_blanks();
            if (token == Token::RPARENT) {
              // mf-boolean
            }
            else if (token == Token::COLON) {
              // mf-plain
              skip_blanks();
              if (!mf_value()) return false;
            }
            else if (mf_range()) {
            }
            else while ((token != Token::END_OF_FILE) && (token != Token::RPARENT)) {
              // any-values
              skip_blanks();
            }
          }
          else if (!mf_range()) return false;
        }
      }
      else { // Token::FUNCTION
        skip_blanks();
        while ((token != Token::END_OF_FILE) && (token != Token::RPARENT)) {
          // any-values
          skip_blanks();
        }
      }

      if (token == Token::RPARENT) skip_blanks();
      else return false;

      return true;
    }

    bool media_condition(bool not_token_present, bool * present, bool with_or) {
      *present = (token == Token::LPARENT) || (token == Token::FUNCTION);
      if (*present) {
        if (!media_in_parens()) return false;
        if ((token == Token::IDENT) && (strcmp((char *)ident, "and") == 0)) {
          while ((token == Token::IDENT) && (strcmp((char *)ident, "and") == 0)) {
            skip_blanks();
            if ((token != Token::LPARENT) && (token != Token::FUNCTION)) return false;
            if (!media_in_parens()) return false;
          }
        }
        else if (with_or && (token == Token::IDENT) && (strcmp((char *)ident, "or") == 0)) {
          while ((token == Token::IDENT) && (strcmp((char *)ident, "or") == 0)) {
            skip_blanks();
            if ((token != Token::LPARENT) && (token != Token::FUNCTION)) return false;
            if (!media_in_parens()) return false;
          }
        }
      }
      return true;
    }

    bool media_query(bool * query_present) {
      *query_present = false;

      bool media_type_present      = false;
      bool condition_present       = false;
      bool media_in_parens_present = false;
      bool not_token_present       = false;
      bool only_token_present      = false;

      if (token == Token::IDENT) {
        not_token_present  = strcmp((char *)ident, "not" ) == 0;
        only_token_present = strcmp((char *)ident, "only") == 0;

        if (not_token_present || only_token_present) skip_blanks();

        if (token == Token::IDENT) {
          // This is a media type
          media_type_present = true;
          skip_blanks();
          not_token_present = false;
          if ((token == Token::IDENT) && 
              (strcmp((char *)ident, "and") == 0)) {
            skip_blanks();
            if ((token == Token::IDENT) && 
                ((not_token_present = strcmp((char *)ident, "not") == 0))) {
              skip_blanks();
            }
            if (!media_condition(not_token_present, &condition_present, false)) return false;
            if (!condition_present) return false;
          }
        }
        else {
          if (only_token_present) return false; 
          if (!media_condition(not_token_present, &condition_present, true)) return false;
        }
      }
      else if (token == Token::LPARENT) {
        if (!media_in_parens()) return false;
        media_in_parens_present = true;
      }

      *query_present = media_type_present || condition_present || media_in_parens_present;
      return true;
    }

    bool media_statement() {
      skip_blanks();
      bool query_present;
      if (!media_query(&query_present)) return false;
      if (query_present) {
        while (token == Token::COMMA) {
          skip_blanks();
          if (!media_query(&query_present)) return false;
        }
      }
 
      if (token == Token::LBRACE) {
        skip_blanks();
        while (token != Token::RBRACE) {
          if (!ruleset()) return false;
          if (token == Token::ERROR) return false;
        }
        skip_blanks();
      } else return false;
      return true;
    }

  public:
    bool parse(uint8_t * buffer, int32_t size) {

      str     = buffer;
      remains = size;
      skip    = 0;

      next_ch();
      next_token();

      if (token == Token::CHARSET_SYM) {
        next_token();
        if (token == Token::STRING) {
          next_token();
          if (token == Token::SEMICOLON) next_token();
          else return false;
        } else return false;
      }

      while ((token == Token::S  ) ||
             (token == Token::CDO) ||
             (token == Token::CDC)) {
        if      (token == Token::CDO) skip++;
        else if (token == Token::CDC) skip--;
        next_token();
      }

      while (token == Token::IMPORT_SYM) {
        if (!import_statement()) return false;
        while ((token == Token::CDO) || 
               (token == Token::CDC)) {
          if      (token == Token::CDO) skip++;
          else if (token == Token::CDC) skip--;
          skip_blanks();
        }
      }

      bool done = false;

      while (!done) {
        if (token == Token::MEDIA_SYM) {
          if (!media_statement()) return false;
        }
        else if (token == Token::PAGE_SYM) {
          if (!page_statement()) return false;
        }
        else if (token == Token::FONT_FACE_SYM) {
          if (!font_face_statement()) return false;
        }
        else {
          if (!ruleset()) return false;
        }
        while ((token == Token::CDO) || 
               (token == Token::CDC)) {
          if      (token == Token::CDO) skip++;
          else if (token == Token::CDC) skip--;
          skip_blanks();
        }
        if (token == Token::END_OF_FILE) break;
      }
      return true;
    }
};

#if CSS_PARSER_TEST

#include <iostream>
#include <fstream>
#include <filesystem>

CSSParser css_parser;

bool do_file(const char * filename) {

  std::uintmax_t fsize;

  try {
    fsize = std::filesystem::file_size(filename);  
  }
  catch (const std::filesystem::filesystem_error & err) {
    std::cerr << "filesystem error! " << err.what() << '\n';
    if (!err.path1().empty())
      std::cerr << "path1: " << err.path1().string() << '\n';
    if (!err.path2().empty())
      std::cerr << "path2: " << err.path2().string() << '\n';
  }
  catch (const std::exception & ex) {
    std::cerr << "general exception: " << ex.what() << '\n';
  }

  std::ifstream file;

  file.open(filename, std::ifstream::in);
  if (!file.is_open()) {
    std::cerr << "Unable to open file " << filename << std::endl;
    return false;
  }
  else {
    uint8_t * buffer = new uint8_t[fsize+1];

    file.read((char *)buffer, fsize);
    file.close();
    if (css_parser.parse(buffer, fsize)) {
      std::cout << "Completed with success!" << std::endl;
      return true;
    }
    else {
      std::cout << "Completed with error." << std::endl;
      return false;
    }
  }
}

const int FILE_COUNT = 11;

const char * files[FILE_COUNT] = {
  "test/test1.css",
  "test/test2.css",
  "test/test3.css",
  "test/test4.css",
  "test/test5.css",
  "test/test6.css",
  "test/test7.css",
  "test/test8.css",
  "test/test9.css",
  "test/test10.css",
  "test/test11.css",
};

int main() {

  int count = 0;

  for (int i = 9; i <  FILE_COUNT; i++) {
    std::cout << "---- File: " << files[i] << " ----" << std::endl;
    if (do_file(files[i])) count++;
  }

  std::cout << "Test completed, success count: " << count << " out of " << FILE_COUNT << std::endl;

  return 0;
}
#endif