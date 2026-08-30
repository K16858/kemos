#include "shell/shell.hpp"

#include <cstddef>

#include "shell/printk.hpp"
#include "input/readline.hpp"

namespace {

Console* g_console = nullptr;

bool StrEq(const char* a, const char* b) {
  while (*a != '\0' && *a == *b) {
    ++a;
    ++b;
  }
  return *a == *b;
}

void SplitLine(const char* line, char* cmd, size_t cmd_max, const char** args) {
  while (*line == ' ') {
    ++line;
  }

  size_t i = 0;
  while (line[i] != '\0' && line[i] != ' ' && i + 1 < cmd_max) {
    cmd[i] = line[i];
    ++i;
  }
  cmd[i] = '\0';

  const char* rest = line + i;
  while (*rest == ' ') {
    ++rest;
  }
  *args = (*rest == '\0') ? "" : rest;
}

void PrintPrompt() {
  printk("> ");
}

void CmdHelp(const char* /*args*/) {
  printk("commands: help clear echo\n");
}

void CmdClear(const char* /*args*/) {
  if (g_console != nullptr) {
    g_console->Clear();
  }
}

void CmdEcho(const char* args) {
  if (args[0] == '\0') {
    printk("\n");
    return;
  }
  printk("%s\n", args);
}

using CommandHandler = void (*)(const char* args);

struct Command {
  const char* name;
  CommandHandler handler;
};

const Command commands[] = {
    {"help", CmdHelp},
    {"clear", CmdClear},
    {"clean", CmdClear},
    {"echo", CmdEcho},
};

void Execute(const char* line) {
  char cmd[32];
  const char* args = nullptr;
  SplitLine(line, cmd, sizeof(cmd), &args);

  if (cmd[0] == '\0') {
    return;
  }

  for (const Command& command : commands) {
    if (StrEq(cmd, command.name)) {
      command.handler(args);
      return;
    }
  }
  printk("unknown: %s\n", cmd);
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
