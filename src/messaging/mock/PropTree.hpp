#pragma once
#include <string>
#include <vector>
namespace messaging::mock {

enum class PropType : uint8_t {
  INT = 0,
  DOUBLE = 2,
  STRING = 4,
};

struct Prop {
  PropType propType;
  std::string name;
  std::string value;
  std::vector<Prop> sub_properties;
};

struct PropTree {
  std::vector<Prop> properties;
};
} // namespace messaging::mock
