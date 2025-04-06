export class BitInFormat {
  constructor(value: number);
  value(): number;
}

export enum BitCompressionMethod {
  Copy = 0,
  Deflate = 1,
  Deflate64 = 2,
  BZip2 = 3,
  Lzma = 4,
  Lzma2 = 5,
  Ppmd = 6
}

export enum FormatFeatures {
  MultipleFiles = 1 << 0,
  SolidArchive = 1 << 1,
  CompressionLevel = 1 << 2,
  Encryption = 1 << 3,
  HeaderEncryption = 1 << 4,
  MultipleMethods = 1 << 5
}

export class BitInOutFormat extends BitInFormat {
  constructor(value: number, ext: string, defaultMethod: BitCompressionMethod, features: FormatFeatures);
  defaultMethod(): BitCompressionMethod;
  extension(): string;
  features(): FormatFeatures;
  hasFeature(feature: FormatFeatures): boolean;
}

export namespace BitFormat {
  const SevenZip: BitInOutFormat;
}

export class Bit7zLibrary {
  // default dll names
  constructor(dllName: "7z.dll" | "7za.dll");
  constructor(dllName: string);
}

export class BitArchiveWriter {
  constructor(lib: Bit7zLibrary, format: BitInOutFormat);
  addDirectory(directoryPath: string): void;
  addFile(filePath: string, name?: string): void;
  compressTo(outFilePath: string): void;
}
