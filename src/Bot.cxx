#include "pch.hpp"
#include "Bot.hpp"

using namespace mi0::bot;

Bot::Bot(const std::string &TOKEN,
                   std::shared_ptr<mi0::sync::ChannelsVector> channels,
                   std::shared_ptr<mi0::sync::ServerManagerTimer> mngr)
    : _mngr(std::move(mngr)), _channels(std::move(channels)),
      _bot(TOKEN, dpp::i_default_intents | dpp::i_privileged_intents) {

    namespace ph = std::placeholders;

    _bot.on_log(dpp::utility::cout_logger());

    _bot.on_slashcommand([this](auto &&command) {
        on_command_handle(std::forward<decltype(command)>(command));
    });

    _bot.on_ready([this](auto &&ready_event) {
        on_ready_handle(std::forward<decltype(ready_event)>(ready_event));
    });

    _bot.start(dpp::st_return);
}

void Bot::on_ready_handle(const dpp::ready_t & /*unused*/) {
    if (dpp::run_once<struct register_bot_commands>()) {
        _bot.global_command_create(
            dpp::slashcommand("ping", "I'm not sure whether description matters", _bot.me.id)
                .add_option(dpp::command_option(dpp::co_user, "user", "the nyega", true)));

        _bot.global_command_create(
            dpp::slashcommand("bind", "Bind this channel to the server bot so you can get server updates.", _bot.me.id)
                .add_option(dpp::command_option(dpp::co_string, "serverip", "the ip of the server", true))
                .add_option(dpp::command_option(dpp::co_string, "serverport", "the port of the server", true)));
    }
}

void Bot::on_command_handle(const dpp::slashcommand_t &event) {
    if (event.command.get_command_name() == "ping") {
        auto user = std::get<dpp::snowflake>(event.get_parameter("user"));

        event.reply(dpp::message(event.command.channel_id, std::string("<@") + std::to_string(user) + "> yo SUP NYEGA"));
    } else if (event.command.get_command_name() == "bind") {
        auto ip           = std::get<std::string>(event.get_parameter("serverip"));
        auto port         = stoi(std::get<std::string>(event.get_parameter("serverport")));
        auto channel_info = mi0::sync::ChannelsVector::channel_details(ip, port, event.command.channel_id);

        _channels->push_back(std::move(channel_info));
        _mngr->EarlyWake();

        event.reply(dpp::message(event.command.channel_id, "Stuff seems to be happening"));
    }
// TODO: Find a better reply to the initial message
}

