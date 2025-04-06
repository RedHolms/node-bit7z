const lib = require("bindings")("7zlib-impl");
module.exports = exports = lib;

// make typescript enum
const makeEnum = (predicate) => {
  const e = { ...predicate };
  for (const key of Object.keys(e))
    e[e[key]] = key;
  return e;
};

const BitCompressionMethod = lib.BitCompressionMethod = makeEnum({
  Copy: 0,
  Deflate: 1,
  Deflate64: 2,
  BZip2: 3,
  Lzma: 4,
  Lzma2: 5,
  Ppmd: 6
});

const FormatFeatures = lib.FormatFeatures = makeEnum({
  MultipleFiles: 1 << 0,
  SolidArchive: 1 << 1,
  CompressionLevel: 1 << 2,
  Encryption: 1 << 3,
  HeaderEncryption: 1 << 4,
  MultipleMethods: 1 << 5
});

lib.BitFormat = {
  SevenZip: new lib.BitInOutFormat(
    7, ".7z", BitCompressionMethod.Lzma2,
    FormatFeatures.MultipleFiles | FormatFeatures.SolidArchive |
    FormatFeatures.CompressionLevel | FormatFeatures.Encryption |
    FormatFeatures.HeaderEncryption | FormatFeatures.MultipleMethods
  )
};
