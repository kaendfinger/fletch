// Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE.md file.

// FletchDebuggerCommands=b,r,so,so,so,s,so,s,so,b y,s,so,finish,q

class A {
  final int _x, _y;
  A() : _x = 1, _y = 2;
  int get x => _x;
  int y() => _y;
}

int foo() => 2 + 3;

main() {
  foo();
  foo();
  var a = new A();
  a.x;
  a.y();
  a.y();
  1;
}
