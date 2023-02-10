#pragma once

#include "pch.hpp"

namespace mi0::srv {

const uint16_t SRV_PORT = 12934;

// A RequestFactory constructs requests
template <class T>
concept CRequestFactory = requires(T a) {
    { a.MakeRequest() } -> std::same_as<std::vector<uint8_t>>;
};

// TSize is the size of the challenge in bytes.
template <class T, size_t TSize>
concept CChallengedRequestFactory = requires(T a) {
    { a.MakeRequest() } -> std::same_as<std::vector<uint8_t>>;
    a.SetChallenge(std::declval<std::array<uint8_t, TSize>>());
};

// Should technically include all fundamental types.
// To be changed if it does not.
template<class T>
concept numeric = std::integral<T> || std::floating_point<T>;


// TODO: add more servers - possibly minecraft, fivem, samp...

}
