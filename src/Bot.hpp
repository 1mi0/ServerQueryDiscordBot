#include "pch.hpp"
#include "ChannelsVector.hpp"
#include <sstream>

namespace mi0::bot {

class Bot {
public:
    Bot(const std::string& TOKEN, std::shared_ptr<ChannelsVector> channels, std::shared_ptr<ServerManagerTimer> mngr) 
        : _mngr(mngr), _channels(channels), _bot(TOKEN, dpp::i_default_intents | dpp::i_privileged_intents) {

        namespace ph = std::placeholders;

        _bot.on_log(dpp::utility::cout_logger());
        _bot.on_slashcommand(std::bind(&Bot::on_command_handle, this, ph::_1));
        _bot.on_ready(std::bind(&Bot::on_ready_handle, this, ph::_1));

        _bot.start(dpp::st_return);
    }

private:
    void on_ready_handle(const dpp::ready_t&) {
        if (dpp::run_once<struct register_bot_commands>()) {
            _bot.global_command_create(
                dpp::slashcommand("ping", "I'm not sure whether description matters", _bot.me.id)
                    .add_option(dpp::command_option(dpp::co_user, "user", "the nyega", true))
            );
            _bot.global_command_create(
                dpp::slashcommand("bind", "Bind this channel to the server bot so you can get server updates.", _bot.me.id)
                    .add_option(dpp::command_option(dpp::co_string, "serverip", "the ip of the server", true))
                    .add_option(dpp::command_option(dpp::co_string, "serverport", "the port of the server", true))
            );
        }
    }

    void on_command_handle(const dpp::slashcommand_t& event) {
        if (event.command.get_command_name() == "ping") {
            auto user = std::get<dpp::snowflake>(event.get_parameter("user"));
            event.reply(dpp::message(event.command.channel_id, (std::ostringstream() << "<@"<< user << "> yo SUP NYEGA" ).str()));
        }
        else if (event.command.get_command_name() == "bind") {
            auto ip    = std::get<std::string>(event.get_parameter("serverip"));
            auto port  = stoi(std::get<std::string>(event.get_parameter("serverport")));
            auto chinf = ChannelsVector::channel_details(ip, port, event.command.channel_id);

            _channels->push_back(std::move(chinf));
            _mngr->EarlyWake();
            event.reply(dpp::message(event.command.channel_id, "Stuff seems to be happening"));
        }
    }

public:
    std::shared_ptr<ServerManagerTimer> _mngr;
    std::shared_ptr<ChannelsVector>     _channels;
    dpp::cluster                        _bot;
};

}
