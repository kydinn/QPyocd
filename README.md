# QPyocd
English | [简体中文](https://github.com/kydinn/QPyocd/blob/main/README_zh_CN.md)



MCU programming to host computer based on Qt + pyOCD



## Quick Start

### Windows

After downloading the pre-compiled compressed package, double-click to run `QPyocd.exe`, which comes with built-in support for `CW32F030` chip.



## Secondary Development

### Development Environment

- Qt6.9
- MSVC 2019
- Windows 11 24H2
- pyOCD 0.40.0

If you need to adapt to a new chip, simply append the corresponding chip name and pack file to `mcu_update_page.h`.

**Also, the pack file should be placed in the `tools/packs/` directory of the program's root directory.**

```cpp
static QMap<QString, QString> chip_pack_map = {
	{"CW32F030F8", "WHXY.CW32F030_DFP.1.0.4.pack"},
    // Add a new chip
};
```
