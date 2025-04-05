const fs = require("fs/promises");
const { join } = require("path");
const { exec } = require("child_process");

const buildDir = join(__dirname, "bit7z", "build");

function system(cmd) {
  return new Promise((resolve, reject) => {
    exec(cmd, (err, stdout, stderr) => {
      if (err) {
        console.error(stderr);
        reject(new Error(err));
        return;
      }
      resolve(stdout);
    });
  });
}

(async () => {
  await fs.rm(buildDir, { recursive: true, force: true });
  await fs.mkdir(buildDir, { recursive: true });

  await system(`cmake -B "${buildDir}" -S "${join(__dirname, "bit7z")}"`);
  await system(`cmake --build "${buildDir}"`);
})();
