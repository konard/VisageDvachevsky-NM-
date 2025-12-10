/**
 * @file main.cpp
 * @brief NovelMind Editor Application Entry Point
 *
 * This is the main entry point for the NovelMind Visual Novel Editor.
 * The editor provides a complete IDE for creating visual novels:
 * - Visual scene editing with WYSIWYG preview
 * - Node-based story graph editor
 * - Asset management and import pipeline
 * - Project build and export system
 *
 * Version: 0.2.0
 */

#include "NovelMind/editor/editor_app.hpp"
#include "NovelMind/core/logger.hpp"
#include "NovelMind/core/result.hpp"

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

using namespace NovelMind;
using namespace NovelMind::editor;

/**
 * @brief Print application version and build info
 */
void printVersion()
{
    std::cout << "NovelMind Editor v0.2.0 Alpha\n";
    std::cout << "Built with C++20\n";
    std::cout << "Copyright (c) 2024 NovelMind Contributors\n";
    std::cout << "Licensed under MIT License\n";
}

/**
 * @brief Print usage information
 */
void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " [options] [project-file]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help           Show this help message\n";
    std::cout << "  -v, --version        Show version information\n";
    std::cout << "  -n, --new <path>     Create a new project at <path>\n";
    std::cout << "  -o, --open <path>    Open an existing project\n";
    std::cout << "  --no-welcome         Skip the welcome screen\n";
    std::cout << "  --safe-mode          Start in safe mode (minimal UI)\n";
    std::cout << "  --reset-layout       Reset panel layout to defaults\n";
    std::cout << "  --theme <name>       Set UI theme (dark, light)\n";
    std::cout << "  --scale <factor>     Set UI scale factor (0.5-3.0)\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << "                    # Start editor with welcome screen\n";
    std::cout << "  " << programName << " myproject.nmproj   # Open a project\n";
    std::cout << "  " << programName << " -n ~/projects/new  # Create new project\n";
}

/**
 * @brief Parse command-line arguments
 */
struct CommandLineArgs
{
    bool showHelp = false;
    bool showVersion = false;
    bool noWelcome = false;
    bool safeMode = false;
    bool resetLayout = false;
    std::string newProjectPath;
    std::string openProjectPath;
    std::string theme;
    f32 uiScale = 1.0f;
};

CommandLineArgs parseCommandLine(int argc, char* argv[])
{
    CommandLineArgs args;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help")
        {
            args.showHelp = true;
        }
        else if (arg == "-v" || arg == "--version")
        {
            args.showVersion = true;
        }
        else if (arg == "--no-welcome")
        {
            args.noWelcome = true;
        }
        else if (arg == "--safe-mode")
        {
            args.safeMode = true;
        }
        else if (arg == "--reset-layout")
        {
            args.resetLayout = true;
        }
        else if ((arg == "-n" || arg == "--new") && i + 1 < argc)
        {
            args.newProjectPath = argv[++i];
        }
        else if ((arg == "-o" || arg == "--open") && i + 1 < argc)
        {
            args.openProjectPath = argv[++i];
        }
        else if (arg == "--theme" && i + 1 < argc)
        {
            args.theme = argv[++i];
        }
        else if (arg == "--scale" && i + 1 < argc)
        {
            try
            {
                args.uiScale = std::stof(argv[++i]);
                if (args.uiScale < 0.5f || args.uiScale > 3.0f)
                {
                    std::cerr << "Warning: UI scale clamped to valid range [0.5, 3.0]\n";
                    args.uiScale = std::max(0.5f, std::min(3.0f, args.uiScale));
                }
            }
            catch (...)
            {
                std::cerr << "Warning: Invalid UI scale value, using default\n";
                args.uiScale = 1.0f;
            }
        }
        else if (arg[0] != '-' && args.openProjectPath.empty())
        {
            // Positional argument - treat as project file
            args.openProjectPath = arg;
        }
        else if (arg[0] == '-')
        {
            std::cerr << "Unknown option: " << arg << "\n";
            std::cerr << "Use --help for usage information\n";
        }
    }

    return args;
}

/**
 * @brief Main entry point
 */
int main(int argc, char* argv[])
{
    // Parse command line arguments
    CommandLineArgs args = parseCommandLine(argc, argv);

    // Handle help and version flags
    if (args.showHelp)
    {
        printUsage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (args.showVersion)
    {
        printVersion();
        return EXIT_SUCCESS;
    }

    // Configure the editor
    EditorConfig config;
    config.showWelcomeScreen = !args.noWelcome;

    if (!args.theme.empty())
    {
        config.theme = args.theme;
    }

    if (args.uiScale != 1.0f)
    {
        config.uiScale = args.uiScale;
    }

    // Initialize logger
    core::Logger::instance().setLevel(core::LogLevel::Info);
    core::Logger::instance().info("NovelMind Editor starting...");

    // Create and initialize the editor application
    EditorApp editor;

    auto initResult = editor.initialize(config);
    if (!initResult.isOk())
    {
        std::cerr << "Failed to initialize editor: " << initResult.error() << "\n";
        core::Logger::instance().error("Failed to initialize editor: " + initResult.error());
        return EXIT_FAILURE;
    }

    // Handle project creation/opening
    if (!args.newProjectPath.empty())
    {
        auto result = editor.newProject(args.newProjectPath, "New Project");
        if (!result.isOk())
        {
            std::cerr << "Failed to create project: " << result.error() << "\n";
            // Continue anyway - user can create project from UI
        }
    }
    else if (!args.openProjectPath.empty())
    {
        auto result = editor.openProject(args.openProjectPath);
        if (!result.isOk())
        {
            std::cerr << "Failed to open project: " << result.error() << "\n";
            // Continue anyway - user can open project from UI
        }
    }

    // Reset layout if requested
    if (args.resetLayout)
    {
        editor.resetLayout();
    }

    core::Logger::instance().info("Editor initialized successfully");

    // Run the editor main loop
    editor.run();

    // Shutdown
    editor.shutdown();
    core::Logger::instance().info("NovelMind Editor shut down cleanly");

    return EXIT_SUCCESS;
}
