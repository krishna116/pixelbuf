#pragma once

#include <string>
#include <cstdint>
#include <cstdio>

/**
 * Convert Image pixel buffer to C/C++ source code.
 */
class PixelBuf {
public:
  // Rectangle area.
  struct Rect{
    int x;    // x position.
    int y;    // y position.
    int w;    // width.
    int h;    // height.
    Rect():x(0), y(0), w(0), h(0){}
  };

  // Image is used to manage pixel buffer.
  struct Image {
    int w;          // image width.
    int h;          // image height.
    uint8_t *data;  // image pixel buffer(w*h*number_of_channels).

    Image();
    Image(const Image& other);
    Image& operator = (const Image& other);
    ~Image();
  };

  // Pixel format for output.
  struct Format{
    enum class Color{
      Red,
      Green,
      Blue,
      Alpha,
      None   // Only rgb and no alpha channel.
    };

    Color data[4];
    bool isOverrideAlpha;
    int overrideAlphaValue;
    Format():data{Color::Red, Color::Green, Color::Blue, Color::Alpha}, isOverrideAlpha(false), overrideAlphaValue(0){}

    // Get human readable format string.
    std::string toString() const {
      std::string str{"0x"};
      for (int i = 3; i >= 0; --i) {
        switch (data[i]) {
          case Color::Red:   str += "RR"; break;
          case Color::Green: str += "GG"; break;
          case Color::Blue:  str += "BB"; break;
          case Color::Alpha: str += "AA"; break;
          case Color::None:  str += "00"; break;
        }
      }

      return str;
    }

    // Pack(pixel color) --> uint32_t --> string.
    std::string pack(uint8_t r, uint8_t g, uint8_t b, uint8_t a) const {
      uint32_t pixel = 0;
      for (int i = 0; i < 4; ++i) {
        uint32_t value = 0;
        switch (data[i]) {
          case Color::Red:   value = static_cast<uint32_t>(r); break;
          case Color::Green: value = static_cast<uint32_t>(g); break;
          case Color::Blue:  value = static_cast<uint32_t>(b); break;
          case Color::Alpha:
            value = isOverrideAlpha
                        ? static_cast<uint32_t>(overrideAlphaValue)
                        : static_cast<uint32_t>(a);
            break;
          case Color::None:  value = 0; break;
        }
        pixel |= value << (i * 8);
      }

      char buf[16] = {};
      std::snprintf(buf, sizeof(buf), "0x%08X", static_cast<unsigned>(pixel));
      return std::string{buf};
    }
  };

  PixelBuf() = default;

  /**
   * Convert Image pixel buffer to C/C++ source code.
   * 
   * @param sourceImagePath   Source image path.
   * @param sourceRectStr     Source Rectangle string, the format is "x:y:w:h".
   * @param fomratStr         Output pixel format, the format is any permutation of r,g,b or r,g,b,a.
   * @param overrideAlpha     Override alpha value(0~255).
   * @param destFilePath      Output file path.
   * @param nameStr           Name of the generated pixel array.
   * 
   * @return 0                No error.
   * @return not 0            Error happened.
   */
  int convertPixelBuf(const std::string &sourceImagePath, 
                  const std::string &sourceRectStr,
                  const std::string &fomratStr,
                  const std::string &overrideAlpha,
                  const std::string &destFilePath, 
                  const std::string &nameStr
                );

  /**
   * Get last error.
   */
  const std::string& getLastError()const;
private:
  std::string error_;

  void clearLastError();

  void pushError(const std::string& error);

  Image loadImage(const std::string& path);

  Rect parseRect(const std::string& rectStr, std::string& error);

  Format parseFormat(const std::string& fomratStr, const std::string& overridAlpha, std::string& error);

  std::string getPixelBufText(const Image& image, const Rect& rect, const Format& format, const std::string& name);
};