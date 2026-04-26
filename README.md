[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/b842RB8g)
# Tugas Besar 1 IF2010 Pemrograman Berorientasi Objek

## Cara Menjalankan
Pastikan urutan folder sebagai berikut:

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

### Ubuntu atau WSL

```bash
cd ../raylib
mkdir -p build
cd build
cmake -DBUILD_SHARED_LIBS=OFF ..
make -j$(nproc)
```

Jika cmake atau dependency belum ada:

```bash
sudo apt update
sudo apt install git build-essential cmake libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev
```

Setelah Raylib selesai di-build, balik ke project:

```bash
cd ../../tugas-besar-1-swt-5
make clean
make run
```

Jika hanya ingin compile tanpa membuka window:

```bash
make clean
make run-stub
```

## Kontributor
| NIM | Nama |
| --- | --- |
| 13524017 | Aziza Dharma Putri |
| 13524029 | Niko Samuel Simanjuntak |
| 13524045 | Ahmad Zaky Robbani |
| 13524063 | Marcel Luther Sitorus |
| 13524081 | Alya Nur Rahmah |