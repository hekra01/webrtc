/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>

#include "webrtc/common_types.h"
#include "webrtc/modules/rtp_rtcp/source/rtp_header_extension.h"
#include "webrtc/modules/rtp_rtcp/source/rtp_utility.h"

namespace webrtc {

RtpHeaderExtensionMap::RtpHeaderExtensionMap() {
}

RtpHeaderExtensionMap::~RtpHeaderExtensionMap() {
  Erase();
}

void RtpHeaderExtensionMap::Erase() {
  while (!extensionMap_.empty()) {
    std::map<uint8_t, HeaderExtension*>::iterator it =
        extensionMap_.begin();
    delete it->second;
    extensionMap_.erase(it);
  }
}

int32_t RtpHeaderExtensionMap::Register(RTPExtensionType type, uint8_t id) {
  if (id < 1 || id > 14) {
    return -1;
  }
  std::map<uint8_t, HeaderExtension*>::iterator it =
      extensionMap_.find(id);
  if (it != extensionMap_.end()) {
    if (it->second->type != type) {
      // An extension is already registered with the same id
      // but a different type, so return failure.
      return -1;
    }
    // This extension type is already registered with this id,
    // so return success.
    return 0;
  }
  extensionMap_[id] = new HeaderExtension(type);
  return 0;
}

int32_t RtpHeaderExtensionMap::Deregister(const RTPExtensionType type) {
  uint8_t id;
  if (GetId(type, &id) != 0) {
    return 0;
  }
  std::map<uint8_t, HeaderExtension*>::iterator it =
      extensionMap_.find(id);
  assert(it != extensionMap_.end());
  delete it->second;
  extensionMap_.erase(it);
  return 0;
}

bool RtpHeaderExtensionMap::IsRegistered(RTPExtensionType type) const {
  std::map<uint8_t, HeaderExtension*>::const_iterator it =
    extensionMap_.begin();
  for (; it != extensionMap_.end(); ++it) {
    if (it->second->type == type)
      return true;
  }
  return false;
}

int32_t RtpHeaderExtensionMap::GetType(const uint8_t id,
                                       RTPExtensionType* type) const {
  assert(type);
  std::map<uint8_t, HeaderExtension*>::const_iterator it =
      extensionMap_.find(id);
  if (it == extensionMap_.end()) {
    return -1;
  }
  HeaderExtension* extension = it->second;
  *type = extension->type;
  return 0;
}

RTPExtensionType RtpHeaderExtensionMap::GetType(uint8_t id) const {
  auto it = extensionMap_.find(id);
  if (it == extensionMap_.end()) {
    return kInvalidType;
  }
  return it->second->type;
}

int32_t RtpHeaderExtensionMap::GetId(const RTPExtensionType type,
                                     uint8_t* id) const {
  assert(id);
  std::map<uint8_t, HeaderExtension*>::const_iterator it =
      extensionMap_.begin();

  while (it != extensionMap_.end()) {
    HeaderExtension* extension = it->second;
    if (extension->type == type) {
      *id = it->first;
      return 0;
    }
    it++;
  }
  return -1;
}

uint8_t RtpHeaderExtensionMap::GetId(RTPExtensionType type) const {
  for (auto kv : extensionMap_) {
    if (kv.second->type == type)
      return kv.first;
  }
  return kInvalidId;
}

size_t RtpHeaderExtensionMap::GetTotalLengthInBytes() const {
  // Get length for each extension block.
  size_t length = 0;
  for (const auto& kv : extensionMap_)
    length += kv.second->length;
  // Add RTP extension header length.
  if (length > 0)
    length += kRtpOneByteHeaderLength;
  // Pad up to nearest 32bit word.
  length = RtpUtility::Word32Align(length);
  return length;
}

int32_t RtpHeaderExtensionMap::Size() const {
  return extensionMap_.size();
}

void RtpHeaderExtensionMap::GetCopy(RtpHeaderExtensionMap* map) const {
  assert(map);
  std::map<uint8_t, HeaderExtension*>::const_iterator it =
      extensionMap_.begin();
  while (it != extensionMap_.end()) {
    HeaderExtension* extension = it->second;
    map->Register(extension->type, it->first);
    it++;
  }
}
}  // namespace webrtc
