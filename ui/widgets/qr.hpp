#pragma once

namespace izan::ui {

// A QR code for a text payload, drawn as a white card with black
// modules and a proper quiet zone — scanners need contrast, whatever
// the theme says. Sized to roughly size_em, snapped so every module
// lands on whole pixels.
void kit_qr(const char* text, float size_em);

}
