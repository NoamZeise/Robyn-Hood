cd build
cmake .. -G"Ninja Multi-Config" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DGFX_ENV_OPENGL=1 -DGFX_ENV_VULKAN=0
cmake --build . --config Release
cd ..
cp -r resources/audio build/Release
cp -r resources/shaders build/Release
cp -r resources/models build/Release
cp -r resources/textures build/Release
cp -r resources/maps build/Release
cd build/Release
./OpenGL-Environment
cd ../..
