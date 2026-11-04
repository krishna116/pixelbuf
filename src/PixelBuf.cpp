#include "PixelBuf.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#define USING_STB_IMAGE_CHANNELS 4u   // The 4 channel is RGBA and each channel is one byte(uint8_t).

PixelBuf::Image::Image():w(0), h(0), data(nullptr){}

PixelBuf::Image::Image(const Image &other){
  if (other.data == nullptr || other.w <= 0 || other.h <= 0) return;
  const std::size_t size = static_cast<std::size_t>(other.w) * static_cast<std::size_t>(other.h) * USING_STB_IMAGE_CHANNELS;
  data = static_cast<uint8_t*>(std::malloc(size));
  if (data == nullptr) return;
  std::memcpy(data, other.data, size);
  w = other.w;
  h = other.h;
}

PixelBuf::Image &PixelBuf::Image::operator=(const Image &other){
 if (this == &other) return *this;

  if(data){
    std::free(data);
    data = nullptr;
    w = h = 0;
  }

  if (other.data == nullptr || other.w <= 0 || other.h <= 0) return *this;

  const std::size_t size = static_cast<std::size_t>(other.w) * static_cast<std::size_t>(other.h) * USING_STB_IMAGE_CHANNELS;
  data = static_cast<uint8_t*>(std::malloc(size));
  if (data == nullptr) return *this;
  std::memcpy(data, other.data, size);
  w = other.w;
  h = other.h;
  return *this;
}

PixelBuf::Image::~Image(){
  std::free(data);
}

static bool parseInteger(const std::string& str, long long& out) {
  if (str.empty()) return false;
  for (char ch : str) {
    if (std::isspace(static_cast<unsigned char>(ch))) return false;
  }
  const bool isHex = str.size() >= 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X');
  std::size_t pos = 0;
  try {
    out = std::stoll(str, &pos, isHex ? 16 : 10);
  } catch (const std::exception&) {
    return false;
  }
  return pos == str.size();
}

static std::string toUpperString(const std::string& str){
  std::string out;
  for (char ch : str) {
    out.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
  }
  return out;
}

static std::string rectToString(const PixelBuf::Rect& rect){
  std::string str{"{"};
  str += "x = " + std::to_string(rect.x) + ",";
  str += "y = " + std::to_string(rect.y) + ",";
  str += "w = " + std::to_string(rect.w) + ",";
  str += "h = " + std::to_string(rect.h) + "}";
  return str;
}

PixelBuf::Image PixelBuf::loadImage(const std::string& path){
  Image image;
  int imageChannel = 0;
  image.data = stbi_load(path.c_str(), &image.w, &image.h, &imageChannel, USING_STB_IMAGE_CHANNELS);
  return image;
}

PixelBuf::Rect PixelBuf::parseRect(const std::string& rectStr, std::string& error){
  Rect rect{};

  if(rectStr.empty()) return rect;

  std::vector<std::string> tokens;
  std::stringstream ss(rectStr);
  std::string token;

  while (std::getline(ss, token, ':')) {
    tokens.push_back(token);
  }

  if(tokens.size() != 4){
    error += "Invalid rect [" + rectStr + "]: expected 4 colon-separated fields x:y:w:h, got " +
             std::to_string(tokens.size()) + ".\n";
    return {};
  }

  long long v[4] = {};
  for (int i = 0; i < 4; ++i) {
    if (!parseInteger(tokens[i], v[i])) {
      error += "Invalid rect \"" + rectStr + "\": field " + std::to_string(i + 1) + " \"" + tokens[i] + "\" is not an integer.\n";
      return {};
    }
    if (v[i] > std::numeric_limits<int>::max()) {
      error += "Invalid rect \"" + rectStr + "\": field " + std::to_string(i + 1) + " is too large.\n";
      return {};
    }
  }

  rect.x = static_cast<int>(v[0]);
  rect.y = static_cast<int>(v[1]);
  rect.w = static_cast<int>(v[2]);
  rect.h = static_cast<int>(v[3]);

  if (rect.x < 0 || rect.y < 0 || rect.w <= 0 || rect.h <= 0) {
    error += "Input Rect " + rectToString(rect) + " is out of range (x,y >= 0 and w,h > 0 required).\n";
    return {};
  }

  return rect;
}

PixelBuf::Format PixelBuf::parseFormat(const std::string& fomratStr, const std::string& overridAlphaStr, std::string& error){
  Format format{};

  if(!fomratStr.empty()){
    const std::size_t size = fomratStr.size();
    if (size != 3 && size != 4) {
      error += "Invalid format \"" + fomratStr +
               "\": expected exactly 3 letters (r,g,b) or 4 letters "
               "(r,g,b,a), got " + std::to_string(size) + ".\n";
      return {};
    }

    bool seenR = false, seenG = false, seenB = false, seenA = false;
    for (std::size_t i = 0; i < size; ++i) {
      const char ch = static_cast<char>(std::tolower(static_cast<unsigned char>(fomratStr[i])));
      switch (ch) {
        case 'r': seenR = true; format.data[i] = Format::Color::Red;   break;
        case 'g': seenG = true; format.data[i] = Format::Color::Green; break;
        case 'b': seenB = true; format.data[i] = Format::Color::Blue;  break;
        case 'a': seenA = true; format.data[i] = Format::Color::Alpha; break;
        default:
          error += "Invalid format \"" + fomratStr +
                   "\": unknown channel '" + std::string(1, fomratStr[i]) +
                   "' (only r,g,b,a are allowed).\n";
          return {};
      }
    }

    // A valid format is a permutation: every required channel appears once,
    // which also rules out duplicates and missing channels.
    if (!seenR || !seenG || !seenB) {
      error += "Invalid format \"" + fomratStr +
               "\": channels r,g,b must each appear exactly once.\n";
      return {};
    }
    if (size == 4 && !seenA) {
      error += "Invalid format \"" + fomratStr +
               "\": a 4-letter format must be a permutation of r,g,b,a.\n";
      return {};
    }
    if (size == 3 && seenA) {
      error += "Invalid format \"" + fomratStr +
               "\": a 3-letter format must be a permutation of r,g,b "
               "(no alpha channel).\n";
      return {};
    }
    if (size == 3) {
      format.data[3] = Format::Color::None;  // high byte stays zero
    }
  }

  if (!overridAlphaStr.empty()) {
    long long alpha = 0;
    if (!parseInteger(overridAlphaStr, alpha)) {
      error += "Invalid alpha string \"" + overridAlphaStr + "\".\n";
      return {};
    }
    if (alpha < 0 || alpha > 0xFF) {
      error += "Input alpha value is out of range [0,0xFF].\n";
      return {};
    }
    format.isOverrideAlpha = true;
    format.overrideAlphaValue = static_cast<int>(alpha);
  }

  return format;
}

// ---------------------------------------------------------------------------
// Generation
// ---------------------------------------------------------------------------

int PixelBuf::convertPixelBuf(const std::string &sourceImagePath, 
                const std::string &sourceRectStr,
                const std::string &fomratStr,
                const std::string &overrideAlpha,
                const std::string &destFilePath, 
                const std::string &nameStr
              ) {
  clearLastError();

  Image image = loadImage(sourceImagePath);
  if(image.data == nullptr || image.w <= 0 || image.h <= 0){
    const char* reason = stbi_failure_reason();
    pushError(std::string("Read image failed [") + sourceImagePath + "]" +
              (reason ? std::string(": ") + reason : std::string()) + "\n");
    return -1;
  }

  std::string error;
  Rect rect{};
  if(sourceRectStr.empty()){
    rect.w = image.w;
    rect.h = image.h;
  }else{
    rect = parseRect(sourceRectStr, error);
    if(!error.empty()){
      pushError(error);
      return -1;
    }else{
      error.clear();
    }
  }

  Format format = parseFormat(fomratStr, overrideAlpha, error);
  if(!error.empty()){
    pushError(error);
    return -1;
  }else{
    error.clear();
  }

  std::string text = getPixelBufText(image, rect, format, nameStr);
  if (text.empty()) {
    if (error_.empty()) pushError("Failed to generate output.\n");
    return -1;
  }
  
  if (destFilePath.empty()) {
    std::cout << text;
  } else {
    std::ofstream oFile(destFilePath, std::ios::binary);
    if (!oFile.is_open()) {
      pushError("Cannot write file [" + destFilePath + "]\n");
      return -1;
    }
    oFile << text;
    oFile.close();
    if (!oFile) {
      pushError("Cannot write file [" + destFilePath + "]\n");
      return -1;
    }
  }

  return 0;
}

std::string PixelBuf::getPixelBufText(const Image& image, const Rect& rect, const Format& format, const std::string &name){
 if (rect.w <= 0 || rect.h <= 0 || rect.x < 0 || rect.y < 0 ||
      static_cast<long long>(rect.x) + rect.w > image.w ||
      static_cast<long long>(rect.y) + rect.h > image.h) {
    pushError("Input rect " + rectToString(rect) +
              " is out of the source image (" + std::to_string(image.w) +
              " x " + std::to_string(image.h) + " px).\n");
    return {};
  }

  if (!name.empty()) {
    bool valid = std::isalpha(static_cast<unsigned char>(name[0])) || name[0] == '_';
    for (char ch : name) {
      if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_')) {
        valid = false;
        break;
      }
    }
    if (!valid) {
      pushError("Invalid array name \"" + name +
                "\": must be a C identifier (letters, digits and underscore, "
                "not starting with a digit).\n");
      return {};
    }
  }

  std::string cname = name.empty()? "image" : name;
  std::string CNAME = toUpperString(cname);
  std::string WIDTH = std::to_string(rect.w);
  std::string HEIGHT =  std::to_string(rect.h);

  std::string code;
  code += "// Color format is " + format.toString() + "\n";
  code += "#define " + CNAME + "_W   " + WIDTH + "\n";
  code += "#define " + CNAME + "_H   " + HEIGHT + "\n";
  code += "static const uint32_t " + cname + "[" + CNAME + "_W * " + CNAME + "_H] = {\n";

  constexpr int kValuesPerLine = 16;
  for (int y = 0; y < rect.h; ++y) {
    code += "  ";
    for (int x = 0; x < rect.w; ++x) {
      const std::size_t pos = (static_cast<std::size_t>(rect.y + y) * image.w + static_cast<std::size_t>(rect.x + x)) * USING_STB_IMAGE_CHANNELS;
      code += format.pack(image.data[pos], image.data[pos + 1], image.data[pos + 2], image.data[pos + 3]);

      if (x + 1 < rect.w) code += ",";

      const bool rowEnd = (x + 1 == rect.w);
      if (rowEnd) {
        code += (y + 1 < rect.h) ? ",\n" : "\n";
      } else if ((x + 1) % kValuesPerLine == 0) {
        code += "\n  ";
      } else {
        code += " ";
      }
    }
  }
  code += "};\n";
  return code;
}

const std::string& PixelBuf::getLastError()const{
  return error_;
}

void PixelBuf::clearLastError(){
  error_.clear();
}

void PixelBuf::pushError(const std::string& error){
  error_ += error;
}
