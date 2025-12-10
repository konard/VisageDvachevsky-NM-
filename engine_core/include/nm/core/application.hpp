#pragma once

#include "nm/core/types.hpp"
#include "nm/core/result.hpp"
#include "nm/core/timer.hpp"
#include "nm/platform/window.hpp"
#include <memory>
#include <string>

namespace nm::core
{

struct EngineConfig
{
    platform::WindowConfig window;
    std::string packFile;
    std::string startScene;
    bool debug = false;
};

class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    Result<void> initialize(const EngineConfig& config);
    void shutdown();

    void run();
    void quit();

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] f64 getDeltaTime() const;
    [[nodiscard]] f64 getElapsedTime() const;

    [[nodiscard]] platform::IWindow* getWindow();
    [[nodiscard]] const platform::IWindow* getWindow() const;

protected:
    virtual void onInitialize();
    virtual void onShutdown();
    virtual void onUpdate(f64 deltaTime);
    virtual void onRender();

private:
    void mainLoop();

    bool m_running;
    EngineConfig m_config;

    std::unique_ptr<platform::IWindow> m_window;
    Timer m_timer;
};

} // namespace nm::core
