#include "flatbuffers/buffer.h"
#include "flatbuffers/string.h"
#include "flatbuffers/vector.h"
#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/reflection.h>
#include <flatbuffers/reflection_generated.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <messaging/fbs/message/autogen/PropertyTree_generated.h>
#include <sstream>

void print_space_before(size_t cnt) {
  for (size_t i = 0; i < cnt * 4; i++) {
    std::cout << " ";
  }
}

void print_sub_prop(
    const flatbuffers::Vector<::flatbuffers::Offset<Property::Prop>> *props_ptr,
    size_t cnt = 1) {

  using namespace Property;
  if (!props_ptr)
    return;

  print_space_before(cnt);
  std::cout << "{\n";
  for (auto it = props_ptr->begin(); it != props_ptr->end(); it++) {

    print_space_before(cnt + 1);
    std::cout << "name: " << flatbuffers::GetString(it->name()) << '\n';
    print_space_before(cnt + 1);
    std::cout << "name: " << flatbuffers::GetString(it->value()) << '\n';
    print_space_before(cnt + 1);
    std::cout << "type: " << EnumNamePropType(it->type()) << '\n';
    // print_sub_prop(it->sub_properties());
    print_sub_prop(it->sub_properties(), cnt + 1);
  }
  print_space_before(cnt);
  std::cout << "}\n";
}

void print_prop_tree(const Property::PropTree *prop_tree) {
  using namespace Property;
  if (!prop_tree)
    return;

  print_sub_prop(prop_tree->properties(), 1);
}

void create_prop_tree() {
  using namespace Property;
  flatbuffers::FlatBufferBuilder builder(1024);

  auto prop_type = PropType::PropType_INT;
  auto prop_name = builder.CreateString("size");
  auto prop_value = builder.CreateString("10");

  auto sub_prop_type = PropType::PropType_DOUBLE;
  auto sub_prop_name = builder.CreateString("size");
  auto sub_prop_value = builder.CreateString("10");

  std::vector<flatbuffers::Offset<Prop>> props;
  {
    std::vector<flatbuffers::Offset<Prop>> sub_props;
    sub_props.emplace_back(
        CreateProp(builder, sub_prop_type, sub_prop_name, sub_prop_value));

    auto prop = CreateProp(builder, prop_type, prop_name, prop_value,
                           builder.CreateVector(sub_props));
    props.emplace_back(prop);
  }

  auto prop_tree = CreatePropTree(builder, builder.CreateVector(props));

  builder.Finish(prop_tree);
  auto buffer = builder.GetBufferPointer();
  print_prop_tree(GetPropTree(buffer));
}

void readPropTreeReflection(const char *filename) {
  if (std::ifstream file{filename, std::ios::binary}) {
    // Read the binary data into a string.
    [[maybe_unused]] auto tmp = [&file]() {
      std::string bfbsfile;
      std::stringstream buffer;
      buffer << file.rdbuf();
      bfbsfile = buffer.str();
      std::cout << bfbsfile << std::endl;
      const reflection::Schema &schema =
          *reflection::GetSchema(bfbsfile.c_str());
    };

  } else {
    std::cout << "file " << filename << " is not ready.";
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  create_prop_tree();
  return 0;
}
