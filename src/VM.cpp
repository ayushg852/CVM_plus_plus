#include "VM.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace vm {

void printValue(const Value& v) {
    if (std::holds_alternative<int>(v)) std::cout << std::get<int>(v);
    else if (std::holds_alternative<double>(v)) std::cout << std::get<double>(v);
    else if (std::holds_alternative<char>(v)) std::cout << std::get<char>(v);
    else if (std::holds_alternative<std::string>(v)) std::cout << std::get<std::string>(v);
    else if (std::holds_alternative<bool>(v)) std::cout << (std::get<bool>(v) ? "true" : "false");
    else if (std::holds_alternative<ArrayPtr>(v)) {
        std::cout << "[";
        const auto& arr = *std::get<ArrayPtr>(v);
        for (size_t i = 0; i < arr.size(); ++i) {
            printValue(arr[i]);
            if (i < arr.size() - 1) std::cout << ", ";
        }
        std::cout << "]";
    }
}

VM::VM() {
    globals.resize(256);
}

InterpretResult VM::interpret(const Chunk& mainChunk, const std::map<std::string, Function>& funcs) {
    functions = funcs;
    
    Function mainFunc;
    mainFunc.name = "main";
    mainFunc.chunk = mainChunk;
    mainFunc.arity = 0;
    
    functions["__main__"] = mainFunc;
    frames.clear();
    stack.clear();
    frames.push_back({&functions["__main__"], 0, 0});
    
    return run();
}

uint8_t VM::readByte() {
    return frames.back().function->chunk.code[frames.back().ip++];
}

Value VM::readConstant() {
    return frames.back().function->chunk.constants[readByte()];
}

uint16_t VM::readShort() {
    frames.back().ip += 2;
    return (uint16_t)((frames.back().function->chunk.code[frames.back().ip - 2] << 8) | 
                      frames.back().function->chunk.code[frames.back().ip - 1]);
}

void VM::push(Value value) {
    stack.push_back(std::move(value));
}

Value VM::pop() {
    Value v = std::move(stack.back());
    stack.pop_back();
    return v;
}

Value VM::peek(int distance) {
    return stack[stack.size() - 1 - distance];
}

InterpretResult VM::run() {
    while (!frames.empty()) {
        OpCode instruction = static_cast<OpCode>(readByte());
        switch (instruction) {
            case OpCode::PUSH_CONSTANT: push(readConstant()); break;
            case OpCode::POP: pop(); break;
            case OpCode::LOAD_VAR: {
                uint8_t index = readByte();
                push(stack[frames.back().stackStart + index]);
                break;
            }
            case OpCode::STORE_VAR: {
                uint8_t index = readByte();
                int target = frames.back().stackStart + index;
                while (stack.size() <= (size_t)target) stack.push_back(0);
                stack[target] = peek(0);
                break;
            }
            case OpCode::ADD: {
                Value b = pop(); Value a = pop();
                if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) push(std::get<int>(a) + std::get<int>(b));
                else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) push(std::get<double>(a) + std::get<double>(b));
                else if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b)) push(std::get<std::string>(a) + std::get<std::string>(b));
                else if (std::holds_alternative<std::string>(a) && std::holds_alternative<char>(b)) push(std::get<std::string>(a) + std::get<char>(b));
                else if (std::holds_alternative<char>(a) && std::holds_alternative<std::string>(b)) push(std::get<char>(a) + std::get<std::string>(b));
                else { runtimeError("Invalid add operands."); return InterpretResult::RUNTIME_ERROR; }
                break;
            }
            case OpCode::SUBTRACT: {
                Value b = pop(); Value a = pop();
                if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) push(std::get<int>(a) - std::get<int>(b));
                else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) push(std::get<double>(a) - std::get<double>(b));
                else { runtimeError("Invalid sub operands."); return InterpretResult::RUNTIME_ERROR; }
                break;
            }
            case OpCode::MULTIPLY: {
                Value b = pop(); Value a = pop();
                if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) push(std::get<int>(a) * std::get<int>(b));
                else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) push(std::get<double>(a) * std::get<double>(b));
                else { runtimeError("Invalid mul operands."); return InterpretResult::RUNTIME_ERROR; }
                break;
            }
            case OpCode::DIVIDE: {
                Value b = pop(); Value a = pop();
                if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) push(std::get<int>(a) / std::get<int>(b));
                else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) push(std::get<double>(a) / std::get<double>(b));
                else { runtimeError("Invalid div operands."); return InterpretResult::RUNTIME_ERROR; }
                break;
            }
            case OpCode::MODULO: {
                Value b = pop(); Value a = pop();
                if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) push(std::get<int>(a) % std::get<int>(b));
                else { runtimeError("Modulo only supported for integers."); return InterpretResult::RUNTIME_ERROR; }
                break;
            }
            case OpCode::POWER: {
                Value b = pop(); Value a = pop();
                double base = std::holds_alternative<int>(a) ? (double)std::get<int>(a) : std::get<double>(a);
                double exp = std::holds_alternative<int>(b) ? (double)std::get<int>(b) : std::get<double>(b);
                push(std::pow(base, exp));
                break;
            }
            case OpCode::EQUAL: { Value b = pop(); Value a = pop(); push(a == b); break; }
            case OpCode::NOT_EQUAL: { Value b = pop(); Value a = pop(); push(!(a == b)); break; }
            case OpCode::LESS: {
                Value b = pop(); Value a = pop();
                if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) push(std::get<int>(a) < std::get<int>(b));
                else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) push(std::get<double>(a) < std::get<double>(b));
                else { runtimeError("Invalid < operands."); return InterpretResult::RUNTIME_ERROR; }
                break;
            }
            case OpCode::LESS_EQUAL: {
                Value b = pop(); Value a = pop();
                if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) push(std::get<int>(a) <= std::get<int>(b));
                else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) push(std::get<double>(a) <= std::get<double>(b));
                else { runtimeError("Invalid <= operands."); return InterpretResult::RUNTIME_ERROR; }
                break;
            }
            case OpCode::GREATER: {
                Value b = pop(); Value a = pop();
                if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) push(std::get<int>(a) > std::get<int>(b));
                else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) push(std::get<double>(a) > std::get<double>(b));
                else { runtimeError("Invalid > operands."); return InterpretResult::RUNTIME_ERROR; }
                break;
            }
            case OpCode::GREATER_EQUAL: {
                Value b = pop(); Value a = pop();
                if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) push(std::get<int>(a) >= std::get<int>(b));
                else if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) push(std::get<double>(a) >= std::get<double>(b));
                else { runtimeError("Invalid >= operands."); return InterpretResult::RUNTIME_ERROR; }
                break;
            }
            case OpCode::AND: { Value b = pop(); Value a = pop(); push(std::get<bool>(a) && std::get<bool>(b)); break; }
            case OpCode::OR: { Value b = pop(); Value a = pop(); push(std::get<bool>(a) || std::get<bool>(b)); break; }
            case OpCode::NOT: { Value a = pop(); push(!std::get<bool>(a)); break; }
            case OpCode::JUMP: { uint16_t offset = readShort(); frames.back().ip += offset; break; }
            case OpCode::JUMP_IF_FALSE: {
                uint16_t offset = readShort(); Value cond = pop();
                if (std::holds_alternative<bool>(cond) && !std::get<bool>(cond)) frames.back().ip += offset;
                break;
            }
            case OpCode::LOOP: { uint16_t offset = readShort(); frames.back().ip -= offset; break; }
            case OpCode::CALL: {
                Value nameVal = readConstant();
                std::string name = std::get<std::string>(nameVal);
                if (functions.find(name) == functions.end()) { runtimeError("Undefined function '" + name + "'."); return InterpretResult::RUNTIME_ERROR; }
                const Function& func = functions[name];
                frames.push_back({&func, 0, (int)stack.size() - func.arity});
                break;
            }
            case OpCode::RETURN: {
                Value result = pop();
                int stackStart = frames.back().stackStart;
                frames.pop_back();
                while ((int)stack.size() > stackStart) stack.pop_back();
                push(result);
                if (frames.empty()) return InterpretResult::OK;
                break;
            }
            case OpCode::BUILD_ARRAY: {
                uint8_t count = readByte();
                auto elements = std::make_shared<std::vector<Value>>();
                elements->reserve(count);
                for (int i = 0; i < count; ++i) {
                    elements->push_back(pop());
                }
                std::reverse(elements->begin(), elements->end());
                push(Value(elements));
                break;
            }
            case OpCode::INDEX_GET: {
                Value indexVal = pop();
                Value arrayVal = pop();
                int index = std::get<int>(indexVal);
                if (std::holds_alternative<ArrayPtr>(arrayVal)) {
                    const auto& arrayPtr = std::get<ArrayPtr>(arrayVal);
                    push(arrayPtr->at(index));
                } else if (std::holds_alternative<std::string>(arrayVal)) {
                    const auto& str = std::get<std::string>(arrayVal);
                    push(str.at(index));
                } else {
                    runtimeError("Can only index arrays and strings.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::INDEX_SET: {
                Value value = pop();
                Value indexVal = pop();
                Value arrayVal = pop();
                int index = std::get<int>(indexVal);
                if (std::holds_alternative<ArrayPtr>(arrayVal)) {
                    auto& arrayPtr = std::get<ArrayPtr>(arrayVal);
                    arrayPtr->at(index) = value;
                } else if (std::holds_alternative<std::string>(arrayVal)) {
                    if (std::holds_alternative<char>(value)) {
                        auto& str = std::get<std::string>(arrayVal);
                        str.at(index) = std::get<char>(value);
                    }
                }
                push(value); 
                break;
            }
            case OpCode::ARRAY_SIZE: {
                Value arrayVal = pop();
                if (std::holds_alternative<ArrayPtr>(arrayVal)) {
                    const auto& arrayPtr = std::get<ArrayPtr>(arrayVal);
                    push((int)arrayPtr->size());
                } else if (std::holds_alternative<std::string>(arrayVal)) {
                    const auto& str = std::get<std::string>(arrayVal);
                    push((int)str.length());
                } else {
                    runtimeError("Size can only be called on arrays and strings.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::NEW_ARRAY: {
                Value sizeVal = pop();
                int size = std::get<int>(sizeVal);
                auto elements = std::make_shared<std::vector<Value>>(size, 0);
                push(Value(elements));
                break;
            }
            case OpCode::DISPLAY: {
                Value v = pop();
                printValue(v);
                break;
            }
            case OpCode::DISPLAYLN: {
                Value v = pop();
                printValue(v);
                std::cout << std::endl;
                break;
            }
            case OpCode::ENTER: { std::string input; std::getline(std::cin, input); try { push(std::stoi(input)); } catch(...) { push(input); } break; }
            case OpCode::HALT: return InterpretResult::OK;
            default: runtimeError("Unknown opcode."); return InterpretResult::RUNTIME_ERROR;
        }
    }
    return InterpretResult::OK;
}

void VM::runtimeError(const std::string& message) {
    std::cerr << "Runtime Error: " << message << std::endl;
}

} // namespace vm
