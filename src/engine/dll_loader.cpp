#include "dll_loader.hpp"
#include <stdexcept>

namespace qp::engine {

DllLoader::DllLoader(const std::string& dll_path)
    : m_path(dll_path)
{
#ifdef _WIN32
    m_handle = LoadLibraryA(dll_path.c_str());
    if (!m_handle) {
        throw std::runtime_error("Failed to load DLL: " + dll_path);
    }

    auto* create_fn = reinterpret_cast<CreateStrategyFn>(
        GetProcAddress(m_handle, "create_strategy"));
    if (!create_fn) {
        FreeLibrary(m_handle);
        m_handle = nullptr;
        throw std::runtime_error("DLL missing 'create_strategy' export: " + dll_path);
    }

    m_strategy = create_fn();
    if (!m_strategy) {
        FreeLibrary(m_handle);
        m_handle = nullptr;
        throw std::runtime_error("create_strategy() returned null for: " + dll_path);
    }
#else
    m_handle = dlopen(dll_path.c_str(), RTLD_NOW);
    if (!m_handle) {
        throw std::runtime_error(std::string("Failed to load shared library: ") + dlerror());
    }
    auto* create_fn = reinterpret_cast<CreateStrategyFn>(dlsym(m_handle, "create_strategy"));
    if (!create_fn) {
        dlclose(m_handle);
        m_handle = nullptr;
        throw std::runtime_error("Shared library missing 'create_strategy'");
    }
    m_strategy = create_fn();
#endif
}

DllLoader::~DllLoader() {
    if (m_handle) {
        if (m_strategy) {
#ifdef _WIN32
            auto* destroy_fn = reinterpret_cast<DestroyStrategyFn>(
                GetProcAddress(m_handle, "destroy_strategy"));
#else
            auto* destroy_fn = reinterpret_cast<DestroyStrategyFn>(
                dlsym(m_handle, "destroy_strategy"));
#endif
            if (destroy_fn) {
                destroy_fn(m_strategy);
            }
        }
#ifdef _WIN32
        FreeLibrary(m_handle);
#else
        dlclose(m_handle);
#endif
    }
}

DllLoader::DllLoader(DllLoader&& other) noexcept
    : m_path(std::move(other.m_path))
    , m_handle(other.m_handle)
    , m_strategy(other.m_strategy)
{
    other.m_handle   = nullptr;
    other.m_strategy = nullptr;
}

DllLoader& DllLoader::operator=(DllLoader&& other) noexcept {
    if (this != &other) {
        this->~DllLoader();
        m_path     = std::move(other.m_path);
        m_handle   = other.m_handle;
        m_strategy = other.m_strategy;
        other.m_handle   = nullptr;
        other.m_strategy = nullptr;
    }
    return *this;
}

}  // namespace qp::engine
