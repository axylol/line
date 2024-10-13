# Build
./scripts/build.sh

if [ -e "./build/line.exe" ]; then
    mkdir -p dist
    
    cp -f "./build/line.exe" "./dist"
    cp -f "C:\msys32\usr\bin\msys-2.0.dll" "./dist"
    cp -f "C:\msys32\usr\bin\msys-gcc_s-1.dll" "./dist"
    cp -f "C:\msys32\usr\bin\msys-stdc++-6.dll" "./dist"

    # strip the binary because its a release build
    strip "./dist/line.exe"

    # copy license
    cp -f "LICENSE" "./dist/LICENSE.txt"

    mkdir -p ./dist/3rd-party-softwares
    wget -O "./dist/3rd-party-softwares/msys2-runtime.txt" "https://raw.githubusercontent.com/msys2/msys2-runtime/refs/heads/msys2-3_2_0-release/winsup/COPYING.LIB"
    wget -O "./dist/3rd-party-softwares/ELFIO.txt" "https://raw.githubusercontent.com/serge1/ELFIO/refs/tags/Release_3.12/LICENSE.txt"
    wget -O "./dist/3rd-party-softwares/MinHook.txt" "https://raw.githubusercontent.com/TsudaKageyu/minhook/refs/tags/v1.3.3/LICENSE.txt"
fi