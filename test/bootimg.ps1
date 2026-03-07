$ErrorActionPreference = "Stop"
cmake --build build --target initcar
.\test\bootimg\mkimg.ps1
.\test\bootimg\vm\bochs.bat
