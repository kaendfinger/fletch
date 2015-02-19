// Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE.md file.

// Generated file. Do not edit.

#include "conformance_service.h"
#include "include/service_api.h"
#include <stdlib.h>

static ServiceId service_id_ = kNoServiceId;

void ConformanceService::setup() {
  service_id_ = ServiceApiLookup("ConformanceService");
}

void ConformanceService::tearDown() {
  ServiceApiTerminate(service_id_);
  service_id_ = kNoServiceId;
}

static const MethodId kGetAgeId_ = reinterpret_cast<MethodId>(1);

int32_t ConformanceService::getAge(PersonBuilder person) {
  return person.InvokeMethod(service_id_, kGetAgeId_);
}

static const MethodId kGetBoxedAgeId_ = reinterpret_cast<MethodId>(2);

int32_t ConformanceService::getBoxedAge(PersonBoxBuilder box) {
  return box.InvokeMethod(service_id_, kGetBoxedAgeId_);
}

static const MethodId kGetAgeStatsId_ = reinterpret_cast<MethodId>(3);

AgeStats ConformanceService::getAgeStats(PersonBuilder person) {
  int64_t result = person.InvokeMethod(service_id_, kGetAgeStatsId_);
  char* memory = reinterpret_cast<char*>(result);
  Segment* segment = MessageReader::GetRootSegment(memory);
  return AgeStats(segment, 8);
}

static const MethodId kCreateAgeStatsId_ = reinterpret_cast<MethodId>(4);

AgeStats ConformanceService::createAgeStats(int32_t averageAge, int32_t sum) {
  static const int kSize = 40;
  char _bits[kSize];
  char* _buffer = _bits;
  *reinterpret_cast<int32_t*>(_buffer + 32) = averageAge;
  *reinterpret_cast<int32_t*>(_buffer + 36) = sum;
  ServiceApiInvoke(service_id_, kCreateAgeStatsId_, _buffer, kSize);
  int64_t result = *reinterpret_cast<int64_t*>(_buffer + 32);
  char* memory = reinterpret_cast<char*>(result);
  Segment* segment = MessageReader::GetRootSegment(memory);
  return AgeStats(segment, 8);
}

static const MethodId kCreatePersonId_ = reinterpret_cast<MethodId>(5);

Person ConformanceService::createPerson(int32_t children) {
  static const int kSize = 40;
  char _bits[kSize];
  char* _buffer = _bits;
  *reinterpret_cast<int32_t*>(_buffer + 32) = children;
  ServiceApiInvoke(service_id_, kCreatePersonId_, _buffer, kSize);
  int64_t result = *reinterpret_cast<int64_t*>(_buffer + 32);
  char* memory = reinterpret_cast<char*>(result);
  Segment* segment = MessageReader::GetRootSegment(memory);
  return Person(segment, 8);
}

static const MethodId kCreateNodeId_ = reinterpret_cast<MethodId>(6);

Node ConformanceService::createNode(int32_t depth) {
  static const int kSize = 40;
  char _bits[kSize];
  char* _buffer = _bits;
  *reinterpret_cast<int32_t*>(_buffer + 32) = depth;
  ServiceApiInvoke(service_id_, kCreateNodeId_, _buffer, kSize);
  int64_t result = *reinterpret_cast<int64_t*>(_buffer + 32);
  char* memory = reinterpret_cast<char*>(result);
  Segment* segment = MessageReader::GetRootSegment(memory);
  return Node(segment, 8);
}

static const MethodId kCountId_ = reinterpret_cast<MethodId>(7);

int32_t ConformanceService::count(PersonBuilder person) {
  return person.InvokeMethod(service_id_, kCountId_);
}

static const MethodId kDepthId_ = reinterpret_cast<MethodId>(8);

int32_t ConformanceService::depth(NodeBuilder node) {
  return node.InvokeMethod(service_id_, kDepthId_);
}

static const MethodId kFooId_ = reinterpret_cast<MethodId>(9);

void ConformanceService::foo() {
  static const int kSize = 40;
  char _bits[kSize];
  char* _buffer = _bits;
  ServiceApiInvoke(service_id_, kFooId_, _buffer, kSize);
}

static void Unwrap_void_8(void* raw) {
  typedef void (*cbt)();
  char* buffer = reinterpret_cast<char*>(raw);
  cbt callback = *reinterpret_cast<cbt*>(buffer + 40);
  free(buffer);
  callback();
}

void ConformanceService::fooAsync(void (*callback)()) {
  static const int kSize = 40 + 1 * sizeof(void*);
  char* _buffer = reinterpret_cast<char*>(malloc(kSize));
  *reinterpret_cast<void**>(_buffer + 40) = reinterpret_cast<void*>(callback);
  ServiceApiInvokeAsync(service_id_, kFooId_, Unwrap_void_8, _buffer, kSize);
}

static const MethodId kPingId_ = reinterpret_cast<MethodId>(10);

int32_t ConformanceService::ping() {
  static const int kSize = 40;
  char _bits[kSize];
  char* _buffer = _bits;
  ServiceApiInvoke(service_id_, kPingId_, _buffer, kSize);
  return *reinterpret_cast<int64_t*>(_buffer + 32);
}

static void Unwrap_int32_8(void* raw) {
  typedef void (*cbt)(int);
  char* buffer = reinterpret_cast<char*>(raw);
  int64_t result = *reinterpret_cast<int64_t*>(buffer + 32);
  cbt callback = *reinterpret_cast<cbt*>(buffer + 40);
  free(buffer);
  callback(result);
}

void ConformanceService::pingAsync(void (*callback)(int32_t)) {
  static const int kSize = 40 + 1 * sizeof(void*);
  char* _buffer = reinterpret_cast<char*>(malloc(kSize));
  *reinterpret_cast<void**>(_buffer + 40) = reinterpret_cast<void*>(callback);
  ServiceApiInvokeAsync(service_id_, kPingId_, Unwrap_int32_8, _buffer, kSize);
}

List<uint8_t> PersonBuilder::initName(int length) {
  Reader result = NewList(0, length, 1);
  return List<uint8_t>(result.segment(), result.offset(), length);
}

List<PersonBuilder> PersonBuilder::initChildren(int length) {
  Reader result = NewList(8, length, 24);
  return List<PersonBuilder>(result.segment(), result.offset(), length);
}

SmallBuilder LargeBuilder::initS() {
  return SmallBuilder(segment(), offset() + 0);
}

Small Large::getS() const { return Small(segment(), offset() + 0); }

PersonBuilder PersonBoxBuilder::initPerson() {
  Builder result = NewStruct(0, 24);
  return PersonBuilder(result);
}

Person PersonBox::getPerson() const { return ReadStruct<Person>(0); }

ConsBuilder NodeBuilder::initCons() {
  setTag(3);
  return ConsBuilder(segment(), offset() + 0);
}

Cons Node::getCons() const { return Cons(segment(), offset() + 0); }

NodeBuilder ConsBuilder::initFst() {
  Builder result = NewStruct(0, 24);
  return NodeBuilder(result);
}

NodeBuilder ConsBuilder::initSnd() {
  Builder result = NewStruct(8, 24);
  return NodeBuilder(result);
}

Node Cons::getFst() const { return ReadStruct<Node>(0); }

Node Cons::getSnd() const { return ReadStruct<Node>(8); }