#include "pch.hpp"
#include <boost/asio/placeholders.hpp>

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

namespace ip = boost::asio::ip;
using     ip::udp;
namespace baph = boost::asio::placeholders;

enum src_req_type : uint8_t {
    SR_A2S_INFO   = 0x54,
    SR_A2S_PLAYER = 0x55,
    SR_A2S_RULES  = 0x56,
};

enum server_type : uint8_t {
    IRP_ST_DEDICATED    = 'd',
    IRP_ST_NONDEDICATED = 'l',
    IRP_ST_PROXY        = 'p',
};

enum environment_type : uint8_t {
    IRP_ET_LINUX     = 'l',
    IRP_ET_WINDOWS   = 'w',
    IRP_ET_MACOS_OLD = 'm',
    IRP_ET_MACOS_NEW = 'o',
    IRP_ET_MAC       = IRP_ET_MACOS_OLD | IRP_ET_MACOS_NEW,
};

enum visibility_type : uint8_t {
    IRP_VT_PUBLIC  = 0,
    IRP_VT_PRIVATE = 1,
};

enum vac_type : uint8_t {
    IRP_VACT_UNSECURED = 0,
    IRP_VACT_SECURED   = 1,
};

enum edf_type : uint8_t {
    IRP_EDF_PORT     = 0x80,
    IRP_EDF_STEAMID  = 0x10,
    IRP_EDF_SOURCETV = 0x40,
    IRP_EDF_GAME     = 0x20,
    IRP_EDF_GAMEID   = 0x01,
};

template<class T>
concept numeric = std::integral<T> || std::floating_point<T>;

constexpr size_t src_eng_q_size = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t) * 21;
constexpr size_t src_nor_q_size = 9;
[[nodiscard]] inline std::vector<uint8_t> src_make_req(const src_req_type type, const std::array<uint8_t, 4> challenge = { 0xff, 0xff, 0xff, 0xff }) noexcept {
    if (type == SR_A2S_INFO) {
        return std::vector<uint8_t> { 
            0xff, 0xff, 0xff, 0xff, 0x54, 0x53, 0x6F, 0x75, 0x72, 0x63, 0x65, 0x20, 0x45, 0x6E, 0x67, 0x69, 0x6E, 0x65, 0x20, 0x51, 0x75, 0x65, 0x72, 0x79, 0x00
        };
    }
    else {
        std::vector<uint8_t> header_req;
        header_req.insert(header_req.begin(), { 0xff, 0xff, 0xff, 0xff, type });
        header_req.insert(header_req.begin() + 5, challenge.begin(), challenge.end());
        return header_req;
    }
}

class ResponseParser {
protected:
    explicit ResponseParser(const src_req_type type, const std::vector<uint8_t> buf, const size_t transferred) noexcept
        : type(type), _buf(buf), _limit(transferred), _current(0) {
        // No point in dealing with the split header or the first 4 bytes
        // of the message header which are always 0xfffffff
        if (_limit > 1400) {
            _current += 9;
        }
        _current += 4;
    }

    ResponseParser(const ResponseParser& obj) = delete;
    ResponseParser(ResponseParser&& obj)
        : type(std::move(obj.type)), _buf(std::move(obj._buf)), _limit(std::move(obj._limit)), _current(std::move(obj._current)) {}


public:

    [[nodiscard]] bool HasMore() noexcept {
        return _current + 1 < _limit;
    }

protected:
    [[nodiscard]] std::string GetString() noexcept {
        auto termed = std::find(_buf.begin() + _current, _buf.end(), 0x00);
        assert(termed != _buf.end());

        std::string res = std::string(_buf.begin() + _current, termed);
        _current        = termed - _buf.begin() + 1;

        return res;
    }


    template<numeric T>
    [[nodiscard]] T GetNum() noexcept {
        assert(_current + sizeof(T) <= _limit);

        T res     = *reinterpret_cast<T *>(const_cast<uint8_t *>(_buf.data()) + _current);
        _current += sizeof(T);
        return res;
    }

public:
    const src_req_type type;
protected:
    const std::vector<uint8_t> _buf;
    const size_t _limit;
    size_t _current;
};

class RulesResponseParser : public ResponseParser {
public:
    struct rule {
        std::string rule, value;
    };

    explicit RulesResponseParser(const std::vector<uint8_t> buf, const size_t transferred) noexcept
        : ResponseParser(SR_A2S_RULES, std::move(buf), transferred) {
        // The first byte is the type of the response, but we
        // already know what it is, so we don't really care...
        // The next short is the amount of the rules.
        _current   += 1;
        rule_amount = GetNum<uint16_t>();
    }

    RulesResponseParser(const RulesResponseParser& obj) = delete;
    RulesResponseParser(RulesResponseParser&& obj)
        : ResponseParser(std::move(obj)), rule_amount(std::move(obj.rule_amount)) {} 

    [[nodiscard]] rule GetNextRule() noexcept {
        // After the header (split header of 9 bytes if exists,
        // msg header, amount of rules) come the rules which
        // all have 2 properties: string name, string value
        return rule { GetString(), GetString() };
    }

public:
    int16_t rule_amount;
};

class PlayersResponseParser : public ResponseParser {
    public:
        struct player {
            uint8_t index;
            std::string name;
            int32_t score;
            float_t duration;
        };

        explicit PlayersResponseParser(const std::vector<uint8_t> buf, const size_t transferred) noexcept
            : ResponseParser(SR_A2S_RULES, std::move(buf), transferred) {
            // The first byte is the type of the response, but we
            // already know what it is, so we don't really care...
            // Unlike in A2S_RULES, A2S_PLAYERS has the amount of
            // players as a byte
            _current     += 1;
            player_amount = GetNum<uint8_t>();
        }

        PlayersResponseParser(const PlayersResponseParser& obj) = delete;
        PlayersResponseParser(PlayersResponseParser&& obj)
            : ResponseParser(std::move(obj)), player_amount(std::move(obj.player_amount)) {}

        [[nodiscard]] player GetNextPlayer() noexcept {
            // After the header (split header of 9 bytes if exists,
            // msg header, amount of players) come the players which
            // all have 4 properties: byte index, string name,
            //                        long score, float joinduration
            return player { GetNum<uint8_t>(), GetString(), GetNum<int32_t>(), GetNum<float_t>() };
        }
    public:
        uint8_t player_amount;
};

class InfoResponseParser : public ResponseParser {
public:
    explicit InfoResponseParser(const std::vector<uint8_t> buf, const size_t transferred) noexcept
        : ResponseParser(SR_A2S_INFO, std::move(buf), transferred) {
        // The first byte is the type of the response, but we
        // already know what it is, so we don't really care...
        _current   += 1;

        protocol    = GetNum<uint8_t>();
        name        = GetString();
        map         = GetString();
        folder      = GetString();
        game        = GetString();
        game_id     = GetNum<int16_t>();
        players     = GetNum<uint8_t>();
        max_players = GetNum<uint8_t>();
        bots        = GetNum<uint8_t>();
        servertype  = static_cast<server_type>(GetNum<uint8_t>());
        environment = static_cast<environment_type>(GetNum<uint8_t>());
        visibility  = static_cast<visibility_type>(GetNum<uint8_t>());
        vac         = static_cast<vac_type>(GetNum<uint8_t>());
        version     = GetString();
        edf         = static_cast<edf_type>(GetNum<uint8_t>());

        if (edf & IRP_EDF_PORT) {
            port = GetNum<int16_t>();
        }
        if (edf & IRP_EDF_STEAMID) {
            steamid = GetNum<int64_t>();
        }
        if (edf & IRP_EDF_SOURCETV) {
            sourcetv_port = GetNum<int16_t>();
            sourcetv_name = GetString();
        }
        if (edf & IRP_EDF_GAME) {
            keywords = GetString();
        }
        if (edf & IRP_EDF_GAMEID) {
            gameid_accurate = GetNum<int64_t>();
            appid_accurate  = static_cast<int32_t>(gameid_accurate.value() & 0xFFFFFFl);
        }
    }

    InfoResponseParser(const InfoResponseParser& obj) = delete;
    InfoResponseParser(InfoResponseParser&& obj) :
        ResponseParser(std::move(obj)),
        protocol(std::move(obj.protocol)),
        name(std::move(obj.name)),
        map(std::move(obj.map)),
        folder(std::move(obj.folder)),
        game(std::move(obj.game)),
        game_id(std::move(obj.game_id)),
        players(std::move(obj.players)),
        max_players(std::move(obj.max_players)),
        bots(std::move(obj.bots)),
        servertype(std::move(obj.servertype)),
        environment(std::move(obj.environment)),
        visibility(std::move(obj.visibility)),
        vac(std::move(obj.vac)),
        version(std::move(obj.version)),
        edf(std::move(obj.edf)),
        port(std::move(obj.port)),
        steamid(std::move(obj.steamid)),
        sourcetv_port(std::move(obj.sourcetv_port)),
        sourcetv_name(std::move(obj.sourcetv_name)),
        keywords(std::move(obj.keywords)),
        gameid_accurate(std::move(obj.gameid_accurate)),
        appid_accurate(std::move(obj.appid_accurate)) {}

public:
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

class SourceRequest {
public:
    explicit SourceRequest(boost::asio::io_service& io_service,
                  const std::string ip,      const uint16_t port,
                  const std::string dest_ip, const uint16_t dest_port) {
        boost::system::error_code ec;
        ip::address ip_address = ip::address::from_string(ip, ec);

        if (ec.value() != 0) {
            throw std::invalid_argument((std::ostringstream() << "Could not parse IP! " << ec.value()).str());
        }

        ip::address ip_address_dest = ip::address::from_string(dest_ip, ec);

        if (ec.value() != 0) {
            throw std::invalid_argument((std::ostringstream() << "Could not parse IP! " << ec.value()).str());
        }

        _endpoint = udp::endpoint(ip_address_dest, dest_port);
        _socket   = udp::socket(io_service);

        _socket->open(udp::v4());
        _socket->bind(udp::endpoint(ip_address, port));
    }

    void Send(src_req_type type, const std::array<uint8_t, 4> challenge = { 0xff, 0xff, 0xff, 0xff }) {
        auto req = src_make_req(type, challenge);
        _socket->async_send_to(
            boost::asio::buffer(req, req.size()), _endpoint,
            boost::bind(&SourceRequest::handleSend, this, type,
                baph::error));
    }

private:
    void handleSend(const src_req_type type,
                    const boost::system::error_code& ec) {
        if (ec.value() != 0) {
            throw std::runtime_error((std::ostringstream() << "ec failed at handleSend: " << ec.value()).str());
        }
        startReceive(type);
    }

    void startReceive(const src_req_type type) {
        _socket->async_receive_from(boost::asio::buffer(_buffer, _buffer.size()), _endpoint,
            boost::bind(&SourceRequest::handleReceive, this, type,
                baph::error,
                baph::bytes_transferred));
    }

    void handleReceive(const src_req_type type,
                       const boost::system::error_code& ec,
                       size_t bytes_transferred) {
        if (ec.value() != 0) {
            throw std::runtime_error((std::ostringstream() << "ec failed at handleReceive: " << ec.value()).str());
        }

        // Getting 0x41 means that a challenge was returned.
        // When the server returns a challenge we need to
        // resend the request with the challenge to get the 
        // actual response
        if (_buffer[4] == 0x41) {
            std::array<uint8_t, 4> challenge;
            std::memcpy(challenge.data(), _buffer.data() + 5, 4);
            Send(type, challenge);
            return;
        }

        // It's important to keep track of the bytes we get,
        // std::vector cannot do it for us when it's used as
        // a buffer.
        size_t transferred = bytes_transferred;

        // 0xff means single response, 0xfe means split in 
        // multiple packets.
        std::vector<uint8_t> buf;
        if (_buffer[0] == 0xff) {
            buf = std::vector<uint8_t>(std::begin(_buffer), std::end(_buffer));
        }
        else if (_buffer[0] == 0xfe) {
            // The first half of the 8th byte is the current page, 
            // the second half of it is the total amount of pages
            uint8_t current_amount = _buffer[8];
            uint8_t current        = current_amount >> 4;
            uint8_t amount         = current_amount & 0b1111;

            // The size of the buffer has to be as much as we can 
            // possibly get as a response. A single response can 
            // be 1400 bytes long. Remove 9 from the following 
            // ones since they are the same header over and over
            buf = std::vector<uint8_t>(1400 + (amount - 1) * (1400 - 9));
            buf.insert(buf.begin(), _buffer.begin(), _buffer.begin() + transferred);
            for (int32_t i = current + 1; i < amount; i++) {
                size_t curr_transferred = _socket->receive_from(boost::asio::buffer(_buffer.begin(), 1400), _endpoint);
                buf.insert(buf.begin() + transferred, _buffer.begin() + 9, _buffer.begin() + curr_transferred);
                transferred += curr_transferred - 9;
            }
        }

        switch (type) {
            case SR_A2S_RULES:
                response_parser = std::make_shared<RulesResponseParser>(std::move(buf), transferred);
                break;
            case SR_A2S_INFO:
                response_parser = std::make_shared<InfoResponseParser>(std::move(buf), transferred);
                break;
            case SR_A2S_PLAYER:
                response_parser = std::make_shared<PlayersResponseParser>(std::move(buf), transferred);
                break;
        }
    }

public:
    std::variant<std::shared_ptr<InfoResponseParser>, std::shared_ptr<RulesResponseParser>, std::shared_ptr<PlayersResponseParser>> response_parser;
private:
    std::optional<udp::socket> _socket;
    std::array<uint8_t, 1400>  _buffer;
    udp::endpoint              _endpoint;
};
}
