#ifndef BLS12_381_G1_PROJECTIVE_H
#define BLS12_381_G1_PROJECTIVE_H

#include <array>
#include <cstdint>
#include <vector>

#include "core/rng.h"

#include "field/fp.h"

namespace bls12_381::scalar { class Scalar; }
namespace bls12_381::group { class G1Affine; }

namespace bls12_381::group {

class G1Projective {
private:
    field::Fp x;
    field::Fp y;
    field::Fp z;

public:
    G1Projective();

    G1Projective(const G1Projective &point);
    explicit G1Projective(const G1Affine &point);
    explicit G1Projective(const field::Fp &x, const field::Fp &y, const field::Fp &z);

    G1Projective(G1Projective &&point) noexcept;
    explicit G1Projective(G1Affine &&point);
    explicit G1Projective(field::Fp &&x, field::Fp &&y, field::Fp &&z);

    static G1Projective identity() noexcept;
    static G1Projective generator() noexcept;
    static G1Projective random(rng::core::RngCore &rng);

    static std::vector<G1Affine> batch_normalize(const std::vector<G1Projective> &points);

    [[nodiscard]] const field::Fp &get_x() const noexcept;
    [[nodiscard]] const field::Fp &get_y() const noexcept;
    [[nodiscard]] const field::Fp &get_z() const noexcept;

    [[nodiscard]] bool is_identity() const;
    [[nodiscard]] bool is_on_curve() const;

    [[nodiscard]] G1Projective doubles() const;
    [[nodiscard]] G1Projective add(const G1Projective &rhs) const;
    [[nodiscard]] G1Projective add_mixed(const G1Affine &rhs) const;
    [[nodiscard]] G1Projective mul_by_x() const;
    [[nodiscard]] G1Projective clear_cofactor() const;

private:
    [[nodiscard]] G1Projective multiply(const std::array<uint8_t, 32> &bytes) const;

public:
    G1Projective operator-() const;
    G1Projective &operator=(const G1Projective &rhs);
    G1Projective &operator=(G1Projective &&rhs) noexcept;

    G1Projective &operator+=(const G1Projective &rhs);
    G1Projective &operator-=(const G1Projective &rhs);
    G1Projective &operator+=(const G1Affine &rhs);
    G1Projective &operator-=(const G1Affine &rhs);
    G1Projective &operator*=(const scalar::Scalar &rhs);

    G1Projective operator+(const G1Projective &rhs) const;
    G1Projective operator-(const G1Projective &rhs) const;
    G1Projective operator+(const G1Affine &rhs) const;
    G1Projective operator-(const G1Affine &rhs) const;
    G1Projective operator*(const scalar::Scalar &rhs) const;

public:
    friend inline bool operator==(const G1Projective &a, const G1Projective &b) {
        field::Fp x1 = a.x * b.z;
        field::Fp x2 = b.x * a.z;
        field::Fp y1 = a.y * b.z;
        field::Fp y2 = b.y * a.z;

        bool a_is_zero = a.z.is_zero();
        bool b_is_zero = b.z.is_zero();

        return (a_is_zero & b_is_zero) | ((!a_is_zero) & (!b_is_zero) & (x1 == x2) & (y1 == y2));
    }

    friend inline bool operator!=(const G1Projective &a, const G1Projective &b) {
        return !(a == b);
    }
};

} // namespace bls12_381::group

#endif //BLS12_381_G1_PROJECTIVE_H