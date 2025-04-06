#include "system_error_fmt.hpp"

// Make single statement from multiple
#define statement(...) do { __VA_ARGS__ } while(0)

// Throw napi value as JS exception
#define Nthrow(v) statement((v).ThrowAsJavaScriptException(); Nreturn_void;)

// Catch all C++ exceptions and rethrow them as JS exceptions
#define Nsafe_begin try
#define Nsafe_end                                                \
  catch (std::system_error& exc) {                               \
    Nthrow(Napi::Error::New(__napienv, formatSystemError(exc))); \
  }                                                              \
  catch (std::exception& exc) {                                  \
    Nthrow(Napi::Error::New(__napienv, exc.what()));             \
  }                                                              \
  catch (...) {                                                  \
    Nthrow(Napi::Error::New(__napienv, "Unknown exception"));    \
  }

// place at the top of any constructor
#define Nconstructor static constexpr bool __constructor = true; int __argidx = 0; auto __napienv = info.Env()
// place at the top of any other function
#define Ncallback    static constexpr bool __constructor = false; int __argidx = 0; auto __napienv = info.Env()
// return with no value from constructor/callback
#define Nreturn_void statement(if constexpr (__constructor) return; else return __napienv.Undefined();)

/// Type checkers

// int64_t
#define Nnumber_arg(name)                                                \
  if (!info[__argidx].IsNumber())                                        \
    Nthrow(Napi::Error::New(__napienv, #name " must be a number"));      \
  auto name = info[__argidx].As<Napi::Number>().Int64Value(); ++__argidx

// std::string
#define Nstring_arg(name)                                               \
  if (!info[__argidx].IsString())                                       \
    Nthrow(Napi::Error::New(__napienv, #name " must be a string"));     \
  auto name = info[__argidx].As<Napi::String>().Utf8Value(); ++__argidx

// std::string
#define Nstring_arg_opt(name,...)                         \
  std::string name{__VA_ARGS__};                          \
  if (info[__argidx].IsString())                          \
    name = info[__argidx].As<Napi::String>().Utf8Value(); \
  ++__argidx

// T*
#define Nclass_arg(name,T)                                             \
  if (!info[__argidx].IsObject() ||                                    \
      !T::IsInstance(info[__argidx].As<Napi::Object>()))               \
    Nthrow(Napi::Error::New(__napienv, #name " must be " #T));         \
  auto name = T::Unwrap(info[__argidx].As<Napi::Object>()); ++__argidx

/// =====

// Staticly allocated space for object
template <class T>
class ObjectStorage {
private:
  bool m_initialized;
  alignas(T) char m_data[sizeof(T)];

public:
  constexpr ObjectStorage() : m_initialized(false) {}
  inline ~ObjectStorage() { destruct(); }

  inline T* operator->() const noexcept { return (T*)m_data; }
  inline T& get() const noexcept { return *(T*)m_data; }

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

/// Tools to build C++ classes wrappers for bit7z


#define _Wrapper_(C,...)                                                      \
private:                                                                      \
  ObjectStorage<bit7z::C> m;                                                  \
  static inline Napi::FunctionReference* __Constructor = nullptr;             \
public:                                                                       \
  static inline std::vector<Napi::FunctionReference*> __Children = {};        \
  inline operator bit7z::C&() const noexcept { return m.get(); }              \
  static inline bool IsInstance(Napi::Object const& object) {                 \
    if (object.InstanceOf(__Constructor->Value())) return true;               \
    for (auto c : __Children) if (object.InstanceOf(c->Value())) return true; \
    return false;                                                             \
  }                                                                           \
  static inline Napi::Object Init(Napi::Env env, Napi::Object exports) {      \
    Napi::Function clazz = DefineClass(env, #C, __VA_ARGS__);                 \
    __Constructor = new Napi::FunctionReference();                            \
    *__Constructor = Napi::Persistent(clazz);                                 \
    env.SetInstanceData<Napi::FunctionReference>(__Constructor);              \
    exports.Set(#C, clazz);                                                   \
    __RegAsChild();                                                           \
    return exports;                                                           \
  }                                                                           \
  static void inline __RegAsChild() { 

#define Wrapper(C,...)  _Wrapper_(C,__VA_ARGS__)
#define Wmethod(Fn) InstanceMethod<&Fn>(#Fn, static_cast<napi_property_attributes>(napi_writable | napi_configurable))
#define Wderive(From)   From::__Children.push_back(__Constructor);
#define Wend            }

#define Winit(...)  statement(Nsafe_begin { m.construct(__VA_ARGS__); } Nsafe_end)
