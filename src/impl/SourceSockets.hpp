#pragma once

#include "../pch.hpp"
#include "../Common.hpp"
#include "SourceConstants.hpp"
#include "SourceResponseParser.hpp"

namespace mi0::srv {

namespace ip = boost::asio::ip;
using     ip::udp;
namespace baph = boost::asio::placeholders;

class SourceRequestFactory {
public:
  [[nodiscard]] explicit SourceRequestFactory(src_req_type req);

  SourceRequestFactory(const SourceRequestFactory &obj) = delete;
  SourceRequestFactory(SourceRequestFactory &&obj) = default;

  void SetChallenge(std::array<uint8_t, 4> challenge);

  [[nodiscard]] auto MakeRequest() -> std::vector<uint8_t>;

private:
    const src_req_type     _request_type;
    std::array<uint8_t, 4> _challenge;
};

// 4 is the size of the SourceQuery challenge in bytes.
template <CChallengedRequestFactory<4> TReqFactory>
class SourceSocket {
public:
    [[nodiscard]] explicit SourceSocket(const std::string& dest_ip, const uint16_t dest_port,
                                        TReqFactory&& req, source_engine_type engine)
    : _engine_type(engine), _socket(udp::socket(_io_service)), _req(std::move(req)) {

        boost::system::error_code error_code;
        
        ip::address dest_ip_address = ip::address::from_string(dest_ip, error_code);
        if (error_code.value() != 0) [[unlikely]] {
            throw std::invalid_argument("Could not parse IP! " + std::to_string(error_code.value()));
        }

        _endpoint = udp::endpoint(dest_ip_address, dest_port);

        ip::address ip_address = ip::address::from_string("172.16.1.147", error_code);
        if (error_code.value() != 0) [[unlikely]] {
            throw std::invalid_argument("Could not parse default IP! " + std::to_string(error_code.value()));
        }

        _socket.open(udp::v4());
        _socket.bind(udp::endpoint(ip_address, static_cast<uint16_t>(SRV_PORT)));
    }

    [[nodiscard]] auto Run() -> std::future<InfoResponseParser> {

        send();

        _io_service.run();

        return _response_parser.get_future();
    }

    
private:
    void send() {
        std::vector<uint8_t> req = _req.MakeRequest();

        _socket.async_send_to(
            boost::asio::buffer(req, req.size()), _endpoint,
            boost::bind(&SourceSocket::after_send, this, baph::error));
    }

    void after_send(const boost::system::error_code& error_code) {
        
        if (error_code.value() != 0) [[unlikely]] {
            throw std::runtime_error("ErrorCode! Failed at afterSend: " + std::to_string(error_code.value()));
        }

        std::vector<uint8_t> recv_buffer(SourceConstants::gld_size_of_page);
        size_t bytes_transferred = _socket.receive_from(boost::asio::buffer(recv_buffer, recv_buffer.size()), _endpoint);

        if (recv_buffer[4] == SourceConstants::challenge_resp) {
            std::array<uint8_t, 4> challenge {};
            std::memcpy(challenge.data(), &recv_buffer[5], 4);
            _req.SetChallenge(challenge);

            send();

            return;
        }

        parse_response(std::move(recv_buffer), bytes_transferred);
    }

    void parse_response(std::vector<uint8_t>&& buf, size_t bytes_transferred) {

        // TODO: A lot of things happening without out of bounds checks.
        //       This will seg fault.
        split_packet_page_info page_info = {};
        if (buf[0] == SourceConstants::split_header) {
            if (_engine_type == SE_GOLD) {
                uint8_t page    = buf[SourceConstants::gld_page_info_byte];
                page_info.current_page      = page >> 4;
                page_info.amount_page       = page & 0b1111;
                page_info.size_page         = SourceConstants::gld_size_of_page;
                page_info.split_header_size = SourceConstants::gld_split_header_size;

            }
            else if (_engine_type == SE_SOURCE) {
                int32_t packet_id     = *reinterpret_cast<int32_t *>(&buf[4]);
                bool    is_compressed = (packet_id & (0b1 << 31)) != 0;

                if (is_compressed) {
                    // COPE
                    throw std::runtime_error("We don't deal with compressed packets!");
                }

                page_info.amount_page       = buf[SourceConstants::src_amount_page_info_byte];
                page_info.current_page      = buf[SourceConstants::src_current_page_info_byte];
                page_info.size_page         = *reinterpret_cast<int16_t *>(&buf[SourceConstants::src_size_page_info_byte]);
                page_info.split_header_size = SourceConstants::src_split_header_size;
            }
            else [[unlikely]] {
                throw std::runtime_error("Undefined engine type. Type: " + std::to_string(_engine_type));
            }

            buf = read_split_page(std::move(buf), bytes_transferred, page_info);
        }

        _response_parser.set_value(InfoResponseParser(std::move(buf), bytes_transferred, page_info));
    }

    auto read_split_page(std::vector<uint8_t>&& buf, size_t& bytes_transferred, split_packet_page_info page_info) -> std::vector<uint8_t>  {

        buf.reserve((page_info.amount_page - 1) * (page_info.size_page - page_info.split_header_size));

        // This is completely unecessary and is really
        // expensive, however it saves a bunch of hassle down
        // the line, I'd much rather deal with the header here.
        std::vector<uint8_t> bufWithHeader(page_info.size_page);

        for (uint8_t i = page_info.current_page + 1; i < page_info.amount_page; i++) {
            size_t curr_transferred = _socket.receive_from(boost::asio::buffer(bufWithHeader.begin().base(), page_info.size_page), _endpoint);

            const auto end_of_written = buf.begin() + bytes_transferred;
            const auto begin_of_recv  = bufWithHeader.begin() + page_info.split_header_size;
            const auto end_of_recv    = bufWithHeader.begin() + curr_transferred;
            buf.insert(end_of_written, begin_of_recv, end_of_recv);

            bytes_transferred += curr_transferred - page_info.split_header_size;
        }

        return buf;
    }

    source_engine_type               _engine_type;
    boost::asio::io_service          _io_service;
    udp::socket                      _socket;
    udp::endpoint                    _endpoint;
    TReqFactory                      _req;
    std::promise<InfoResponseParser> _response_parser;
};
}
