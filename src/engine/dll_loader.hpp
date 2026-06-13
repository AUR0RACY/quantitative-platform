#pragma once

#include "strategy_interface.hpp"
#include <string>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace qp::engine {

// ---- DLL loader/unloader for strategy DLLs ----
class DllLoader {
public:
    explicit DllLoader(const std::string& dll_path);
    ~DllLoader();

    // Non-copyable
    DllLoader(const DllLoader&) = delete;
    DllLoader& operator=(const DllLoader&) = delete;

    // Movable
    DllLoader(DllLoader&& other) noexcept;
    DllLoader& operator=(DllLoader&& other) noexcept;

    bool is_loaded() const { return m_handle != nullptr && m_strategy != nullptr; }
    IStrategy* strategy() { return m_strategy; }
    const std::string& path() const { return m_path; }

private:
    std::string  m_path;
#ifdef _WIN32
    HMODULE      m_handle = nullptr;
#else
    void*        m_handle = nullptr;
#endif
    IStrategy*   m_strategy = nullptr;
};

}  // namespace qp::engine
