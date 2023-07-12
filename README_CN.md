# bing-wallpaper-macos

[English](./README.md) | **中文**

在 macOS 上每天自动更新 Bing 壁纸，支持多显示器

## 原理

Bing 壁纸会被下载到 `~/.local/bing-wallpaper-macos` 目录内

并在 `~/.config/bing-wallpaper-macos` 目录内记录上次更新日期

更新壁纸时会先删除目录内的旧壁纸文件

## 安装

### Homebrew

```zsh
brew tap moesnow/tools
brew install bing-wallpaper-macos
```

## 使用

### 自动更新壁纸

```zsh
brew services start bing-wallpaper-macos
```

### 手动更新壁纸

```zsh
bing-wallpaper-macos
```

### 可选参数

```zsh
Usage: bing-wallpaper-macos [options]
Options:
  --auto     : Check if program run today
  --version  : Display program version
  --help     : Display this help message
```

## 编译

```zsh
brew install nlohmann-json
git clone https://github.com/moesnow/bing-wallpaper-macos
cd bing-wallpaper-macos
make
```
