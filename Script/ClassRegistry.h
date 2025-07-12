#ifndef CLASSREGISTRY_H
#define CLASSREGISTRY_H

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

#include "ScriptObject.h"

class ClassRegistry {
public:
    using Creator = std::function<std::shared_ptr<ScriptObject>()>;

    static ClassRegistry& instance();

    void registerClass(const std::string& className, Creator creator);

    template <typename Ty_>
    void registerClass(const std::string& className) {
        registerClass(className, []() { return std::make_shared<Ty_>(); });
    }

    std::shared_ptr<ScriptObject> create(const std::string& className) const;

private:
    std::unordered_map<std::string, Creator> m_creators;
};

#define REGISTER_CLASS(NAME) \
    static bool _reg_##NAME = []{ \
        ClassRegistry::instance().registerClass<NAME>(#NAME); \
        return true; \
    }();

#endif //CLASSREGISTRY_H
