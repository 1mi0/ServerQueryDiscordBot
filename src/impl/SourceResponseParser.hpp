#pragma once

#include "../pch.hpp"
#include "../Common.hpp"
#include "SourceConstants.hpp"

namespace mi0::srv {

// Request responses can be split on multiple pages. One full page is 1400 bytes.
//  ----SPLIT----
// If the request is split the first 4 bytes are 0xfeffffff to indicate it's split.
// Then 4 bytes dedicated to a unique ID the server gives to all requests.
// The last byte of the 9 byte header the split responses have indicates the current
// page index starting from 0 in the upper 4 bits and the total pages in the bottom.
//  ----SPLIT---- END
//
//  ----HEADER----
// The first 5 bytes of the message are the header which always start with 0xfffffff.
// Then there's a byte that represents the response type.
// A2S_INFO    - 0x49
// A2S_PLAYERS - 0x51
// A2S_RULES   - 0x41
//  ----HEADER---- END
//
//  Note:
//  All requests have the same header format, and only differ in the content after
//  the header. 
//

class ResponseParser {
protected:
    // transferred is just the size of the vector,
    // keeping track of the size is important, because
    // the standard vector doesn't do it properly
    explicit ResponseParser(const src_req_type type, const std::vector<uint8_t>&& buf, const size_t transferred, split_packet_page_info page_info) noexcept
        : _type(type), _buf(std::forward<decltype(buf)>(buf)), _limit(static_cast<ssize_t>(transferred)), _page_info(std::forward<decltype(page_info)>(page_info)) {

        if (_limit > _page_info.size_page) {
            _current += page_info.split_header_size;
        }
        _current += SourceConstants::default_header_size;
    }

public:
  // Copying whole buffers is discouraged.
  ResponseParser(const ResponseParser &obj) = delete;
  auto operator=(const ResponseParser &) -> ResponseParser & = delete;
  auto operator=(ResponseParser &&) -> ResponseParser & = delete;
protected:
    // Moving is necessary.
    ResponseParser(ResponseParser&& obj) = default;

public:

    [[nodiscard]] auto HasMore() const noexcept -> bool {
        return _current + 1L < _limit;
    }

protected:
    [[nodiscard]] auto GetString() noexcept -> std::string {
        auto termed = std::find(_buf.begin() + _current, _buf.end(), 0x00);
        assert(termed != _buf.end());

        std::string res = std::string(_buf.begin() + _current, termed);
        _current        = termed - _buf.begin() + 1;

        return res;
    }

    template<numeric T>
    [[nodiscard]] auto GetNum() noexcept -> T {
        assert(_current + static_cast<ssize_t>(sizeof(T)) <= _limit);

        T res     = *reinterpret_cast<T *>(const_cast<uint8_t *>(_buf.data()) + _current);
        _current += sizeof(T);
        return res;
    }

    const src_req_type           _type;
    const std::vector<uint8_t>   _buf;
    const ssize_t                _limit;
    ssize_t                      _current = 0;
    const split_packet_page_info _page_info;
};

class RulesResponseParser : public ResponseParser {
public:
    struct [[nodiscard]] rule {
        const std::string rule, value;
    };

    [[nodiscard]] explicit RulesResponseParser(
        const std::vector<uint8_t>&& buf, size_t transferred, split_packet_page_info page_info) noexcept;

    RulesResponseParser(const RulesResponseParser& obj) = delete;
    RulesResponseParser(RulesResponseParser&& obj) = default;

    auto GetNextRule() noexcept -> rule;

    uint16_t rule_amount;
};

class PlayersResponseParser : public ResponseParser {
public:
    struct [[nodiscard]] player {
        const uint8_t index;
        const std::string name;
        const int32_t score;
        const float_t duration;
    };

    [[nodiscard]] explicit PlayersResponseParser(
        const std::vector<uint8_t>&& buf, size_t transferred, split_packet_page_info page_info) noexcept;

    PlayersResponseParser(const PlayersResponseParser& obj) = delete;
    PlayersResponseParser(PlayersResponseParser&& obj) = default;

    auto GetNextPlayer() noexcept -> player;

    uint8_t player_amount;
};

class InfoResponseParser : public ResponseParser {
public:
    [[nodiscard]] explicit InfoResponseParser(
        const std::vector<uint8_t>&& buf, size_t transferred, split_packet_page_info page_info) noexcept;

    InfoResponseParser(const InfoResponseParser &obj) = delete;
    InfoResponseParser(InfoResponseParser &&obj) = default;

    uint8_t          protocol;
    std::string      name;
    std::string      map;
    std::string      folder;
    std::string      game;
    int16_t          game_id;
    uint8_t          players;
    uint8_t          max_players;
    uint8_t          bots;
    server_type      servertype;
    environment_type environment;
    visibility_type  visibility;
    vac_type         vac;
    std::string      version;
    edf_type         edf;

    std::optional<int16_t>     port;
    std::optional<int64_t>     steamid;
    std::optional<int16_t>     sourcetv_port;
    std::optional<std::string> sourcetv_name;
    std::optional<std::string> keywords;
    std::optional<int64_t>     gameid_accurate;
    std::optional<int32_t>     appid_accurate; 
};

}
