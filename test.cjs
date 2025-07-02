const {existsSync} = require("fs");
const bit7z = require(".");

(async () => {
  try {
    const lib = new bit7z.Bit7zLibrary("7z.dll");

    if (!existsSync("test.7z")) {
      const writer = new bit7z.BitArchiveWriter(lib, bit7z.BitFormat.SevenZip);
      console.log(writer);
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
    else {
      const extractor = new bit7z.BitFileExtractor(lib, bit7z.BitFormat.SevenZip);

      console.log("unpacking");
      await extractor.extract("test.7z", "test.unpacked");
      console.log("unpacked");

      // const editor = new bit7z.BitArchiveEditor(lib, "test.7z", bit7z.BitFormat.SevenZip);
      // editor.updateItem("index.js", "code.js");
      // await editor.applyChanges();
    }
  }
  catch(error) {
    console.error(error);
  }
})();
