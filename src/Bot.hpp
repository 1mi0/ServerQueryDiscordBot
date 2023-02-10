#pragma once

#include "pch.hpp"
#include "ChannelsVector.hpp"

namespace mi0::bot {

class Bot {
public:
    Bot(const std::string &TOKEN, std::shared_ptr<ChannelsVector> channels, std::shared_ptr<ServerManagerTimer> mngr);

private:
    void on_ready_handle(const dpp::ready_t &);

    void on_command_handle(const dpp::slashcommand_t &event);

  public:
    std::shared_ptr<ServerManagerTimer> _mngr;
    std::shared_ptr<ChannelsVector>     _channels;
    dpp::cluster                        _bot;
};

}
