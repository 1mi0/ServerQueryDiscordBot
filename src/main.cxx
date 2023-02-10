#include "pch.hpp"
#include "Bot.hpp"
#include "impl/SourceSockets.hpp"
#include "Secret.hpp"
#include "sql/Sql.hpp"

class ServerManager {
public:
    ServerManager(const std::string& token) noexcept
        : _channels(std::make_shared<ChannelsVector>()), _mngr(std::make_shared<ServerManagerTimer>()) {
        
        // Bot starts running
        _bot = std::make_unique<mi0::bot::Bot>(token, _channels, _mngr);

        const auto channel_for_each = [this](const ChannelsVector::channel_details& details) {
            try { 
                // SourceSocket handles the network stuff and just returns
                // a future InfoResponseParser -- that's the response which
                // is used to parse the info retreived from the SourceSocket
                mi0::srv::SourceSocket sock(details.ip, details.port, mi0::srv::SourceRequestFactory(mi0::srv::SR_A2S_INFO), mi0::srv::SE_GOLD);
                auto res = sock.Run().get();
                sendEmbed(details, std::move(res));
            }
            catch (std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        };

        do {
            // this loop gets executed once every minute
            _channels->for_each_channel(channel_for_each);
            _mngr->Tick();
        } while (!_mngr->IsDead());
    }

private:
    void sendEmbed(const ChannelsVector::channel_details& details, mi0::srv::InfoResponseParser pr) {
        this->_bot->_bot.message_create(
            dpp::message(details.channel, "")
                .add_embed(dpp::embed()
                    .set_author(pr.name, "https://github.com/1mi0/ServerQueryDiscordBot/", "https://icons.iconarchive.com/icons/franksouza183/fs/256/apps-counter-strike-icon.png")
                    .set_image(               std::string("https://image.gametracker.com/images/maps/160x120/cs/") + pr.map + ".jpg")
                    .add_field("players",     std::to_string((int32_t)pr.players) + "(" + std::to_string((int32_t)(pr.players - pr.bots)) + ")/" + std::to_string((int32_t)pr.max_players), true)
                    .add_field("map",         pr.map, true)
                    .add_field("game",        pr.game, true)
                    .add_field("ip",          details.ip, true)
                    .add_field("port",        std::to_string(details.port), true)
                    .add_field("connect url", std::string("steam://connect/") + details.ip + ":" + std::to_string(details.port), true)));
    }
// TODO: move embed to the server specific stuff

    std::shared_ptr<ChannelsVector>     _channels;
    std::shared_ptr<ServerManagerTimer> _mngr;
    std::unique_ptr<mi0::bot::Bot>      _bot;
};


auto main() -> int {
    //ServerManager sm(BOT_TOKEN);
    SQL sql;
    auto shit = sql.GetActiveServers();
    pqxx::result asd;
    std::cout << shit[0]["address"] << ":" << shit[0]["port"] << std::endl;
}
