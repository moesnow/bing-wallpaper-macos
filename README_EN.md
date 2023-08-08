# bing-wallpaper-macos

[中文](./README.md) | **English**

Automatically update Bing wallpapers on macOS, with support for multiple monitors.

## Principle

Bing wallpapers are downloaded to the `~/.local/bing-wallpaper-macos` directory.

The last update date is recorded in the `~/.local/bing-wallpaper-macos` directory.

The old wallpaper files in the directory that are older than 30 days will be removed.

## Installation

### Homebrew

```zsh
brew tap moesnow/tools
brew install bing-wallpaper-macos
```

## Usage

### Automatically update wallpapers

```zsh
brew services start bing-wallpaper-macos
```

### Manually update wallpapers

```zsh
bing-wallpaper-macos
```

### Optional parameters

```zsh
Usage: bing-wallpaper-macos [options]
Options:
  --auto     : Check if program run today
  --version  : Display program version
  --help     : Display this help message
```

## Compilation

```zsh
brew install nlohmann-json
git clone https://github.com/moesnow/bing-wallpaper-macos
cd bing-wallpaper-macos
make
```