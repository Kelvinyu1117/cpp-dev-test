#pragma once
#include "flatbuffers/minireflect.h"
#include "flatbuffers/reflection.h"
#include "flatbuffers/reflection_generated.h"
#include "flatbuffers/table.h"
#include "flatbuffers/verifier.h"
#include <concepts>
#include <exception>
#include <fstream>
#include <iostream>
#include <messaging/flatbuffers/messages/autogen/PropertyTree_generated.h>
#include <messaging/mock/PropTree.hpp>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {
using namespace messaging;

inline void
createPropTree(flatbuffers::FlatBufferBuilder &builder,
               std::vector<flatbuffers::Offset<Property::Prop>> &result,
               std::vector<mock::Prop> props) {

  if (props.empty()) {
    return;
  }

  for (auto &&prop : props) {
    std::vector<flatbuffers::Offset<Property::Prop>> tmp;
    createPropTree(builder, tmp, prop.sub_properties);
    Property::PropType prop_type;
    switch (prop.propType) {
    case mock::PropType::INT:
      prop_type = Property::PropType_INT;
      break;
    case mock::PropType::DOUBLE:
      prop_type = Property::PropType_DOUBLE;
      break;
    default:
      prop_type = Property::PropType_STRING;
      break;
    }

    result.emplace_back(CreatePropDirect(builder, prop_type, prop.name.c_str(),
                                         prop.value.c_str(), &tmp));
  }
}
} // namespace
namespace messaging::fbs::codec {

enum class CodecRole { ENCODER, DECODER, CODEC };

template <CodecRole TRole> class PropertyTreeCodec {
  static constexpr std::string_view ROOT_TABLE_NAME = "Property.PropTree";

public:
  PropertyTreeCodec() requires(TRole == CodecRole::ENCODER ||
                               TRole == CodecRole::CODEC) = default;

  PropertyTreeCodec(const char *schema_file) requires(TRole ==
                                                          CodecRole::DECODER ||
                                                      TRole == CodecRole::CODEC)
      : encoderBuilder(1024) {
    schema = read_binary_schema_file(schema_file);

    if (!schema) {
      throw std::invalid_argument("Flatbuffer schema of property tree cannot "
                                  "be created: schema file in not valid.");
    }

    if (ROOT_TABLE_NAME != schema->root_table()->name()->c_str()) {
      throw std::invalid_argument("Flatbuffer schema of property tree is "
                                  "invalid: Root table name does not match.");
    }
  }

  flatbuffers::span<uint8_t>
  encode(messaging::mock::PropTree prop_tree) requires(
      TRole == CodecRole::ENCODER || TRole == CodecRole ::CODEC) {

    std::vector<flatbuffers::Offset<Property::Prop>> properties;
    createPropTree(encoderBuilder, properties, prop_tree.properties);
    auto result = Property::CreatePropTreeDirect(encoderBuilder, &properties);
    encoderBuilder.Finish(result);

    return encoderBuilder.GetBufferSpan();
  }

  bool decode(flatbuffers::span<uint8_t> buffer,
              flatbuffers::Table *&table) requires(TRole ==
                                                       CodecRole::DECODER ||
                                                   TRole == CodecRole::CODEC) {
    table = flatbuffers::GetAnyRoot(buffer.data());
    return table != nullptr;
  }

  const reflection::Object *get_reflection_root_table() {
    return schema->root_table();
  }

  void clear() { encoderBuilder.Clear(); }

private:
  const reflection::Schema *read_binary_schema_file(const char *schema_file) {
    if (std::ifstream file{schema_file, std::ios::binary}) {

      std::stringstream ss;
      ss << file.rdbuf();
      auto bfbsfile = ss.str();

      flatbuffers::Verifier verifier(
          reinterpret_cast<const uint8_t *>(bfbsfile.c_str()),
          bfbsfile.length());

      if (!reflection::VerifySchemaBuffer(verifier)) {
        return nullptr;
      }

      return reflection::GetSchema(bfbsfile.c_str());

    } else {
      return nullptr;
    }
  }

private:
  const reflection::Schema *schema;
  flatbuffers::FlatBufferBuilder encoderBuilder;
};
} // namespace messaging::fbs::codec
