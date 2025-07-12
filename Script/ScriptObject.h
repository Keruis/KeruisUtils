#ifndef SCRIPTOBJECT_H
#define SCRIPTOBJECT_H

#include <string>
#include <any>
#include <vector>
#include <unordered_map>
#include <functional>
#include <stdexcept>

class ScriptObject {
public:
    using Method = std::function<std::any(const std::vector<std::any>&)>;

    virtual ~ScriptObject() = default;

    void registerMethod(const std::string& name, Method func) {
        methods[name] = std::move(func);
    }

    virtual std::any call(const std::string& name, const std::vector<std::any>& args) {
        auto it = methods.find(name);
        if (it != methods.end()) {
            return it->second(args);
        }
        throw std::runtime_error("Method not found: " + name);
    }

protected:
    std::unordered_map<std::string, Method> methods;
};

#endif // SCRIPTOBJECT_H