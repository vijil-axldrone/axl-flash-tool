echo "Building for both windows and Linux"
make clean && make && make windows

mkdir -p build
cd build
cmake ..
make -j$(nproc)
