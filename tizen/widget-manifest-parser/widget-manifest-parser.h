// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_WIDGET_MANIFEST_PARSER_WIDGET_MANIFEST_PARSER_H_
#define XWALK_TIZEN_WIDGET_MANIFEST_PARSER_WIDGET_MANIFEST_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

// Represents XML manifest data
struct XMLManifestData {
  int reserved;
};

// Represents JSON manifest data
struct JSONManifestData {
  int reserved;
};

// Reads XML manifest from specified file and filles specified data argument
// with read data. Returns true on success or false otherwise.
bool ParseXMLManifest(const char* path, XMLManifestData** data);

// Reads JSON manifest from specified file and filles specified data argument
// with read data. Returns true on success or false otherwise.
bool ParseJSONManifest(const char* path, JSONManifestData** data);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // XWALK_TIZEN_WIDGET_MANIFEST_PARSER_WIDGET_MANIFEST_PARSER_H_
