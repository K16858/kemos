#include "shell/printk.hpp"

#include <cstdarg>
#include <cstdint>

namespace {

Console* g_log_console = nullptr;

void PutChar(char c) {
  if (g_log_console == nullptr) {
    return;
  }
  char buf[2] = {c, '\0'};
  g_log_console->PutString(buf);
}

void PutString(const char* s) {
  if (g_log_console == nullptr || s == nullptr) {
    return;
  }
  g_log_console->PutString(s);
}

void PutUint(uint64_t value, unsigned base, bool uppercase) {
  char buf[32];
  int i = 0;
  if (value == 0) {
    PutChar('0');
    return;
  }
  while (value > 0) {
    const uint64_t digit = value % base;
    if (digit < 10) {
      buf[i++] = static_cast<char>('0' + digit);
    } else {
      buf[i++] = static_cast<char>((uppercase ? 'A' : 'a') + (digit - 10));
    }
    value /= base;
  }
  while (i > 0) {
    PutChar(buf[--i]);
  }
}

void PutInt(int64_t value) {
  if (value < 0) {
    PutChar('-');
    PutUint(static_cast<uint64_t>(-value), 10, false);
  } else {
    PutUint(static_cast<uint64_t>(value), 10, false);
  }
}

}  // namespace

void SetLogConsole(Console* console) {
  g_log_console = console;
}

int printk(const char* format, ...) {
  if (format == nullptr) {
    return 0;
  }

  va_list args;
  va_start(args, format);
  int written = 0;

  for (const char* p = format; *p != '\0'; ++p) {
    if (*p != '%') {
      PutChar(*p);
      ++written;
      continue;
    }

    ++p;
    if (*p == '\0') {
      break;
    }

    switch (*p) {
      case '%':
        PutChar('%');
        ++written;
        break;
      case 'c': {
        const char c = static_cast<char>(va_arg(args, int));
        PutChar(c);
        ++written;
        break;
      }
      case 's': {
        const char* s = va_arg(args, const char*);
        if (s == nullptr) {
          s = "(null)";
        }
        for (const char* q = s; *q != '\0'; ++q) {
          PutChar(*q);
          ++written;
        }
        break;
      }
      case 'd':
        PutInt(va_arg(args, int));
        ++written;
        break;
      case 'u':
        PutUint(va_arg(args, unsigned int), 10, false);
        ++written;
        break;
      case 'x':
        PutUint(va_arg(args, unsigned int), 16, false);
        ++written;
        break;
      case 'p': {
        PutString("0x");
        PutUint(reinterpret_cast<uint64_t>(va_arg(args, void*)), 16, false);
        written += 2;
        break;
      }
      default:
        PutChar('%');
        PutChar(*p);
        written += 2;
        break;
    }
  }

  va_end(args);
  return written;
}
