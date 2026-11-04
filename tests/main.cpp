#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

#include "PixelBuf.h"
#include "config.h"

static const char *help = R"(
Usage:
  pixelbuf -i <image> [options]

Options:
  -i, --input <image>     Input an image file.
  -o, --out <filename>    Output a text file.
  -f, --format <string>   Output color format (default: rgba). Any permutation
                          of r,g,b or r,g,b,a, case-insensitive, little-endian.
  -a, --alpha <value>     Override alpha value (0~255).
  -r, --rect <x:y:w:h>    Source image rectangle area for output.
  -n, --name <string>     Name of the generated array (default: image).
  -h, --help              Print this help.
  -v, --version           Print version.
)";

static bool match(const char *str1, const char *str2) {
  return std::strcmp(str1, str2) == 0;
}

static int printHelp() {
  std::printf("%s", help + 1);
  return 0;
}

static int printVersion() {
  std::printf("%s version %s\n", APP_NAME_STR, APP_VERSION_STR);
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc <= 1) return printHelp();

  std::string sourceImagePath;
  std::string destFilePath;
  std::string fomratStr;
  std::string overridAlpha;
  std::string sourceRectStr;
  std::string nameStr;

  for (int i = 1; i < argc; ++i) {
    char* arg = argv[i];
    if (match(arg, "-i") || match(arg, "--input")) {
      if (i + 1 < argc) {
        sourceImagePath = argv[++i];
        continue;
      }
      std::cerr << "Option " << arg << " need an argument.\n";
      return -1;
    } else if (match(arg, "-o") || match(arg, "--out") || match(arg, "--output")) {
      if (i + 1 < argc) {
        destFilePath = argv[++i];
        continue;
      }
      std::cerr << "Option " << arg << " need an argument.\n";
      return -1;
    } else if (match(arg, "-f") || match(arg, "--format")) {
      if (i + 1 < argc) {
        fomratStr = argv[++i];
        continue;
      }
      std::cerr << "Option " << arg << " need an argument.\n";
      return -1;
    } else if (match(arg, "-a") || match(arg, "--alpha")) {
      if (i + 1 < argc) {
        overridAlpha = argv[++i];
        continue;
      }
      std::cerr << "Option " << arg << " need an argument.\n";
      return -1;
    } else if (match(arg, "-r") || match(arg, "--rect")) {
      if (i + 1 < argc) {
        sourceRectStr = argv[++i];
        continue;
      }
      std::cerr << "Option " << arg << " need an argument.\n";
      return -1;
    } else if (match(arg, "-n") || match(arg, "--name")) {
      if (i + 1 < argc) {
        nameStr = argv[++i];
        continue;
      }
      std::cerr << "Option " << arg << " need an argument.\n";
      return -1;
    } else if (match(arg, "-h") || match(arg, "--help")) {
      return printHelp();
    } else if (match(arg, "-v") || match(arg, "--version")) {
      return printVersion();
    } else {
      std::cerr << "Unknown parameter [" << arg << "]\n";
      return -1;
    }
  }

  if (sourceImagePath.empty()) {
    std::cerr << "Input an image is required.\n";
    return -1;
  }

  PixelBuf pixelBuf;
  const int rc = pixelBuf.convertPixelBuf(sourceImagePath, sourceRectStr, fomratStr, overridAlpha, destFilePath, nameStr);
  if (!pixelBuf.getLastError().empty()) {
    std::cerr << "[Error] " << pixelBuf.getLastError();
  }

  return rc;
}