const bit7z = require(".");

try {
  const lib = new bit7z.Bit7zLibrary("7za.dll");
  const writer = new bit7z.BitArchiveWriter(lib, bit7z.BitFormat.SevenZip);

  writer.addFile("package.json");
  writer.addFile("package-lock.json", "my-package-lock.json");
  writer.addDirectory("src");
  writer.compressTo("C:\\src\\node-bit7z\\test.7z");
}
catch(error) {
  console.error(error);
}
