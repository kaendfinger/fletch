// Copyright (c) 2015, the Fletch project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

library servicec.plugins.cc;

import 'dart:core' hide Type;

import 'package:path/path.dart' show basenameWithoutExtension, join;

import '../parser.dart';
import '../emitter.dart';

const COPYRIGHT = """
// Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE.md file.
""";

void generateHeaderFile(String path, Unit unit, String outputDirectory) {
  _HeaderVisitor visitor = new _HeaderVisitor(path);
  visitor.visit(unit);
  String contents = visitor.buffer.toString();
  String directory = join(outputDirectory, "cc");
  writeToFile(directory, path, "h", contents);
}

void generateImplementationFile(String path,
                                Unit unit,
                                String outputDirectory) {
  _ImplementationVisitor visitor = new _ImplementationVisitor(path);
  visitor.visit(unit);
  String contents = visitor.buffer.toString();
  String directory = join(outputDirectory, "cc");
  writeToFile(directory, path, "cc", contents);
}

abstract class CcVisitor extends Visitor {
  final String path;
  final StringBuffer buffer = new StringBuffer();
  CcVisitor(this.path);

  visit(Node node) => node.accept(this);

  visitFormal(Formal node) {
    visit(node.type);
    buffer.write(' ${node.name}');
  }

  visitType(Type node) {
    Map<String, String> types = const { 'Int32': 'int' };
    String type = types[node.identifier];
    buffer.write(type);
  }

  visitArguments(List<Formal> formals) {
    bool first = true;
    formals.forEach((Formal formal) {
      if (!first) buffer.write(', ');
      first = false;
      visit(formal);
    });
  }

  visitMethodBody(String id, List<Formal> arguments) {
    const int REQUEST_HEADER_SIZE = 32;
    final int size = REQUEST_HEADER_SIZE + (arguments.length * 4);
    String pointerToArgument(int index) {
      int offset = REQUEST_HEADER_SIZE + index * 4;
      return 'reinterpret_cast<int*>(_buffer + $offset)';
    }

    buffer.writeln('  char _bits[$size];');
    buffer.writeln('  char* _buffer = _bits;');
    for (int i = 0; i < arguments.length; i++) {
      String name = arguments[i].name;
      buffer.writeln('  *${pointerToArgument(i)} = $name;');
    }
    buffer.writeln('  ServiceApiInvokeX(_service_id, $id, _buffer, $size);');
    buffer.writeln('  return *${pointerToArgument(0)};');
  }
}

class _HeaderVisitor extends CcVisitor {
  _HeaderVisitor(String path) : super(path);

  String computeHeaderGuard() {
    String base = basenameWithoutExtension(path).toUpperCase();
    return '${base}_H';
  }

  visitUnit(Unit node) {
    String headerGuard = computeHeaderGuard();
    buffer.writeln(COPYRIGHT);

    buffer.writeln('// Generated file. Do not edit.');
    buffer.writeln();

    buffer.writeln('#ifndef $headerGuard');
    buffer.writeln('#define $headerGuard');
    buffer.writeln();

    buffer.writeln('#include "include/service_api.h"');

    node.services.forEach(visit);

    buffer.writeln();
    buffer.writeln('#endif  // $headerGuard');
  }

  visitService(Service node) {
    buffer.writeln();
    buffer.writeln('class ${node.name} {');
    buffer.writeln(' public:');
    buffer.writeln('  static void Setup();');
    buffer.writeln('  static void TearDown();');

    node.methods.forEach(visit);

    buffer.writeln('};');
  }

  visitMethod(Method node) {
    buffer.write('  static ');
    visit(node.returnType);
    buffer.write(' ${node.name}(');
    visitArguments(node.arguments);
    buffer.writeln(');');

    if (node.arguments.length != 1) return;

    buffer.write('  static void ${node.name}Async(');
    visitArguments(node.arguments);
    if (node.arguments.isNotEmpty) buffer.write(', ');
    buffer.writeln('ServiceApiCallback callback);');
  }
}

class _ImplementationVisitor extends CcVisitor {
  int methodId = 1;
  String serviceName;

  _ImplementationVisitor(String path) : super(path);

  String computeHeaderFile() {
    String base = basenameWithoutExtension(path);
    return '$base.h';
  }

  visitUnit(Unit node) {
    String headerFile = computeHeaderFile();
    buffer.writeln(COPYRIGHT);

    buffer.writeln('// Generated file. Do not edit.');
    buffer.writeln();

    buffer.writeln('#include "$headerFile"');

    node.services.forEach(visit);
  }

  visitService(Service node) {
    buffer.writeln();
    buffer.writeln('static ServiceId _service_id = kNoServiceId;');

    serviceName = node.name;

    buffer.writeln();
    buffer.writeln('void ${serviceName}::Setup() {');
    buffer.writeln('  _service_id = ServiceApiLookup("$serviceName");');
    buffer.writeln('}');

    buffer.writeln();
    buffer.writeln('void ${serviceName}::TearDown() {');
    buffer.writeln('  ServiceApiTerminate(_service_id);');
    buffer.writeln('  _service_id = kNoServiceId;');
    buffer.writeln('}');

    node.methods.forEach(visit);
  }

  visitMethod(Method node) {
    const String type = 'ServiceApiValueType';
    const String ctype = 'ServiceApiCallback';
    String name = node.name;
    String id = '_k${name}Id';

    buffer.writeln();
    buffer.write('static const MethodId $id = ');
    buffer.writeln('reinterpret_cast<MethodId>(${methodId++});');

    buffer.writeln();
    visit(node.returnType);
    buffer.write(' $serviceName::${name}(');
    visitArguments(node.arguments);
    buffer.writeln(') {');
    visitMethodBody(id, node.arguments);
    buffer.writeln('}');

    if (node.arguments.length != 1) return;

    buffer.writeln();
    buffer.writeln('void $serviceName::${name}Async($type arg, $ctype cb) {');
    buffer.writeln('  ServiceApiInvokeAsync(_service_id, $id, arg, cb, NULL);');
    buffer.writeln('}');
  }
}
