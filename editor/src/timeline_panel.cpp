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

    // Split into header and content regions
    // ImGui::BeginChild("TrackHeaders", ImVec2(m_headerWidth, 0), true);
    // renderTrackHeaders();
    // ImGui::EndChild();

    // ImGui::SameLine();

    // ImGui::BeginChild("TrackContent", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
    renderRuler();
    renderTrackContent();
    renderPlayhead();
    // ImGui::EndChild();

    // Curve editor overlay
    if (m_showCurveEditor)
    {
        renderCurveEditor();
    }
}

void TimelinePanel::renderToolbar()
{
    widgets::BeginToolbar("TimelineToolbar");
    renderToolbarItems(getToolbarItems());

    // Time display
    // ImGui::SameLine();
    // ImGui::Text("%.2f / %.2f", m_currentTime, m_duration);

    // Speed control
    // ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100);
    // ImGui::SetNextItemWidth(80);
    // ImGui::SliderFloat("Speed", &m_playbackSpeed, 0.1f, 4.0f, "%.1fx");

    widgets::EndToolbar();
}

void TimelinePanel::renderRuler()
{
    // ImDrawList* drawList = ImGui::GetWindowDrawList();
    // ImVec2 contentMin = ImGui::GetCursorScreenPos();

    // Draw ruler background
    // drawList->AddRectFilled(contentMin,
    //     ImVec2(contentMin.x + m_contentWidth, contentMin.y + m_rulerHeight),
    //     IM_COL32(m_rulerColor.r, m_rulerColor.g, m_rulerColor.b, m_rulerColor.a));

    // Draw time markers
    f64 timeRange = m_viewEndTime - m_viewStartTime;
    f64 majorInterval = 1.0;  // 1 second major marks
    if (timeRange < 2.0) majorInterval = 0.5;
    if (timeRange < 1.0) majorInterval = 0.1;

    // for (f64 t = std::ceil(m_viewStartTime / majorInterval) * majorInterval; t <= m_viewEndTime; t += majorInterval)
    // {
    //     f32 x = timeToPixel(t);
    //     drawList->AddLine(
    //         ImVec2(contentMin.x + x, contentMin.y + m_rulerHeight - 10),
    //         ImVec2(contentMin.x + x, contentMin.y + m_rulerHeight),
    //         IM_COL32(200, 200, 200, 255));
    //
    //     char label[32];
    //     snprintf(label, sizeof(label), "%.1f", t);
    //     drawList->AddText(ImVec2(contentMin.x + x + 2, contentMin.y + m_rulerHeight - 20), IM_COL32(200, 200, 200, 255), label);
    // }

    (void)timeRange;
    (void)majorInterval;
}

void TimelinePanel::renderTrackHeaders()
{
    for (size_t i = 0; i < m_tracks.size(); ++i)
    {
        const auto& track = m_tracks[i];

        // Draw track header
        // ImGui::PushID(static_cast<int>(i));

        // Expand/collapse button
        // if (ImGui::ArrowButton("##expand", track.expanded ? ImGuiDir_Down : ImGuiDir_Right))
        // {
        //     m_tracks[i].expanded = !m_tracks[i].expanded;
        // }

        // ImGui::SameLine();

        // Visibility toggle
        // if (ImGui::Checkbox("##visible", &m_tracks[i].visible))
        // {
        //     // Handle visibility change
        // }

        // ImGui::SameLine();

        // Track name
        // ImGui::TextColored(ImVec4(track.color.r / 255.0f, track.color.g / 255.0f, track.color.b / 255.0f, 1.0f),
        //     "%s", track.name.c_str());

        // Lock toggle
        // ImGui::SameLine();
        // if (ImGui::Button(track.locked ? "L" : "U"))
        // {
        //     m_tracks[i].locked = !m_tracks[i].locked;
        // }

        // ImGui::PopID();
        (void)track;
    }
}

void TimelinePanel::renderTrackContent()
{
    // ImDrawList* drawList = ImGui::GetWindowDrawList();
    // ImVec2 contentMin = ImGui::GetCursorScreenPos();

    f32 trackY = m_rulerHeight;

    for (size_t i = 0; i < m_tracks.size(); ++i)
    {
        const auto& track = m_tracks[i];

        // Draw track background
        renderer::Color bgColor = (i % 2 == 0) ? m_trackBackgroundColor : m_trackBackgroundAltColor;
        // drawList->AddRectFilled(
        //     ImVec2(contentMin.x, contentMin.y + trackY),
        //     ImVec2(contentMin.x + m_contentWidth, contentMin.y + trackY + m_trackHeight),
        //     IM_COL32(bgColor.r, bgColor.g, bgColor.b, bgColor.a));

        // Draw keyframes
        if (track.visible)
        {
            renderKeyframes(track, trackY, m_trackHeight);
        }

        trackY += m_trackHeight;
        (void)bgColor;
    }
}

void TimelinePanel::renderPlayhead()
{
    if (m_currentTime < m_viewStartTime || m_currentTime > m_viewEndTime)
    {
        return;
    }

    f32 x = timeToPixel(m_currentTime);

    // ImDrawList* drawList = ImGui::GetWindowDrawList();
    // ImVec2 contentMin = ImGui::GetCursorScreenPos();

    // Draw playhead line
    // drawList->AddLine(
    //     ImVec2(contentMin.x + x, contentMin.y),
    //     ImVec2(contentMin.x + x, contentMin.y + m_contentHeight),
    //     IM_COL32(m_playheadColor.r, m_playheadColor.g, m_playheadColor.b, m_playheadColor.a),
    //     2.0f);

    // Draw playhead triangle at top
    // drawList->AddTriangleFilled(
    //     ImVec2(contentMin.x + x - 6, contentMin.y),
    //     ImVec2(contentMin.x + x + 6, contentMin.y),
    //     ImVec2(contentMin.x + x, contentMin.y + 10),
    //     IM_COL32(m_playheadColor.r, m_playheadColor.g, m_playheadColor.b, m_playheadColor.a));

    (void)x;
}

void TimelinePanel::renderKeyframes(const Track& track, f32 trackY, f32 trackHeight)
{
    // ImDrawList* drawList = ImGui::GetWindowDrawList();
    // ImVec2 contentMin = ImGui::GetCursorScreenPos();

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

        // Draw keyframe diamond
        // drawList->AddQuadFilled(
        //     ImVec2(contentMin.x + x, contentMin.y + centerY - keyframeRadius),
        //     ImVec2(contentMin.x + x + keyframeRadius, contentMin.y + centerY),
        //     ImVec2(contentMin.x + x, contentMin.y + centerY + keyframeRadius),
        //     ImVec2(contentMin.x + x - keyframeRadius, contentMin.y + centerY),
        //     IM_COL32(color.r, color.g, color.b, color.a));

        (void)x;
        (void)centerY;
        (void)keyframeRadius;
        (void)color;
    }
}

void TimelinePanel::renderCurveEditor()
{
    // Overlay curve editor window
    // ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    // if (ImGui::Begin("Curve Editor", &m_showCurveEditor))
    // {
    //     // Draw curve for selected track/keyframes
    //     // Allow editing of tangent handles
    // }
    // ImGui::End();
}

void TimelinePanel::handleMouseInput()
{
    if (!m_isHovered)
    {
        return;
    }

    // Handle mouse interactions
    // - Click on ruler to scrub
    // - Click on keyframe to select
    // - Drag keyframes to move
    // - Middle mouse to pan
    // - Mouse wheel to zoom horizontally
}

void TimelinePanel::handleKeyboardInput()
{
    if (!m_isFocused)
    {
        return;
    }

    // Space: play/pause
    // if (ImGui::IsKeyPressed(ImGuiKey_Space)) togglePlayPause();

    // Home/End: go to start/end
    // if (ImGui::IsKeyPressed(ImGuiKey_Home)) goToStart();
    // if (ImGui::IsKeyPressed(ImGuiKey_End)) goToEnd();

    // Left/Right arrows: step
    // if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) stepBackward();
    // if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) stepForward();

    // Delete: delete selected keyframes
    // if (ImGui::IsKeyPressed(ImGuiKey_Delete)) deleteSelectedKeyframes();
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
