#include <napi.h>
#include <bit7z/bit7zlibrary.hpp>
#include <Windows.h>

#define _NAPI_THROW(v) (v).ThrowAsJavaScriptException(); return
#define _NAPI_RETHROW_ALL                                       \
  catch (std::system_error& exc) {                              \
    _NAPI_THROW(Napi::Error::New(env, formatSystemError(exc))); \
  }                                                             \
  catch (std::exception& exc) {                                 \
    _NAPI_THROW(Napi::Error::New(env, exc.what()));             \
  }                                                             \
  catch (...) {                                                 \
    _NAPI_THROW(Napi::Error::New(env, "Unknown exception"));    \
  }
#define _NAPI_PROT(expr) do { try { expr; } _NAPI_RETHROW_ALL } while(0)

#define _STRING(n) if (!info[idx].IsString()) { _NAPI_THROW(Napi::Error::New(env, #n " must be a string")); } auto n = info[idx].As<Napi::String>(); ++idx

template <class T>
class TStorage {
private:
  bool m_initialized;
  alignas(T) char m_data[sizeof(T)];

public:
  constexpr TStorage() : m_initialized(false) {}
  inline ~TStorage() { destruct(); }

  inline operator T&() const noexcept { return *(T*)m_data; }

  template <typename... ArgsT>
  inline void construct(ArgsT&&... args) {
    if (m_initialized) return;
    m_initialized = true;
    new (m_data) T (std::forward<ArgsT>(args)...);
  }

  inline void destruct() {
    if (!m_initialized) return;
    m_initialized = false;
    ((T*)m_data)->~T();
  }
};

std::string formatSystemError(std::system_error& error) {
  // FUCKING MICROSOFT uses their fucking ANSI fucking everywhere
  // It's not 98 you bag of shit, UTF-8 was invented IN 19 FUCKING 92 MAN
  // i'm crying
  // this functions retuns NICE AND PRETTY UTF-8 string in format:
  // [<code_cat>:<code>] <info>: <code info>

  std::string result = "[";
  result += error.code().category().name();
  result += ':';
  result += std::to_string(error.code().value());
  result += "] ";

  if (error.code().category() != std::system_category()) {
    // don't do much shit
    result += error.what();
    return result;
  }

  // get info without stupid ansi str of code
  {
    std::string info = error.what();
    auto index = info.find_last_of(':');
    if (index == std::string::npos) {
      // no original info
      info = "No info";
    }
    else {
      info.resize(index);
    }
    result += info;
  }

  int code = error.code().value();

  LPWSTR messageBuffer = nullptr;
  size_t wideCount = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL, code, 0, (LPWSTR)&messageBuffer, 0, NULL);

  size_t allocated = (wideCount * 4) + 1;
  char* buffer = new char[allocated];
  size_t written = WideCharToMultiByte(
    CP_UTF8, 0,
    messageBuffer, wideCount,
    buffer, allocated,
    nullptr, nullptr
  );
  buffer[written] = 0;

  // remove whitespaces at the end (STUPID FUKICNG MICROSOFT AGAIN WHY THE FUCK DO YOU WANT TO PUT FUCKING NEW LINE CHARACTER AT THE END OF FORMATTED FUCKING ERROR STRING)
  for (int i = written - 1; i >= 0; --i) {
    if (isspace(buffer[i]))
      buffer[i] = 0;
    else break;
  }

  LocalFree(messageBuffer);

  result += ": ";
  result += buffer;
  delete[] buffer;

  return result;
}

class N_Bit7zLibrary : public Napi::ObjectWrap<N_Bit7zLibrary> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  N_Bit7zLibrary(const Napi::CallbackInfo& info); 

private:
  TStorage<bit7z::Bit7zLibrary> m;
};

N_Bit7zLibrary::N_Bit7zLibrary(const Napi::CallbackInfo& info) : Napi::ObjectWrap<N_Bit7zLibrary>(info) {
  Napi::Env env = info.Env();
  int idx = 0;
  _STRING(dllName);
  _NAPI_PROT(m.construct(dllName.Utf8Value()));
}

Napi::Object N_Bit7zLibrary::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function clazz = DefineClass(env, "Bit7zLibrary", {});
  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(clazz);
  env.SetInstanceData<Napi::FunctionReference>(constructor);
  exports.Set("Bit7zLibrary", clazz);
  return exports;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  N_Bit7zLibrary::Init(env, exports);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)

