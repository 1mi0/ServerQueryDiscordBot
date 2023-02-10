#pragma once

#include "../pch.hpp"

namespace mi0::srv {

enum [[nodiscard]] source_engine_type : uint8_t {
    SE_GOLD,
    SE_SOURCE
};

enum [[nodiscard]] server_type : uint8_t {
    IRP_ST_DEDICATED    = 'd',
    IRP_ST_NONDEDICATED = 'l',
    IRP_ST_PROXY        = 'p'
};

enum [[nodiscard]] environment_type : uint8_t {
    IRP_ET_LINUX     = 'l',
    IRP_ET_WINDOWS   = 'w',
    IRP_ET_MACOS_OLD = 'm',
    IRP_ET_MACOS_NEW = 'o',
    IRP_ET_MAC       = IRP_ET_MACOS_OLD | IRP_ET_MACOS_NEW
};

enum [[nodiscard]] visibility_type : uint8_t {
    IRP_VT_PUBLIC  = 0,
    IRP_VT_PRIVATE = 1
};

enum [[nodiscard]] vac_type : uint8_t {
    IRP_VACT_UNSECURED = 0,
    IRP_VACT_SECURED   = 1
};

enum [[nodiscard]] edf_type : uint8_t {
    IRP_EDF_PORT     = 0x80,
    IRP_EDF_STEAMID  = 0x10,
    IRP_EDF_SOURCETV = 0x40,
    IRP_EDF_KEYWORDS = 0x20,
    IRP_EDF_GAMEID   = 0x01
};

enum [[nodiscard]] src_req_type : uint8_t {
    SR_A2S_INFO   = 0x54,
    SR_A2S_PLAYER = 0x55,
    SR_A2S_RULES  = 0x56
};

struct [[nodiscard]] split_packet_page_info {
    uint8_t current_page;
    uint8_t amount_page;
    uint8_t split_header_size;
    ssize_t size_page;
};

struct SourceConstants {
    // Header types:
    static const uint8_t default_header;
    static const ssize_t default_header_size;
    static const uint8_t split_header;

    static const uint8_t challenge_resp;

    static const ssize_t gld_split_header_size;
    static const int32_t gld_size_of_page;
    static const ssize_t gld_page_info_byte;

    static const ssize_t src_split_header_size;
    static const ssize_t src_amount_page_info_byte;
    static const ssize_t src_current_page_info_byte;
    static const ssize_t src_size_page_info_byte;
};
};
