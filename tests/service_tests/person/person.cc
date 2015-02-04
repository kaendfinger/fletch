// Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE.md file.

#include "person_shared.h"
#include "cc/person_counter.h"

#include <cstdio>
#include <stdint.h>
#include <sys/time.h>

static uint64_t GetMicroseconds() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) < 0) return -1;
  uint64_t result = tv.tv_sec * 1000000LL;
  result += tv.tv_usec;
  return result;
}

static void BuildPerson(PersonBuilder person, int n) {
  person.set_age(n * 20);
  if (n > 1) {
    List<PersonBuilder> children = person.NewChildren(2);
    BuildPerson(children[0], n - 1);
    BuildPerson(children[1], n - 1);
  }
}

static void RunPersonTests() {
  MessageBuilder builder(512);

  uint64_t start = GetMicroseconds();
  PersonBuilder person = builder.NewRoot<PersonBuilder>();
  BuildPerson(person, 5);
  uint64_t end = GetMicroseconds();

  int used = builder.ComputeUsed();
  int building_us = static_cast<int>(end - start);
  printf("Building took %.2f ms.\n", building_us / 1000.0);
  printf("    - %.2f MB/s\n", static_cast<double>(used) / building_us);

  int age = PersonCounter::GetAge(person);
  start = GetMicroseconds();
  int count = PersonCounter::Count(person);
  end = GetMicroseconds();
  AgeStats stats = PersonCounter::GetAgeStats(person);
  printf("AgeStats avg: %d sum: %d\n", stats.averageAge(), stats.sum());
  stats.Delete();
  int reading_us = static_cast<int>(end - start);
  printf("Reading took %.2f us.\n", reading_us / 1000.0);
  printf("    - %.2f MB/s\n", static_cast<double>(used) / reading_us);

  printf("Verification: age = %d, count = %d\n", age, count);
}

static void RunPersonBoxTests() {
  MessageBuilder builder(512);

  PersonBoxBuilder box = builder.NewRoot<PersonBoxBuilder>();
  PersonBuilder person = box.NewPerson();
  person.set_age(87);
  int age = PersonCounter::GetBoxedAge(box);
  printf("Verification: age = %d\n", age);
}

static void InteractWithService() {
  PersonCounter::Setup();
  RunPersonTests();
  RunPersonBoxTests();
  PersonCounter::TearDown();
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Usage: %s <snapshot>\n", argv[0]);
    return 1;
  }
  SetupPersonTest(argc, argv);
  InteractWithService();
  TearDownPersonTest();
  return 0;
}
