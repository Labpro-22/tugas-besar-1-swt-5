# Raylib Runtime Fix

Stub hanya untuk memastikan UI bisa dikompilasi ketika backend belum selesai.

Untuk menjalankan GUI sungguhan, pakai Raylib asli. Pastikan urutan foldernya begini:

```text
NAMA_FOLDER/
├── raylib/
└── tugas-besar-1-swt-5/
```

Cara clone Raylib:

```bash
cd ..
git clone https://github.com/raysan5/raylib.git
```

## Ubuntu atau WSL

```bash
cd ../raylib
mkdir -p build
cd build
cmake -DBUILD_SHARED_LIBS=OFF ..
make -j$(nproc)
```

Kalau cmake atau dependency belum ada:

```bash
sudo apt update
sudo apt install git build-essential cmake libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev
```

Setelah Raylib selesai dibuild, balik ke project:

```bash
cd ../../tugas-besar-1-swt-5
make clean
make run
```

Kalau hanya ingin compile tanpa membuka window:

```bash
make clean
make run-stub
```
