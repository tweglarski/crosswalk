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
const char kErrMsgValueNotFound[] =
    "Value not found. Value name: ";
const char kErrMsgInvalidDictionary[] =
    "Cannot get key value as a dictionary. Key name: ";
const char kErrMsgInvalidList[] =
    "Cannot get key value as a list. Key name: ";
const char kErrMsgNoMandatoryKey[] =
    "Cannot find mandatory key. Key name: ";

const char kName[] = "name";
const char kShortName[] = "shortName";
const char kVersion[] = "version";
const char kWidgetPrivilegeFullKey[] = "widget.privilege";
const char kWidgetPrivilegeNameKey[] = "@name";
const char kWidgetIconFullKey[] = "widget.icon";
const char kWidgetIconSrcKey[] = "@src";

class LocalManifestData {
 public:
  LocalManifestData() {
    data_.package = package_.c_str();
    data_.id = id_.c_str();
    data_.name = name_.c_str();
    data_.short_name = short_name_.c_str();
    data_.version = version_.c_str();
    data_.icon = icon_.c_str();
    data_.api_version = api_version_.c_str();
    data_.privilege_count = privileges_.size();
    data_.privilege_list = privilege_list_.data();
  }

  void SetPackage(const std::string& value) {
    package_ = value;
    data_.package = package_.c_str();
  }

  void SetId(const std::string& value) {
    id_ = value;
    data_.id = id_.c_str();
  }

  void SetName(const std::string& value) {
    name_ = value;
    data_.name = name_.c_str();
  }

  void SetShortName(const std::string& value) {
    short_name_ = value;
    data_.short_name = short_name_.c_str();
  }

  void SetVersion(const std::string& value) {
    version_ = value;
    data_.version = version_.c_str();
  }

  void SetIcon(const std::string& value) {
    icon_ = value;
    data_.icon = icon_.c_str();
  }

  void SetApiVersion(const std::string& value) {
    api_version_ = value;
    data_.api_version = api_version_.c_str();
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
  std::string package_;
  std::string id_;
  std::string name_;
  std::string short_name_;
  std::string version_;
  std::string icon_;
  std::string api_version_;
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

void SetError(const std::string& message, const std::string& arg,
    const char** error) {
  if (error)
    *error = LocalStorage::GetInstance()->Add(new std::string(message + arg));
}

bool ExtractPackage(const xwalk::application::ApplicationData& app_data,
    std::string* value) {
  xwalk::application::TizenApplicationInfo* info =
    static_cast<xwalk::application::TizenApplicationInfo*>(
        app_data.GetManifestData(
            xwalk::application_widget_keys::kTizenApplicationKey));
  if (!info)
    return false;
  *value = info->package();
  return true;
}

bool ExtractId(const xwalk::application::ApplicationData& app_data,
    std::string* value) {
  xwalk::application::TizenApplicationInfo* info =
    static_cast<xwalk::application::TizenApplicationInfo*>(
        app_data.GetManifestData(
            xwalk::application_widget_keys::kTizenApplicationKey));
  if (!info)
    return false;
  *value = info->id();
  return true;
}

bool ExtractName(const xwalk::application::ApplicationData& app_data,
    std::string* value) {
  xwalk::application::WidgetInfo* info =
    static_cast<xwalk::application::WidgetInfo*>(
        app_data.GetManifestData(xwalk::application_widget_keys::kWidgetKey));
  if (!info)
    return false;
  return info->GetWidgetInfo()->GetString(kName, value);
}

bool ExtractShortName(const xwalk::application::ApplicationData& app_data,
    std::string* value) {
  xwalk::application::WidgetInfo* info =
    static_cast<xwalk::application::WidgetInfo*>(
        app_data.GetManifestData(xwalk::application_widget_keys::kWidgetKey));
  if (!info)
    return false;
  return info->GetWidgetInfo()->GetString(kShortName, value);
}

bool ExtractVersion(const xwalk::application::ApplicationData& app_data,
    std::string* value) {
  xwalk::application::WidgetInfo* info =
    static_cast<xwalk::application::WidgetInfo*>(
        app_data.GetManifestData(xwalk::application_widget_keys::kWidgetKey));
  if (!info)
    return false;
  return info->GetWidgetInfo()->GetString(kVersion, value);
}

bool ExtractIconSrc(const base::Value& dict,
    std::string* value, const char** error) {
  const base::DictionaryValue* inner_dict;
  if (!dict.GetAsDictionary(&inner_dict)) {
    SetError(kErrMsgInvalidDictionary, kWidgetIconFullKey, error);
    return false;
  }
  std::string src;
  if (!inner_dict->GetString(kWidgetIconSrcKey, &src)) {
    SetError(kErrMsgNoMandatoryKey, kWidgetIconSrcKey, error);
    return false;
  }
  *value = src;
  return  true;
}

bool ExtractIcons(const xwalk::application::Manifest& manifest,
    std::vector<std::string>* value, const char** error) {
  base::Value* key_value;
  if (!manifest.Get(kWidgetIconFullKey, &key_value)) {
    value->clear();
    return true; // no icon, no error
  }

  std::vector<std::string> icons;
  if (key_value->IsType(base::Value::TYPE_DICTIONARY)) {
    std::string icon;
    if (!ExtractIconSrc(*key_value, &icon, error))
      return false;
    icons.push_back(icon);
  } else if (key_value->IsType(base::Value::TYPE_LIST)) {
    const base::ListValue* list;
    if (!key_value->GetAsList(&list)) {
      SetError(kErrMsgInvalidList, kWidgetIconFullKey, error);
      return false;
    }
    for (const base::Value* list_value : *list) {
      std::string icon;
      if (!ExtractIconSrc(*list_value, &icon, error))
        return false;
      icons.push_back(icon);
    }
  }

  value->swap(icons);
  return true;
}

bool ExtractApiVersion(const xwalk::application::ApplicationData& app_data,
    std::string* value) {
  xwalk::application::TizenApplicationInfo* info =
    static_cast<xwalk::application::TizenApplicationInfo*>(
        app_data.GetManifestData(
            xwalk::application_widget_keys::kTizenApplicationKey));
  if (!info)
    return false;
  *value = info->required_version();
  return true;
}

bool ExtractPrivilegeName(const base::Value& dict,
    std::string* value, const char** error) {
  const base::DictionaryValue* inner_dict;
  if (!dict.GetAsDictionary(&inner_dict)) {
    SetError(kErrMsgInvalidDictionary, kWidgetPrivilegeFullKey, error);
    return false;
  }
  std::string name;
  if (!inner_dict->GetString(kWidgetPrivilegeNameKey, &name)) {
    SetError(kErrMsgNoMandatoryKey, kWidgetPrivilegeNameKey, error);
    return false;
  }
  *value = name;
  return  true;
}

bool ExtractPrivileges(const xwalk::application::Manifest& manifest,
    std::vector<std::string>* value, const char** error) {
  base::Value* key_value;
  if (!manifest.Get(kWidgetPrivilegeFullKey, &key_value)) {
    value->clear();
    return true; // no privileges, no error
  }

  std::vector<std::string> privileges;
  if (key_value->IsType(base::Value::TYPE_DICTIONARY)) {
    std::string privilege;
    if (!ExtractPrivilegeName(*key_value, &privilege, error))
      return false;
    privileges.push_back(privilege);
  } else if (key_value->IsType(base::Value::TYPE_LIST)) {
    const base::ListValue* list;
    if (!key_value->GetAsList(&list)) {
      SetError(kErrMsgInvalidList, kWidgetPrivilegeFullKey, error);
      return false;
    }
    for (const base::Value* list_value : *list) {
      std::string privilege;
      if (!ExtractPrivilegeName(*list_value, &privilege, error))
        return false;
      privileges.push_back(privilege);
    }
  }

  value->swap(privileges);
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

  std::string package;
  if (!ExtractPackage(*app_data, &package)) {
    SetError(kErrMsgValueNotFound, "package id", error);
    return false;
  }

  std::string id;
  if (!ExtractId(*app_data, &id)) {
    SetError(kErrMsgValueNotFound, "application id", error);
    return false;
  }

  std::string name;
  if (!ExtractName(*app_data, &name)) {
    SetError(kErrMsgValueNotFound, "application name", error);
    return false;
  }

  std::string short_name;
  if (!ExtractShortName(*app_data, &short_name)) {
    SetError(kErrMsgValueNotFound, "application short name", error);
    return false;
  }

  std::string version;
  if (!ExtractVersion(*app_data, &version)) {
    SetError(kErrMsgValueNotFound, "application version", error);
    return false;
  }

  std::vector<std::string> icons;
  if (!ExtractIcons(*app_data->GetManifest(), &icons, error))
    return false;

  std::string api_version;
  if (!ExtractApiVersion(*app_data, &api_version)) {
    SetError(kErrMsgValueNotFound, "required api version", error);
    return false;
  }

  std::vector<std::string> privileges;
  if (!ExtractPrivileges(*app_data->GetManifest(), &privileges, error))
    return false;

  if (data) {
    scoped_ptr<LocalManifestData> local_manifest_data(new LocalManifestData);
    local_manifest_data->SetPackage(package);
    local_manifest_data->SetId(id);
    local_manifest_data->SetName(name);
    local_manifest_data->SetShortName(short_name);
    local_manifest_data->SetVersion(version);
    local_manifest_data->SetIcon(icons.empty() ? "" : icons.front());
    local_manifest_data->SetApiVersion(api_version);
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
