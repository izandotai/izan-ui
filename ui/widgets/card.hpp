#pragma once

namespace izan::ui {

// Inset grouped card (System Settings style): rounded, slightly
// elevated, own padding. Height fits content. Rows inside separate
// with kit_hairline().
void kit_group_begin(const char* id, float width = 0.0f);
void kit_group_end();
void kit_hairline();

}
