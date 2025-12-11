/**
 * @file build_settings_panel.cpp
 * @brief Build Settings Panel implementation with presets, size estimation, and build logs
 */

#include "NovelMind/editor/build_settings_panel.hpp"
#include "NovelMind/editor/imgui_integration.hpp"
#include <cstring>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace NovelMind::editor {

BuildSettingsPanel::BuildSettingsPanel()
    : GUIPanelBase("Build Settings")
{
}

// =========================================================================
// Presets
// =========================================================================

void BuildSettingsPanel::loadPreset(const std::string& name)
{
    for (const auto& preset : m_presets)
    {
        if (preset.name == name)
        {
            m_settings.platform = preset.platform;
            m_settings.config = preset.config;
            m_currentPreset = name;
            m_sizeEstimationDirty = true;
            break;
        }
    }
}

void BuildSettingsPanel::savePreset(const std::string& name, const std::string& description)
{
    // Check if preset already exists
    for (auto& preset : m_presets)
    {
        if (preset.name == name)
        {
            preset.platform = m_settings.platform;
            preset.config = m_settings.config;
            preset.description = description;
            return;
        }
    }

    // Create new preset
    BuildPreset preset;
    preset.name = name;
    preset.description = description;
    preset.platform = m_settings.platform;
    preset.config = m_settings.config;
    preset.isBuiltIn = false;
    m_presets.push_back(preset);
}

void BuildSettingsPanel::deletePreset(const std::string& name)
{
    m_presets.erase(
        std::remove_if(m_presets.begin(), m_presets.end(),
            [&name](const BuildPreset& p) { return p.name == name && !p.isBuiltIn; }),
        m_presets.end());
}

// =========================================================================
// Build Control
// =========================================================================

void BuildSettingsPanel::startBuild()
{
    if (m_isBuilding)
    {
        return;
    }

    m_isBuilding = true;
    m_buildCancelled = false;
    m_buildProgress = 0.0f;
    m_buildStatus = "Starting build...";
    m_buildStartTime = 0.0; // Would use actual time

    // Initialize build steps
    m_buildSteps.clear();
    m_buildSteps.push_back({"Validating scripts", 0.1f, false, false, ""});
    m_buildSteps.push_back({"Compiling story graph", 0.15f, false, false, ""});
    m_buildSteps.push_back({"Processing images", 0.25f, false, false, ""});
    m_buildSteps.push_back({"Processing audio", 0.2f, false, false, ""});
    m_buildSteps.push_back({"Compiling localization", 0.1f, false, false, ""});
    m_buildSteps.push_back({"Packaging", 0.15f, false, false, ""});
    m_buildSteps.push_back({"Finalizing", 0.05f, false, false, ""});

    logBuildMessage(BuildLogEntry::Level::Info, "Build started for " +
        std::string(getPlatformName(m_settings.platform)) + " (" +
        std::string(getConfigName(m_settings.config)) + ")");

    if (m_onBuildStart)
    {
        m_onBuildStart();
    }
}

void BuildSettingsPanel::cancelBuild()
{
    if (m_isBuilding)
    {
        m_buildCancelled = true;
        m_buildStatus = "Cancelling build...";
        logBuildMessage(BuildLogEntry::Level::Warning, "Build cancelled by user");
    }
}

void BuildSettingsPanel::clearBuildLog()
{
    m_buildLog.clear();
}

// =========================================================================
// Size Estimation
// =========================================================================

void BuildSettingsPanel::estimateBuildSize()
{
    // Simulate size estimation based on settings
    m_sizeBreakdown.clear();

    // Base size estimates (would be calculated from actual assets)
    u64 scriptsSize = 50 * 1024;  // 50 KB base
    u64 imagesSize = 100 * 1024 * 1024;  // 100 MB base
    u64 audioSize = 50 * 1024 * 1024;  // 50 MB base
    u64 localizationSize = 500 * 1024;  // 500 KB per language

    // Apply compression settings
    if (m_settings.compressAssets)
    {
        switch (m_settings.textureCompression)
        {
            case TextureCompression::BC1_DXT1:
                imagesSize = static_cast<u64>(static_cast<double>(imagesSize) * 0.25);
                break;
            case TextureCompression::BC3_DXT5:
            case TextureCompression::ETC2:
                imagesSize = static_cast<u64>(static_cast<double>(imagesSize) * 0.5);
                break;
            case TextureCompression::BC7:
            case TextureCompression::ASTC:
                imagesSize = static_cast<u64>(static_cast<double>(imagesSize) * 0.4);
                break;
            case TextureCompression::WebP:
                imagesSize = static_cast<u64>(static_cast<double>(imagesSize) * 0.3);
                break;
            default:
                break;
        }

        switch (m_settings.audioCompression)
        {
            case AudioCompression::MP3:
            case AudioCompression::OGG:
                audioSize = static_cast<u64>(static_cast<double>(audioSize) * 0.1);
                break;
            case AudioCompression::AAC:
            case AudioCompression::Opus:
                audioSize = static_cast<u64>(static_cast<double>(audioSize) * 0.08);
                break;
            default:
                break;
        }
    }

    // Scripts compression
    if (m_settings.compressScripts)
    {
        scriptsSize = static_cast<u64>(static_cast<double>(scriptsSize) * 0.5);
    }

    // Calculate localization size
    localizationSize *= m_settings.languages.size();

    // Runtime size varies by platform
    u64 runtimeSize = 0;
    switch (m_settings.platform)
    {
        case BuildPlatform::Windows:
            runtimeSize = 15 * 1024 * 1024;  // 15 MB
            break;
        case BuildPlatform::macOS:
            runtimeSize = 20 * 1024 * 1024;  // 20 MB
            break;
        case BuildPlatform::Linux:
            runtimeSize = 12 * 1024 * 1024;  // 12 MB
            break;
        case BuildPlatform::Web:
            runtimeSize = 5 * 1024 * 1024;  // 5 MB
            break;
        case BuildPlatform::Android:
            runtimeSize = 8 * 1024 * 1024;  // 8 MB
            break;
        case BuildPlatform::iOS:
            runtimeSize = 10 * 1024 * 1024;  // 10 MB
            break;
    }

    // Store breakdown
    m_sizeBreakdown["Runtime"] = runtimeSize;
    m_sizeBreakdown["Scripts"] = scriptsSize;
    m_sizeBreakdown["Images"] = imagesSize;
    m_sizeBreakdown["Audio"] = audioSize;
    m_sizeBreakdown["Localization"] = localizationSize;

    // Calculate total
    m_estimatedSize = runtimeSize + scriptsSize + imagesSize + audioSize + localizationSize;
    m_sizeEstimationDirty = false;
}

// =========================================================================
// Menu/Toolbar Items
// =========================================================================

std::vector<ToolbarItem> BuildSettingsPanel::getToolbarItems() const
{
    std::vector<ToolbarItem> items;

    items.push_back({"Build", "Build Project", [this]() { const_cast<BuildSettingsPanel*>(this)->startBuild(); },
                     [this]() { return !m_isBuilding; }});
    items.push_back({"Build & Run", "Build and Run Project", []() { /* Build and run */ },
                     [this]() { return !m_isBuilding; }});

    if (m_isBuilding)
    {
        items.push_back({"Cancel", "Cancel Build", [this]() { const_cast<BuildSettingsPanel*>(this)->cancelBuild(); }});
    }

    return items;
}

std::vector<MenuItem> BuildSettingsPanel::getMenuItems() const
{
    std::vector<MenuItem> items;

    MenuItem buildMenu;
    buildMenu.label = "Build";
    buildMenu.subItems = {
        {"Build", "Ctrl+B", [this]() { const_cast<BuildSettingsPanel*>(this)->startBuild(); },
            [this]() { return !m_isBuilding; }},
        {"Build & Run", "Ctrl+F5", []() { /* Build and run */ },
            [this]() { return !m_isBuilding; }},
        MenuItem::separator(),
        {"Clean Build", "", []() { /* Clean build */ },
            [this]() { return !m_isBuilding; }},
    };
    items.push_back(buildMenu);

    // Presets submenu
    MenuItem presetsMenu;
    presetsMenu.label = "Presets";
    for (const auto& preset : m_presets)
    {
        std::string name = preset.name;
        presetsMenu.subItems.push_back({name, "",
            [this, name]() { const_cast<BuildSettingsPanel*>(this)->loadPreset(name); }});
    }
    if (!m_presets.empty())
    {
        presetsMenu.subItems.push_back(MenuItem::separator());
    }
    presetsMenu.subItems.push_back({"Save Current as Preset...", "", []() { /* Save dialog */ }});
    items.push_back(presetsMenu);

    return items;
}

void BuildSettingsPanel::onInitialize()
{
    initializeDefaultPresets();

    // Load saved build settings
    // Would load from project settings file
}

void BuildSettingsPanel::onUpdate(f64 deltaTime)
{
    // Simulate build progress
    if (m_isBuilding && !m_buildCancelled)
    {
        m_buildProgress += static_cast<f32>(deltaTime) * 0.1f;

        // Update build steps
        f32 progressThreshold = 0.0f;
        for (auto& step : m_buildSteps)
        {
            if (m_buildProgress > progressThreshold && !step.completed)
            {
                if (!step.active)
                {
                    step.active = true;
                    step.status = "In progress...";
                    m_buildStatus = step.name + "...";
                    logBuildMessage(BuildLogEntry::Level::Info, "Starting: " + step.name);
                }

                if (m_buildProgress >= progressThreshold + step.weight)
                {
                    step.active = false;
                    step.completed = true;
                    step.status = "Done";
                    logBuildMessage(BuildLogEntry::Level::Info, "Completed: " + step.name);
                }
            }
            progressThreshold += step.weight;
        }

        // Report progress
        if (m_onBuildProgress)
        {
            m_onBuildProgress(m_buildProgress, m_buildStatus);
        }

        // Build complete
        if (m_buildProgress >= 1.0f)
        {
            m_isBuilding = false;
            m_buildProgress = 1.0f;
            m_buildStatus = "Build completed successfully!";
            logBuildMessage(BuildLogEntry::Level::Info, "Build completed successfully!");

            if (m_onBuildComplete)
            {
                m_onBuildComplete(true, m_settings.outputPath);
            }
        }
    }
    else if (m_buildCancelled)
    {
        m_isBuilding = false;
        m_buildCancelled = false;
        m_buildStatus = "Build cancelled";

        if (m_onBuildComplete)
        {
            m_onBuildComplete(false, "");
        }
    }

    // Update size estimation if dirty
    if (m_sizeEstimationDirty)
    {
        estimateBuildSize();
    }
}

void BuildSettingsPanel::onRender()
{
    // Tab bar for different sections
    // Would use ImGui::BeginTabBar

    renderPresetSection();
    renderPlatformSection();
    renderOutputSection();
    renderAssetSection();
    renderLocalizationSection();
    renderWindowSection();

    if (m_showAdvanced)
    {
        renderAdvancedSection();
    }

    if (m_showPlatformSpecific)
    {
        renderPlatformSpecificSection();
    }

    renderSizeEstimation();
    renderBuildSection();

    if (m_showBuildLog)
    {
        renderBuildLog();
    }
}

void BuildSettingsPanel::renderPresetSection()
{
    // Would render preset dropdown and save/delete buttons
}

void BuildSettingsPanel::renderPlatformSection()
{
    if (widgets::CollapsingHeader("Platform"))
    {
        std::vector<std::string> platforms = {"Windows", "macOS", "Linux", "Web", "Android", "iOS"};
        i32 platformIndex = static_cast<i32>(m_settings.platform);
        if (widgets::Dropdown("Target Platform", &platformIndex, platforms))
        {
            m_settings.platform = static_cast<BuildPlatform>(platformIndex);
            m_sizeEstimationDirty = true;
        }

        std::vector<std::string> configs = {"Debug", "Release", "Distribution"};
        i32 configIndex = static_cast<i32>(m_settings.config);
        if (widgets::Dropdown("Configuration", &configIndex, configs))
        {
            m_settings.config = static_cast<BuildConfig>(configIndex);
        }
    }
}

void BuildSettingsPanel::renderOutputSection()
{
    if (widgets::CollapsingHeader("Output"))
    {
        char outputBuffer[256];
        std::strncpy(outputBuffer, m_settings.outputPath.c_str(), sizeof(outputBuffer) - 1);
        outputBuffer[sizeof(outputBuffer) - 1] = '\0';
        (void)outputBuffer;

        char nameBuffer[256];
        std::strncpy(nameBuffer, m_settings.productName.c_str(), sizeof(nameBuffer) - 1);
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        (void)nameBuffer;

        char versionBuffer[64];
        std::strncpy(versionBuffer, m_settings.version.c_str(), sizeof(versionBuffer) - 1);
        versionBuffer[sizeof(versionBuffer) - 1] = '\0';
        (void)versionBuffer;

        char companyBuffer[256];
        std::strncpy(companyBuffer, m_settings.companyName.c_str(), sizeof(companyBuffer) - 1);
        companyBuffer[sizeof(companyBuffer) - 1] = '\0';
        (void)companyBuffer;

        char bundleBuffer[256];
        std::strncpy(bundleBuffer, m_settings.bundleIdentifier.c_str(), sizeof(bundleBuffer) - 1);
        bundleBuffer[sizeof(bundleBuffer) - 1] = '\0';
        (void)bundleBuffer;
    }
}

void BuildSettingsPanel::renderAssetSection()
{
    if (widgets::CollapsingHeader("Assets"))
    {
        std::vector<std::string> qualities = {"Low", "Medium", "High"};
        if (widgets::Dropdown("Texture Quality", &m_settings.textureQuality, qualities))
        {
            m_sizeEstimationDirty = true;
        }

        std::vector<std::string> textureFormats = {"None", "BC1/DXT1", "BC3/DXT5", "BC7", "ETC2", "ASTC", "WebP"};
        i32 textureCompIndex = static_cast<i32>(m_settings.textureCompression);
        if (widgets::Dropdown("Texture Compression", &textureCompIndex, textureFormats))
        {
            m_settings.textureCompression = static_cast<TextureCompression>(textureCompIndex);
            m_sizeEstimationDirty = true;
        }

        std::vector<std::string> audioFormats = {"None", "MP3", "OGG", "AAC", "Opus"};
        i32 audioCompIndex = static_cast<i32>(m_settings.audioCompression);
        if (widgets::Dropdown("Audio Compression", &audioCompIndex, audioFormats))
        {
            m_settings.audioCompression = static_cast<AudioCompression>(audioCompIndex);
            m_sizeEstimationDirty = true;
        }
    }
}

void BuildSettingsPanel::renderLocalizationSection()
{
    if (widgets::CollapsingHeader("Localization"))
    {
        // Would render language list with checkboxes
    }
}

void BuildSettingsPanel::renderWindowSection()
{
    if (widgets::CollapsingHeader("Window"))
    {
        // Would render window settings
    }
}

void BuildSettingsPanel::renderAdvancedSection()
{
    if (widgets::CollapsingHeader("Advanced", nullptr, false))
    {
        // Would render advanced settings
    }
}

void BuildSettingsPanel::renderPlatformSpecificSection()
{
    // Render platform-specific settings based on current platform
    switch (m_settings.platform)
    {
        case BuildPlatform::Windows:
            if (widgets::CollapsingHeader("Windows Settings"))
            {
                // Icon path, installer options, etc.
            }
            break;
        case BuildPlatform::Web:
            if (widgets::CollapsingHeader("Web Settings"))
            {
                // Memory settings, threading, etc.
            }
            break;
        case BuildPlatform::Android:
            if (widgets::CollapsingHeader("Android Settings"))
            {
                // SDK versions, keystore, etc.
            }
            break;
        case BuildPlatform::iOS:
            if (widgets::CollapsingHeader("iOS Settings"))
            {
                // Team ID, provisioning profile, etc.
            }
            break;
        default:
            break;
    }
}

void BuildSettingsPanel::renderBuildSection()
{
    if (m_isBuilding)
    {
        widgets::ProgressBarLabeled(m_buildProgress, m_buildStatus.c_str());
        renderBuildSteps();
    }
    else if (!m_buildStatus.empty())
    {
        // Show last build status
    }
}

void BuildSettingsPanel::renderBuildSteps()
{
    for (const auto& step : m_buildSteps)
    {
        const char* icon = step.completed ? "[OK]" : (step.active ? "[->]" : "[ ]");
        // Would render step with icon and status
        (void)icon;
    }
}

void BuildSettingsPanel::renderBuildLog()
{
    if (widgets::CollapsingHeader("Build Log"))
    {
        // Would render scrollable log with colored entries
        for (const auto& entry : m_buildLog)
        {
            // Color based on level
            (void)entry;
        }
    }
}

void BuildSettingsPanel::renderSizeEstimation()
{
    if (widgets::CollapsingHeader("Size Estimation"))
    {
        // Total size
        // Would render: "Estimated size: X MB"

        // Breakdown
        for (const auto& [category, size] : m_sizeBreakdown)
        {
            // Would render: "category: size"
            (void)category;
            (void)size;
        }
    }
}

// =========================================================================
// Helpers
// =========================================================================

const char* BuildSettingsPanel::getPlatformName(BuildPlatform platform) const
{
    switch (platform)
    {
        case BuildPlatform::Windows: return "Windows";
        case BuildPlatform::macOS: return "macOS";
        case BuildPlatform::Linux: return "Linux";
        case BuildPlatform::Web: return "Web";
        case BuildPlatform::Android: return "Android";
        case BuildPlatform::iOS: return "iOS";
        default: return "Unknown";
    }
}

const char* BuildSettingsPanel::getConfigName(BuildConfig config) const
{
    switch (config)
    {
        case BuildConfig::Debug: return "Debug";
        case BuildConfig::Release: return "Release";
        case BuildConfig::Distribution: return "Distribution";
        default: return "Unknown";
    }
}

const char* BuildSettingsPanel::getTextureCompressionName(TextureCompression compression) const
{
    switch (compression)
    {
        case TextureCompression::None: return "None";
        case TextureCompression::BC1_DXT1: return "BC1/DXT1";
        case TextureCompression::BC3_DXT5: return "BC3/DXT5";
        case TextureCompression::BC7: return "BC7";
        case TextureCompression::ETC2: return "ETC2";
        case TextureCompression::ASTC: return "ASTC";
        case TextureCompression::WebP: return "WebP";
        default: return "Unknown";
    }
}

const char* BuildSettingsPanel::getAudioCompressionName(AudioCompression compression) const
{
    switch (compression)
    {
        case AudioCompression::None: return "None";
        case AudioCompression::MP3: return "MP3";
        case AudioCompression::OGG: return "OGG";
        case AudioCompression::AAC: return "AAC";
        case AudioCompression::Opus: return "Opus";
        default: return "Unknown";
    }
}

void BuildSettingsPanel::initializeDefaultPresets()
{
    m_presets.clear();

    // Built-in presets
    m_presets.push_back({"Windows Release", "Standard Windows release build",
        BuildPlatform::Windows, BuildConfig::Release, true});
    m_presets.push_back({"Windows Debug", "Windows debug build with development tools",
        BuildPlatform::Windows, BuildConfig::Debug, true});
    m_presets.push_back({"Web Release", "Optimized web build",
        BuildPlatform::Web, BuildConfig::Release, true});
    m_presets.push_back({"Steam Distribution", "Windows distribution build for Steam",
        BuildPlatform::Windows, BuildConfig::Distribution, true});
}

void BuildSettingsPanel::logBuildMessage(BuildLogEntry::Level level, const std::string& message)
{
    BuildLogEntry entry;
    entry.level = level;
    entry.message = message;

    // Format timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S");
    entry.timestamp = ss.str();

    m_buildLog.push_back(entry);

    if (m_onBuildLog)
    {
        m_onBuildLog(entry);
    }
}

std::string BuildSettingsPanel::formatSize(u64 bytes) const
{
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unitIndex < 3)
    {
        size /= 1024.0;
        ++unitIndex;
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << size << " " << units[unitIndex];
    return ss.str();
}

} // namespace NovelMind::editor
