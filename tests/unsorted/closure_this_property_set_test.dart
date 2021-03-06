// Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE.md file.

import 'package:expect/expect.dart';

class A {
  int x = 0;
  foo(y) {
    return () { x = y; };
  }
}

main() {
  var a = new A();
  var f = a.foo(42);
  Expect.equals(0, a.x);
  f();
  Expect.equals(42, a.x);
}
