{
  "targets": [
    {
      "target_name": "7zlib-impl",
      "dependencies": [
        "./bit7z/bit7z.gyp:bit7z"
      ],
      "cflags": [ "-fno-exceptions", "-std=c++17" ],
      "cflags_cc": [ "-fno-exceptions", "-std=c++17" ],
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 1, # Sync
          "RuntimeLibrary": 2, # MultiThreadedDLL
          "AdditionalOptions": [ "-std:c++17", "/utf-8", "/MD" ]
        }
      },
      "sources": [
        "src/main.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "libraries": [
      ],
    }
  ]
}