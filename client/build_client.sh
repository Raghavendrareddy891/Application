set -e

echo "⭐ Running build_client.sh"
cd /d/Application/client

mkdir -p build
cd build

echo "🔧 Running CMake..."
cmake -G "MinGW Makefiles" ..

echo "🔨 Running mingw32-make..."
mingw32-make

echo "🎉 Build completed successfully!"
