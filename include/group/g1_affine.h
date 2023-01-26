#ifndef BLS12_381_G1_H
#define BLS12_381_G1_H

#include "field/fp.h"

class G1Projective;

class G1Affine {
private:
    Fp x;
    Fp y;
    bool infinity;

public:
    G1Affine();
    explicit G1Affine(G1Projective point);
    explicit G1Affine(Fp x, Fp y, bool infinity);

    static G1Affine identity();
    static G1Affine generator();

    static std::optional<G1Affine> from_compressed(std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t)> bytes);
    static std::optional<G1Affine> from_compressed_unchecked(std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t)> bytes);
    static std::optional<G1Affine> from_uncompressed(std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t) * 2> bytes);
    static std::optional<G1Affine> from_uncompressed_unchecked(std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t) * 2> bytes);

    [[nodiscard]] Fp getX() const;
    [[nodiscard]] Fp getY() const;

    [[nodiscard]] bool is_identity() const;
    [[nodiscard]] bool is_on_curve() const;
    [[nodiscard]] bool is_torsion_free() const;

    [[nodiscard]] std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t)> to_compressed() const;
    [[nodiscard]] std::array<uint8_t, Fp::WIDTH * sizeof(uint64_t) * 2> to_uncompressed() const;

    [[nodiscard]] G1Affine endomorphism() const;

public:
    G1Affine &operator=(const G1Affine &rhs);
    G1Affine operator-() const;

public:
    friend inline bool operator==(const G1Affine &a, const G1Affine &b) {
        return (a.infinity & b.infinity) | ((!a.infinity) & (!b.infinity) & (a.x == b.x) & (a.y == b.y));
    }
    friend inline bool operator!=(const G1Affine &a, const G1Affine &b) {
        return !(a == b);
    }
};

#endif //BLS12_381_G1_H