# QPyocd
[English](https://github.com/kydinn/QPyocd/blob/main/README.md) | 简体中文



基于 Qt + pyOCD 实现的 MCU 烧写上位机



## 快速开始

### Windows

下载预编译的压缩包后双击执行`QPyocd.exe`，自带支持了芯源的`CW32F030`芯片。



## 二次开发

### 开发环境
- Qt6.9
- MSVC 2019
- Windows 11  24H2
- pyOCD 0.40.0

如果需要适配新的芯片，只需要在`mcu_update_page.h`中追加对应的芯片名称和pack包即可，

**同时要将pack文件放置在程序根目录的`tools/packs/`中。**

```cpp
static QMap<QString, QString> chip_pack_map = {
	{"CW32F030F8", "WHXY.CW32F030_DFP.1.0.4.pack"},
    // 添加新的芯片
};
```

