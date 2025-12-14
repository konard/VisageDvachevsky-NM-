#include "NovelMind/editor/qt/nm_icon_manager.hpp"
#include <QBuffer>
#include <QPainter>
#include <QSvgRenderer>

namespace NovelMind::editor::qt {

NMIconManager &NMIconManager::instance() {
  static NMIconManager instance;
  return instance;
}

NMIconManager::NMIconManager()
    : m_defaultColor(220, 220, 220) // Light gray for dark theme
{
  initializeIcons();
}

void NMIconManager::initializeIcons() {
  // =========================================================================
  // FILE OPERATIONS
  // =========================================================================
  m_iconSvgData["file-new"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M14 3h-2V1c0-.55-.45-1-1-1H1C.45 0 0 .45 0 "
      "1v14c0 .55.45 1 1 1h10c.55 0 1-.45 1-1v-2h2c.55 0 1-.45 "
      "1-1V4c0-.55-.45-1-1-1zM11 15H1V1h10v14zm3-3h-2V4h2v8z'/>"
      "<path fill='%COLOR%' d='M3 6h6v1H3zm0 2h6v1H3zm0 2h6v1H3z'/>"
      "</svg>";

  m_iconSvgData["file-open"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M14 4h-4.38l-1.3-1.3c-.2-.2-.45-.7-.7-.7H1c-.55 "
      "0-1 .45-1 1v10c0 .55.45 1 1 1h13c.55 0 1-.45 1-1V5c0-.55-.45-1-1-1zm0 "
      "9H1V3h5l2 2h6v8z'/>"
      "</svg>";

  m_iconSvgData["file-save"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M14 0H2C.9 0 0 .9 0 2v12c0 1.1.9 2 2 2h12c1.1 0 "
      "2-.9 2-2V2c0-1.1-.9-2-2-2zM5 2h6v3H5V2zm9 12H2V2h1v4h10V2h1v12z'/>"
      "<rect fill='%COLOR%' x='4' y='9' width='8' height='1'/>"
      "<rect fill='%COLOR%' x='4' y='11' width='8' height='1'/>"
      "<rect fill='%COLOR%' x='4' y='13' width='8' height='1'/>"
      "</svg>";

  m_iconSvgData["file-close"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M14 3h-2V1c0-.55-.45-1-1-1H1C.45 0 0 .45 0 "
      "1v14c0 .55.45 1 1 1h10c.55 0 1-.45 1-1v-2h2c.55 0 1-.45 "
      "1-1V4c0-.55-.45-1-1-1zM11 15H1V1h10v14z'/>"
      "<path fill='%COLOR%' d='M10.3 4.7l-1.4-1.4L8 4.2 7.1 3.3 5.7 4.7 6.6 "
      "5.6 5.7 6.5l1.4 1.4.9-.9.9.9 1.4-1.4-.9-.9z'/>"
      "</svg>";

  // =========================================================================
  // EDIT OPERATIONS
  // =========================================================================
  m_iconSvgData["edit-undo"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 1c-1.7 0-3.3.6-4.6 1.7L1 1v5h5L3.5 3.5C4.5 "
      "2.6 6.2 2 8 2c3.3 0 6 2.7 6 6s-2.7 6-6 6-6-2.7-6-6H1c0 3.9 3.1 7 7 "
      "7s7-3.1 7-7-3.1-7-7-7z'/>"
      "</svg>";

  m_iconSvgData["edit-redo"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 1c1.7 0 3.3.6 4.6 1.7L15 1v5h-5l2.5-2.5C11.5 "
      "2.6 9.8 2 8 2 4.7 2 2 4.7 2 8s2.7 6 6 6 6-2.7 6-6h1c0 3.9-3.1 7-7 "
      "7s-7-3.1-7-7 3.1-7 7-7z'/>"
      "</svg>";

  m_iconSvgData["edit-cut"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M13.5 3l-6 6-2-2L3 9.5c-.8.8-.8 2.1 0 2.8.8.8 "
      "2.1.8 2.8 0L8.5 9.7l2 2 5-5-2-3.7zM4.5 11c-.3 "
      "0-.5-.2-.5-.5s.2-.5.5-.5.5.2.5.5-.2.5-.5.5z'/>"
      "<circle fill='%COLOR%' cx='4.5' cy='3.5' r='1.5'/>"
      "</svg>";

  m_iconSvgData["edit-copy"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M13 0H3C2.4 0 2 .4 2 1v2H0v11c0 .6.4 1 1 "
      "1h10c.6 0 1-.4 1-1v-2h2c.6 0 1-.4 1-1V1c0-.6-.4-1-1-1zm-2 "
      "14H1V4h10v10zm2-3h-1V4c0-.6-.4-1-1-1H4V1h9v10z'/>"
      "</svg>";

  m_iconSvgData["edit-paste"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M11 2h1c.6 0 1 .4 1 1v11c0 .6-.4 1-1 1H4c-.6 "
      "0-1-.4-1-1V3c0-.6.4-1 1-1h1V1h6v1zM5 4v10h6V4H5z'/>"
      "<rect fill='%COLOR%' x='6' y='0' width='4' height='2' rx='1'/>"
      "</svg>";

  m_iconSvgData["edit-delete"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M13 3h-2V1c0-.55-.45-1-1-1H6c-.55 0-1 .45-1 "
      "1v2H3c-.55 0-1 .45-1 1v1h12V4c0-.55-.45-1-1-1zM6 1h4v2H6V1z'/>"
      "<path fill='%COLOR%' d='M4 6v9c0 .55.45 1 1 1h6c.55 0 1-.45 1-1V6H4zm2 "
      "8H5V8h1v6zm2 0H7V8h1v6zm2 0H9V8h1v6z'/>"
      "</svg>";

  // =========================================================================
  // PLAYBACK CONTROLS
  // =========================================================================
  m_iconSvgData["play"] = "<svg viewBox='0 0 16 16'>"
                          "<path fill='%COLOR%' d='M3 2v12l10-6z'/>"
                          "</svg>";

  m_iconSvgData["pause"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='3' y='2' width='3' height='12'/>"
      "<rect fill='%COLOR%' x='10' y='2' width='3' height='12'/>"
      "</svg>";

  m_iconSvgData["stop"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='3' y='3' width='10' height='10'/>"
      "</svg>";

  m_iconSvgData["step-forward"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M2 2v12l9-6z'/>"
      "<rect fill='%COLOR%' x='12' y='2' width='2' height='12'/>"
      "</svg>";

  m_iconSvgData["step-backward"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M14 2v12L5 8z'/>"
      "<rect fill='%COLOR%' x='2' y='2' width='2' height='12'/>"
      "</svg>";

  // =========================================================================
  // PANEL ICONS
  // =========================================================================
  m_iconSvgData["panel-scene"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='2' y='2' width='12' height='12' rx='1' "
      "fill='none' stroke='%COLOR%' stroke-width='1.5'/>"
      "<circle fill='%COLOR%' cx='8' cy='6' r='2'/>"
      "<path fill='%COLOR%' d='M4 14l3-4 2 2 3-4 2 2v4z'/>"
      "</svg>";

  m_iconSvgData["panel-graph"] =
      "<svg viewBox='0 0 16 16'>"
      "<circle fill='%COLOR%' cx='3' cy='8' r='2'/>"
      "<circle fill='%COLOR%' cx='13' cy='8' r='2'/>"
      "<circle fill='%COLOR%' cx='8' cy='4' r='2'/>"
      "<path fill='none' stroke='%COLOR%' stroke-width='1.5' d='M5 8h6M5 "
      "7l3-2M11 7l-3-2'/>"
      "</svg>";

  m_iconSvgData["panel-inspector"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='2' y='2' width='12' height='2'/>"
      "<rect fill='%COLOR%' x='2' y='6' width='12' height='2'/>"
      "<rect fill='%COLOR%' x='2' y='10' width='12' height='2'/>"
      "<circle fill='%COLOR%' cx='12' cy='3' r='1.5'/>"
      "<circle fill='%COLOR%' cx='12' cy='7' r='1.5'/>"
      "<circle fill='%COLOR%' cx='12' cy='11' r='1.5'/>"
      "</svg>";

  m_iconSvgData["panel-console"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='1' y='1' width='14' height='14' rx='1' "
      "fill='none' stroke='%COLOR%' stroke-width='1.5'/>"
      "<path fill='%COLOR%' d='M3 5l3 2-3 2v-4z'/>"
      "<rect fill='%COLOR%' x='8' y='8' width='5' height='1'/>"
      "</svg>";

  m_iconSvgData["panel-hierarchy"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='2' y='2' width='4' height='2'/>"
      "<rect fill='%COLOR%' x='4' y='6' width='4' height='2'/>"
      "<rect fill='%COLOR%' x='4' y='10' width='4' height='2'/>"
      "<rect fill='%COLOR%' x='10' y='6' width='4' height='2'/>"
      "<path fill='none' stroke='%COLOR%' stroke-width='1' d='M4 4h2M6 4v3M6 "
      "7h2M6 7v4M6 11h2M8 7h2'/>"
      "</svg>";

  m_iconSvgData["panel-assets"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='1' y='1' width='6' height='6' rx='0.5'/>"
      "<rect fill='%COLOR%' x='9' y='1' width='6' height='6' rx='0.5'/>"
      "<rect fill='%COLOR%' x='1' y='9' width='6' height='6' rx='0.5'/>"
      "<rect fill='%COLOR%' x='9' y='9' width='6' height='6' rx='0.5'/>"
      "</svg>";

  m_iconSvgData["panel-timeline"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='1' y='3' width='14' height='2'/>"
      "<rect fill='%COLOR%' x='1' y='7' width='14' height='2'/>"
      "<rect fill='%COLOR%' x='1' y='11' width='14' height='2'/>"
      "<rect fill='%COLOR%' x='3' y='3' width='1' height='10'/>"
      "<circle fill='%COLOR%' cx='6' cy='4' r='1'/>"
      "<circle fill='%COLOR%' cx='9' cy='8' r='1'/>"
      "<circle fill='%COLOR%' cx='12' cy='12' r='1'/>"
      "</svg>";

  m_iconSvgData["panel-curve"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='none' stroke='%COLOR%' stroke-width='1.5' d='M2 14Q5 2 8 "
      "8T14 2'/>"
      "<circle fill='%COLOR%' cx='2' cy='14' r='1.5'/>"
      "<circle fill='%COLOR%' cx='8' cy='8' r='1.5'/>"
      "<circle fill='%COLOR%' cx='14' cy='2' r='1.5'/>"
      "</svg>";

  m_iconSvgData["panel-voice"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='7' y='2' width='2' height='7' rx='1'/>"
      "<path fill='%COLOR%' d='M4 7v1c0 2.2 1.8 4 4 4s4-1.8 4-4V7h1v1c0 "
      "2.5-1.8 4.6-4.2 4.9V15h2v1H5v-1h2v-2.1C4.8 12.6 3 10.5 3 8V7h1z'/>"
      "</svg>";

  m_iconSvgData["panel-localization"] =
      "<svg viewBox='0 0 16 16'>"
      "<circle fill='none' stroke='%COLOR%' stroke-width='1.5' cx='8' cy='8' "
      "r='6'/>"
      "<path fill='none' stroke='%COLOR%' stroke-width='1.5' d='M2 8h12M8 "
      "2c-1.5 0-3 2.7-3 6s1.5 6 3 6 3-2.7 3-6-1.5-6-3-6z'/>"
      "</svg>";

  m_iconSvgData["panel-diagnostics"] =
      "<svg viewBox='0 0 16 16'>"
      "<circle fill='none' stroke='%COLOR%' stroke-width='1.5' cx='8' cy='8' "
      "r='6'/>"
      "<path fill='%COLOR%' d='M7.5 4h1v5h-1z'/>"
      "<circle fill='%COLOR%' cx='8' cy='11' r='0.8'/>"
      "</svg>";

  m_iconSvgData["panel-build"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M13.8 6.2l-4-4c-.3-.3-.7-.2-1 0l-1 1c-.3.3-.3.7 "
      "0 1l1.3 1.3-5.4 5.4-1.3-1.3c-.3-.3-.7-.3-1 0l-1 1c-.3.3-.3.7 0 1l4 "
      "4c.3.3.7.3 1 0l1-1c.3-.3.3-.7 0-1l-1.3-1.3 5.4-5.4 1.3 1.3c.3.3.7.3 1 "
      "0l1-1c.3-.3.3-.7 0-1z'/>"
      "</svg>";

  // =========================================================================
  // ZOOM AND VIEW CONTROLS
  // =========================================================================
  m_iconSvgData["zoom-in"] =
      "<svg viewBox='0 0 16 16'>"
      "<circle fill='none' stroke='%COLOR%' stroke-width='1.5' cx='6.5' "
      "cy='6.5' r='4.5'/>"
      "<path fill='%COLOR%' d='M9.5 11l4.5 4.5-1 1L8.5 12z'/>"
      "<path fill='%COLOR%' d='M6 4v2h2v1H6v2H5V7H3V6h2V4z'/>"
      "</svg>";

  m_iconSvgData["zoom-out"] =
      "<svg viewBox='0 0 16 16'>"
      "<circle fill='none' stroke='%COLOR%' stroke-width='1.5' cx='6.5' "
      "cy='6.5' r='4.5'/>"
      "<path fill='%COLOR%' d='M9.5 11l4.5 4.5-1 1L8.5 12z'/>"
      "<rect fill='%COLOR%' x='3' y='6' width='7' height='1'/>"
      "</svg>";

  m_iconSvgData["zoom-fit"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='none' stroke='%COLOR%' stroke-width='1.5' x='2' y='2' "
      "width='12' height='12'/>"
      "<path fill='%COLOR%' d='M5 5h2V4H4v3h1zM11 5h-2V4h3v3h-1zM11 "
      "11h-2v1h3v-3h-1zM5 11h2v1H4v-3h1z'/>"
      "</svg>";

  // =========================================================================
  // UTILITY ICONS
  // =========================================================================
  m_iconSvgData["settings"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M14.5 "
      "6.6l-1.4-.4c-.1-.3-.2-.6-.4-.9l.7-1.3c.2-.4.1-.8-.2-1.1l-.8-.8c-.3-.3-."
      "7-.4-1.1-.2l-1.3.7c-.3-.2-.6-.3-.9-.4L8.7 1c-.1-.5-.5-.8-1-.8h-1.1c-.5 "
      "0-.9.3-1 .8l-.4 1.4c-.3.1-.6.2-.9.4L3 "
      "2.1c-.4-.2-.8-.1-1.1.2l-.8.8c-.3.3-.4.7-.2 1.1l.7 "
      "1.3c-.2.3-.3.6-.4.9L.2 6.8c-.5.1-.8.5-.8 1v1.1c0 .5.3.9.8 "
      "1l1.4.4c.1.3.2.6.4.9l-.7 1.3c-.2.4-.1.8.2 1.1l.8.8c.3.3.7.4 "
      "1.1.2l1.3-.7c.3.2.6.3.9.4l.4 1.4c.1.5.5.8 1 .8h1.1c.5 0 .9-.3 "
      "1-.8l.4-1.4c.3-.1.6-.2.9-.4l1.3.7c.4.2.8.1 "
      "1.1-.2l.8-.8c.3-.3.4-.7.2-1.1l-.7-1.3c.2-.3.3-.6.4-.9l1.4-.4c.5-.1.8-.5."
      "8-1V7.6c0-.5-.3-.9-.8-1zM8 11c-1.7 0-3-1.3-3-3s1.3-3 3-3 3 1.3 3 3-1.3 "
      "3-3 3z'/>"
      "</svg>";

  m_iconSvgData["help"] =
      "<svg viewBox='0 0 16 16'>"
      "<circle fill='none' stroke='%COLOR%' stroke-width='1.5' cx='8' cy='8' "
      "r='6'/>"
      "<path fill='%COLOR%' d='M8 6c-.8 0-1.5.7-1.5 1.5h-1C5.5 6.1 6.6 5 8 "
      "5s2.5 1.1 2.5 2.5c0 1-.6 1.8-1.5 2.2v.8h-1v-1.2c0-.3.2-.5.5-.5.8 0 "
      "1.5-.7 1.5-1.5S8.8 5.8 8 5.8z'/>"
      "<circle fill='%COLOR%' cx='8' cy='12' r='0.8'/>"
      "</svg>";

  m_iconSvgData["search"] =
      "<svg viewBox='0 0 16 16'>"
      "<circle fill='none' stroke='%COLOR%' stroke-width='1.5' cx='6.5' "
      "cy='6.5' r='4.5'/>"
      "<path fill='%COLOR%' d='M9.5 11l4.5 4.5-1 1L8.5 12z'/>"
      "</svg>";

  m_iconSvgData["filter"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M1 2h14l-5.5 6.5v4.3l-3 1.5V8.5z'/>"
      "</svg>";

  m_iconSvgData["refresh"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M14 2v5h-5l2-2C9.7 3.7 8 3 8 3 4.7 3 2 5.7 2 "
      "9s2.7 6 6 6c2.8 0 5.2-2 5.8-4.6h1.1C14.3 13.6 11.4 16 8 16c-3.9 "
      "0-7-3.1-7-7s3.1-7 7-7c1.9 0 3.6.8 4.9 2l2-2z'/>"
      "</svg>";

  m_iconSvgData["add"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M14 7H9V2H7v5H2v2h5v5h2V9h5z'/>"
      "</svg>";

  m_iconSvgData["remove"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='2' y='7' width='12' height='2'/>"
      "</svg>";

  m_iconSvgData["warning"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 1L1 14h14L8 1zm0 4h1v5H8V5zm.5 7c-.3 "
      "0-.5-.2-.5-.5s.2-.5.5-.5.5.2.5.5-.2.5-.5.5z'/>"
      "</svg>";

  m_iconSvgData["error"] = "<svg viewBox='0 0 16 16'>"
                           "<circle fill='%COLOR%' cx='8' cy='8' r='7'/>"
                           "<path fill='#2d2d2d' d='M7.5 4h1v5h-1z'/>"
                           "<circle fill='#2d2d2d' cx='8' cy='11' r='0.8'/>"
                           "</svg>";

  m_iconSvgData["info"] = "<svg viewBox='0 0 16 16'>"
                          "<circle fill='none' stroke='%COLOR%' "
                          "stroke-width='1.5' cx='8' cy='8' r='6'/>"
                          "<circle fill='%COLOR%' cx='8' cy='5' r='0.8'/>"
                          "<path fill='%COLOR%' d='M7.5 7h1v5h-1z'/>"
                          "</svg>";

  // =========================================================================
  // ARROW AND NAVIGATION
  // =========================================================================
  m_iconSvgData["arrow-up"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 3L3 8l1.4 1.4L7 6.8V13h2V6.8l2.6 2.6L13 8z'/>"
      "</svg>";

  m_iconSvgData["arrow-down"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 13l5-5-1.4-1.4L9 9.2V3H7v6.2L4.4 6.6 3 8z'/>"
      "</svg>";

  m_iconSvgData["arrow-left"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M3 8l5-5 1.4 1.4L6.8 7H13v2H6.8l2.6 2.6L8 13z'/>"
      "</svg>";

  m_iconSvgData["arrow-right"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M13 8l-5 5-1.4-1.4L9.2 9H3V7h6.2L6.6 4.4 8 3z'/>"
      "</svg>";

  m_iconSvgData["chevron-up"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 5l6 6-1 1-5-5-5 5-1-1z'/>"
      "</svg>";

  m_iconSvgData["chevron-down"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 11L2 5l1-1 5 5 5-5 1 1z'/>"
      "</svg>";

  m_iconSvgData["chevron-left"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M5 8l6-6 1 1-5 5 5 5-1 1z'/>"
      "</svg>";

  m_iconSvgData["chevron-right"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M11 8L5 2 4 3l5 5-5 5 1 1z'/>"
      "</svg>";

  // =========================================================================
  // GIZMO/TRANSFORM ICONS
  // =========================================================================
  m_iconSvgData["gizmo-move"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 1l3 3H9v3h3V5l3 3-3 3V9H9v3h2l-3 3-3-3h2V9H4v2l-3-3 3-3v2h3V4H5l3-3z'/>"
      "</svg>";

  m_iconSvgData["gizmo-rotate"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 1c3.9 0 7 3.1 7 7h-2c0-2.8-2.2-5-5-5S3 5.2 3 8s2.2 5 5 5v2c-3.9 0-7-3.1-7-7s3.1-7 7-7z'/>"
      "<path fill='%COLOR%' d='M11 8l3-3v6z'/>"
      "</svg>";

  m_iconSvgData["gizmo-scale"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='2' y='2' width='4' height='4'/>"
      "<rect fill='%COLOR%' x='10' y='2' width='4' height='4'/>"
      "<rect fill='%COLOR%' x='2' y='10' width='4' height='4'/>"
      "<rect fill='%COLOR%' x='10' y='10' width='4' height='4'/>"
      "<path fill='%COLOR%' d='M7 4h2v8H7zM4 7h8v2H4z'/>"
      "</svg>";

  // =========================================================================
  // NODE TYPE ICONS
  // =========================================================================
  m_iconSvgData["node-entry"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M3 2v12l10-6z'/>"
      "</svg>";

  m_iconSvgData["node-dialogue"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M2 2h12c.6 0 1 .4 1 1v8c0 .6-.4 1-1 1h-3l-3 3v-3H2c-.6 0-1-.4-1-1V3c0-.6.4-1 1-1z'/>"
      "<rect fill='#2d2d2d' x='4' y='5' width='8' height='1'/>"
      "<rect fill='#2d2d2d' x='4' y='7' width='5' height='1'/>"
      "</svg>";

  m_iconSvgData["node-choice"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 1l7 7-7 7-7-7z'/>"
      "<path fill='#2d2d2d' d='M8 5v2h2v2H8v2l-3-3z'/>"
      "</svg>";

  m_iconSvgData["node-scene"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='1' y='2' width='14' height='10' rx='1'/>"
      "<rect fill='#2d2d2d' x='2' y='3' width='12' height='8'/>"
      "<circle fill='%COLOR%' cx='5' cy='6' r='2'/>"
      "<path fill='%COLOR%' d='M3 11l3-3 2 1 3-4 3 3v3z'/>"
      "</svg>";

  m_iconSvgData["node-label"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M14 2H2c-.6 0-1 .4-1 1v10c0 .6.4 1 1 1h12c.6 0 1-.4 1-1V3c0-.6-.4-1-1-1zM2 13V3h12v10H2z'/>"
      "<rect fill='%COLOR%' x='4' y='6' width='8' height='1'/>"
      "<rect fill='%COLOR%' x='4' y='9' width='5' height='1'/>"
      "</svg>";

  m_iconSvgData["node-script"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M4 2L1 5v9h9l3-3V2H4zM3 5l2-2h7v7l-2 2H3V5z'/>"
      "<rect fill='%COLOR%' x='4' y='5' width='5' height='1'/>"
      "<rect fill='%COLOR%' x='4' y='7' width='6' height='1'/>"
      "<rect fill='%COLOR%' x='4' y='9' width='4' height='1'/>"
      "</svg>";

  // =========================================================================
  // OBJECT TYPE ICONS
  // =========================================================================
  m_iconSvgData["object-background"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='1' y='2' width='14' height='12' rx='1'/>"
      "<circle fill='%COLOR%' cx='12' cy='5' r='1.5' opacity='0.6'/>"
      "<path fill='%COLOR%' d='M1 11l4-4 2 2 4-5 4 5v5H1z' opacity='0.6'/>"
      "</svg>";

  m_iconSvgData["object-character"] =
      "<svg viewBox='0 0 16 16'>"
      "<circle fill='%COLOR%' cx='8' cy='4' r='3'/>"
      "<path fill='%COLOR%' d='M8 8c-3 0-5 2-5 4v2h10v-2c0-2-2-4-5-4z'/>"
      "</svg>";

  m_iconSvgData["object-ui"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='1' y='1' width='14' height='14' rx='1' fill='none' stroke='%COLOR%' stroke-width='1.5'/>"
      "<rect fill='%COLOR%' x='3' y='10' width='10' height='3' rx='0.5'/>"
      "</svg>";

  m_iconSvgData["object-effect"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 1l1.5 4H14l-3.5 3 1.5 5L8 10l-4 3 1.5-5L2 5h4.5z'/>"
      "</svg>";

  // =========================================================================
  // FOLDER AND ASSET ICONS
  // =========================================================================
  m_iconSvgData["folder"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M14 4h-4.38l-1.3-1.3c-.2-.2-.45-.3-.7-.3H2c-.55 0-1 .45-1 1v10c0 .55.45 1 1 1h12c.55 0 1-.45 1-1V5c0-.55-.45-1-1-1z'/>"
      "</svg>";

  m_iconSvgData["folder-open"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M14 4h-4.38l-1.3-1.3c-.2-.2-.45-.3-.7-.3H2c-.55 0-1 .45-1 1v10c0 .55.45 1 1 1h12c.55 0 1-.45 1-1V5c0-.55-.45-1-1-1zm0 9H2V6h12v7z'/>"
      "</svg>";

  m_iconSvgData["asset-image"] =
      "<svg viewBox='0 0 16 16'>"
      "<rect fill='%COLOR%' x='1' y='2' width='14' height='12' rx='1'/>"
      "<circle fill='#2d2d2d' cx='4' cy='5' r='1.5'/>"
      "<path fill='#2d2d2d' d='M1 11l3-3 2 1 4-4 5 4v5H1z'/>"
      "</svg>";

  m_iconSvgData["asset-audio"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M11 1v12.7c-.3-.2-.7-.3-1.1-.3-1.3 0-2.4.9-2.4 2.1s1.1 2.1 2.4 2.1 2.4-.9 2.4-2.1V4h3V1h-4.3z'/>"
      "</svg>";

  m_iconSvgData["asset-script"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M10 0H3c-.55 0-1 .45-1 1v14c0 .55.45 1 1 1h10c.55 0 1-.45 1-1V4l-4-4zM3 15V1h6v4h4v10H3z'/>"
      "<rect fill='%COLOR%' x='4' y='7' width='6' height='1'/>"
      "<rect fill='%COLOR%' x='4' y='9' width='8' height='1'/>"
      "<rect fill='%COLOR%' x='4' y='11' width='5' height='1'/>"
      "</svg>";

  // =========================================================================
  // BREAKPOINT AND DEBUG ICONS
  // =========================================================================
  m_iconSvgData["breakpoint"] =
      "<svg viewBox='0 0 16 16'>"
      "<circle fill='%COLOR%' cx='8' cy='8' r='6'/>"
      "</svg>";

  m_iconSvgData["breakpoint-disabled"] =
      "<svg viewBox='0 0 16 16'>"
      "<circle fill='none' stroke='%COLOR%' stroke-width='2' cx='8' cy='8' r='5'/>"
      "</svg>";

  m_iconSvgData["debug-continue"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M3 2v12l8-6z'/>"
      "<rect fill='%COLOR%' x='12' y='2' width='2' height='12'/>"
      "</svg>";

  m_iconSvgData["debug-step-into"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 2v7l-3-3-1 1 4.5 4.5L13 7l-1-1-3 3V2z'/>"
      "<rect fill='%COLOR%' x='3' y='13' width='10' height='1'/>"
      "</svg>";

  m_iconSvgData["debug-step-over"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M2 8c0-3.3 2.7-6 6-6v2c-2.2 0-4 1.8-4 4s1.8 4 4 4v-2l4 3-4 3v-2c-3.3 0-6-2.7-6-6z'/>"
      "</svg>";

  m_iconSvgData["debug-step-out"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 14V7l3 3 1-1-4.5-4.5L3 9l1 1 3-3v7z'/>"
      "<rect fill='%COLOR%' x='3' y='2' width='10' height='1'/>"
      "</svg>";

  // =========================================================================
  // VISIBILITY AND LOCK ICONS
  // =========================================================================
  m_iconSvgData["eye-open"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M8 3C3 3 0 8 0 8s3 5 8 5 8-5 8-5-3-5-8-5zm0 8c-1.7 0-3-1.3-3-3s1.3-3 3-3 3 1.3 3 3-1.3 3-3 3z'/>"
      "<circle fill='%COLOR%' cx='8' cy='8' r='1.5'/>"
      "</svg>";

  m_iconSvgData["eye-closed"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M14.6 2l-1.4-1.4L2 11.8l1.4 1.4L14.6 2z'/>"
      "<path fill='%COLOR%' d='M8 3C4.5 3 2 5.5 0 8c.6.8 1.5 1.8 2.6 2.6l1.5-1.5C3.4 8.4 3 7.3 3 6c0-1.9 1.6-3.5 3.5-3.5.9 0 1.7.4 2.3 1l1.5-1.5C9.4 2 8.7 2 8 2zM8 11c-.9 0-1.7-.4-2.3-1L4.2 11.5c.8.3 1.6.5 2.3.5 3.5 0 6-2.5 8-5-.6-.8-1.5-1.8-2.6-2.6l-1.5 1.5c.7.7 1.1 1.8 1.1 3 0 1.9-1.6 3.5-3.5 3.5z'/>"
      "</svg>";

  m_iconSvgData["lock"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M12 6V4c0-2.2-1.8-4-4-4S4 1.8 4 4v2c-1.1 0-2 .9-2 2v6c0 1.1.9 2 2 2h8c1.1 0 2-.9 2-2V8c0-1.1-.9-2-2-2zM6 4c0-1.1.9-2 2-2s2 .9 2 2v2H6V4zm6 10H4V8h8v6z'/>"
      "</svg>";

  m_iconSvgData["unlock"] =
      "<svg viewBox='0 0 16 16'>"
      "<path fill='%COLOR%' d='M12 6H5V4c0-1.1.9-2 2-2s2 .9 2 2h2c0-2.2-1.8-4-4-4S3 1.8 3 4v2c-1.1 0-2 .9-2 2v6c0 1.1.9 2 2 2h9c1.1 0 2-.9 2-2V8c0-1.1-.9-2-2-2zm0 8H3V8h9v6z'/>"
      "</svg>";
}

QIcon NMIconManager::getIcon(const QString &iconName, int size,
                             const QColor &color) {
  QColor iconColor = color.isValid() ? color : m_defaultColor;

  QString cacheKey =
      QString("%1_%2_%3").arg(iconName).arg(size).arg(iconColor.name());

  if (m_iconCache.contains(cacheKey)) {
    return m_iconCache[cacheKey];
  }

  QPixmap pixmap = getPixmap(iconName, size, iconColor);
  QIcon icon(pixmap);
  m_iconCache[cacheKey] = icon;

  return icon;
}

QPixmap NMIconManager::getPixmap(const QString &iconName, int size,
                                 const QColor &color) {
  QString svgData = getSvgData(iconName);
  if (svgData.isEmpty()) {
    // Return empty pixmap if icon not found
    return QPixmap(size, size);
  }

  QColor iconColor = color.isValid() ? color : m_defaultColor;
  return renderSvg(svgData, size, iconColor);
}

void NMIconManager::clearCache() { m_iconCache.clear(); }

void NMIconManager::setDefaultColor(const QColor &color) {
  if (m_defaultColor != color) {
    m_defaultColor = color;
    clearCache(); // Clear cache when color changes
  }
}

QString NMIconManager::getSvgData(const QString &iconName) {
  return m_iconSvgData.value(iconName, QString());
}

QPixmap NMIconManager::renderSvg(const QString &svgData, int size,
                                 const QColor &color) {
  // Replace color placeholder with actual color
  QString coloredSvg = svgData;
  coloredSvg.replace("%COLOR%", color.name());

  // Create SVG renderer
  QByteArray svgBytes = coloredSvg.toUtf8();
  QSvgRenderer renderer(svgBytes);

  // Create pixmap
  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);

  // Render SVG
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);
  renderer.render(&painter);

  return pixmap;
}

} // namespace NovelMind::editor::qt
