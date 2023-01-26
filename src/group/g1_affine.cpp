#include "group/g1_affine.h"
#include "group/g1_projective.h"
#include "field/constant.h"

G1Affine::G1Affine() : x{Fp::zero()}, y{Fp::one()}, infinity{true} {}

G1Affine::G1Affine(const G1Projective point) : x{Fp::zero()}, y{Fp::one()}, infinity{true} {
    std::optional<Fp> z_inv = point.getZ().invert();
    if (!z_inv.has_value()) {
        return;
    } else {
        this->x = point.getX() * z_inv.value();
        this->y = point.getY() * z_inv.value();
    }
}

G1Affine::G1Affine(const Fp x, const Fp y, bool infinity) : x{x}, y{y}, infinity{infinity} {}

G1Affine G1Affine::identity() {
    return G1Affine{};
}

G1Affine G1Affine::generator() {
    return G1Affine{
            Fp({
                       0x5cb38790fd530c16, 0x7817fc679976fff5, 0x154f95c7143ba1c1,
                       0xf0ae6acdf3d0e747, 0xedce6ecc21dbf440, 0x120177419e0bfb75,
               }),
            Fp({
                       0xbaac93d50ce72271, 0x8c22631a7918fd8e, 0xdd595f13570725ce,
                       0x51ac582950405194, 0x0e1c8c3fad0059c0, 0x0bbc3efc5008a26a,
               }),
            false,
    };
}

std::optional<G1Affine> G1Affine::from_compressed(const std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t)> bytes) {
    std::optional<G1Affine> res = G1Affine::from_compressed_unchecked(bytes);
    if (!res.has_value()) return std::nullopt;
    if (!res.value().is_torsion_free()) return std::nullopt;
    return res.value();
}

std::optional<G1Affine> G1Affine::from_compressed_unchecked(const std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t)> bytes) {
    bool compression_flag_set = (bytes[0] >> 7) & 1;
    bool infinity_flat_set = (bytes[0] >> 6) & 1;
    bool sort_flat_set = (bytes[0] >> 5) & 1;

    // try to decode the x-coordinate
    std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t)> temp{0};
    for (int i = 0; i < std::size(temp); ++i)
        temp[i] = bytes[i];
    temp[0] &= 0b00011111;
    std::optional<Fp> sx = Fp::from_bytes(temp);

    if (!sx.has_value()) return std::nullopt;
    if (infinity_flat_set & compression_flag_set & (!sort_flat_set) & sx.value().is_zero()) return G1Affine::identity();

    Fp x_cord = sx.value();

    std::optional<Fp> sy = (x_cord.square() * x_cord + B).sqrt();
    if (!sy.has_value()) return std::nullopt;

    Fp y_cord = sy.value();
    if (!(y_cord.lexicographically_largest() ^ sort_flat_set)) y_cord = -y_cord;

    if (infinity_flat_set || (!compression_flag_set)) return std::nullopt;
    return G1Affine{
            x_cord,
            y_cord,
            infinity_flat_set,
    };
}

std::optional<G1Affine> G1Affine::from_uncompressed(const std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t) * 2> bytes) {
    assert(bytes.size() == Fp::WIDTH * sizeof(uint64_t) * 2);
    std::optional<G1Affine> res = G1Affine::from_uncompressed_unchecked(bytes);
    if (!res.has_value()) return std::nullopt;
    if (!res.value().is_on_curve() || !res.value().is_torsion_free()) return std::nullopt;
    return res.value();
}

std::optional<G1Affine> G1Affine::from_uncompressed_unchecked(const std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t) * 2> bytes) {
    assert(bytes.size() == Fp::WIDTH * sizeof(uint64_t) * 2);

    bool compression_flag_set = (bytes[0] >> 7) & 1;
    bool infinity_flat_set = (bytes[0] >> 6) & 1;
    bool sort_flat_set = (bytes[0] >> 5) & 1;

    // try to decode the x-coordinate
    std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t)> temp{0};
    for (int i = 0; i < std::size(temp); ++i)
        temp[i] = bytes[i];
    temp[0] &= 0b00011111;
    std::optional<Fp> sx = Fp::from_bytes(temp);

    // try to decode the y-coordinate
    for (int i = 0; i < std::size(temp); ++i)
        temp[i] = bytes[i + Fp::WIDTH * sizeof(uint64_t)];
    std::optional<Fp> sy = Fp::from_bytes(temp);

    if (!sx.has_value() || !sy.has_value()) return std::nullopt;
    if (compression_flag_set || sort_flat_set) return std::nullopt;
    if (infinity_flat_set && (!sx.value().is_zero() || !sy.value().is_zero())) return std::nullopt;

    G1Affine res{
            sx.value(),
            sy.value(),
            infinity_flat_set,
    };
    if (infinity_flat_set) {
        return res;
    } else {
        return G1Affine::identity();
    }
}

Fp G1Affine::getX() const {
    return this->x;
}

Fp G1Affine::getY() const {
    return this->y;
}

bool G1Affine::is_identity() const {
    return this->infinity;
}

bool G1Affine::is_on_curve() const {
    return (this->y.square() - (this->x.square() * this->x)) == B | this->infinity;
}

bool G1Affine::is_torsion_free() const {
    G1Projective minus_x_squared_times_p = -G1Projective(*this).mul_by_x().mul_by_x();
    G1Affine endomorphism_p = this->endomorphism();

    return minus_x_squared_times_p == G1Projective(endomorphism_p);
}

std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t)> G1Affine::to_compressed() const {
    std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t)> bytes{};

    if (this->infinity)
        bytes = this->x.to_bytes();
    else
        bytes = Fp::zero().to_bytes();

    bytes[0] |= (static_cast<uint8_t>(1) << 7);                                                                         // compression flag
    bytes[0] |= (this->infinity ? static_cast<uint8_t>(0) : (static_cast<uint8_t>(1) << 6));                            // infinity flag
    bytes[0] |= (((!this->infinity) && this->y.lexicographically_largest()) ? static_cast<uint8_t>(0) : (               // sort flag
            static_cast<uint8_t>(1) << 5));

    return bytes;
}

std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t) * 2> G1Affine::to_uncompressed() const {
    std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t) * 2> bytes{};

    std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t)> x_bytes = (this->infinity ? this->x : Fp::zero()).to_bytes();
    std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t)> y_bytes = (this->infinity ? this->y : Fp::zero()).to_bytes();

    std::copy(x_bytes.begin(), x_bytes.end(), bytes.begin());
    std::copy(y_bytes.begin(), y_bytes.end(), bytes.begin() + Fp::WIDTH * sizeof(uint64_t));

    bytes[0] |= (this->infinity ? static_cast<uint8_t>(0) : (static_cast<uint8_t>(1) << 6));                            // infinity flag

    return bytes;
}

G1Affine G1Affine::endomorphism() const {
    G1Affine res = *this;
    res.x *= BETA;
    return res;
}

G1Affine &G1Affine::operator=(const G1Affine &rhs) {
    if (this == &rhs) return *this;
    this->x = rhs.x;
    this->y = rhs.y;
    this->infinity = rhs.infinity;
    return *this;
}

G1Affine G1Affine::operator-() const {
    return G1Affine{
            this->x,
            this->infinity ? (-this->y) : Fp::one(),
            this->infinity,
    };
}