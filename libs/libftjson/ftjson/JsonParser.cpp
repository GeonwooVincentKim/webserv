/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   JsonParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hshimizu <hshimizu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/18 04:30:35 by hshimizu          #+#    #+#             */
/*   Updated: 2025/04/17 01:35:21 by hshimizu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ftjson/Json.hpp>
#include <ftjson/JsonError.hpp>
#include <ftjson/JsonParser.hpp>

#include <ftpp/any/Any.hpp>
#include <ftpp/string/string.hpp>
#include <ftpp/unicode/surrogate.hpp>
#include <ftpp/unicode/utf8.hpp>

#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <istream>
#include <sstream>

namespace ftjson {

JsonParser::JsonParser(std::istream &stream)
    : _lexer(stream), _current_state(STATE_VALUE) {
}

JsonParser::~JsonParser() {
}

ftpp::Any JsonParser::_parse() {
  assert(_tmp.empty());
  assert(_current_state == STATE_VALUE);
  for (; _current_state != STATE_END;)
    _transition(_lexer.nextToken());
  assert(_tmp.size() == 1);
  ftpp::Any result = _tmp.top();
  _tmp.pop();
  return result;
}

void JsonParser::_transition(JsonToken const &token) {
  try {
    switch (token.getType()) {
    case JsonToken::LEFT_BRACE:
      _case_left_brace();
      break;
    case JsonToken::LEFT_BRACKET:
      _case_left_bracket();
      break;
    case JsonToken::RIGHT_BRACE:
      _case_right_brace();
      break;
    case JsonToken::RIGHT_BRACKET:
      _case_right_bracket();
      break;
    case JsonToken::STRING:
      _case_string(token.getValue());
      break;
    case JsonToken::NUMBER:
      _case_number(token.getValue());
      break;
    case JsonToken::TRUE:
      _case_true();
      break;
    case JsonToken::FALSE:
      _case_false();
      break;
    case JsonToken::NULL_:
      _case_null();
      break;
    case JsonToken::COMMA:
      _case_comma();
      break;
    case JsonToken::COLON:
      _case_colon();
      break;
    case JsonToken::END:
      _case_end();
      break;
    default:
      throw JsonError("Unexpected token");
    }
  } catch (...) {
    _current_state = STATE_INVALID;
    throw;
  }
}

void JsonParser::_case_left_brace() {
  switch (_current_state) {
  case STATE_VALUE:
    _state.push(STATE_EOF);
    break;
  case STATE_OBJECT_VALUE:
    _state.push(STATE_OBJECT_NEXT_OR_END);
    break;
  case STATE_ARRAY_VALUE:
  case STATE_ARRAY_VALUE_OR_END:
    _state.push(STATE_ARRAY_NEXT_OR_END);
    break;
  default:
    throw JsonError("Unexpected token: {");
  }
  _current_state = STATE_OBJECT_KEY_OR_END;
  _tmp.push(Object());
}

void JsonParser::_case_left_bracket() {
  switch (_current_state) {
  case STATE_VALUE:
    _state.push(STATE_EOF);
    break;
  case STATE_OBJECT_VALUE:
    _state.push(STATE_OBJECT_NEXT_OR_END);
    break;
  case STATE_ARRAY_VALUE_OR_END:
  case STATE_ARRAY_VALUE:
    _state.push(STATE_ARRAY_NEXT_OR_END);
    break;
  default:
    throw JsonError("Unexpected token: [");
  }
  _current_state = STATE_ARRAY_VALUE_OR_END;
  _tmp.push(Array());
}

void JsonParser::_case_right_brace() {
  switch (_current_state) {
  case STATE_OBJECT_NEXT_OR_END:
    _insert_object();
    // fall through
  case STATE_OBJECT_KEY_OR_END:
    _current_state = _state.top();
    _state.pop();
    break;
  default:
    throw JsonError("Unexpected token: }");
  }
}

void JsonParser::_case_right_bracket() {
  switch (_current_state) {
  case STATE_ARRAY_NEXT_OR_END:
    _insert_array();
    // fall through
  case STATE_ARRAY_VALUE_OR_END:
    _current_state = _state.top();
    _state.pop();
    break;
  default:
    throw JsonError("Unexpected token: ]");
  }
}

void JsonParser::_case_comma() {
  switch (_current_state) {
  case STATE_OBJECT_NEXT_OR_END:
    _current_state = STATE_OBJECT_KEY;
    _insert_object();
    break;
  case STATE_ARRAY_NEXT_OR_END:
    _current_state = STATE_ARRAY_VALUE;
    _insert_array();
    break;
  default:
    throw JsonError("Unexpected token: ,");
  }
}

void JsonParser::_case_colon() {
  switch (_current_state) {
  case STATE_OBJECT_COLON:
    _current_state = STATE_OBJECT_VALUE;
    break;
  default:
    throw JsonError("Unexpected token: :");
  }
}

void JsonParser::_case_string(std::string const &value) {
  switch (_current_state) {
  case STATE_VALUE:
    _current_state = STATE_EOF;
    break;
  case STATE_OBJECT_VALUE:
    _current_state = STATE_OBJECT_NEXT_OR_END;
    break;
  case STATE_ARRAY_VALUE_OR_END:
  case STATE_ARRAY_VALUE:
    _current_state = STATE_ARRAY_NEXT_OR_END;
    break;
  case STATE_OBJECT_KEY_OR_END:
  case STATE_OBJECT_KEY:
    _current_state = STATE_OBJECT_COLON;
    break;
  default:
    throw JsonError("Unexpected token: string");
  }
  _tmp.push(_string_dequote(value));
}

void JsonParser::_case_number(std::string const &value) {
  switch (_current_state) {
  case STATE_VALUE:
    _current_state = STATE_EOF;
    break;
  case STATE_OBJECT_VALUE:
    _current_state = STATE_OBJECT_NEXT_OR_END;
    break;
  case STATE_ARRAY_VALUE_OR_END:
  case STATE_ARRAY_VALUE:
    _current_state = STATE_ARRAY_NEXT_OR_END;
    break;
  default:
    throw JsonError("Unexpected token: number");
  }
  _tmp.push(ftpp::stod(value));
}

void JsonParser::_case_true() {
  switch (_current_state) {
  case STATE_VALUE:
    _current_state = STATE_EOF;
    break;
  case STATE_OBJECT_VALUE:
    _current_state = STATE_OBJECT_NEXT_OR_END;
    break;
  case STATE_ARRAY_VALUE_OR_END:
  case STATE_ARRAY_VALUE:
    _current_state = STATE_ARRAY_NEXT_OR_END;
    break;
  default:
    throw JsonError("Unexpected token: true");
  }
  _tmp.push(true);
}

void JsonParser::_case_false() {
  switch (_current_state) {
  case STATE_VALUE:
    _current_state = STATE_EOF;
    break;
  case STATE_OBJECT_VALUE:
    _current_state = STATE_OBJECT_NEXT_OR_END;
    break;
  case STATE_ARRAY_VALUE_OR_END:
  case STATE_ARRAY_VALUE:
    _current_state = STATE_ARRAY_NEXT_OR_END;
    break;
  default:
    throw JsonError("Unexpected token: false");
  }
  _tmp.push(false);
}

void JsonParser::_case_null() {
  switch (_current_state) {
  case STATE_VALUE:
    _current_state = STATE_EOF;
    break;
  case STATE_OBJECT_VALUE:
    _current_state = STATE_OBJECT_NEXT_OR_END;
    break;
  case STATE_ARRAY_VALUE_OR_END:
  case STATE_ARRAY_VALUE:
    _current_state = STATE_ARRAY_NEXT_OR_END;
    break;
  default:
    throw JsonError("Unexpected token: null");
  }
  _tmp.push(ftpp::Any());
}

void JsonParser::_case_end() {
  switch (_current_state) {
  case STATE_EOF:
    _current_state = STATE_END;
    break;
  default:
    throw JsonError("Unexpected end");
  }
}

void JsonParser::_insert_object() {
  assert(3 <= _tmp.size());
  ftpp::Any value;
  value.swap(_tmp.top());
  _tmp.pop();
  assert(_tmp.top().isType<String>());
  std::string key;
  key.swap(_tmp.top().as_unsafe<String>());
  _tmp.pop();
  assert(_tmp.top().isType<Object>());
  Object &obj = _tmp.top().as_unsafe<Object>();
  obj[key].swap(value);
}

void JsonParser::_insert_array() {
  assert(2 <= _tmp.size());
  ftpp::Any value;
  value.swap(_tmp.top());
  _tmp.pop();
  assert(_tmp.top().isType<Array>());
  Array &arr = _tmp.top().as_unsafe<Array>();
  arr.push_back(value);
}

static inline void _unicode_escape(std::ostringstream &oss,
                                   std::string const &str, std::size_t &index) {
  ++index;
  bool valid = true;
  unsigned int code;
  code = ftpp::stoul(str.substr(index, 4), NULL, 16);
  index += 4;
  if (ftpp::ishghtsurrogate(code)) {
    if (str[index] == '\\' && str[index + 1] == 'u') {
      index += 2;
      unsigned int code2;
      code2 = ftpp::stoul(str.substr(index, 4), NULL, 16);
      index += 4;
      if (ftpp::islowsurrogate(code2))
        code = ftpp::surrogatepair(code, code2);
      else
        index -= 6, valid = false;
    } else
      valid = false;
  }
  if (valid)
    oss << ftpp::utf8_codepoint(code);
  else
    oss << "\\u" << std::nouppercase << std::setw(4) << std::setfill('0')
        << std::hex << code;
  --index;
}

std::string JsonParser::_string_dequote(std::string const &str) {
  std::ostringstream oss;
  std::size_t index = 0;
  assert(str[index] == '"');
  index++;
  for (; index < str.size() - 1; ++index) {
    if (str[index] == '\\') {
      ++index;
      assert(index < str.size() - 1);
      switch (str[index]) {
      case 'b':
        oss.put('\b');
        break;
      case 'f':
        oss.put('\f');
        break;
      case 'n':
        oss.put('\n');
        break;
      case 'r':
        oss.put('\r');
        break;
      case 't':
        oss.put('\t');
        break;
      case 'u':
        _unicode_escape(oss, str, index);
        break;
      default:
        oss.put(str[index]);
        break;
      }
    } else
      oss.put(str[index]);
  }
  assert(str[index] == '"');
  return oss.str();
}

ftpp::Any JsonParser::parse(char const *str) {
  std::istringstream iss(str);
  return JsonParser(iss)._parse();
}

ftpp::Any JsonParser::parse(std::istream &stream) {
  return JsonParser(stream)._parse();
}

} // namespace ftjson
