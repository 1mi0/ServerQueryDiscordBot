#include "../pch.hpp"
#include "SourceSockets.hpp"

using namespace mi0::srv;

SourceRequestFactory::SourceRequestFactory(const src_req_type req)
    : _request_type(req), _challenge({0xff, 0xff, 0xff, 0xff}) {}

void SourceRequestFactory::SetChallenge(std::array<uint8_t, 4> challenge) {
    _challenge = std::move(challenge);
}

auto SourceRequestFactory::MakeRequest() -> std::vector<uint8_t> {
    if (_request_type == SR_A2S_INFO) {
        // A weird byte trickery coming straight from valve website
        return std::vector<uint8_t>{0xff, 0xff, 0xff, 0xff, 0x54,
                                    0x53, 0x6F, 0x75, 0x72, 0x63,
                                    0x65, 0x20, 0x45, 0x6E, 0x67,
                                    0x69, 0x6E, 0x65, 0x20, 0x51,
                                    0x75, 0x65, 0x72, 0x79, 0x00};
    }          // 0xffffffff + req_type + (0xffffffff or _challenge)

    std::vector<uint8_t> header_req;
    header_req.insert(header_req.begin(), {0xff, 0xff, 0xff, 0xff, _request_type});
    header_req.insert(header_req.begin() + 5, _challenge.begin(), _challenge.end());
    return header_req;
}

