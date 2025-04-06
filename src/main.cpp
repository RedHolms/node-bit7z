#include <napi.h>
#include <bit7z/bit7zlibrary.hpp>
#include <bit7z/bitarchivewriter.hpp>
#include <bit7z/bitformat.hpp>
#include "system_error_fmt.hpp"
#include "helpers.hpp"

class BitInFormat : public Napi::ObjectWrap<BitInFormat> {
  _BIT7Z_WRAPPER(BitInFormat, {})

public:
  BitInFormat(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BitInFormat>(info) {
    _CONSTRUCTOR;
    Napi::Env env = info.Env();
    int idx = 0;
    _NUMBER(value);
    _INIT(value);
  }
};

class BitInOutFormat : public Napi::ObjectWrap<BitInOutFormat> {
  _BIT7Z_WRAPPER_DERIVED(BitInOutFormat, BitInFormat, {})

public:
  BitInOutFormat(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BitInOutFormat>(info) {
    _CONSTRUCTOR;
    Napi::Env env = info.Env();
    int idx = 0;
    _NUMBER(value);
    _STRING(ext);
    _NUMBER(defaultMethod);
    _NUMBER(features);
    _INIT(value, ext.c_str(), (bit7z::BitCompressionMethod)defaultMethod, (bit7z::FormatFeatures)features);
  }
};

class Bit7zLibrary : public Napi::ObjectWrap<Bit7zLibrary> {
  _BIT7Z_WRAPPER(Bit7zLibrary, {})

public:
  Bit7zLibrary(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Bit7zLibrary>(info) {
    _CONSTRUCTOR;
    Napi::Env env = info.Env();
    int idx = 0;
    _STRING(dllName);
    _INIT(dllName);
  }
};

class BitArchiveWriter : public Napi::ObjectWrap<BitArchiveWriter> {
  _BIT7Z_WRAPPER(BitArchiveWriter, {
    _METHOD(addDirectory),
    _METHOD(addFile),
    _METHOD(compressTo)
  })

public:
  BitArchiveWriter(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BitArchiveWriter>(info) {
    _CONSTRUCTOR;
    Napi::Env env = info.Env();
    int idx = 0;
    _CLASS(lib, Bit7zLibrary);
    _CLASS(format, BitInOutFormat);
    _INIT(*Bit7zLibrary::Unwrap(lib), *BitInOutFormat::Unwrap(format));
  }

  Napi::Value addDirectory(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    int idx = 0;
    _STRING(directoryPath);
    _NAPI_SAFE(m->addDirectory(directoryPath));
    return env.Undefined();
  }

  Napi::Value addFile(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    int idx = 0;
    _STRING(filePath);
    _STRING_OPT(name);
    _NAPI_SAFE(m->addFile(filePath, name));
    return env.Undefined();
  }

  Napi::Value compressTo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    int idx = 0;
    _STRING(outFilePath);
    _NAPI_SAFE(m->compressTo(outFilePath));
    return env.Undefined();
  }
};

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  BitInFormat::Init(env, exports);
  BitInOutFormat::Init(env, exports);
  Bit7zLibrary::Init(env, exports);
  BitArchiveWriter::Init(env, exports);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
