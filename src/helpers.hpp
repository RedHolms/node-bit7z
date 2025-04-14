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

#define Nenv __napienv

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

// Napi::Function
#define Nfunction(name,...)                                           \
  if (!info[__argidx].IsFunction())                                   \
    Nthrow(Napi::Error::New(__napienv, #name " must be a function")); \
  auto name = info[__argidx].As<Napi::Function>(); ++__argidx

// T*
#define Nclass_arg(name,T)                                             \
  if (!info[__argidx].IsObject() ||                                    \
      !T::IsInstance(info[__argidx].As<Napi::Object>()))               \
    Nthrow(Napi::Error::New(__napienv, #name " must be " #T));         \
  auto name = T::Unwrap(info[__argidx].As<Napi::Object>()); ++__argidx

/// =====

// Calls any function at scope exit
template <typename FnT>
class ScopeExit {
private:
  FnT m_fn;
public:
  inline ScopeExit(const FnT& fn) : m_fn(fn) {}
  inline ~ScopeExit() { m_fn(); }
};

/// Tools to build C++ classes wrappers for bit7z

// storage of bit7z object
template <class T>
class Bit7ZObject {
private:
  alignas(T) char m_data[sizeof(T)];
  bool m_initialized;

public:
  constexpr Bit7ZObject() : m_initialized(false) {}
  inline ~Bit7ZObject() { destruct(); }

  inline T* _obj() const noexcept { return (T*)m_data; }
  inline T* operator->() const noexcept { return _obj(); }

  template <typename... ArgsT>
  inline void construct(ArgsT&&... args) {
    if (m_initialized) return;
    m_initialized = true;
    new (m_data) T(std::forward<ArgsT>(args)...);
  }

  inline void destruct() {
    if (!m_initialized) return;
    m_initialized = false;
    ((T*)m_data)->~T();
  }
};

#define HeadBit7zWrapper(C)                                                   \
  using PropertiesList = std::vector<Napi::ClassPropertyDescriptor<C>>;       \
public:                                                                       \
  static inline std::vector<Napi::FunctionReference*> __Children = {};        \
  bit7z::C* getObj() { return m._obj(); }                                     \
  static inline Napi::Object Init(Napi::Env env, Napi::Object exports) {      \
    PropertiesList props;                                                     \
    __Derive();                                                               \
    RegisterProperties(props);                                                \
    Napi::Function clazz = DefineClass(env, #C, std::move(props));            \
    __Constructor = new Napi::FunctionReference();                            \
    *__Constructor = Napi::Persistent(clazz);                                 \
    env.SetInstanceData<Napi::FunctionReference>(__Constructor);              \
    exports.Set(#C, clazz);                                                   \
    return exports;                                                           \
  }                                                                           \
  static inline bool IsInstance(Napi::Object const& object) {                 \
    if (object.InstanceOf(__Constructor->Value())) return true;               \
    for (auto c : __Children) if (object.InstanceOf(c->Value())) return true; \
    return false;                                                             \
  }                                                                           \
private:                                                                      \
  Bit7ZObject<bit7z::C> m;                                                    \
  static inline Napi::FunctionReference* __Constructor = nullptr;             \
  static inline void __Derive()

#define METHOD(Fn)   InstanceMethod<&Fn>(#Fn, static_cast<napi_property_attributes>(napi_writable | napi_configurable))
#define DERIVE(From) From::__Children.push_back(__Constructor)

#define Bit7zWrapperInit(...) statement(Nsafe_begin { m.construct(__VA_ARGS__); } Nsafe_end)
