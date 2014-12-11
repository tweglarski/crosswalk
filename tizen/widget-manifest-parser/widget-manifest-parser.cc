// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/widget-manifest-parser/widget-manifest-parser.h"

#include <string>

#include "xwalk/application/common/application_file_util.h"

#define API_EXPORT __attribute__((visibility("default")))

extern "C" {

API_EXPORT bool ParseXMLManifest(const char* path, XMLManifestData** data) {
  if (!path || !data)
    return false;
  std::string str_path = path;
  if (str_path.empty())
    return false;

  std::string error;
  scoped_ptr<xwalk::application::Manifest> manifest =
      xwalk::application::LoadManifest(
          base::FilePath(path),
          xwalk::application::Manifest::TYPE_WIDGET, &error);

  // TODO(tweglarski): implement me

  return false;
}

API_EXPORT bool ParseJSONManifest(const char* path, JSONManifestData** data) {
  if (!path || !data)
    return false;
  std::string str_path = path;
  if (str_path.empty())
    return false;

  std::string error;
  scoped_ptr<xwalk::application::Manifest> manifest =
      xwalk::application::LoadManifest(
          base::FilePath(path),
          xwalk::application::Manifest::TYPE_MANIFEST, &error);

  // TODO(tweglarski): implement me

  return false;
}

}  // extern "C"
