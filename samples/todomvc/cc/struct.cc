// Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE.md file.

#include "struct.h"

#include <stdlib.h>
#include <stddef.h>

#include "unicode.h"

static const int HEADER_SIZE = 56;

char* pointerToArgument(char* buffer, int offset) {
  return buffer + HEADER_SIZE + offset;
}

int64_t* pointerToInt64Argument(char* buffer, int offset) {
  return reinterpret_cast<int64_t*>(pointerToArgument(buffer, offset));
}

int64_t* pointerToResult(char* buffer) {
  return pointerToInt64Argument(buffer, 0);
}

int64_t* pointerToSegments(char* buffer) {
  return pointerToInt64Argument(buffer, -8);
}

void** pointerToCallbackFunction(char* buffer) {
  return reinterpret_cast<void**>(pointerToArgument(buffer, -16));
}

void** pointerToCallbackData(char* buffer) {
  return reinterpret_cast<void**>(pointerToArgument(buffer, -24));
}

MessageBuilder::MessageBuilder(int space)
    : first_(this, 0, space),
      last_(&first_),
      segments_(1) {
}

int MessageBuilder::ComputeUsed() const {
  int result = 0;
  const BuilderSegment* current = &first_;
  while (current != NULL) {
    result += current->used();
    current = current->next();
  }
  return result;
}

Builder MessageBuilder::InternalInitRoot(int size) {
  // Return value and arguments use the same space. Therefore,
  // the size of any struct needs to be at least 8 bytes in order
  // to have room for the return address.
  if (size == 0) size = 8;
  int offset = first_.Allocate(HEADER_SIZE + size);
  return Builder(&first_, offset + HEADER_SIZE);
}

BuilderSegment* MessageBuilder::FindSegmentForBytes(int bytes) {
  if (last_->HasSpaceForBytes(bytes)) return last_;
  int capacity = (bytes > 8192) ? bytes : 8192;
  BuilderSegment* segment = new BuilderSegment(this, segments_++, capacity);
  last_->set_next(segment);
  last_ = segment;
  return segment;
}

void MessageBuilder::DeleteMessage(char* buffer) {
  int64_t segments = *pointerToSegments(buffer);
  for (int i = 0; i < segments; i++) {
    int64_t address = *pointerToInt64Argument(buffer, 8 + (i * 16));
    char* memory = reinterpret_cast<char*>(address);
    free(memory);
  }
  free(buffer);
}

MessageReader::MessageReader(int segments, char* memory)
    : segment_count_(segments),
      segments_(new Segment*[segments]) {
  for (int i = 0; i < segments; i++) {
    int64_t address = *reinterpret_cast<int64_t*>(memory + (i * 16));
    int size = *reinterpret_cast<int*>(memory + 8 + (i * 16));
    segments_[i] = new Segment(this, reinterpret_cast<char*>(address), size);
  }
}

MessageReader::~MessageReader() {
  for (int i = 0; i < segment_count_; ++i) {
    delete segments_[i];
  }
  delete[] segments_;
}

Segment* MessageReader::GetRootSegment(char* memory) {
  int32_t segments = *reinterpret_cast<int32_t*>(memory);
  if (segments == 0) {
    int32_t size = *reinterpret_cast<int32_t*>(memory + 4);
    return new Segment(memory, size);
  } else {
    MessageReader* reader = new MessageReader(segments, memory + 8);
    free(memory);
    return new Segment(reader);
  }
}

Segment::Segment(char* memory, int size)
    : reader_(NULL),
      memory_(memory),
      size_(size),
      is_root_(false) {
}

Segment::Segment(MessageReader* reader, char* memory, int size)
    : reader_(reader),
      memory_(memory),
      size_(size),
      is_root_(false) {
}

Segment::Segment(MessageReader* reader)
    : reader_(reader),
      is_root_(true) {
  Segment* first = reader->GetSegment(0);
  memory_ = first->memory();
  size_ = first->size();
}

Segment::~Segment() {
  if (is_root_) {
    delete reader_;
  } else {
    free(memory_);
  }
}

BuilderSegment::BuilderSegment(MessageBuilder* builder, int id, int capacity)
    : Segment(reinterpret_cast<char*>(calloc(capacity, 1)), capacity),
      builder_(builder),
      id_(id),
      next_(NULL),
      used_(0) {
}

BuilderSegment::~BuilderSegment() {
  if (next_ != NULL) {
    delete next_;
    next_ = NULL;
  }
}

int BuilderSegment::Allocate(int bytes) {
  if (!HasSpaceForBytes(bytes)) return -1;
  int result = used_;
  used_ += bytes;
  return result;
}

void BuilderSegment::Detach() {
  Segment::Detach();
  if (next_ != NULL) next_->Detach();
}

static int ComputeStructBuffer(BuilderSegment* segment,
                               char** buffer) {
  if (segment->HasNext()) {
    // Build a segmented message. The segmented message has the
    // usual 48-byte header, room for a result, and then a list
    // of all the segments in the message. The segments in the
    // message are extracted and freed on return from the service
    // call.
    int segments = segment->builder()->segments();
    int size = HEADER_SIZE + 8 + (segments * 16);
    *buffer = reinterpret_cast<char*>(malloc(size));
    int offset = HEADER_SIZE + 8;
    do {
      *reinterpret_cast<void**>(*buffer + offset) = segment->At(0);
      *reinterpret_cast<int*>(*buffer + offset + 8) = segment->used();
      segment = segment->next();
      offset += 16;
    } while (segment != NULL);

    // Mark the request as being segmented.
    *pointerToSegments(*buffer) = segments;
    return size;
  }

  *buffer = reinterpret_cast<char*>(segment->At(0));
  int size = segment->used();
  // Mark the request as being non-segmented.
  *pointerToSegments(*buffer) = 0;
  return size;
}

int64_t Builder::InvokeMethod(ServiceId service, MethodId method) {
  BuilderSegment* segment = this->segment();
  char* buffer;
  int size = ComputeStructBuffer(segment, &buffer);
  segment->Detach();
  ServiceApiInvoke(service, method, buffer, size);
  int64_t result = *pointerToResult(buffer);
  MessageBuilder::DeleteMessage(buffer);
  return result;
}

void Builder::InvokeMethodAsync(ServiceId service,
                                MethodId method,
                                ServiceApiCallback api_callback,
                                void* callback_function,
                                void* callback_data) {
  BuilderSegment* segment = this->segment();
  char* buffer;
  int size = ComputeStructBuffer(segment, &buffer);
  segment->Detach();
  // Set the callback function (the user supplied callback).
  *pointerToCallbackFunction(buffer) = callback_function;
  *pointerToCallbackData(buffer) = callback_data;
  ServiceApiInvokeAsync(service, method, api_callback, buffer, size);
}

Builder Builder::NewStruct(int offset, int size) {
  offset += this->offset();
  BuilderSegment* segment = this->segment();
  while (true) {
    int* lo = reinterpret_cast<int*>(segment->At(offset + 0));
    int* hi = reinterpret_cast<int*>(segment->At(offset + 4));
    int result = segment->Allocate(size);
    if (result >= 0) {
      *lo = (result << 2) | 1;
      *hi = 0;
      return Builder(segment, result);
    }

    BuilderSegment* other = segment->builder()->FindSegmentForBytes(size + 8);
    int target = other->Allocate(8);
    *lo = (target << 2) | 3;
    *hi = other->id();

    segment = other;
    offset = target;
  }
}

Reader Builder::NewList(int offset, int length, int size) {
  offset += this->offset();
  size *= length;
  BuilderSegment* segment = this->segment();
  while (true) {
    int* lo = reinterpret_cast<int*>(segment->At(offset + 0));
    int* hi = reinterpret_cast<int*>(segment->At(offset + 4));
    int result = segment->Allocate(size);
    if (result >= 0) {
      *lo = (result << 2) | 2;
      *hi = length;
      return Reader(segment, result);
    }

    BuilderSegment* other = segment->builder()->FindSegmentForBytes(size + 8);
    int target = other->Allocate(8);
    *lo = (target << 2) | 3;
    *hi = other->id();

    segment = other;
    offset = target;
  }
}

void Builder::NewString(int offset, const char* value) {
  size_t value_len = strlen(value);
  Utf8::Type type;
  intptr_t len = Utf8::CodeUnitCount(value, value_len, &type);
  Reader reader = NewList(offset, len, 2);
  uint16_t* dst =
      reinterpret_cast<uint16_t*>(reader.segment()->memory() + reader.offset());
  Utf8::DecodeToUTF16(value, value_len, dst, len);
}

int Reader::ComputeUsed() const {
  MessageReader* reader = segment_->reader();
  int used = 0;
  for (int i = 0; i < reader->segment_count(); i++) {
    used += reader->GetSegment(i)->size();
  }
  return used;
}

char* Reader::ReadString(int offset) const {
  List<uint16_t> data = ReadList<uint16_t>(offset);
  intptr_t len = Utf8::Length(data);
  char* result = reinterpret_cast<char*>(malloc(len + 1));
  Utf8::Encode(data, result, len);
  result[len] = 0;
  return result;
}
