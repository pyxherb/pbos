$ErrorActionPreference = "Stop"
cmake --build build --target pbkim
.\test\bootimg\mkimg.ps1
.\test\bootimg\vm\bochs.bat
