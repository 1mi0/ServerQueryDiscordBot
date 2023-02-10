#include "../pch.hpp"
#include "SourceConstants.hpp"
#include "SourceResponseParser.hpp"

using namespace mi0::srv;

RulesResponseParser::RulesResponseParser(const std::vector<uint8_t>&& buf, const size_t transferred, split_packet_page_info page_info) noexcept
    : ResponseParser(SR_A2S_RULES, std::forward<decltype(buf)>(buf), transferred, std::forward<decltype(page_info)>(page_info)) {

    // Already dealt with the first byte...
    _current += 1;

    rule_amount = GetNum<uint16_t>();
}

auto RulesResponseParser::GetNextRule() noexcept -> rule {
    return rule{GetString(), GetString()};
}

PlayersResponseParser::PlayersResponseParser(const std::vector<uint8_t>&& buf, const size_t transferred, split_packet_page_info page_info) noexcept
    : ResponseParser(SR_A2S_RULES, std::forward<decltype(buf)>(buf), transferred, std::forward<decltype(page_info)>(page_info)) {

    // Already dealt with the first byte...
    _current += 1;

    player_amount = GetNum<uint8_t>();
}

auto PlayersResponseParser::GetNextPlayer() noexcept -> player {
    return player{ GetNum<uint8_t>(), GetString(), GetNum<int32_t>(), GetNum<float_t>() };
}

InfoResponseParser::InfoResponseParser(const std::vector<uint8_t>&& buf, const size_t transferred, split_packet_page_info page_info) noexcept
    : ResponseParser(SR_A2S_INFO, std::forward<decltype(buf)>(buf), transferred, std::forward<decltype(page_info)>(page_info)) {

    // Already dealt with the first byte...
    _current += 1;

    protocol = GetNum<uint8_t>();
    name = GetString();
    map = GetString();
    folder = GetString();
    game = GetString();
    game_id = GetNum<int16_t>();
    players = GetNum<uint8_t>();
    max_players = GetNum<uint8_t>();
    bots = GetNum<uint8_t>();
    servertype = static_cast<server_type>(GetNum<uint8_t>());
    environment = static_cast<environment_type>(GetNum<uint8_t>());
    visibility = static_cast<visibility_type>(GetNum<uint8_t>());
    vac = static_cast<vac_type>(GetNum<uint8_t>());
    version = GetString();
    edf = static_cast<edf_type>(GetNum<uint8_t>());

    // EDF(extra data flag) tells us whether extra data's
    // being passed.
    if ((edf & IRP_EDF_PORT) != 0) {
        port = GetNum<int16_t>();
    }
    if ((edf & IRP_EDF_STEAMID) != 0) {
        steamid = GetNum<int64_t>();
    }
    if ((edf & IRP_EDF_SOURCETV) != 0) {
        sourcetv_port = GetNum<int16_t>();
        sourcetv_name = GetString();
    }
    if ((edf & IRP_EDF_KEYWORDS) != 0) {
        keywords = GetString();
    }
    if ((edf & IRP_EDF_GAMEID) != 0) {
        gameid_accurate = GetNum<int64_t>();
        appid_accurate  = static_cast<int32_t>(gameid_accurate.value() & 0xFFFFFFL);
    }
}
