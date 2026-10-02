#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <new>
#include "esp_err.h"

using nvs_handle_t = uintptr_t;
constexpr int NVS_READONLY = 0;
constexpr int NVS_READWRITE = 1;
constexpr esp_err_t ESP_ERR_NVS_NOT_FOUND = -4;
constexpr esp_err_t ESP_ERR_NVS_INVALID_LENGTH = -5;

namespace simulator_nvs {
struct Handle { char name[16] = {}; bool readOnly = false; };
inline bool validName(const char* name) {
  if (!name || !*name || std::strlen(name) > 15) return false;
  for (const char* p=name; *p; ++p) {
    if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_')) return false;
  }
  return true;
}
inline std::filesystem::path path(const Handle& handle, const char* key) {
  const char* root = std::getenv("CROSSPOINT_SIM_NVS");
  if (root && *root) return std::filesystem::path(root) / handle.name / key;
  const char* sd = std::getenv("CROSSPOINT_SIM_SD");
  return std::filesystem::path(std::string(sd && *sd ? sd : "sd") + ".nvs") / handle.name / key;
}
}  // namespace simulator_nvs

inline esp_err_t nvs_open(const char* name, int mode, nvs_handle_t* result) {
  if (!result || !simulator_nvs::validName(name)) return ESP_ERR_INVALID_ARG;
  // A bounded native handle (17 bytes); nvs_close owns it. No MCU code uses this shim.
  auto* handle = new (std::nothrow) simulator_nvs::Handle;
  if (!handle) return ESP_ERR_NO_MEM;
  std::memcpy(handle->name, name, std::strlen(name));
  handle->readOnly = mode == NVS_READONLY;
  *result = reinterpret_cast<nvs_handle_t>(handle);
  return ESP_OK;
}
inline esp_err_t nvs_get_blob(nvs_handle_t raw, const char* key, void* value, size_t* length) {
  if (!raw || !length || !simulator_nvs::validName(key)) return ESP_ERR_INVALID_ARG;
  const auto file = simulator_nvs::path(*reinterpret_cast<simulator_nvs::Handle*>(raw), key);
  std::error_code error;
  const auto size = std::filesystem::file_size(file, error);
  if (error) return error == std::errc::no_such_file_or_directory ? ESP_ERR_NVS_NOT_FOUND : ESP_FAIL;
  if (!value) { *length = size; return ESP_OK; }
  if (*length < size) { *length = size; return ESP_ERR_NVS_INVALID_LENGTH; }
  std::ifstream input(file, std::ios::binary);
  if (!input.read(static_cast<char*>(value), size)) return ESP_FAIL;
  *length = size;
  return ESP_OK;
}
inline esp_err_t nvs_set_blob(nvs_handle_t raw, const char* key, const void* value, size_t length) {
  if (!raw || (!value && length) || !simulator_nvs::validName(key)) return ESP_ERR_INVALID_ARG;
  const auto& handle = *reinterpret_cast<simulator_nvs::Handle*>(raw);
  if (handle.readOnly) return ESP_FAIL;
  const auto file = simulator_nvs::path(handle, key);
  std::error_code error;
  std::filesystem::create_directories(file.parent_path(), error);
  if (error) return ESP_FAIL;
  const auto temporary = file.string() + ".tmp";
  {
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (!output.write(static_cast<const char*>(value), length)) return ESP_FAIL;
    output.close();
    if (output.fail()) return ESP_FAIL;
  }
  std::filesystem::rename(temporary, file, error);
  return error ? ESP_FAIL : ESP_OK;
}
inline esp_err_t nvs_get_u8(nvs_handle_t, const char*, uint8_t*) { return ESP_FAIL; }
inline esp_err_t nvs_set_u8(nvs_handle_t, const char*, uint8_t) { return ESP_OK; }
inline esp_err_t nvs_commit(nvs_handle_t) { return ESP_OK; }
inline void nvs_close(nvs_handle_t raw) { delete reinterpret_cast<simulator_nvs::Handle*>(raw); }
