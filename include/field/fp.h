#ifndef BLS12_381_FP_H
#define BLS12_381_FP_H

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "core/rng.h"

namespace bls12_381::field {

class Fp {
public:
    static constexpr int32_t WIDTH = 6;
    static constexpr int32_t BYTE_SIZE = Fp::WIDTH * sizeof(uint64_t);

private:
    std::array<uint64_t, Fp::WIDTH> data;

public:
    Fp();

    Fp(const Fp &fp);
    explicit Fp(uint64_t val);
    explicit Fp(const std::array<uint64_t, Fp::WIDTH> &data);

    Fp(Fp &&fp) noexcept;
    explicit Fp(std::array<uint64_t, Fp::WIDTH> &&data);

    static Fp zero() noexcept;
    static Fp one() noexcept;
    static Fp random(rng::core::RngCore &rng);

    static Fp montgomery_reduce(const std::array<uint64_t, Fp::WIDTH * 2> &ts);
    static Fp sum_of_products(const std::vector<Fp> &a, const std::vector<Fp> &b);
    static std::optional<Fp> from_bytes(const std::array<uint8_t, Fp::BYTE_SIZE> &bytes);

    [[nodiscard]] const std::array<uint64_t, Fp::WIDTH> &get_data() const;

    [[nodiscard]] bool is_zero() const;
    [[nodiscard]] bool lexicographically_largest() const;

    [[nodiscard]] std::string to_hex_str() const;
    [[nodiscard]] std::array<uint8_t, Fp::BYTE_SIZE> to_bytes() const;

    [[nodiscard]] Fp square() const;
    [[nodiscard]] Fp subtract_modulus() const;
    [[nodiscard]] Fp pow(const std::array<uint64_t, Fp::WIDTH> &exp) const;

    [[nodiscard]] std::optional<Fp> sqrt() const;
    [[nodiscard]] std::optional<Fp> invert() const;

private:
    static Fp reduce(const std::array<uint64_t, Fp::WIDTH * 2> &limbs);

public:
    Fp operator-() const;
    Fp &operator=(Fp &&rhs) noexcept;
    Fp &operator=(const Fp &rhs);

    Fp &operator+=(const Fp &rhs);
    Fp &operator-=(const Fp &rhs);
    Fp &operator*=(const Fp &rhs);

    Fp operator+(const Fp &rhs) const;
    Fp operator-(const Fp &rhs) const;
    Fp operator*(const Fp &rhs) const;

public:
    friend inline bool operator==(const Fp &a, const Fp &b) { return a.data == b.data; }
    friend inline bool operator!=(const Fp &a, const Fp &b) { return a.data != b.data; }
};

} // namespace bls12_381::field

#endif //BLS12_381_FP_H