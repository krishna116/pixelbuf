# pixelbuf

此工具可以将图像的像素矩阵，转换成可嵌入的C/C++代码。

例如, 要从图像中截取矩形区域 [x = 0, y = 0, width = 4, height = 4] 的像素, 输入命令：

```Bash
./pixelbuf -i hero.png --format rgba --rect 0:0:4:4
```

输出如下的结果:

```C
// Color format is 0xAABBGGRR
#define IMAGE_W  4
#define IMAGE_H  4
static const uint32_t image[IMAGE_W * IMAGE_H] = {
  0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
  0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
  0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
  0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF
};
```

## Tool Interface

```Bash
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
```

## Reference

此工具使用[stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)读取图像，支持以下图像格式：JPEG/PNG/TGA/BMP...

