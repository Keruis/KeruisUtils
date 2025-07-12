#include "ClassRegistry.h"

ClassRegistry &ClassRegistry::instance() {
    static ClassRegistry instance;
    return instance;
}

void ClassRegistry::registerClass(const std::string &className, Creator creator) {
    m_creators[className] = std::move(creator);
}

std::shared_ptr<ScriptObject> ClassRegistry::create(const std::string &className) const {
    auto it = m_creators.find(className);
    if (it != m_creators.end()) {
        return it->second();
    }
    return nullptr;
}


