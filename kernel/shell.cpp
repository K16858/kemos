#include "shell.hpp"

#include "printk.hpp"
#include "readline.hpp"

namespace {

Console* g_console = nullptr;

bool StrEq(const char* a, const char* b) {
  while (*a != '\0' && *a == *b) {
    ++a;
    ++b;
  }
  return *a == *b;
}

void PrintPrompt() {
  printk("> ");
}

void Execute(const char* line) {
  if (line[0] == '\0') {
    return;
  }
  if (StrEq(line, "help")) {
    printk("commands: help clear\n");
    return;
  }
  if (StrEq(line, "clear") || StrEq(line, "clean")) {
    if (g_console != nullptr) {
      g_console->Clear();
    }
    return;
  }
  printk("unknown: %s\n", line);
}

}  // namespace

void InitializeShell(Console* console) {
  g_console = console;
  InitializeReadLine(console);
  PrintPrompt();
}

void ShellPoll() {
  char line[128];
  if (!ReadLineTryGetLine(line, sizeof(line))) {
    return;
  }
  Execute(line);
  PrintPrompt();
}
