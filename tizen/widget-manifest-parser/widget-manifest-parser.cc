// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/widget-manifest-parser/widget-manifest-parser.h"

#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/manifest_handlers/tizen_application_handler.h"
#include "xwalk/application/common/manifest_handlers/widget_handler.h"

#define API_EXPORT __attribute__((visibility("default")))

namespace {

const char kErrMsgNoPath[] =
    "Path not specified.";
const char kErrMsgInvalidPath[] =
    "Invalid path.";
const char kErrMsgNameNotFound[] =
    "Application name not found.";
const char kErrMsgRequiredVersionNotFound[] =
    "Required version not found.";
const char kErrMsgInvalidDictionary[] =
    "Cannot get key value as a dictionary. Key name: ";
const char kErrMsgInvalidList[] =
    "Cannot get key value as a list. Key name: ";
const char kErrMsgNoMandatoryKey[] =
    "Cannot find mandatory key. Key name: ";

const char kName[] = "name";
const char kWidgetPrivilegeFullKey[] = "widget.privilege";
const char kWidgetPrivilegeNameKey[] = "@name";

class LocalManifestData {
 public:
  LocalManifestData() { }

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

  const ManifestData* GetManifestData() const {
    return &data_;
  }

private:
  std::string application_name_;
  std::string required_version_;
  std::vector<std::string> privileges_;
  std::vector<const char*> privilege_list_;
  ManifestData data_;

  DISALLOW_COPY_AND_ASSIGN(LocalManifestData);
};

typedef std::string LocalError;

class LocalStorage {
 public:
  static LocalStorage* GetInstance() {
    static LocalStorage instance;
    return &instance;
  }

  const ManifestData* Add(LocalManifestData* data) {
    const ManifestData* result = data->GetManifestData();
    data_vector_.push_back(data);
    return result;
  }

  bool Remove(const ManifestData* data) {
    LocalManifestDataVector::iterator it;
    for (it = data_vector_.begin(); it != data_vector_.end(); ++it)
      if ((*it)->GetManifestData() == data) {
        data_vector_.erase(it);
        return true;
      }
    return false;
  }

  const char* Add(LocalError* error) {
    const char* result = error->c_str();
    error_vector_.push_back(error);
    return result;
  }

  bool Remove(const char* error) {
    ErrorVector::iterator it;
    for (it = error_vector_.begin(); it != error_vector_.end(); ++it)
      if ((*it)->c_str() == error) {
        error_vector_.erase(it);
        return true;
      }
    return false;
  }

 private:
  typedef ScopedVector<LocalManifestData> LocalManifestDataVector;
  typedef ScopedVector<LocalError> ErrorVector;

  LocalManifestDataVector data_vector_;
  ErrorVector error_vector_;

  LocalStorage() { }

  DISALLOW_COPY_AND_ASSIGN(LocalStorage);
};

void SetError(const std::string& message, const char** error) {
  if (error)
    *error = LocalStorage::GetInstance()->Add(new std::string(message));
}

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

void SetError(const std::string& message,
    const std::string& arg, std::string* error) {
  if (error)
    *error = message + arg;
}

bool ExtractPrivilegeName(const base::Value& value,
    std::string* privilege, std::string* error) {
  const base::DictionaryValue* inner_dict;
  if (!value.GetAsDictionary(&inner_dict)) {
    SetError(kErrMsgInvalidDictionary, kWidgetPrivilegeFullKey, error);
    return false;
  }
  std::string name;
  if (!inner_dict->GetString(kWidgetPrivilegeNameKey, &name)) {
    SetError(kErrMsgNoMandatoryKey, kWidgetPrivilegeNameKey, error);
    return false;
  }
  *privilege = name;
  return  true;
}

bool ExtractPrivileges(const xwalk::application::Manifest& manifest,
    std::vector<std::string>* privileges, std::string* error) {
  base::Value* value;
  if (!manifest.Get(kWidgetPrivilegeFullKey, &value))
    return true; // no privileges, no error

  std::vector<std::string> tmp_privileges;
  if (value->IsType(base::Value::TYPE_DICTIONARY)) {
    std::string privilege;
    if (!ExtractPrivilegeName(*value, &privilege, error))
      return false;
    tmp_privileges.push_back(privilege);
  } else if (value->IsType(base::Value::TYPE_LIST)) {
    const base::ListValue* list;
    if (!value->GetAsList(&list)) {
      SetError(kErrMsgInvalidList, kWidgetPrivilegeFullKey, error);
      return false;
    }
    for (const base::Value* list_value : *list) {
      std::string privilege;
      if (!ExtractPrivilegeName(*list_value, &privilege, error))
        return false;
      tmp_privileges.push_back(privilege);
    }
  }

  privileges->swap(tmp_privileges);
  return true;
}

}  // namespace

extern "C" {

API_EXPORT bool ParseManifest(const char* path,
    const ManifestData** data, const char** error) {
  if (!path) {
    SetError(kErrMsgNoPath, error);
    return false;
  }
  std::string str_path = path;
  if (str_path.empty()) {
    SetError(kErrMsgInvalidPath, error);
    return false;
  }

  std::string local_error;

  scoped_ptr<xwalk::application::Manifest> manifest =
      xwalk::application::LoadManifest(
          base::FilePath(path),
          xwalk::application::Manifest::TYPE_WIDGET, &local_error);
  if (!manifest) {
    SetError(local_error, error);
    return false;
  }

  scoped_refptr<xwalk::application::ApplicationData> app_data =
      xwalk::application::ApplicationData::Create(base::FilePath(),
          std::string(), xwalk::application::ApplicationData::INTERNAL,
          manifest.Pass(), &local_error);
  if (!app_data.get()) {
    SetError(local_error, error);
    return false;
  }

  std::string name;
  if (!ExtractName(*app_data, &name)) {
    SetError(kErrMsgNameNotFound, error);
    return false;
  }

  std::string required_version;
  if (!ExtractRequiredVersion(*app_data, &required_version)) {
    SetError(kErrMsgRequiredVersionNotFound, error);
    return false;
  }

  std::vector<std::string> privileges;
  if (!ExtractPrivileges(*app_data->GetManifest(), &privileges, &local_error)) {
    SetError(local_error, error);
    return false;
  }

  if (data) {
    scoped_ptr<LocalManifestData> local_manifest_data(new LocalManifestData);
    local_manifest_data->SetApplicationName(name);
    local_manifest_data->SetRequiredVersion(required_version);
    local_manifest_data->SetPrivileges(privileges);
    *data = LocalStorage::GetInstance()->Add(local_manifest_data.release());
  }

  return true;
}

API_EXPORT bool ReleaseData(const ManifestData* data, const char* error) {
  bool result = true;
  if (data)
    result = result && LocalStorage::GetInstance()->Remove(data);
  if (error)
    result = result && LocalStorage::GetInstance()->Remove(error);
  return result;
}

}  // extern "C"
