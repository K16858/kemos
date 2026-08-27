#include "readline.hpp"

namespace {

constexpr size_t kReadLineMax = 128;

Console* g_console = nullptr;
char g_buffer[kReadLineMax];
size_t g_length = 0;
volatile bool g_line_ready = false;

void Echo(char c) {
  if (g_console == nullptr) {
    return;
  }
  char buf[2] = {c, '\0'};
  g_console->PutString(buf);
}

}  // namespace

void InitializeReadLine(Console* console) {
  g_console = console;
  g_length = 0;
  g_line_ready = false;
  for (size_t i = 0; i < kReadLineMax; ++i) {
    g_buffer[i] = 0;
  }
}

void ReadLinePushChar(char c) {
  if (g_line_ready) {
    return;
  }

  if (c == '\n') {
    g_buffer[g_length] = '\0';
    Echo('\n');
    g_line_ready = true;
    return;
  }

  if (c == '\b') {
    if (g_length > 0) {
      --g_length;
      g_buffer[g_length] = '\0';
      Echo('\b');
    }
    return;
  }

  if (g_length + 1 >= kReadLineMax) {
    return;
  }
  g_buffer[g_length++] = c;
  g_buffer[g_length] = '\0';
  Echo(c);
}

bool ReadLineTryGetLine(char* out, size_t max_len) {
  if (!g_line_ready || out == nullptr || max_len == 0) {
    return false;
  }

  size_t i = 0;
  for (; i + 1 < max_len && g_buffer[i] != '\0'; ++i) {
    out[i] = g_buffer[i];
  }
  out[i] = '\0';

  g_length = 0;
  g_buffer[0] = '\0';
  g_line_ready = false;
  return true;
}
