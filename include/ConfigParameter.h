

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <algorithm>
#include <cstring>

// Interface for any configuration parameter exposed over BLE
struct IConfigParameter {
  // BLE Characteristic UUID
  virtual const char* uuid() const = 0;
  // Human-readable name for logging
  virtual const char* name() const = 0;
  // Size in bytes when serialized
  virtual size_t size() const = 0;
  // Read the current value into a little-endian byte buffer
  virtual void read(uint8_t* buffer) const = 0;
  // Write a new value from a little-endian byte buffer of length len
  virtual void write(const uint8_t* buffer, size_t len) = 0;
  virtual ~IConfigParameter() = default;
};

// Templated implementation of IConfigParameter for primitive types
template<typename T>
class ConfigParameter : public IConfigParameter {
public:
  ConfigParameter(const char* u, const char* n,
                  std::function<T()> getter,
                  std::function<void(T)> setter)
    : _uuid(u), _name(n), _getter(std::move(getter)), _setter(std::move(setter)) {}

  const char* uuid() const override { return _uuid; }
  const char* name() const override { return _name; }
  size_t size() const override { return sizeof(T); }

  void read(uint8_t* buffer) const override {
    T value = _getter();
    // Copy little-endian representation
    std::memcpy(buffer, &value, sizeof(T));
  }

  void write(const uint8_t* buffer, size_t len) override {
    T value = 0;
    size_t copyLen = std::min(len, sizeof(T));
    std::memcpy(&value, buffer, copyLen);
    _setter(value);
  }

private:
  const char* _uuid;
  const char* _name;
  std::function<T()> _getter;
  std::function<void(T)> _setter;
};