// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/widget-manifest-parser/widget-manifest-parser.h"

#include <string>

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/manifest_handlers/tizen_application_handler.h"
#include "xwalk/application/common/manifest_handlers/widget_handler.h"

#define API_EXPORT __attribute__((visibility("default")))

namespace {

const char kErrMsgNoPath[] = "Path not specified.";
const char kErrMsgInvalidPath[] = "Invalid path.";
const char kErrMsgNameNotFound[] = "Application name not found.";
const char kErrMsgRequiredVersionNotFound[] = "Required version not found.";

const char kName[] = "name";

class InnerManifestData {
 public:
  void SetApplicationName(const std::string& value) {
    application_name_ = value;
    data_.application_name = application_name_.c_str();
  }

  void SetRequiredVersion(const std::string& value) {
    required_version_ = value;
    data_.required_version = required_version_.c_str();
  }

  void SetPrivileges(const std::vector<std::string>& value) {
    privileges_ = value;
    privilege_list_.clear();
    for (const std::string& p : privileges_)
      privilege_list_.push_back(p.c_str());
    data_.privilege_count = privileges_.size();
    data_.privilege_list = privilege_list_.data();
  }

  const ManifestData* GetAsManifestData() const {
    return &data_;
  }

private:
  std::string application_name_;
  std::string required_version_;
  std::vector<std::string> privileges_;
  std::vector<const char*> privilege_list_;
  ManifestData data_;
};

// TODO(tweglarski): change this temporary solution -[
std::string parse_error;
InnerManifestData inner_manifest_data;
// ]-

bool ExtractName(const xwalk::application::ApplicationData& app_data,
    std::string* name) {
  xwalk::application::WidgetInfo* info =
    static_cast<xwalk::application::WidgetInfo*>(
        app_data.GetManifestData(xwalk::application_widget_keys::kWidgetKey));
  if (!info)
    return false;
  return info->GetWidgetInfo()->GetString(kName, name);
}

bool ExtractRequiredVersion(const xwalk::application::ApplicationData& app_data,
    std::string* required_version) {
  xwalk::application::TizenApplicationInfo* info =
    static_cast<xwalk::application::TizenApplicationInfo*>(
        app_data.GetManifestData(
            xwalk::application_widget_keys::kTizenApplicationKey));
  if (!info)
    return false;
  *required_version = info->required_version();
  return true;
}

}  // namespace

extern "C" {

API_EXPORT bool ParseManifest(const char* path,
    const ManifestData** data, const char** error) {
  if (!path) {
    if (error)
      *error = kErrMsgNoPath;
    return false;
  }
  std::string str_path = path;
  if (str_path.empty()) {
    if (error)
      *error = kErrMsgInvalidPath;
    return false;
  }

  scoped_ptr<xwalk::application::Manifest> manifest =
      xwalk::application::LoadManifest(
          base::FilePath(path),
          xwalk::application::Manifest::TYPE_WIDGET, &parse_error);
  if (!manifest) {
    if (error)
      *error = parse_error.c_str();
    return false;
  }

  scoped_refptr<xwalk::application::ApplicationData> app_data =
      xwalk::application::ApplicationData::Create(base::FilePath(),
          std::string(), xwalk::application::ApplicationData::INTERNAL,
          manifest.Pass(), &parse_error);
  if (!app_data.get()) {
    if (error)
      *error = parse_error.c_str();
    return false;
  }

  std::string name;
  if (!ExtractName(*app_data, &name)) {
    if (error)
      *error = kErrMsgNameNotFound;
    return false;
  }

  std::string required_version;
  if (!ExtractRequiredVersion(*app_data, &required_version)) {
    if (error)
      *error = kErrMsgRequiredVersionNotFound;
    return false;
  }

  inner_manifest_data.SetApplicationName(name);
  inner_manifest_data.SetRequiredVersion(required_version);

  if (data)
    *data = inner_manifest_data.GetAsManifestData();

  parse_error = std::string();
  return true;
}

API_EXPORT bool ReleaseData(const ManifestData* data) {
  if (!data)
    return false;

  if (data != inner_manifest_data.GetAsManifestData())
    return false;

  inner_manifest_data = InnerManifestData();
  return true;
}

}  // extern "C"
