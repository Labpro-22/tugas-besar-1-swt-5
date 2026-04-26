#include "coredummy/DummyBoardFactory.hpp"

namespace {
TileViewData makeTile(
    const std::string& code,
    const std::string& name,
    TileKind kind,
    const std::string& colorGroup,
    int price = 0
) {
    TileViewData tile;
    tile.code = code;
    tile.name = name;
    tile.kind = kind;
    tile.colorGroup = colorGroup;
    tile.price = price;
    tile.ownerIndex = -1;
    tile.status = PropertyStatusView::Bank;
    tile.houses = 0;
    tile.hotel = false;
    tile.festivalMultiplier = 1;
    return tile;
}
}

std::vector<TileViewData> DummyBoardFactory::createDefaultBoard() {
    return {
        makeTile("GO", "Petak Mulai", TileKind::Special, "DEFAULT"),
        makeTile("GRT", "Garut", TileKind::Street, "COKLAT", 60),
        makeTile("DNU", "Dana Umum", TileKind::Card, "DEFAULT"),
        makeTile("TSK", "Tasikmalaya", TileKind::Street, "COKLAT", 60),
        makeTile("PPH", "Pajak Penghasilan", TileKind::Tax, "DEFAULT"),
        makeTile("GBR", "Stasiun Gambir", TileKind::Railroad, "DEFAULT", 0),
        makeTile("BGR", "Bogor", TileKind::Street, "BIRU_MUDA", 100),
        makeTile("FES", "Festival", TileKind::Festival, "DEFAULT"),
        makeTile("DPK", "Depok", TileKind::Street, "BIRU_MUDA", 100),
        makeTile("BKS", "Bekasi", TileKind::Street, "BIRU_MUDA", 120),
        makeTile("PEN", "Penjara", TileKind::Special, "DEFAULT"),
        makeTile("MGL", "Magelang", TileKind::Street, "MERAH_MUDA", 140),
        makeTile("PLN", "PLN", TileKind::Utility, "ABU_ABU", 0),
        makeTile("SOL", "Solo", TileKind::Street, "MERAH_MUDA", 140),
        makeTile("YOG", "Yogyakarta", TileKind::Street, "MERAH_MUDA", 160),
        makeTile("STB", "Stasiun Bandung", TileKind::Railroad, "DEFAULT", 0),
        makeTile("MAL", "Malang", TileKind::Street, "ORANGE", 180),
        makeTile("DNU", "Dana Umum", TileKind::Card, "DEFAULT"),
        makeTile("SMG", "Semarang", TileKind::Street, "ORANGE", 180),
        makeTile("SBY", "Surabaya", TileKind::Street, "ORANGE", 200),
        makeTile("BBP", "Bebas Parkir", TileKind::Special, "DEFAULT"),
        makeTile("MKS", "Makassar", TileKind::Street, "MERAH", 220),
        makeTile("KSP", "Kesempatan", TileKind::Card, "DEFAULT"),
        makeTile("BLP", "Balikpapan", TileKind::Street, "MERAH", 220),
        makeTile("MND", "Manado", TileKind::Street, "MERAH", 240),
        makeTile("TUG", "Stasiun Tugu", TileKind::Railroad, "DEFAULT", 0),
        makeTile("PLB", "Palembang", TileKind::Street, "KUNING", 260),
        makeTile("PKB", "Pekanbaru", TileKind::Street, "KUNING", 260),
        makeTile("PAM", "PAM", TileKind::Utility, "ABU_ABU", 0),
        makeTile("MED", "Medan", TileKind::Street, "KUNING", 280),
        makeTile("PPJ", "Pergi ke Penjara", TileKind::Special, "DEFAULT"),
        makeTile("BDG", "Bandung", TileKind::Street, "HIJAU", 300),
        makeTile("DEN", "Denpasar", TileKind::Street, "HIJAU", 300),
        makeTile("FES", "Festival", TileKind::Festival, "DEFAULT"),
        makeTile("MTR", "Mataram", TileKind::Street, "HIJAU", 320),
        makeTile("GUB", "Stasiun Gubeng", TileKind::Railroad, "DEFAULT", 0),
        makeTile("KSP", "Kesempatan", TileKind::Card, "DEFAULT"),
        makeTile("JKT", "Jakarta", TileKind::Street, "BIRU_TUA", 350),
        makeTile("PBM", "Pajak Barang Mewah", TileKind::Tax, "DEFAULT"),
        makeTile("IKN", "Ibu Kota Nusantara", TileKind::Street, "BIRU_TUA", 400)
    };
}
