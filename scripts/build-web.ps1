$ErrorActionPreference = "Stop"

emcmake cmake -S . -B build/web -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DMATCH3_BUILD_TESTS=OFF

cmake --build build/web --config Release
Write-Host "Web build: build/web/match3.html"
