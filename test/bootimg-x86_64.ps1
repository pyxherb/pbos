$ErrorActionPreference = "Stop"
cmake --build build --target initcar
.\test\bootimg\mkimg-x86_64.ps1
.\test\bootimg\vm\bochs-x86_64.bat
