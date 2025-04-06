#include "system_error_fmt.hpp"

#include <Windows.h>

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
