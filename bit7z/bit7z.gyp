{
  "targets": [
    {
      "target_name": "bit7z",
      "type": "static_library",
      "cflags": [ "-std=c++17" ],
      "cflags_cc": [ "-std=c++17" ],
      "msbuild_toolset": "v141",
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 1, # Sync
          "RuntimeLibrary": 2, # MultiThreadedDLL
          "AdditionalOptions": [ "-std:c++17", "/utf-8", "/MD" ]
        }
      },
      "sources": [
        "src/bit7zlibrary.cpp",
        "src/bitabstractarchivecreator.cpp",
        "src/bitabstractarchivehandler.cpp",
        "src/bitabstractarchiveopener.cpp",
        "src/bitarchiveeditor.cpp",
        "src/bitarchiveitem.cpp",
        "src/bitarchiveiteminfo.cpp",
        "src/bitarchiveitemoffset.cpp",
        "src/bitarchivereader.cpp",
        "src/bitarchivewriter.cpp",
        "src/biterror.cpp",
        "src/bitexception.cpp",
        "src/bitfilecompressor.cpp",
        "src/bitformat.cpp",
        "src/bitinputarchive.cpp",
        "src/bititemsvector.cpp",
        "src/bitoutputarchive.cpp",
        "src/bitpropvariant.cpp",
        "src/bittypes.cpp",
        "src/internal/bufferextractcallback.cpp",
        "src/internal/bufferitem.cpp",
        "src/internal/bufferutil.cpp",
        "src/internal/callback.cpp",
        "src/internal/cbufferinstream.cpp",
        "src/internal/cbufferoutstream.cpp",
        "src/internal/cfileinstream.cpp",
        "src/internal/cfileoutstream.cpp",
        "src/internal/cfixedbufferoutstream.cpp",
        "src/internal/cmultivolumeinstream.cpp",
        "src/internal/cmultivolumeoutstream.cpp",
        "src/internal/cstdinstream.cpp",
        "src/internal/cstdoutstream.cpp",
        "src/internal/csymlinkinstream.cpp",
        "src/internal/cvolumeinstream.cpp",
        "src/internal/cvolumeoutstream.cpp",
        "src/internal/dateutil.cpp",
        "src/internal/extractcallback.cpp",
        "src/internal/failuresourcecategory.cpp",
        "src/internal/fileextractcallback.cpp",
        "src/internal/fixedbufferextractcallback.cpp",
        "src/internal/formatdetect.cpp",
        "src/internal/fsindexer.cpp",
        "src/internal/fsitem.cpp",
        "src/internal/fsutil.cpp",
        "src/internal/genericinputitem.cpp",
        "src/internal/guids.cpp",
        "src/internal/hresultcategory.cpp",
        "src/internal/internalcategory.cpp",
        "src/internal/opencallback.cpp",
        "src/internal/operationcategory.cpp",
        "src/internal/operationresult.cpp",
        "src/internal/processeditem.cpp",
        "src/internal/renameditem.cpp",
        "src/internal/stdinputitem.cpp",
        "src/internal/streamextractcallback.cpp",
        "src/internal/stringutil.cpp",
        "src/internal/updatecallback.cpp",
        "src/internal/windows.cpp"
      ],
      "include_dirs": [
        "src",
        "include/bit7z",
        "../7zip/CPP"
      ],
      "direct_dependent_settings": {
        "include_dirs": [ "include" ],
        "defines": [ "UNICODE", "_UNICODE" ]
      },
      "defines": [ "UNICODE", "_UNICODE" ]
    }
  ]
}