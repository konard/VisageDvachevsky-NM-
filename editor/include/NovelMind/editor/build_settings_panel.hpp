#pragma once

/**
 * @file build_settings_panel.hpp
 * @brief Build Settings Panel for NovelMind Editor v0.2.0
 *
 * The Build Settings panel provides:
 * - Target platform selection
 * - Build configuration (debug/release/distribution)
 * - Build presets (save/load configurations)
 * - Output settings
 * - Asset compression options
 * - Localization settings
 * - Build button and progress
 * - Size estimation
 * - Build log viewer
 */

#include "NovelMind/editor/gui_panel_base.hpp"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace NovelMind::editor {

/**
 * @brief Target platform
 */
enum class BuildPlatform : u8 {
    Windows,
    macOS,
    Linux,
    Web,
    Android,
    iOS
};

/**
 * @brief Build configuration
 */
enum class BuildConfig : u8 {
    Debug,
    Release,
    Distribution
};

/**
 * @brief Texture compression format
 */
enum class TextureCompression : u8 {
    None,
    BC1_DXT1,
    BC3_DXT5,
    BC7,
    ETC2,
    ASTC,
    WebP
};

/**
 * @brief Audio compression format
 */
enum class AudioCompression : u8 {
    None,
    MP3,
    OGG,
    AAC,
    Opus
};

/**
 * @brief Build preset
 */
struct BuildPreset {
    std::string name;
    std::string description;
    BuildPlatform platform = BuildPlatform::Windows;
    BuildConfig config = BuildConfig::Release;
    bool isBuiltIn = false;
};

/**
 * @brief Build step for progress tracking
 */
struct BuildStep {
    std::string name;
    f32 weight = 1.0f;  // Relative time weight
    bool completed = false;
    bool active = false;
    std::string status;
};

/**
 * @brief Build log entry
 */
struct BuildLogEntry {
    enum class Level { Info, Warning, Error };
    Level level;
    std::string message;
    std::string timestamp;
};

/**
 * @brief Build settings data
 */
struct BuildSettings {
    BuildPlatform platform = BuildPlatform::Windows;
    BuildConfig config = BuildConfig::Release;
    std::string outputPath = "build";
    std::string productName = "MyVisualNovel";
    std::string version = "1.0.0";
    std::string companyName = "";
    std::string bundleIdentifier = "com.company.game";

    // Asset settings
    bool compressAssets = true;
    bool encryptAssets = false;
    i32 textureQuality = 2;  // 0=low, 1=medium, 2=high
    TextureCompression textureCompression = TextureCompression::BC3_DXT5;
    AudioCompression audioCompression = AudioCompression::OGG;
    i32 audioQuality = 80;  // 0-100

    // Localization
    std::vector<std::string> languages = {"en"};
    std::string defaultLanguage = "en";

    // Window settings
    i32 windowWidth = 1920;
    i32 windowHeight = 1080;
    bool fullscreenByDefault = false;
    bool allowResizing = true;
    bool enableVSync = true;

    // Advanced
    bool includeDevelopmentBuild = false;
    bool stripDebugSymbols = true;
    bool enableProfiling = false;
    bool enableCrashReporting = true;
    i32 maxSaveSlots = 20;
    bool compressScripts = true;

    // Platform-specific settings
    struct WindowsSettings {
        std::string iconPath;
        bool createInstaller = false;
        std::string installerPath;
    } windows;

    struct WebSettings {
        bool enableThreading = false;
        i32 initialMemoryMB = 256;
        i32 maxMemoryMB = 512;
        std::string htmlTemplate;
    } web;

    struct AndroidSettings {
        i32 minSdkVersion = 24;
        i32 targetSdkVersion = 33;
        std::string keystorePath;
        bool splitByAbi = true;
    } android;

    struct iOSSettings {
        std::string teamId;
        std::string provisioningProfile;
        bool enableBitcode = false;
    } ios;
};

/**
 * @brief Callback for build events
 */
using BuildStartCallback = std::function<void()>;
using BuildProgressCallback = std::function<void(f32 progress, const std::string& status)>;
using BuildCompleteCallback = std::function<void(bool success, const std::string& outputPath)>;
using BuildLogCallback = std::function<void(const BuildLogEntry& entry)>;

/**
 * @brief Build Settings Panel implementation
 */
class BuildSettingsPanel : public GUIPanelBase {
public:
    BuildSettingsPanel();
    ~BuildSettingsPanel() override = default;

    // =========================================================================
    // Settings
    // =========================================================================

    /**
     * @brief Get current build settings
     */
    [[nodiscard]] const BuildSettings& getSettings() const { return m_settings; }

    /**
     * @brief Set build settings
     */
    void setSettings(const BuildSettings& settings) { m_settings = settings; }

    // =========================================================================
    // Presets
    // =========================================================================

    /**
     * @brief Get available presets
     */
    [[nodiscard]] const std::vector<BuildPreset>& getPresets() const { return m_presets; }

    /**
     * @brief Load preset
     */
    void loadPreset(const std::string& name);

    /**
     * @brief Save current settings as preset
     */
    void savePreset(const std::string& name, const std::string& description = "");

    /**
     * @brief Delete preset
     */
    void deletePreset(const std::string& name);

    // =========================================================================
    // Build Control
    // =========================================================================

    /**
     * @brief Start a build
     */
    void startBuild();

    /**
     * @brief Cancel running build
     */
    void cancelBuild();

    /**
     * @brief Check if currently building
     */
    [[nodiscard]] bool isBuilding() const { return m_isBuilding; }

    /**
     * @brief Get build progress (0-1)
     */
    [[nodiscard]] f32 getBuildProgress() const { return m_buildProgress; }

    /**
     * @brief Get current build status
     */
    [[nodiscard]] const std::string& getBuildStatus() const { return m_buildStatus; }

    /**
     * @brief Get build steps
     */
    [[nodiscard]] const std::vector<BuildStep>& getBuildSteps() const { return m_buildSteps; }

    /**
     * @brief Get build log
     */
    [[nodiscard]] const std::vector<BuildLogEntry>& getBuildLog() const { return m_buildLog; }

    /**
     * @brief Clear build log
     */
    void clearBuildLog();

    // =========================================================================
    // Size Estimation
    // =========================================================================

    /**
     * @brief Estimate build size
     */
    void estimateBuildSize();

    /**
     * @brief Get estimated size in bytes
     */
    [[nodiscard]] u64 getEstimatedSize() const { return m_estimatedSize; }

    /**
     * @brief Get size breakdown by category
     */
    [[nodiscard]] const std::unordered_map<std::string, u64>& getSizeBreakdown() const { return m_sizeBreakdown; }

    // =========================================================================
    // Callbacks
    // =========================================================================

    void setOnBuildStart(BuildStartCallback callback) { m_onBuildStart = std::move(callback); }
    void setOnBuildProgress(BuildProgressCallback callback) { m_onBuildProgress = std::move(callback); }
    void setOnBuildComplete(BuildCompleteCallback callback) { m_onBuildComplete = std::move(callback); }
    void setOnBuildLog(BuildLogCallback callback) { m_onBuildLog = std::move(callback); }

    [[nodiscard]] std::vector<ToolbarItem> getToolbarItems() const override;
    [[nodiscard]] std::vector<MenuItem> getMenuItems() const override;

protected:
    void onInitialize() override;
    void onUpdate(f64 deltaTime) override;
    void onRender() override;

private:
    // Rendering helpers
    void renderPresetSection();
    void renderPlatformSection();
    void renderOutputSection();
    void renderAssetSection();
    void renderLocalizationSection();
    void renderWindowSection();
    void renderAdvancedSection();
    void renderPlatformSpecificSection();
    void renderBuildSection();
    void renderBuildSteps();
    void renderBuildLog();
    void renderSizeEstimation();

    // Helpers
    const char* getPlatformName(BuildPlatform platform) const;
    const char* getConfigName(BuildConfig config) const;
    const char* getTextureCompressionName(TextureCompression compression) const;
    const char* getAudioCompressionName(AudioCompression compression) const;
    void initializeDefaultPresets();
    void logBuildMessage(BuildLogEntry::Level level, const std::string& message);
    std::string formatSize(u64 bytes) const;

    BuildSettings m_settings;
    std::vector<BuildPreset> m_presets;
    std::string m_currentPreset;

    // Build state
    bool m_isBuilding = false;
    bool m_buildCancelled = false;
    f32 m_buildProgress = 0.0f;
    std::string m_buildStatus;
    std::vector<BuildStep> m_buildSteps;
    std::vector<BuildLogEntry> m_buildLog;
    f64 m_buildStartTime = 0.0;

    // Size estimation
    u64 m_estimatedSize = 0;
    std::unordered_map<std::string, u64> m_sizeBreakdown;
    bool m_sizeEstimationDirty = true;

    // UI state
    bool m_showAdvanced = false;
    bool m_showPlatformSpecific = false;
    bool m_showBuildLog = true;
    i32 m_selectedTab = 0;  // 0=General, 1=Assets, 2=Platform, 3=Build

    // Callbacks
    BuildStartCallback m_onBuildStart;
    BuildProgressCallback m_onBuildProgress;
    BuildCompleteCallback m_onBuildComplete;
    BuildLogCallback m_onBuildLog;
};

} // namespace NovelMind::editor
