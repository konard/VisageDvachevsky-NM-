/**
 * @file timeline_panel.cpp
 * @brief Timeline Panel implementation
 */

#include "NovelMind/editor/timeline_panel.hpp"
#include "NovelMind/editor/imgui_integration.hpp"
#include <algorithm>
#include <cmath>

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
#include <imgui.h>
#endif

namespace NovelMind::editor {

TimelinePanel::TimelinePanel()
    : GUIPanelBase("Timeline")
{
    m_flags = PanelFlags::MenuBar | PanelFlags::HorizontalScrollbar;
}

void TimelinePanel::play()
{
    m_isPlaying = true;
    m_isPaused = false;

    TimelinePlaybackChangedEvent event;
    event.currentTime = m_currentTime;
    event.isPlaying = true;
    event.isPaused = false;
    event.playbackSpeed = m_playbackSpeed;
    publishEvent(event);
}

void TimelinePanel::pause()
{
    m_isPaused = true;

    TimelinePlaybackChangedEvent event;
    event.currentTime = m_currentTime;
    event.isPlaying = m_isPlaying;
    event.isPaused = true;
    event.playbackSpeed = m_playbackSpeed;
    publishEvent(event);
}

void TimelinePanel::stop()
{
    m_isPlaying = false;
    m_isPaused = false;
    m_currentTime = m_loopPlayback ? m_loopStart : 0.0;

    TimelinePlaybackChangedEvent event;
    event.currentTime = m_currentTime;
    event.isPlaying = false;
    event.isPaused = false;
    event.playbackSpeed = m_playbackSpeed;
    publishEvent(event);
}

void TimelinePanel::togglePlayPause()
{
    if (m_isPlaying && !m_isPaused)
    {
        pause();
    }
    else
    {
        play();
    }
}

void TimelinePanel::stepForward()
{
    f64 frameTime = 1.0 / static_cast<f64>(m_fps);
    setCurrentTime(m_currentTime + frameTime);
}

void TimelinePanel::stepBackward()
{
    f64 frameTime = 1.0 / static_cast<f64>(m_fps);
    setCurrentTime(m_currentTime - frameTime);
}

void TimelinePanel::goToStart()
{
    setCurrentTime(m_loopPlayback ? m_loopStart : 0.0);
}

void TimelinePanel::goToEnd()
{
    setCurrentTime(m_loopPlayback ? m_loopEnd : m_duration);
}

void TimelinePanel::setCurrentTime(f64 time)
{
    f64 minTime = m_loopPlayback ? m_loopStart : 0.0;
    f64 maxTime = m_loopPlayback ? m_loopEnd : m_duration;

    if (m_loopPlayback && time > maxTime)
    {
        time = minTime + std::fmod(time - minTime, maxTime - minTime);
    }
    else
    {
        time = std::clamp(time, minTime, maxTime);
    }

    if (m_currentTime != time)
    {
        m_currentTime = time;

        TimelinePlaybackChangedEvent event;
        event.currentTime = m_currentTime;
        event.isPlaying = m_isPlaying;
        event.isPaused = m_isPaused;
        event.playbackSpeed = m_playbackSpeed;
        publishEvent(event);
    }
}

void TimelinePanel::setDuration(f64 duration)
{
    m_duration = std::max(0.1, duration);
    if (m_currentTime > m_duration)
    {
        setCurrentTime(m_duration);
    }
    if (m_viewEndTime > m_duration)
    {
        m_viewEndTime = m_duration;
    }
}

void TimelinePanel::setFPS(f32 fps)
{
    m_fps = std::clamp(fps, 1.0f, 120.0f);
}

void TimelinePanel::setPlaybackSpeed(f32 speed)
{
    m_playbackSpeed = std::clamp(speed, 0.1f, 4.0f);
}

void TimelinePanel::setViewRange(f64 start, f64 end)
{
    m_viewStartTime = std::max(0.0, start);
    m_viewEndTime = std::min(m_duration, end);

    if (m_viewEndTime <= m_viewStartTime)
    {
        m_viewEndTime = m_viewStartTime + 1.0;
    }
}

void TimelinePanel::zoomToFit()
{
    if (m_tracks.empty())
    {
        m_viewStartTime = 0.0;
        m_viewEndTime = m_duration;
        return;
    }

    f64 minTime = m_duration;
    f64 maxTime = 0.0;

    for (const auto& track : m_tracks)
    {
        for (const auto& kf : track.keyframes)
        {
            minTime = std::min(minTime, kf.time);
            maxTime = std::max(maxTime, kf.time);
        }
    }

    // Add padding
    f64 padding = (maxTime - minTime) * 0.1;
    m_viewStartTime = std::max(0.0, minTime - padding);
    m_viewEndTime = std::min(m_duration, maxTime + padding);

    if (m_viewEndTime <= m_viewStartTime)
    {
        m_viewStartTime = 0.0;
        m_viewEndTime = m_duration;
    }
}

void TimelinePanel::resetView()
{
    m_viewStartTime = 0.0;
    m_viewEndTime = std::min(10.0, m_duration);
}

std::string TimelinePanel::addTrack(const std::string& name, TrackType type, const std::string& objectId)
{
    Track track;
    track.id = "track_" + std::to_string(m_nextTrackId++);
    track.name = name;
    track.objectId = objectId;
    track.type = type;

    // Assign color based on type
    switch (type)
    {
        case TrackType::Transform:
            track.color = {100, 150, 200, 255};
            break;
        case TrackType::Alpha:
            track.color = {200, 200, 100, 255};
            break;
        case TrackType::Color:
            track.color = {200, 100, 200, 255};
            break;
        case TrackType::Audio:
            track.color = {100, 200, 100, 255};
            break;
        case TrackType::Event:
            track.color = {200, 150, 100, 255};
            break;
        case TrackType::Camera:
            track.color = {150, 100, 200, 255};
            break;
        default:
            track.color = {150, 150, 150, 255};
            break;
    }

    m_tracks.push_back(track);

    TimelineTrackEvent event(EditorEventType::TimelineTrackAdded);
    event.trackId = track.id;
    publishEvent(event);

    return track.id;
}

void TimelinePanel::removeTrack(const std::string& trackId)
{
    auto it = std::remove_if(m_tracks.begin(), m_tracks.end(),
        [&trackId](const Track& t) { return t.id == trackId; });

    if (it != m_tracks.end())
    {
        m_tracks.erase(it, m_tracks.end());

        // Remove from selection
        m_selectedKeyframes.erase(
            std::remove_if(m_selectedKeyframes.begin(), m_selectedKeyframes.end(),
                [&trackId](const auto& sel) { return sel.first == trackId; }),
            m_selectedKeyframes.end());

        TimelineTrackEvent event(EditorEventType::TimelineTrackRemoved);
        event.trackId = trackId;
        publishEvent(event);
    }
}

Track* TimelinePanel::getTrack(const std::string& trackId)
{
    auto it = std::find_if(m_tracks.begin(), m_tracks.end(),
        [&trackId](const Track& t) { return t.id == trackId; });
    return it != m_tracks.end() ? &(*it) : nullptr;
}

void TimelinePanel::clearTracks()
{
    m_tracks.clear();
    m_selectedKeyframes.clear();
}

void TimelinePanel::addKeyframe(const std::string& trackId, f64 time, f32 value, EasingType easing)
{
    Track* track = getTrack(trackId);
    if (!track)
    {
        return;
    }

    // Find insert position to keep keyframes sorted
    auto insertPos = std::lower_bound(track->keyframes.begin(), track->keyframes.end(), time,
        [](const Keyframe& kf, f64 t) { return kf.time < t; });

    // Check if keyframe already exists at this time
    if (insertPos != track->keyframes.end() && std::abs(insertPos->time - time) < 0.001)
    {
        // Update existing keyframe
        insertPos->value = value;
        insertPos->easing = easing;
    }
    else
    {
        // Insert new keyframe
        Keyframe kf;
        kf.time = time;
        kf.value = value;
        kf.easing = easing;
        track->keyframes.insert(insertPos, kf);
    }

    TimelineKeyframeEvent event(EditorEventType::TimelineKeyframeAdded);
    event.trackId = trackId;
    event.time = time;
    publishEvent(event);
}

void TimelinePanel::removeKeyframe(const std::string& trackId, size_t keyframeIndex)
{
    Track* track = getTrack(trackId);
    if (!track || keyframeIndex >= track->keyframes.size())
    {
        return;
    }

    f64 time = track->keyframes[keyframeIndex].time;
    track->keyframes.erase(track->keyframes.begin() + static_cast<ptrdiff_t>(keyframeIndex));

    // Update selection
    m_selectedKeyframes.erase(
        std::remove(m_selectedKeyframes.begin(), m_selectedKeyframes.end(),
            std::make_pair(trackId, keyframeIndex)),
        m_selectedKeyframes.end());

    TimelineKeyframeEvent event(EditorEventType::TimelineKeyframeRemoved);
    event.trackId = trackId;
    event.time = time;
    publishEvent(event);
}

void TimelinePanel::moveKeyframe(const std::string& trackId, size_t keyframeIndex, f64 newTime)
{
    Track* track = getTrack(trackId);
    if (!track || keyframeIndex >= track->keyframes.size())
    {
        return;
    }

    newTime = std::clamp(newTime, 0.0, m_duration);
    if (m_snapEnabled)
    {
        newTime = snapTime(newTime);
    }

    track->keyframes[keyframeIndex].time = newTime;

    // Re-sort keyframes
    std::sort(track->keyframes.begin(), track->keyframes.end(),
        [](const Keyframe& a, const Keyframe& b) { return a.time < b.time; });

    TimelineKeyframeEvent event(EditorEventType::TimelineKeyframeMoved);
    event.trackId = trackId;
    event.time = newTime;
    publishEvent(event);
}

void TimelinePanel::setKeyframeValue(const std::string& trackId, size_t keyframeIndex, f32 value)
{
    Track* track = getTrack(trackId);
    if (!track || keyframeIndex >= track->keyframes.size())
    {
        return;
    }

    track->keyframes[keyframeIndex].value = value;
}

void TimelinePanel::setKeyframeEasing(const std::string& trackId, size_t keyframeIndex, EasingType easing)
{
    Track* track = getTrack(trackId);
    if (!track || keyframeIndex >= track->keyframes.size())
    {
        return;
    }

    track->keyframes[keyframeIndex].easing = easing;
}

void TimelinePanel::deleteSelectedKeyframes()
{
    // Delete in reverse order to maintain indices
    std::sort(m_selectedKeyframes.begin(), m_selectedKeyframes.end(),
        [](const auto& a, const auto& b) {
            return a.first == b.first ? a.second > b.second : a.first > b.first;
        });

    for (const auto& [trackId, index] : m_selectedKeyframes)
    {
        removeKeyframe(trackId, index);
    }

    m_selectedKeyframes.clear();
}

void TimelinePanel::selectKeyframe(const std::string& trackId, size_t keyframeIndex, bool addToSelection)
{
    if (!addToSelection)
    {
        clearKeyframeSelection();
    }

    auto pair = std::make_pair(trackId, keyframeIndex);
    if (std::find(m_selectedKeyframes.begin(), m_selectedKeyframes.end(), pair) == m_selectedKeyframes.end())
    {
        m_selectedKeyframes.push_back(pair);

        // Update keyframe selection state
        Track* track = getTrack(trackId);
        if (track && keyframeIndex < track->keyframes.size())
        {
            track->keyframes[keyframeIndex].selected = true;
        }
    }
}

void TimelinePanel::clearKeyframeSelection()
{
    for (const auto& [trackId, index] : m_selectedKeyframes)
    {
        Track* track = getTrack(trackId);
        if (track && index < track->keyframes.size())
        {
            track->keyframes[index].selected = false;
        }
    }
    m_selectedKeyframes.clear();
}

std::vector<MenuItem> TimelinePanel::getMenuItems() const
{
    std::vector<MenuItem> items;

    MenuItem editMenu;
    editMenu.label = "Edit";
    editMenu.subItems = {
        {"Add Keyframe", "K", [this]() { /* Add keyframe at current time */ }},
        {"Delete Keyframes", "Delete", [this]() { const_cast<TimelinePanel*>(this)->deleteSelectedKeyframes(); },
         [this]() { return !m_selectedKeyframes.empty(); }},
        MenuItem::separator(),
        {"Select All Keyframes", "Ctrl+A", []() { /* Select all */ }},
    };
    items.push_back(editMenu);

    MenuItem viewMenu;
    viewMenu.label = "View";
    viewMenu.subItems = {
        {"Zoom to Fit", "F", [this]() { const_cast<TimelinePanel*>(this)->zoomToFit(); }},
        {"Reset View", "Home", [this]() { const_cast<TimelinePanel*>(this)->resetView(); }},
        MenuItem::separator(),
        {"Show Curve Editor", "", [this]() { const_cast<TimelinePanel*>(this)->m_showCurveEditor = !m_showCurveEditor; },
         []() { return true; }, [this]() { return m_showCurveEditor; }},
    };
    items.push_back(viewMenu);

    return items;
}

std::vector<ToolbarItem> TimelinePanel::getToolbarItems() const
{
    std::vector<ToolbarItem> items;

    // Playback controls
    items.push_back({"|<", "Go to Start", [this]() { const_cast<TimelinePanel*>(this)->goToStart(); }});
    items.push_back({"<", "Step Back", [this]() { const_cast<TimelinePanel*>(this)->stepBackward(); }});
    items.push_back({m_isPlaying && !m_isPaused ? "||" : ">", m_isPlaying ? "Pause" : "Play",
                     [this]() { const_cast<TimelinePanel*>(this)->togglePlayPause(); }});
    items.push_back({">", "Step Forward", [this]() { const_cast<TimelinePanel*>(this)->stepForward(); }});
    items.push_back({">|", "Go to End", [this]() { const_cast<TimelinePanel*>(this)->goToEnd(); }});
    items.push_back({"[]", "Stop", [this]() { const_cast<TimelinePanel*>(this)->stop(); }});

    items.push_back(ToolbarItem::separator());

    items.push_back({"Loop", "Toggle Loop", [this]() { const_cast<TimelinePanel*>(this)->m_loopPlayback = !m_loopPlayback; },
                     []() { return true; }, [this]() { return m_loopPlayback; }});

    items.push_back(ToolbarItem::separator());

    items.push_back({"Snap", "Toggle Snap", [this]() { const_cast<TimelinePanel*>(this)->m_snapEnabled = !m_snapEnabled; },
                     []() { return true; }, [this]() { return m_snapEnabled; }});

    return items;
}

void TimelinePanel::onInitialize()
{
    // Subscribe to timeline events from other systems
    subscribeEvent<TimelinePlaybackChangedEvent>(
        [this](const TimelinePlaybackChangedEvent& event) {
            if (event.source != getName())
            {
                m_currentTime = event.currentTime;
                m_isPlaying = event.isPlaying;
                m_isPaused = event.isPaused;
            }
        });
}

void TimelinePanel::onShutdown()
{
    clearTracks();
}

void TimelinePanel::onUpdate(f64 deltaTime)
{
    // Update playback
    if (m_isPlaying && !m_isPaused)
    {
        setCurrentTime(m_currentTime + deltaTime * static_cast<f64>(m_playbackSpeed));

        // Check if reached end
        if (!m_loopPlayback && m_currentTime >= m_duration)
        {
            stop();
        }
    }

    handleMouseInput();
    handleKeyboardInput();
}

void TimelinePanel::onRender()
{
    // Layout:
    // +------------------+------------------------+
    // | Track Headers    | Ruler                  |
    // +------------------+------------------------+
    // | Track 1 Header   | Track 1 Keyframes     |
    // | Track 2 Header   | Track 2 Keyframes     |
    // | ...              | ...                   |
    // +------------------+------------------------+

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImVec2 availSize = ImGui::GetContentRegionAvail();
    m_contentWidth = availSize.x;
    m_contentHeight = availSize.y;

    // Time display at top
    ImGui::Text("Time: %.2f / %.2f  |  FPS: %.0f  |  Speed: %.1fx",
                m_currentTime, m_duration, m_fps, m_playbackSpeed);
    ImGui::Separator();

    // Split into header and content regions
    ImGui::BeginChild("TrackHeaders", ImVec2(m_headerWidth, 0), true);
    ImGui::Dummy(ImVec2(0, m_rulerHeight)); // Space for ruler alignment
    renderTrackHeaders();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("TrackContent", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
    renderRuler();
    renderTrackContent();
    renderPlayhead();
    ImGui::EndChild();

    // Curve editor overlay
    if (m_showCurveEditor)
    {
        renderCurveEditor();
    }
#else
    renderRuler();
    renderTrackContent();
    renderPlayhead();

    if (m_showCurveEditor)
    {
        renderCurveEditor();
    }
#endif
}

void TimelinePanel::renderToolbar()
{
    widgets::BeginToolbar("TimelineToolbar");
    renderToolbarItems(getToolbarItems());

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Time display
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "%.2f / %.2f", m_currentTime, m_duration);

    // Speed control
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    if (ImGui::SliderFloat("##Speed", &m_playbackSpeed, 0.1f, 4.0f, "%.1fx"))
    {
        // Speed changed
    }
#endif

    widgets::EndToolbar();
}

void TimelinePanel::renderRuler()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 contentMin = ImGui::GetCursorScreenPos();
    f32 contentWidth = ImGui::GetContentRegionAvail().x;

    // Draw ruler background
    drawList->AddRectFilled(contentMin,
        ImVec2(contentMin.x + contentWidth, contentMin.y + m_rulerHeight),
        IM_COL32(m_rulerColor.r, m_rulerColor.g, m_rulerColor.b, m_rulerColor.a));

    // Draw time markers
    f64 timeRange = m_viewEndTime - m_viewStartTime;
    f64 majorInterval = 1.0;  // 1 second major marks
    if (timeRange < 2.0) majorInterval = 0.5;
    if (timeRange < 1.0) majorInterval = 0.1;
    if (timeRange > 10.0) majorInterval = 5.0;
    if (timeRange > 60.0) majorInterval = 10.0;

    f64 minorInterval = majorInterval / 4.0;

    // Draw minor marks
    for (f64 t = std::ceil(m_viewStartTime / minorInterval) * minorInterval; t <= m_viewEndTime; t += minorInterval)
    {
        f32 x = timeToPixel(t);
        if (x < 0 || x > contentWidth) continue;

        drawList->AddLine(
            ImVec2(contentMin.x + x, contentMin.y + m_rulerHeight - 5),
            ImVec2(contentMin.x + x, contentMin.y + m_rulerHeight),
            IM_COL32(150, 150, 150, 200));
    }

    // Draw major marks with labels
    for (f64 t = std::ceil(m_viewStartTime / majorInterval) * majorInterval; t <= m_viewEndTime; t += majorInterval)
    {
        f32 x = timeToPixel(t);
        if (x < 0 || x > contentWidth) continue;

        drawList->AddLine(
            ImVec2(contentMin.x + x, contentMin.y + m_rulerHeight - 12),
            ImVec2(contentMin.x + x, contentMin.y + m_rulerHeight),
            IM_COL32(200, 200, 200, 255));

        char label[32];
        if (majorInterval >= 1.0)
        {
            snprintf(label, sizeof(label), "%.0fs", t);
        }
        else
        {
            snprintf(label, sizeof(label), "%.2f", t);
        }
        drawList->AddText(ImVec2(contentMin.x + x + 2, contentMin.y + 2),
                          IM_COL32(200, 200, 200, 255), label);
    }

    // Advance cursor past ruler
    ImGui::Dummy(ImVec2(contentWidth, m_rulerHeight));
#else
    f64 timeRange = m_viewEndTime - m_viewStartTime;
    f64 majorInterval = 1.0;
    if (timeRange < 2.0) majorInterval = 0.5;
    if (timeRange < 1.0) majorInterval = 0.1;
    (void)timeRange;
    (void)majorInterval;
#endif
}

void TimelinePanel::renderTrackHeaders()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    for (size_t i = 0; i < m_tracks.size(); ++i)
    {
        auto& track = m_tracks[i];

        ImGui::PushID(static_cast<int>(i));

        // Track row with fixed height
        ImGui::BeginGroup();

        // Expand/collapse button
        if (ImGui::ArrowButton("##expand", track.expanded ? ImGuiDir_Down : ImGuiDir_Right))
        {
            track.expanded = !track.expanded;
        }

        ImGui::SameLine();

        // Visibility toggle
        if (ImGui::Checkbox("##visible", &track.visible))
        {
            // Visibility changed
        }

        ImGui::SameLine();

        // Track color indicator
        ImVec4 trackColor(track.color.r / 255.0f, track.color.g / 255.0f,
                          track.color.b / 255.0f, track.color.a / 255.0f);
        ImGui::ColorButton("##color", trackColor, ImGuiColorEditFlags_NoTooltip, ImVec2(10, 16));

        ImGui::SameLine();

        // Track name (colored)
        ImGui::TextColored(trackColor, "%s", track.name.c_str());

        // Lock toggle on the right
        ImGui::SameLine(m_headerWidth - 25);
        if (ImGui::Button(track.locked ? "L" : "U", ImVec2(20, 0)))
        {
            track.locked = !track.locked;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(track.locked ? "Unlock track" : "Lock track");
        }

        ImGui::EndGroup();

        // Context menu
        if (ImGui::BeginPopupContextItem("TrackContextMenu"))
        {
            if (ImGui::MenuItem("Rename Track"))
            {
                // Would open rename dialog
            }
            if (ImGui::MenuItem("Delete Track"))
            {
                // Mark for deletion
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Add Keyframe at Playhead"))
            {
                addKeyframe(track.id, m_currentTime, 0.0f, EasingType::Linear);
            }
            if (ImGui::MenuItem("Clear All Keyframes"))
            {
                track.keyframes.clear();
            }
            ImGui::EndPopup();
        }

        ImGui::PopID();

        // Add spacing to match track height
        if (ImGui::GetCursorPosY() < m_trackHeight * (i + 1))
        {
            ImGui::Dummy(ImVec2(0, m_trackHeight - 22)); // Adjust for row height
        }
    }

    // Empty state
    if (m_tracks.empty())
    {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No tracks");
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "Right-click to add");
    }

    // Context menu for adding tracks
    if (ImGui::BeginPopupContextWindow("AddTrackMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        if (ImGui::BeginMenu("Add Track"))
        {
            if (ImGui::MenuItem("Transform Track"))
            {
                addTrack("Transform", TrackType::Transform, "");
            }
            if (ImGui::MenuItem("Alpha Track"))
            {
                addTrack("Alpha", TrackType::Alpha, "");
            }
            if (ImGui::MenuItem("Color Track"))
            {
                addTrack("Color", TrackType::Color, "");
            }
            if (ImGui::MenuItem("Audio Track"))
            {
                addTrack("Audio", TrackType::Audio, "");
            }
            if (ImGui::MenuItem("Event Track"))
            {
                addTrack("Events", TrackType::Event, "");
            }
            if (ImGui::MenuItem("Camera Track"))
            {
                addTrack("Camera", TrackType::Camera, "");
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
#else
    for (size_t i = 0; i < m_tracks.size(); ++i)
    {
        const auto& track = m_tracks[i];
        (void)track;
    }
#endif
}

void TimelinePanel::renderTrackContent()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 contentMin = ImGui::GetCursorScreenPos();
    f32 contentWidth = ImGui::GetContentRegionAvail().x;

    f32 trackY = 0; // Start after ruler (already rendered)

    for (size_t i = 0; i < m_tracks.size(); ++i)
    {
        const auto& track = m_tracks[i];

        // Draw track background (alternating)
        renderer::Color bgColor = (i % 2 == 0) ? m_trackBackgroundColor : m_trackBackgroundAltColor;
        drawList->AddRectFilled(
            ImVec2(contentMin.x, contentMin.y + trackY),
            ImVec2(contentMin.x + contentWidth, contentMin.y + trackY + m_trackHeight),
            IM_COL32(bgColor.r, bgColor.g, bgColor.b, bgColor.a));

        // Draw track color strip on the left
        drawList->AddRectFilled(
            ImVec2(contentMin.x, contentMin.y + trackY),
            ImVec2(contentMin.x + 3, contentMin.y + trackY + m_trackHeight),
            IM_COL32(track.color.r, track.color.g, track.color.b, track.color.a));

        // Draw keyframes if track is visible
        if (track.visible)
        {
            renderKeyframes(track, trackY, m_trackHeight);
        }

        // Draw horizontal separator line
        drawList->AddLine(
            ImVec2(contentMin.x, contentMin.y + trackY + m_trackHeight),
            ImVec2(contentMin.x + contentWidth, contentMin.y + trackY + m_trackHeight),
            IM_COL32(60, 60, 60, 255));

        trackY += m_trackHeight;
    }

    // Reserve space for all tracks
    ImGui::Dummy(ImVec2(contentWidth, trackY > 0 ? trackY : 100));

    // Empty state message
    if (m_tracks.empty())
    {
        ImVec2 center(contentMin.x + contentWidth / 2, contentMin.y + 50);
        const char* text = "No tracks - Right-click track headers to add";
        ImVec2 textSize = ImGui::CalcTextSize(text);
        drawList->AddText(ImVec2(center.x - textSize.x / 2, center.y),
                          IM_COL32(128, 128, 128, 255), text);
    }
#else
    f32 trackY = m_rulerHeight;

    for (size_t i = 0; i < m_tracks.size(); ++i)
    {
        const auto& track = m_tracks[i];
        renderer::Color bgColor = (i % 2 == 0) ? m_trackBackgroundColor : m_trackBackgroundAltColor;

        if (track.visible)
        {
            renderKeyframes(track, trackY, m_trackHeight);
        }

        trackY += m_trackHeight;
        (void)bgColor;
    }
#endif
}

void TimelinePanel::renderPlayhead()
{
    if (m_currentTime < m_viewStartTime || m_currentTime > m_viewEndTime)
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    f32 x = timeToPixel(m_currentTime);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    // Calculate playhead position relative to content area
    f32 playheadX = windowPos.x + x;
    f32 playheadTop = windowPos.y;
    f32 playheadBottom = windowPos.y + windowSize.y;

    // Draw playhead line (full height)
    drawList->AddLine(
        ImVec2(playheadX, playheadTop),
        ImVec2(playheadX, playheadBottom),
        IM_COL32(m_playheadColor.r, m_playheadColor.g, m_playheadColor.b, m_playheadColor.a),
        2.0f);

    // Draw playhead triangle at top
    drawList->AddTriangleFilled(
        ImVec2(playheadX - 8, playheadTop),
        ImVec2(playheadX + 8, playheadTop),
        ImVec2(playheadX, playheadTop + 12),
        IM_COL32(m_playheadColor.r, m_playheadColor.g, m_playheadColor.b, m_playheadColor.a));

    // Draw time label near playhead
    char timeLabel[16];
    snprintf(timeLabel, sizeof(timeLabel), "%.2f", m_currentTime);
    ImVec2 labelSize = ImGui::CalcTextSize(timeLabel);
    drawList->AddRectFilled(
        ImVec2(playheadX - labelSize.x / 2 - 2, playheadTop + 14),
        ImVec2(playheadX + labelSize.x / 2 + 2, playheadTop + 14 + labelSize.y + 2),
        IM_COL32(60, 60, 60, 220));
    drawList->AddText(
        ImVec2(playheadX - labelSize.x / 2, playheadTop + 15),
        IM_COL32(m_playheadColor.r, m_playheadColor.g, m_playheadColor.b, 255),
        timeLabel);
#else
    f32 x = timeToPixel(m_currentTime);
    (void)x;
#endif
}

void TimelinePanel::renderKeyframes(const Track& track, f32 trackY, f32 trackHeight)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();

    f32 keyframeRadius = 6.0f;
    f32 centerY = trackY + trackHeight / 2.0f;

    // Draw interpolation curves between keyframes
    for (size_t i = 0; i + 1 < track.keyframes.size(); ++i)
    {
        const auto& kf1 = track.keyframes[i];
        const auto& kf2 = track.keyframes[i + 1];

        if (kf2.time < m_viewStartTime || kf1.time > m_viewEndTime)
        {
            continue;
        }

        f32 x1 = timeToPixel(kf1.time);
        f32 x2 = timeToPixel(kf2.time);

        // Draw interpolation line
        ImU32 lineColor = IM_COL32(track.color.r, track.color.g, track.color.b, 128);
        drawList->AddLine(
            ImVec2(windowPos.x + x1, windowPos.y + centerY),
            ImVec2(windowPos.x + x2, windowPos.y + centerY),
            lineColor, 1.5f);
    }

    // Draw keyframes
    for (size_t i = 0; i < track.keyframes.size(); ++i)
    {
        const auto& kf = track.keyframes[i];

        if (kf.time < m_viewStartTime || kf.time > m_viewEndTime)
        {
            continue;
        }

        f32 x = timeToPixel(kf.time);
        bool isSelected = std::find(m_selectedKeyframes.begin(), m_selectedKeyframes.end(),
            std::make_pair(track.id, i)) != m_selectedKeyframes.end();

        renderer::Color color = isSelected ? m_keyframeSelectedColor : m_keyframeColor;

        // Draw keyframe diamond shape
        ImVec2 center(windowPos.x + x, windowPos.y + centerY);
        drawList->AddQuadFilled(
            ImVec2(center.x, center.y - keyframeRadius),       // Top
            ImVec2(center.x + keyframeRadius, center.y),       // Right
            ImVec2(center.x, center.y + keyframeRadius),       // Bottom
            ImVec2(center.x - keyframeRadius, center.y),       // Left
            IM_COL32(color.r, color.g, color.b, color.a));

        // Draw outline
        ImU32 outlineColor = isSelected ? IM_COL32(255, 255, 255, 255) : IM_COL32(40, 40, 40, 255);
        drawList->AddQuad(
            ImVec2(center.x, center.y - keyframeRadius),
            ImVec2(center.x + keyframeRadius, center.y),
            ImVec2(center.x, center.y + keyframeRadius),
            ImVec2(center.x - keyframeRadius, center.y),
            outlineColor, 1.5f);

        // Draw easing indicator (small icon inside)
        if (kf.easing != EasingType::Linear)
        {
            ImU32 easingColor = IM_COL32(255, 255, 255, 200);
            switch (kf.easing)
            {
                case EasingType::EaseIn:
                    // Small curve indicator
                    drawList->AddBezierQuadratic(
                        ImVec2(center.x - 3, center.y + 2),
                        ImVec2(center.x - 2, center.y - 2),
                        ImVec2(center.x + 3, center.y - 2),
                        easingColor, 1.0f);
                    break;
                case EasingType::EaseOut:
                    drawList->AddBezierQuadratic(
                        ImVec2(center.x - 3, center.y - 2),
                        ImVec2(center.x + 2, center.y - 2),
                        ImVec2(center.x + 3, center.y + 2),
                        easingColor, 1.0f);
                    break;
                case EasingType::EaseInOut:
                    drawList->AddBezierQuadratic(
                        ImVec2(center.x - 3, center.y + 1),
                        ImVec2(center.x, center.y - 3),
                        ImVec2(center.x + 3, center.y + 1),
                        easingColor, 1.0f);
                    break;
                case EasingType::Bounce:
                    drawList->AddText(ImVec2(center.x - 2, center.y - 4), easingColor, "~");
                    break;
                case EasingType::Elastic:
                    drawList->AddText(ImVec2(center.x - 2, center.y - 4), easingColor, "e");
                    break;
                default:
                    break;
            }
        }
    }
#else
    f32 keyframeRadius = 5.0f;
    f32 centerY = trackY + trackHeight / 2.0f;

    for (size_t i = 0; i < track.keyframes.size(); ++i)
    {
        const auto& kf = track.keyframes[i];

        if (kf.time < m_viewStartTime || kf.time > m_viewEndTime)
        {
            continue;
        }

        f32 x = timeToPixel(kf.time);
        bool isSelected = std::find(m_selectedKeyframes.begin(), m_selectedKeyframes.end(),
            std::make_pair(track.id, i)) != m_selectedKeyframes.end();

        renderer::Color color = isSelected ? m_keyframeSelectedColor : m_keyframeColor;

        (void)x;
        (void)centerY;
        (void)keyframeRadius;
        (void)color;
    }
#endif
}

void TimelinePanel::renderCurveEditor()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImGui::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Curve Editor", &m_showCurveEditor))
    {
        // Get selected track for curve editing
        Track* selectedTrack = nullptr;
        if (!m_selectedKeyframes.empty())
        {
            selectedTrack = getTrack(m_selectedKeyframes[0].first);
        }

        if (!selectedTrack || selectedTrack->keyframes.empty())
        {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                "Select keyframes to edit their curves");
        }
        else
        {
            ImGui::Text("Track: %s", selectedTrack->name.c_str());

            // Easing selector
            static const char* easingNames[] = {
                "Linear", "Ease In", "Ease Out", "Ease In Out", "Bounce", "Elastic"
            };

            int currentEasing = 0;
            if (!m_selectedKeyframes.empty())
            {
                size_t idx = m_selectedKeyframes[0].second;
                if (idx < selectedTrack->keyframes.size())
                {
                    currentEasing = static_cast<int>(selectedTrack->keyframes[idx].easing);
                }
            }

            ImGui::Text("Easing:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150);
            if (ImGui::Combo("##Easing", &currentEasing, easingNames, IM_ARRAYSIZE(easingNames)))
            {
                // Apply to all selected keyframes
                for (const auto& [trackId, idx] : m_selectedKeyframes)
                {
                    setKeyframeEasing(trackId, idx, static_cast<EasingType>(currentEasing));
                }
            }

            ImGui::Separator();

            // Draw curve preview
            ImVec2 canvasSize = ImGui::GetContentRegionAvail();
            canvasSize.y = std::max(200.0f, canvasSize.y);

            ImVec2 canvasPos = ImGui::GetCursorScreenPos();
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            // Background
            drawList->AddRectFilled(canvasPos,
                ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                IM_COL32(30, 30, 30, 255));

            // Grid
            for (int i = 0; i <= 10; ++i)
            {
                f32 x = canvasPos.x + (canvasSize.x * i / 10.0f);
                f32 y = canvasPos.y + (canvasSize.y * i / 10.0f);
                ImU32 gridColor = (i == 5) ? IM_COL32(80, 80, 80, 255) : IM_COL32(50, 50, 50, 255);
                drawList->AddLine(ImVec2(x, canvasPos.y), ImVec2(x, canvasPos.y + canvasSize.y), gridColor);
                drawList->AddLine(ImVec2(canvasPos.x, y), ImVec2(canvasPos.x + canvasSize.x, y), gridColor);
            }

            // Draw easing curve preview
            EasingType easing = static_cast<EasingType>(currentEasing);
            ImVec2 prevPoint = canvasPos;
            prevPoint.y += canvasSize.y;

            for (int i = 1; i <= 50; ++i)
            {
                f64 t = static_cast<f64>(i) / 50.0;
                f64 easedT = t;

                // Apply easing
                switch (easing)
                {
                    case EasingType::EaseIn:
                        easedT = t * t;
                        break;
                    case EasingType::EaseOut:
                        easedT = 1.0 - (1.0 - t) * (1.0 - t);
                        break;
                    case EasingType::EaseInOut:
                        easedT = t < 0.5 ? 2.0 * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 2.0) / 2.0;
                        break;
                    case EasingType::Bounce:
                        if (t < 1.0 / 2.75)
                            easedT = 7.5625 * t * t;
                        else if (t < 2.0 / 2.75)
                        {
                            f64 tt = t - 1.5 / 2.75;
                            easedT = 7.5625 * tt * tt + 0.75;
                        }
                        else if (t < 2.5 / 2.75)
                        {
                            f64 tt = t - 2.25 / 2.75;
                            easedT = 7.5625 * tt * tt + 0.9375;
                        }
                        else
                        {
                            f64 tt = t - 2.625 / 2.75;
                            easedT = 7.5625 * tt * tt + 0.984375;
                        }
                        break;
                    case EasingType::Elastic:
                        if (t != 0.0 && t != 1.0)
                            easedT = std::pow(2.0, -10.0 * t) * std::sin((t - 0.075) * (2.0 * 3.14159) / 0.3) + 1.0;
                        break;
                    default:
                        break;
                }

                ImVec2 point;
                point.x = canvasPos.x + static_cast<f32>(t) * canvasSize.x;
                point.y = canvasPos.y + canvasSize.y - static_cast<f32>(easedT) * canvasSize.y;

                drawList->AddLine(prevPoint, point, IM_COL32(100, 200, 255, 255), 2.0f);
                prevPoint = point;
            }

            // Labels
            drawList->AddText(ImVec2(canvasPos.x + 2, canvasPos.y + 2), IM_COL32(200, 200, 200, 255), "1.0");
            drawList->AddText(ImVec2(canvasPos.x + 2, canvasPos.y + canvasSize.y - 14), IM_COL32(200, 200, 200, 255), "0.0");
            drawList->AddText(ImVec2(canvasPos.x + canvasSize.x - 20, canvasPos.y + canvasSize.y - 14), IM_COL32(200, 200, 200, 255), "1.0");

            ImGui::Dummy(canvasSize);
        }
    }
    ImGui::End();
#endif
}

void TimelinePanel::handleMouseInput()
{
    if (!m_isHovered)
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Get mouse position relative to timeline
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 windowPos = ImGui::GetWindowPos();
    f32 relX = mousePos.x - windowPos.x;

    // Click on ruler to scrub
    if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
    {
        f64 clickTime = pixelToTime(relX);
        if (clickTime >= 0 && clickTime <= m_duration)
        {
            setCurrentTime(clickTime);
        }
    }

    // Drag to scrub (hold left mouse)
    if (ImGui::IsMouseDragging(0) && ImGui::IsWindowHovered())
    {
        f64 dragTime = pixelToTime(relX);
        if (dragTime >= 0 && dragTime <= m_duration)
        {
            setCurrentTime(dragTime);
        }
    }

    // Middle mouse to pan
    if (ImGui::IsMouseDragging(2))
    {
        ImVec2 delta = ImGui::GetMouseDragDelta(2);
        ImGui::ResetMouseDragDelta(2);

        f64 timeDelta = (delta.x / m_contentWidth) * (m_viewEndTime - m_viewStartTime);
        m_viewStartTime -= timeDelta;
        m_viewEndTime -= timeDelta;

        // Clamp to valid range
        if (m_viewStartTime < 0)
        {
            f64 offset = -m_viewStartTime;
            m_viewStartTime = 0;
            m_viewEndTime += offset;
        }
        if (m_viewEndTime > m_duration)
        {
            f64 offset = m_viewEndTime - m_duration;
            m_viewEndTime = m_duration;
            m_viewStartTime -= offset;
            if (m_viewStartTime < 0) m_viewStartTime = 0;
        }
    }

    // Mouse wheel to zoom horizontally
    f32 wheel = ImGui::GetIO().MouseWheel;
    if (std::abs(wheel) > 0.0f && ImGui::IsWindowHovered())
    {
        f64 zoomFactor = 1.0 + wheel * 0.1;
        f64 mousePosTime = pixelToTime(relX);

        // Zoom around mouse position
        f64 leftDist = mousePosTime - m_viewStartTime;
        f64 rightDist = m_viewEndTime - mousePosTime;

        m_viewStartTime = mousePosTime - leftDist / zoomFactor;
        m_viewEndTime = mousePosTime + rightDist / zoomFactor;

        // Clamp
        if (m_viewStartTime < 0) m_viewStartTime = 0;
        if (m_viewEndTime > m_duration) m_viewEndTime = m_duration;
        if (m_viewEndTime - m_viewStartTime < 0.1)
        {
            // Minimum zoom
            m_viewEndTime = m_viewStartTime + 0.1;
        }
    }
#endif
}

void TimelinePanel::handleKeyboardInput()
{
    if (!m_isFocused)
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Space: play/pause
    if (ImGui::IsKeyPressed(ImGuiKey_Space))
    {
        togglePlayPause();
    }

    // Home/End: go to start/end
    if (ImGui::IsKeyPressed(ImGuiKey_Home))
    {
        goToStart();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_End))
    {
        goToEnd();
    }

    // Left/Right arrows: step (with Shift for faster stepping)
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
    {
        if (ImGui::GetIO().KeyShift)
        {
            // Step back 10 frames
            for (int i = 0; i < 10; ++i) stepBackward();
        }
        else
        {
            stepBackward();
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
    {
        if (ImGui::GetIO().KeyShift)
        {
            // Step forward 10 frames
            for (int i = 0; i < 10; ++i) stepForward();
        }
        else
        {
            stepForward();
        }
    }

    // Delete: delete selected keyframes
    if (ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        deleteSelectedKeyframes();
    }

    // K: Add keyframe at current time
    if (ImGui::IsKeyPressed(ImGuiKey_K) && !m_tracks.empty())
    {
        // Add to first track if none selected
        addKeyframe(m_tracks[0].id, m_currentTime, 0.0f, EasingType::Linear);
    }

    // L: Toggle loop
    if (ImGui::IsKeyPressed(ImGuiKey_L))
    {
        m_loopPlayback = !m_loopPlayback;
    }

    // S: Toggle snap
    if (ImGui::IsKeyPressed(ImGuiKey_S) && ImGui::GetIO().KeyCtrl)
    {
        m_snapEnabled = !m_snapEnabled;
    }

    // F: Zoom to fit
    if (ImGui::IsKeyPressed(ImGuiKey_F))
    {
        zoomToFit();
    }
#endif
}

f32 TimelinePanel::timeToPixel(f64 time) const
{
    f64 normalizedTime = (time - m_viewStartTime) / (m_viewEndTime - m_viewStartTime);
    return static_cast<f32>(normalizedTime * (m_contentWidth - m_headerWidth));
}

f64 TimelinePanel::pixelToTime(f32 pixel) const
{
    f64 normalizedX = static_cast<f64>(pixel) / (m_contentWidth - m_headerWidth);
    return m_viewStartTime + normalizedX * (m_viewEndTime - m_viewStartTime);
}

f64 TimelinePanel::snapTime(f64 time) const
{
    if (!m_snapEnabled || m_snapInterval <= 0.0)
    {
        return time;
    }
    return std::round(time / m_snapInterval) * m_snapInterval;
}

f32 TimelinePanel::evaluateTrack(const Track& track, f64 time) const
{
    if (track.keyframes.empty())
    {
        return 0.0f;
    }

    // Find surrounding keyframes
    auto upper = std::lower_bound(track.keyframes.begin(), track.keyframes.end(), time,
        [](const Keyframe& kf, f64 t) { return kf.time < t; });

    if (upper == track.keyframes.begin())
    {
        return track.keyframes.front().value;
    }

    if (upper == track.keyframes.end())
    {
        return track.keyframes.back().value;
    }

    auto lower = std::prev(upper);
    return interpolateKeyframes(*lower, *upper, time);
}

f32 TimelinePanel::interpolateKeyframes(const Keyframe& k1, const Keyframe& k2, f64 time) const
{
    if (k2.time == k1.time)
    {
        return k1.value;
    }

    f64 t = (time - k1.time) / (k2.time - k1.time);

    // Apply easing
    switch (k1.easing)
    {
        case EasingType::Linear:
            // t = t (no change)
            break;
        case EasingType::EaseIn:
            t = t * t;
            break;
        case EasingType::EaseOut:
            t = 1.0 - (1.0 - t) * (1.0 - t);
            break;
        case EasingType::EaseInOut:
            t = t < 0.5 ? 2.0 * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 2.0) / 2.0;
            break;
        case EasingType::Bounce:
            // Simplified bounce
            if (t < 1.0 / 2.75)
            {
                t = 7.5625 * t * t;
            }
            else if (t < 2.0 / 2.75)
            {
                t -= 1.5 / 2.75;
                t = 7.5625 * t * t + 0.75;
            }
            else if (t < 2.5 / 2.75)
            {
                t -= 2.25 / 2.75;
                t = 7.5625 * t * t + 0.9375;
            }
            else
            {
                t -= 2.625 / 2.75;
                t = 7.5625 * t * t + 0.984375;
            }
            break;
        case EasingType::Elastic:
            if (t == 0.0 || t == 1.0)
            {
                break;
            }
            t = std::pow(2.0, -10.0 * t) * std::sin((t - 0.075) * (2.0 * 3.14159) / 0.3) + 1.0;
            break;
        default:
            break;
    }

    return static_cast<f32>(k1.value + (k2.value - k1.value) * t);
}

} // namespace NovelMind::editor
