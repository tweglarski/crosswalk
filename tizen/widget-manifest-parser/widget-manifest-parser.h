// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_WIDGET_MANIFEST_PARSER_WIDGET_MANIFEST_PARSER_H_
#define XWALK_TIZEN_WIDGET_MANIFEST_PARSER_WIDGET_MANIFEST_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

// Represents a manifest data
struct ManifestData {
  const char* package;
  const char* id;
  const char* name;
  const char* short_name;
  const char* version;
  const char* icon;
  const char* api_version;
  unsigned int privilege_count;
  const char** privilege_list;
};

// Reads manifest from specified file and filles specified data argument
// with read data. Returns true on success or false otherwise. If the error
// parameter is specified, it is also filled with proper message.
bool ParseManifest(const char* path,
    const ManifestData** data, const char** error);

// Releses the data and the error returned by ParseManifest.
// Returns true on success or false otherwise.
bool ReleaseData(const ManifestData* data, const char* error);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // XWALK_TIZEN_WIDGET_MANIFEST_PARSER_WIDGET_MANIFEST_PARSER_H_
