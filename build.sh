echo "Building for both windows and Linux"
make clean && make && make windows

rm -r windows
rm -r linux

mkdir -p windows
cp axlflash.exe windows
cp setup.bat windows

zip -r windows.zip windows
mv windows.zip windows

mkdir -p linux
cp axlflash linux
cp setup.sh linux

zip -r linux.zip linux
mv linux.zip linux
