// Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE.md file.

#ifndef FLETCH_ENABLE_LIVE_CODING

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "include/fletch_api.h"

#include "src/shared/flags.h"
#include "src/shared/list.h"
#include "src/shared/utils.h"
#include "src/shared/platform.h"

namespace fletch {

static bool IsSnapshot(List<uint8> snapshot) {
  return snapshot.length() > 2 && snapshot[0] == 0xbe && snapshot[1] == 0xef;
}

static int Main(int argc, char** argv) {
  Flags::ExtractFromCommandLine(&argc, argv);
  FletchSetup();

  int program_count = argc - 1;

  FletchProgram* programs = new FletchProgram[program_count];

  for (int i = 0; i < program_count; i++) {
    // Handle the arguments.
    const char* input = argv[i + 1];
    List<uint8> bytes = Platform::LoadFile(input);
    if (!IsSnapshot(bytes)) {
      FATAL("The input file is not a fletch snapshot.");
    }

    FletchProgram program = FletchLoadSnapshot(bytes.data(), bytes.length());
    programs[i] = program;
  }

  int result = FletchRunMultipleMain(program_count, programs);

  for (int i = 0; i < program_count; i++) {
    FletchDeleteProgram(programs[i]);
  }

  delete[] programs;

  FletchTearDown();
  return result;
}

}  // namespace fletch


// Forward main calls to fletch::Main.
int main(int argc, char** argv) {
  return fletch::Main(argc, argv);
}

#endif  // !FLETCH_ENABLE_LIVE_CODING
