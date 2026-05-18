#pragma once
#include <variant>
#include <string>
#include <vector>
#include <memory>

namespace vm {

struct Value;

using ArrayPtr = std::shared_ptr<std::vector<Value>>;

using ValueBase = std::variant<int, double, char, std::string, bool, ArrayPtr>;

struct Value : public ValueBase {
    using ValueBase::ValueBase;
};

} // namespace vm
