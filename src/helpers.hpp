// Throw napi value as JS exception
#define _NAPI_THROW(v) do { (v).ThrowAsJavaScriptException(); _NAPI_RET_VOID; } while(0)

// Catch all C++ exceptions and rethrow them as JS exceptions
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

// Execute expression and rethrow all thrown C++ exceptions as JS exceptions
#define _NAPI_SAFE(expr) do { try { expr; } _NAPI_RETHROW_ALL } while(0)

/// We can't return Undefined() in constructor, so use this guard
static constexpr bool _inNAPIConstructor = false;
#define _CONSTRUCTOR static constexpr bool _inNAPIConstructor  = true;

#define _NAPI_RET_VOID do { if constexpr (_inNAPIConstructor) return; else return env.Undefined(); } while(0)

/// Type checkers
// You should define "idx" variable set to 0 before using any of them

#define _NUMBER(name)                                              \
  if (!info[idx].IsNumber())                                       \
    _NAPI_THROW(Napi::Error::New(env, #name " must be a number")); \
  auto name ## _napival = info[idx].As<Napi::Number>();            \
  auto name = (name ## _napival).Int64Value(); ++idx

#define _STRING(name)                                              \
  if (!info[idx].IsString())                                       \
    _NAPI_THROW(Napi::Error::New(env, #name " must be a string")); \
  auto name ## _napival = info[idx].As<Napi::String>();            \
  auto name = (name ## _napival).Utf8Value(); ++idx

#define _STRING_OPT(name,...)                                             \
  std::string name{__VA_ARGS__};                                          \
  Napi::String name ## _napival;                                          \
  if (info[idx].IsString())                                               \
    name = (name ## _napival = info[idx].As<Napi::String>()).Utf8Value(); \
  ++idx

#define _CLASS(name,cls)                                                       \
  if (!info[idx].IsObject() || !cls::IsInstance(info[idx].As<Napi::Object>())) \
    _NAPI_THROW(Napi::Error::New(env, #name " must be a " #cls));              \
  auto name = info[idx].As<Napi::Object>(); ++idx

/// Staticly allocated space for object
template <class T>
class TStorage {
private:
  bool m_initialized;
  alignas(T) char m_data[sizeof(T)];

public:
  constexpr TStorage() : m_initialized(false) {}
  inline ~TStorage() { destruct(); }

  inline operator T&() const noexcept { return *(T*)m_data; }
  inline T* operator->() const noexcept { return (T*)m_data; }

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

static constexpr uint8_t hexCharToNum(char chr) {
  if (chr >= '0' && chr <= '9')
    return chr - '0';
  if (chr >= 'a')
    return (chr - 'a') + 10;
  return 0;
}

// assume that uuidStr is valid 12345678-9abc-def0-1234-56789abcdef0 type string
//  all chars are lowercase
static constexpr napi_type_tag initTagFromStr(const char* uuidStr) {
  napi_type_tag result = { 0 };
  uint8_t offset = 0;
  while (*uuidStr) {
    if (*uuidStr == '-') ++uuidStr;
    uint8_t byte = (hexCharToNum(uuidStr[0]) << 4) | hexCharToNum(uuidStr[1]);
    if (offset < 64)
      result.lower |= (uint64_t)byte << offset;
    else
      result.upper |= (uint64_t)byte << (offset - 64);
    offset += 8;
    uuidStr += 2;
  }
  return result;
}

#define _INIT(...) _NAPI_SAFE(m.construct(__VA_ARGS__))

/// Layout for Napi::ObjectWrap<...>, also initialization of unique tags for
///  type cheking. If class is inherited from another, use _BIT7Z_WRAPPER_DERIVED
///  !!! Actual class (ObjectWrap) should not be derived

#define _BIT7Z_WRAPPER_BASE_(cls,addinit,...)                                       \
  TStorage<bit7z::cls> m;                                                           \
public:                                                                             \
  static inline Napi::FunctionReference* CONSTRUCTOR = nullptr;                     \
  static inline std::vector<Napi::FunctionReference*> CHILDREN = {};                \
  inline operator bit7z::cls&() const noexcept { return m; }                        \
  static inline bool IsInstance(Napi::Object const& object) {                       \
    if (object.InstanceOf(CONSTRUCTOR->Value())) return true;                       \
    for (auto child : CHILDREN) if (object.InstanceOf(child->Value())) return true; \
    return false;                                                                   \
  }                                                                                 \
  static Napi::Object Init(Napi::Env env, Napi::Object exports) {                   \
    Napi::Function clazz = DefineClass(env, #cls, __VA_ARGS__);                     \
    CONSTRUCTOR = new Napi::FunctionReference();                                    \
    *CONSTRUCTOR = Napi::Persistent(clazz);                                         \
    env.SetInstanceData<Napi::FunctionReference>(CONSTRUCTOR);                      \
    exports.Set(#cls, clazz);                                                       \
    addinit                                                                         \
    return exports;                                                                 \
  }                                                                                               

#define _BIT7Z_WRAPPER(cls,...) _BIT7Z_WRAPPER_BASE_(cls,;,__VA_ARGS__)
#define _BIT7Z_WRAPPER_DERIVED(cls,parent,...) _BIT7Z_WRAPPER_BASE_(cls,parent::CHILDREN.push_back(CONSTRUCTOR);,__VA_ARGS__)

#define _METHOD(name) InstanceMethod<&name>(#name, static_cast<napi_property_attributes>(napi_writable | napi_configurable))
