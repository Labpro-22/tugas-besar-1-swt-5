# UI Integration Notes

Tujuan versi ini adalah membuat UI layer bisa dibuild dan dijalankan lebih dulu tanpa memaksa core dan data-layer asli selesai.

## Prinsip versi ini

- UI berkomunikasi lewat `IGameFacade`.
- Implementasi sementara ada di `include/coredummy` dan `src/coredummy`.
- `src/data-layer` dan `include/data-layer` tidak diubah.
- Makefile sengaja tidak meng-compile core/data-layer yang belum stabil.
- Dummy board dibuat oleh `DummyBoardFactory`, bukan dari config parser.

## File dummy yang ditambahkan

- `include/coredummy/GameViewModel.hpp`
- `include/coredummy/IGameFacade.hpp`
- `include/coredummy/MockGameFacade.hpp`
- `include/coredummy/DummyBoardFactory.hpp`
- `src/coredummy/MockGameFacade.cpp`
- `src/coredummy/DummyBoardFactory.cpp`

## Nanti saat core asli sudah siap

Buat adapter baru, misalnya `RealGameFacade`, yang implement `IGameFacade`.
UI tidak perlu tahu apakah data datang dari dummy atau core asli.

Ganti di `src/views/Nimonspoli.cpp`:

```cpp
std::make_unique<MockGameFacade>()
```

menjadi:

```cpp
std::make_unique<RealGameFacade>()
```

## Catatan Raylib

`make run` memakai Raylib asli secara default.
Kalau hanya ingin tes compile tanpa window:

```bash
make run-stub
```
