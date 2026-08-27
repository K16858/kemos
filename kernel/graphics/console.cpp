#include "graphics/console.hpp"

#include "graphics/font.hpp"

Console::Console(PixelWriter& writer, const PixelColor& fg_color,
                 const PixelColor& bg_color)
    : writer_{writer},
      fg_color_{fg_color},
      bg_color_{bg_color},
      cursor_row_{0},
      cursor_column_{0} {
  for (int row = 0; row < kRows; ++row) {
    for (int col = 0; col < kColumns + 1; ++col) {
      buffer_[row][col] = 0;
    }
  }
}

void Console::PutString(const char* s) {
  while (*s) {
    if (*s == '\n') {
      NewLine();
    } else if (*s == '\b') {
      Backspace();
    } else {
      WriteAscii(writer_, 8 * cursor_column_, 16 * cursor_row_, *s, fg_color_);
      buffer_[cursor_row_][cursor_column_] = *s;
      if (++cursor_column_ == kColumns) {
        NewLine();
      }
    }
    ++s;
  }
}

void Console::Backspace() {
  if (cursor_column_ == 0) {
    return;
  }
  --cursor_column_;
  for (int dy = 0; dy < 16; ++dy) {
    for (int dx = 0; dx < 8; ++dx) {
      writer_.Write(8 * cursor_column_ + dx, 16 * cursor_row_ + dy, bg_color_);
    }
  }
  buffer_[cursor_row_][cursor_column_] = 0;
}

void Console::Clear() {
  for (int y = 0; y < 16 * kRows; ++y) {
    for (int x = 0; x < 8 * kColumns; ++x) {
      writer_.Write(x, y, bg_color_);
    }
  }
  cursor_row_ = 0;
  cursor_column_ = 0;
  for (int row = 0; row < kRows; ++row) {
    for (int col = 0; col < kColumns + 1; ++col) {
      buffer_[row][col] = 0;
    }
  }
}

void Console::NewLine() {
  cursor_column_ = 0;
  if (cursor_row_ < kRows - 1) {
    ++cursor_row_;
    return;
  }

  for (int y = 0; y < 16 * kRows; ++y) {
    for (int x = 0; x < 8 * kColumns; ++x) {
      writer_.Write(x, y, bg_color_);
    }
  }
  for (int row = 0; row < kRows - 1; ++row) {
    for (int col = 0; col < kColumns + 1; ++col) {
      buffer_[row][col] = buffer_[row + 1][col];
    }
    WriteString(writer_, 0, 16 * row, buffer_[row], fg_color_);
  }
  for (int col = 0; col < kColumns + 1; ++col) {
    buffer_[kRows - 1][col] = 0;
  }
}
