const bit7z = require(".");

try {
  const lib = new bit7z.Bit7zLibrary("7za.dll");
  const writer = new bit7z.BitArchiveWriter(lib, bit7z.BitFormat.SevenZip);

  writer.addFile("package.json");
  writer.addFile("package-lock.json", "my-package-lock.json");
  writer.addDirectory("src");

  console.log("compressing test.7z");
  writer.compressTo("test.7z").then(() => {
    console.log("compressed test.7z");
    console.log("compressing a lot");
    Promise.all([
      writer.compressTo("test1.7z").then(() => console.log("test1 done")),
      writer.compressTo("test2.7z").then(() => console.log("test2 done")),
      writer.compressTo("test3.7z").then(() => console.log("test3 done")),
      writer.compressTo("test4.7z").then(() => console.log("test4 done")),
      writer.compressTo("test5.7z").then(() => console.log("test5 done")),
      writer.compressTo("test6.7z").then(() => console.log("test6 done")),
      writer.compressTo("test7.7z").then(() => console.log("test7 done")),
      writer.compressTo("test8.7z").then(() => console.log("test8 done")),
      writer.compressTo("test9.7z").then(() => console.log("test9 done")),
      writer.compressTo("test0.7z").then(() => console.log("test0 done")),
    ]).then(() => {
      console.log("compressed a lot");
    });
  });
}
catch(error) {
  console.error(error);
}
