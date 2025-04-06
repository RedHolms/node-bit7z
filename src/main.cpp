#include <napi.h>
#include <bit7z/bit7zlibrary.hpp>
#include <bit7z/bitarchivewriter.hpp>
#include <bit7z/bitformat.hpp>
#include "helpers.hpp"

template <typename WorkFnT>
class Bit7ZGenericWorker : public Napi::AsyncWorker {
private:
  Napi::Promise::Deferred m_deferred;
  WorkFnT m_work;

public:
  inline Bit7ZGenericWorker(const Napi::Env& env, const WorkFnT& work) : Napi::AsyncWorker(env), m_deferred(env), m_work(work) {}

public:
  inline Napi::Promise GetPromise() { return m_deferred.Promise(); }

protected:
  void Execute() {
    try {
      m_work();
    }
    catch (std::system_error& exc) {
      SetError(formatSystemError(exc));
    }
    catch (std::exception& exc) {
      SetError(exc.what());
    }
    catch (...) {
      SetError("Unknown exception");
    }
  }

  void OnOK() {
    m_deferred.Resolve(Env().Undefined());
  }
  void OnError(const Napi::Error& error) {
    m_deferred.Reject(error.Value());
  }
};

class BitInFormat : public Napi::ObjectWrap<BitInFormat> {
  Wrapper(BitInFormat, {})
  Wend

public:
  BitInFormat(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BitInFormat>(info) {
    Nconstructor;
    Nnumber_arg(value);
    Winit(value);
  }
};

class BitInOutFormat : public Napi::ObjectWrap<BitInOutFormat> {
  Wrapper(BitInOutFormat, {})
    Wderive(BitInFormat)
  Wend

public:
  BitInOutFormat(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BitInOutFormat>(info) {
    Nconstructor;
    Nnumber_arg(value);
    Nstring_arg(ext);
    Nnumber_arg(defaultMethod);
    Nnumber_arg(features);
    Winit(value, ext.c_str(), (bit7z::BitCompressionMethod)defaultMethod, (bit7z::FormatFeatures)features);
  }
};

class Bit7zLibrary : public Napi::ObjectWrap<Bit7zLibrary> {
  Wrapper(Bit7zLibrary, {})
  Wend

public:
  Bit7zLibrary(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Bit7zLibrary>(info) {
    Nconstructor;
    Nstring_arg(dllName);
    Winit(dllName);
  }
};

class BitArchiveWriter : public Napi::ObjectWrap<BitArchiveWriter> {
  Wrapper(BitArchiveWriter, {
    Wmethod(addDirectory),
    Wmethod(addFile),
    Wmethod(compressTo)
  })
  Wend

public:
  BitArchiveWriter(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BitArchiveWriter>(info) {
    Nconstructor;
    Nclass_arg(lib, Bit7zLibrary);
    Nclass_arg(format, BitInOutFormat);
    Winit(*lib, *format);
  }

  Napi::Value addDirectory(const Napi::CallbackInfo& info) {
    Ncallback;
    Nstring_arg(directoryPath);
    Nsafe_begin {
      m->addDirectory(directoryPath);
    } Nsafe_end
    return info.Env().Undefined();
  }

  Napi::Value addFile(const Napi::CallbackInfo& info) {
    Ncallback;
    Nstring_arg(filePath);
    Nstring_arg_opt(name);
    Nsafe_begin {
      m->addFile(filePath, name);
    } Nsafe_end
    return info.Env().Undefined();
  }

  Napi::Value compressTo(const Napi::CallbackInfo& info) {
    Ncallback;
    Nstring_arg(outFilePath);
    auto work = new Bit7ZGenericWorker(info.Env(), [this, outFilePath]() {
      m->compressTo(outFilePath);
    });
    work->Queue();
    return work->GetPromise();
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
