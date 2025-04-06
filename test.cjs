const bit7z = require(".");

(async () => {
  try { 
    const lib = new bit7z.Bit7zLibrary("7za.dll");
    const writer = new bit7z.BitArchiveWriter(lib, bit7z.BitFormat.SevenZip);

    writer.addFile("package.json");
    writer.addFile("package-lock.json", "my-package-lock.json");
    writer.addDirectory("src");

    console.log("compressing test.7z");
    let total = 0;
    writer.setTotalCallback((v) => total = v);
    writer.setProgressCallback((done) => {
      console.log("done %s%%", ((done / total) * 100).toFixed(2));
    });
    await writer.compressTo("test.7z");
    console.log("compressed test.7z");
  }
  catch(error) {
    console.error(error);
  }
})();
