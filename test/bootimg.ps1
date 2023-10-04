$ErrorActionPreference = "Stop"
cmake --build build --target oickim
.\test\bootimg\mkimg.ps1
# .\test\bootimg\vm\bochs.bat
