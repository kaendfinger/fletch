// Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE.md file.

#if defined(FLETCH_TARGET_OS_MACOS)

#ifdef FLETCH_ENABLE_FFI

#include "src/vm/ffi.h"

namespace fletch {

const char* ForeignUtils::kLibBundlePrefix = "/lib";
const char* ForeignUtils::kLibBundlePostfix = ".dylib";

}  // namespace fletch

#endif  // FLETCH_ENABLE_FFI

#endif  // defined(FLETCH_TARGET_OS_MACOS)
