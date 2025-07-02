export enum ArchiveStartOffset {
  None = 0,
  FileStart = 1
}

export enum BitCompressionLevel {
  None = 0,
  Fastest = 1,
  Fast = 3,
  Normal = 5,
  Max = 7,
  Ultra = 9
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

export enum FilterPolicy {
  Include = 0,
  Exclude = 1
}

export enum FormatFeatures {
  MultipleFiles = 1 << 0,
  SolidArchive = 1 << 1,
  CompressionLevel = 1 << 2,
  Encryption = 1 << 3,
  HeaderEncryption = 1 << 4,
  MultipleMethods = 1 << 5
}

export enum OverwriteMode {
  None = 0,
  Overwrite = 1,
  Skip = 2
}

export enum UpdateMode {
  None = 0,
  Append = 1,
  Update = 2
}

export type FileCallback = (filePathInArchive: string) => void;
export type PasswordCallback = () => string;
export type ProgressCallback = (doneBytes: number) => boolean | void;
export type RatioCallback = (inputSize: number, outputSize: number) => void;
export type TotalCallback = (totalBytes: number) => void;

// main class
export class Bit7zLibrary {
  constructor(dllName: "7z.dll" | "7za.dll");
  constructor(dllName: string);
}

export class BitInFormat {
  constructor(value: number);
  value(): number;
}

export class BitInOutFormat extends BitInFormat {
  constructor(value: number, ext: string, defaultMethod: BitCompressionMethod, features: FormatFeatures);
  extension(): string;
  features(): FormatFeatures;
  hasFeature(feature: FormatFeatures): boolean;
  defaultMethod(): BitCompressionMethod;
}

// built-in formats
export namespace BitFormat {
  const SevenZip: BitInOutFormat;
}

export interface BitAbstractArchiveHandler {
  clearPassword(): void;
  fileCallback(): FileCallback;
  format(): BitInFormat;
  isPasswordDefined(): boolean;
  library(): Bit7zLibrary;
  overwriteMode(): OverwriteMode;
  password(): string;
  passwordCallback(): PasswordCallback;
  progressCallback(): ProgressCallback;
  ratioCallback(): RatioCallback;
  retainDirectories(): boolean;
  setFileCallback(callback: FileCallback): void;
  setOverwriteMode(mode: OverwriteMode): void;
  setPassword(password: string): void;
  setPasswordCallback(callback: PasswordCallback): void;
  setProgressCallback(callback: ProgressCallback): void;
  setRatioCallback(callback: RatioCallback): void;
  setRetainDirectories(retain: boolean): void;
  setTotalCallback(callback: TotalCallback): void;
  totalCallback(): TotalCallback;
}

export interface BitAbstractArchiveCreator extends BitAbstractArchiveHandler {
  compressionFormat(): BitInOutFormat;
  compressionLevel(): BitCompressionLevel;
  compressionMethod(): BitCompressionMethod;
  cryptHeaders(): boolean;
  dictionarySize(): number;
  setCompressionLevel(level: BitCompressionLevel): void;
  setCompressionMethod(method: BitCompressionMethod): void;
  setDictionarySize(dictionarySize: number): void;
  // setFormatProperty( const wchar_t( &name )[N], const T& value ) noexcept
  // setFormatProperty( const wchar_t( &name )[N], T value ) noexcept
  setSolidMode(solidMode: boolean): void;
  setStoreSymbolicLinks(storeSymlinks: boolean): void;
  setThreadsCount(threadsCount: number): void;
  setUpdateMode(canUpdate: boolean): void;
  setUpdateMode(mode: UpdateMode): void;
  setVolumeSize(volumeSize: number): void;
  setWordSize(wordSize: number): void;
  solidMode(): boolean;
  storeSymbolicLinks(): boolean;
  threadsCount(): number;
  updateMode(): UpdateMode;
  volumeSize(): number;
  wordSize(): number;
}

export interface BitOutputArchive {
  addDirectory(directoryPath: string): void;
  addDirectoryContents(directoryPath: string, filter: string, recursive: boolean): void;
  addDirectoryContents(
    directoryPath: string,
    filter?: string, // "*" by default
    policy?: FilterPolicy, // FilterPolicy.Include by default
    recursive?: boolean // true by default
  ): void;
  addFile(content: Buffer, name: string): void;
  addFile(filePath: string, name?: string): void;
  addFiles(filesPaths: string[]): void;
  addFiles(directoryPath: string, filter: string, recursive: boolean): void;
  addFiles(
    directoryPath: string,
    filter?: string, // "*" by default
    policy?: FilterPolicy, // FilterPolicy.Include by default
    recursive?: boolean // true by default
  ): void;
  addItems(items: [filePath: string, nameInArchive: string][]): void;
  addItems(filesPaths: string[]): void;
  compressTo(outFilePath: string): Promise<void>;
  creator(): BitAbstractArchiveCreator;
  handler(): BitAbstractArchiveHandler;
  itemsCount(): number;
}

export class BitArchiveWriter implements BitAbstractArchiveCreator, BitOutputArchive {
  constructor(lib: Bit7zLibrary, format: BitInOutFormat);
  constructor(
    lib: Bit7zLibrary, archiveFilePath: string,
    startOffset: ArchiveStartOffset,
    format: BitInOutFormat, password?: string
  );
  constructor(
    lib: Bit7zLibrary, archiveFilePath: string,
    format: BitInOutFormat, password?: string
  );
  clearPassword(): void;
  fileCallback(): FileCallback;
  format(): BitInFormat;
  isPasswordDefined(): boolean;
  library(): Bit7zLibrary;
  overwriteMode(): OverwriteMode;
  password(): string;
  passwordCallback(): PasswordCallback;
  progressCallback(): ProgressCallback;
  ratioCallback(): RatioCallback;
  retainDirectories(): boolean;
  setFileCallback(callback: FileCallback): void;
  setOverwriteMode(mode: OverwriteMode): void;
  setPassword(password: string): void;
  setPasswordCallback(callback: PasswordCallback): void;
  setProgressCallback(callback: ProgressCallback): void;
  setRatioCallback(callback: RatioCallback): void;
  setRetainDirectories(retain: boolean): void;
  setTotalCallback(callback: TotalCallback): void;
  totalCallback(): TotalCallback;
  compressionFormat(): BitInOutFormat;
  compressionLevel(): BitCompressionLevel;
  compressionMethod(): BitCompressionMethod;
  cryptHeaders(): boolean;
  dictionarySize(): number;
  setCompressionLevel(level: BitCompressionLevel): void;
  setCompressionMethod(method: BitCompressionMethod): void;
  setDictionarySize(dictionarySize: number): void;
  // setFormatProperty( const wchar_t( &name )[N], const T& value ) noexcept
  // setFormatProperty( const wchar_t( &name )[N], T value ) noexcept
  setSolidMode(solidMode: boolean): void;
  setStoreSymbolicLinks(storeSymlinks: boolean): void;
  setThreadsCount(threadsCount: number): void;
  setUpdateMode(canUpdate: boolean): void;
  setUpdateMode(mode: UpdateMode): void;
  setVolumeSize(volumeSize: number): void;
  setWordSize(wordSize: number): void;
  solidMode(): boolean;
  storeSymbolicLinks(): boolean;
  threadsCount(): number;
  updateMode(): UpdateMode;
  volumeSize(): number;
  wordSize(): number;
  addDirectory(directoryPath: string): void;
  addDirectoryContents(directoryPath: string, filter: string, recursive: boolean): void;
  addDirectoryContents(
    directoryPath: string,
    filter?: string, // "*" by default
    policy?: FilterPolicy, // FilterPolicy.Include by default
    recursive?: boolean // true by default
  ): void;
  addFile(content: Buffer, name: string): void;
  addFile(filePath: string, name?: string): void;
  addFiles(filesPaths: string[]): void;
  addFiles(directoryPath: string, filter: string, recursive: boolean): void;
  addFiles(
    directoryPath: string,
    filter?: string, // "*" by default
    policy?: FilterPolicy, // FilterPolicy.Include by default
    recursive?: boolean // true by default
  ): void;
  addItems(items: [filePath: string, nameInArchive: string][]): void;
  addItems(filesPaths: string[]): void;
  compressTo(outFilePath: string): Promise<void>;
  creator(): BitAbstractArchiveCreator;
  handler(): BitAbstractArchiveHandler;
  itemsCount(): number;
}

export enum DeletePolicy {
  ItemOnly = 0,
  RecurseDirs = 1
}

export class BitArchiveEditor extends BitArchiveWriter {
  constructor(
    lib: Bit7zLibrary, archiveFilePath: string,
    format: BitInOutFormat, password?: string
  );
  renameItem(index: number, newPath: string): void;
  renameItem(oldPath: string, newPath: string): void;
  updateItem(index: number, inputFilePath: string): void;
  updateItem(itemPath: string, inputFilePath: string): void;
  deleteItem(index: number, policy: DeletePolicy): void; // default policy is DeletePolicy.ItemOnly
  deleteItem(itemPath: string, policy: DeletePolicy): void; // default policy is DeletePolicy.ItemOnly
  applyChanges(): Promise<void>;
}

export class BitFileExtractor {
  constructor(lib: Bit7zLibrary, format: BitInFormat);
  extract(archivePath: string, destDirPath: string): Promise<void>;
}
