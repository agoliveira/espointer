#pragma once
#include <Arduino.h>

// Poll from loop(); reads USB serial, executes commands.
// Commands: help, show, set <key> <value>, save, home, north <deg>,
//           aim <az> <el>, track, status
void cliPoll();
