#pragma once

void InitializeKeyboard();
void SetKeyListener(void (*listener)(char c));
void KeyboardPoll();
