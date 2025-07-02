#include <napi.h>
#include <bit7z/bit7zlibrary.hpp>
#include <bit7z/bitabstractarchiveopener.hpp>
#include <bit7z/bitarchivewriter.hpp>
#include <bit7z/bitarchiveeditor.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bitfileextractor.hpp>
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

class Bit7zLibrary : public Napi::ObjectWrap<Bit7zLibrary> {
  HeadBit7zWrapper(Bit7zLibrary) {}

public:
  static inline void RegisterProperties(PropertiesList& properties) {}

  Bit7zLibrary(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Bit7zLibrary>(info) {
    Nconstructor;
    Nstring_arg(dllName);
    Bit7zWrapperInit(dllName);
  }
};

class BitInFormat : public Napi::ObjectWrap<BitInFormat> {
  HeadBit7zWrapper(BitInFormat) {}

public:
  static inline void RegisterProperties(PropertiesList& properties) {
    properties.push_back(METHOD(value));
  }

  BitInFormat(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BitInFormat>(info) {
    Nconstructor;
    Nnumber_arg(value);
    Bit7zWrapperInit(value);
  }

  Napi::Value value(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), m->value());
  }
};

class BitInOutFormat : public Napi::ObjectWrap<BitInOutFormat> {
  HeadBit7zWrapper(BitInOutFormat) {
    DERIVE(BitInFormat);
  }

public:
  static inline void RegisterProperties(PropertiesList& properties) {
    properties.push_back(METHOD(value));
    properties.push_back(METHOD(extension));
    properties.push_back(METHOD(features));
    properties.push_back(METHOD(hasFeature));
    properties.push_back(METHOD(defaultMethod));
  }

  BitInOutFormat(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BitInOutFormat>(info) {
    Nconstructor;
    Nnumber_arg(value);
    Nstring_arg(ext);
    Nnumber_arg(defaultMethod);
    Nnumber_arg(features);
    Bit7zWrapperInit(value, ext.c_str(), (bit7z::BitCompressionMethod)defaultMethod, (bit7z::FormatFeatures)features);
  }

  Napi::Value value(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), m->value());
  }

  Napi::Value extension(const Napi::CallbackInfo& info) {
    return Napi::String::New(info.Env(), m->extension());
  }

  Napi::Value features(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), (uint32_t)m->features());
  }

  Napi::Value hasFeature(const Napi::CallbackInfo& info) {
    Ncallback;
    Nnumber_arg(feature);
    return Napi::Boolean::New(Nenv, m->hasFeature((bit7z::FormatFeatures)feature));
  }

  Napi::Value defaultMethod(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), (uint32_t)m->defaultMethod());
  }
};

class BitCallbacks {
protected:
  Napi::Reference<Napi::Function> m_fileCallback;
  Napi::ThreadSafeFunction m_fileCallbackLock;
  Napi::Reference<Napi::Function> m_passwordCallback;
  Napi::ThreadSafeFunction m_passwordCallbackLock;
  Napi::Reference<Napi::Function> m_progressCallback;
  Napi::ThreadSafeFunction m_progressCallbackLock;
  Napi::Reference<Napi::Function> m_ratioCallback;
  Napi::ThreadSafeFunction m_ratioCallbackLock;
  Napi::Reference<Napi::Function> m_totalCallback;
  Napi::ThreadSafeFunction m_totalCallbackLock;

  void installCallbacks(bit7z::BitAbstractArchiveHandler* obj) {
    obj->setFileCallback([this](std::string itemPath) {
      if (m_fileCallbackLock) {
        m_fileCallbackLock.BlockingCall([itemPath](Napi::Env env, Napi::Function func) {
          func.Call({ Napi::String::New(env, itemPath) });
        });
      }
    });
    obj->setPasswordCallback([this]() -> std::string {
      std::string returnValue = "";
      if (m_passwordCallbackLock) {
        m_passwordCallbackLock.BlockingCall([&returnValue](Napi::Env env, Napi::Function func) {
          auto value = func.Call({});
          if (value.IsString())
            returnValue = value.As<Napi::String>().Utf8Value();
        });
      }
      return returnValue;
    });
    obj->setProgressCallback([this](uint64_t done) -> bool {
      bool returnValue = true;
      if (m_progressCallbackLock) {
        m_progressCallbackLock.BlockingCall([done, &returnValue](Napi::Env env, Napi::Function func) {
          Napi::Value value = func.Call({ Napi::Number::New(env, done) });
          if (value.IsBoolean())
            returnValue = value.As<Napi::Boolean>().Value();
        });
      }
      return returnValue;
    });
    obj->setRatioCallback([this](uint64_t in, uint64_t out) {
      if (m_ratioCallbackLock) {
        m_ratioCallbackLock.BlockingCall([in, out](Napi::Env env, Napi::Function func) {
          func.Call({ Napi::Number::New(env, in), Napi::Number::New(env, out) });
        });
      }
    });
    obj->setTotalCallback([this](uint64_t total) {
      if (m_totalCallbackLock) {
        m_totalCallbackLock.BlockingCall([total](Napi::Env env, Napi::Function func) {
          func.Call({ Napi::Number::New(env, total) });
        });
      }
    });
  }

  void lockFunctions() {
    if (m_fileCallback) {
      m_fileCallbackLock = Napi::ThreadSafeFunction::New(m_fileCallback.Env(), m_fileCallback.Value(), "bit7z File Callback", 0, 1);
    }
    if (m_passwordCallback) {
      m_passwordCallbackLock = Napi::ThreadSafeFunction::New(m_passwordCallback.Env(), m_passwordCallback.Value(), "bit7z Password Callback", 0, 1);
    }
    if (m_progressCallback) {
      m_progressCallbackLock = Napi::ThreadSafeFunction::New(m_progressCallback.Env(), m_progressCallback.Value(), "bit7z Progress Callback", 0, 1);
    }
    if (m_ratioCallback) {
      m_ratioCallbackLock = Napi::ThreadSafeFunction::New(m_ratioCallback.Env(), m_ratioCallback.Value(), "bit7z Ratio Callback", 0, 1);
    }
    if (m_totalCallback) {
      m_totalCallbackLock = Napi::ThreadSafeFunction::New(m_totalCallback.Env(), m_totalCallback.Value(), "bit7z Total Callback", 0, 1);
    }
  }

  void unlockFunctions() {
    if (m_fileCallbackLock) m_fileCallbackLock.Release();
    if (m_passwordCallbackLock) m_passwordCallbackLock.Release();
    if (m_progressCallbackLock) m_progressCallbackLock.Release();
    if (m_ratioCallbackLock) m_ratioCallbackLock.Release();
    if (m_totalCallbackLock) m_totalCallbackLock.Release();
  }
};

class BitArchiveWriter : public Napi::ObjectWrap<BitArchiveWriter>, public BitCallbacks {
  HeadBit7zWrapper(BitArchiveWriter) {}

public:
  static inline void RegisterProperties(PropertiesList& properties) {
    properties.push_back(METHOD(setFileCallback));
    properties.push_back(METHOD(setPasswordCallback));
    properties.push_back(METHOD(setProgressCallback));
    properties.push_back(METHOD(setRatioCallback));
    properties.push_back(METHOD(setTotalCallback));
    properties.push_back(METHOD(addDirectory));
    properties.push_back(METHOD(addFile));
    properties.push_back(METHOD(compressTo));
  }

  BitArchiveWriter(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BitArchiveWriter>(info) {
    Nconstructor;
    Nclass_arg(lib, Bit7zLibrary);
    Nclass_arg(format, BitInOutFormat);
    Bit7zWrapperInit(*lib->getObj(), *format->getObj());
    installCallbacks(m._obj());
  }

  Napi::Value setFileCallback(const Napi::CallbackInfo& info) {
    Ncallback;
    Nfunction(callback);
    m_fileCallback = Napi::Reference<Napi::Function>::New(callback);
    return Nenv.Undefined();
  }

  Napi::Value setPasswordCallback(const Napi::CallbackInfo& info) {
    Ncallback;
    Nfunction(callback);
    m_passwordCallback = Napi::Reference<Napi::Function>::New(callback);
    return Nenv.Undefined();
  }

  Napi::Value setProgressCallback(const Napi::CallbackInfo& info) {
    Ncallback;
    Nfunction(callback);
    m_progressCallback = Napi::Reference<Napi::Function>::New(callback);
    return Nenv.Undefined();
  }

  Napi::Value setRatioCallback(const Napi::CallbackInfo& info) {
    Ncallback;
    Nfunction(callback);
    m_ratioCallback = Napi::Reference<Napi::Function>::New(callback);
    return Nenv.Undefined();
  }

  Napi::Value setTotalCallback(const Napi::CallbackInfo& info) {
    Ncallback;
    Nfunction(callback);
    m_totalCallback = Napi::Reference<Napi::Function>::New(callback);
    return Nenv.Undefined();
  }

  Napi::Value addDirectory(const Napi::CallbackInfo& info) {
    Ncallback;
    Nstring_arg(directoryPath);
    Nsafe_begin{
      m->addDirectory(directoryPath);
    } Nsafe_end
      return info.Env().Undefined();
  }

  Napi::Value addFile(const Napi::CallbackInfo& info) {
    Ncallback;
    Nstring_arg(filePath);
    Nstring_arg_opt(name);
    Nsafe_begin{
      m->addFile(filePath, name);
    } Nsafe_end
      return info.Env().Undefined();
  }

  Napi::Value compressTo(const Napi::CallbackInfo& info) {
    Ncallback;
    Nstring_arg(outFilePath);
    lockFunctions();
    auto work = new Bit7ZGenericWorker(info.Env(), [this, outFilePath]() {
      ScopeExit _scope{ [this]() { unlockFunctions(); } };
      m->compressTo(outFilePath);
    });
    work->Queue();
    return work->GetPromise();
  }
};

class BitArchiveEditor : public Napi::ObjectWrap<BitArchiveEditor>, public BitCallbacks {
  HeadBit7zWrapper(BitArchiveEditor) {
    DERIVE(BitArchiveWriter);
  }

public:
  static inline void RegisterProperties(PropertiesList& properties) {
    properties.push_back(METHOD(setFileCallback));
    properties.push_back(METHOD(setPasswordCallback));
    properties.push_back(METHOD(setProgressCallback));
    properties.push_back(METHOD(setRatioCallback));
    properties.push_back(METHOD(setTotalCallback));
    properties.push_back(METHOD(addDirectory));
    properties.push_back(METHOD(addFile));
    properties.push_back(METHOD(compressTo));
    properties.push_back(METHOD(updateItem));
    properties.push_back(METHOD(applyChanges));
  }

  BitArchiveEditor(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BitArchiveEditor>(info) {
    Nconstructor;
    Nclass_arg(lib, Bit7zLibrary);
    Nstring_arg(archiveFilePath);
    Nclass_arg(format, BitInOutFormat);
    Nstring_arg_opt(password, "");
    Bit7zWrapperInit(*lib->getObj(), archiveFilePath, *format->getObj(), password);
    installCallbacks(m._obj());
  }

  Napi::Value setFileCallback(const Napi::CallbackInfo& info) {
    Ncallback;
    Nfunction(callback);
    m_fileCallback = Napi::Reference<Napi::Function>::New(callback);
    return Nenv.Undefined();
  }

  Napi::Value setPasswordCallback(const Napi::CallbackInfo& info) {
    Ncallback;
    Nfunction(callback);
    m_passwordCallback = Napi::Reference<Napi::Function>::New(callback);
    return Nenv.Undefined();
  }

  Napi::Value setProgressCallback(const Napi::CallbackInfo& info) {
    Ncallback;
    Nfunction(callback);
    m_progressCallback = Napi::Reference<Napi::Function>::New(callback);
    return Nenv.Undefined();
  }

  Napi::Value setRatioCallback(const Napi::CallbackInfo& info) {
    Ncallback;
    Nfunction(callback);
    m_ratioCallback = Napi::Reference<Napi::Function>::New(callback);
    return Nenv.Undefined();
  }

  Napi::Value setTotalCallback(const Napi::CallbackInfo& info) {
    Ncallback;
    Nfunction(callback);
    m_totalCallback = Napi::Reference<Napi::Function>::New(callback);
    return Nenv.Undefined();
  }

  Napi::Value addDirectory(const Napi::CallbackInfo& info) {
    Ncallback;
    Nstring_arg(directoryPath);
    Nsafe_begin{
      m->addDirectory(directoryPath);
    } Nsafe_end
      return info.Env().Undefined();
  }

  Napi::Value addFile(const Napi::CallbackInfo& info) {
    Ncallback;
    Nstring_arg(filePath);
    Nstring_arg_opt(name);
    Nsafe_begin{
      m->addFile(filePath, name);
    } Nsafe_end
      return info.Env().Undefined();
  }

  Napi::Value compressTo(const Napi::CallbackInfo& info) {
    Ncallback;
    Nstring_arg(outFilePath);
    lockFunctions();
    auto work = new Bit7ZGenericWorker(info.Env(), [this, outFilePath]() {
      ScopeExit _scope{ [this]() { unlockFunctions(); } };
      m->compressTo(outFilePath);
    });
    work->Queue();
    return work->GetPromise();
  }

  Napi::Value updateItem(const Napi::CallbackInfo& info) {
    Ncallback;
    Nstring_arg(itemPath);
    Nstring_arg(inputFilePath);
    Nsafe_begin{
      m->updateItem(itemPath, inputFilePath);
    } Nsafe_end
    return info.Env().Undefined();
  }

  Napi::Value applyChanges(const Napi::CallbackInfo& info) {
    Ncallback;
    lockFunctions();
    auto work = new Bit7ZGenericWorker(Nenv, [this]() {
      ScopeExit _scope{ [this]() { unlockFunctions(); } };
      m->applyChanges();
    });
    work->Queue();
    return work->GetPromise();
  }
};

class BitFileExtractor : public Napi::ObjectWrap<BitFileExtractor>, public BitCallbacks {
  HeadBit7zWrapper(BitFileExtractor) {
  }

public:
  static inline void RegisterProperties(PropertiesList& properties) {
    properties.push_back(METHOD(extract));
  }

  BitFileExtractor(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BitFileExtractor>(info) {
    Nconstructor;
    Nclass_arg(lib, Bit7zLibrary);
    Nclass_arg(format, BitInOutFormat);
    Bit7zWrapperInit(*lib->getObj(), *format->getObj());
  }

  Napi::Value extract(const Napi::CallbackInfo& info) {
    Ncallback;
    Nstring_arg(archivePath);
    Nstring_arg(destDirPath);
    auto work = new Bit7ZGenericWorker(info.Env(), [this, archivePath, destDirPath]() {
      m->extract(archivePath, destDirPath);
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
  BitArchiveEditor::Init(env, exports);
  BitFileExtractor::Init(env, exports);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
