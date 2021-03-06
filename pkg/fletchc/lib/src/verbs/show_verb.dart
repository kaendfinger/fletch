// Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE.md file.

library fletchc.verbs.show_verb;

import 'infrastructure.dart';

import 'documentation.dart' show
    showDocumentation;

const Action showAction = const Action(
    show, showDocumentation, requiresSession: true,
    requiredTarget: TargetKind.LOG);

Future<int> show(AnalyzedSentence sentence, VerbContext context) {
  return context.performTaskInWorker(new ShowLogTask());
}

class ShowLogTask extends SharedTask {
  // Keep this class simple, see note in superclass.

  const ShowLogTask();

  Future<int> call(
      CommandSender commandSender,
      StreamIterator<Command> commandIterator) {
    return showLogTask();
  }
}

Future<int> showLogTask() async {
  print(SessionState.current.getLog());
  return 0;
}
