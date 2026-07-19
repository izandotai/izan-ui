#pragma once

namespace izan::ui {

// A link that reads as a link: accent text, underline and hand cursor
// on hover, the system browser on click, the URL copied on
// right-click. Long labels middle-elide to the available width; the
// tooltip always carries the full URL for review before the jump.
void kit_hyperlink(const char* id, const char* label, const char* url);

// Hands a URL to the system browser — the hyperlink's click, exported
// for rows and buttons that open pages without wearing link clothes.
void kit_open_url(const char* url);

}
