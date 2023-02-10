#include "SourceConstants.hpp"

using namespace mi0::srv;

const uint8_t SourceConstants::default_header             = 0xff;
const ssize_t SourceConstants::default_header_size        = 4;
const uint8_t SourceConstants::split_header               = 0xfe;

const uint8_t SourceConstants::challenge_resp             = 0x41;

const ssize_t SourceConstants::gld_split_header_size      = 9;
const int32_t SourceConstants::gld_size_of_page           = 1400;
const ssize_t SourceConstants::gld_page_info_byte         = 8;

const ssize_t SourceConstants::src_split_header_size      = 12;
const ssize_t SourceConstants::src_amount_page_info_byte  = 8;
const ssize_t SourceConstants::src_current_page_info_byte = 9;
const ssize_t SourceConstants::src_size_page_info_byte    = 10;
