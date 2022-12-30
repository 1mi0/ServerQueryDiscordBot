#include "pch.hpp"
#include "Bot.hpp"
#include "UDPQuery.hpp"
#include "Secret.hpp"

class ServerManager {
public:
    ServerManager(std::string token) noexcept
        : _channels(std::make_shared<ChannelsVector>()), _mngr(std::make_shared<ServerManagerTimer>()) {
        _bot = std::make_unique<mi0::bot::Bot>(token, _channels, _mngr);
        // Bot running at this point
        do {
            _channels->for_each_channel([this](const ChannelsVector::channel_details& details) {
                boost::asio::io_service io_service;
                try {
                    mi0::srv::SourceRequest req(io_service, 
                                        "172.16.1.147", static_cast<uint16_t>(12543),
                                        details.ip, details.port);

                    req.Send(mi0::srv::SR_A2S_INFO);
                    io_service.run();
                    auto res = std::get<std::shared_ptr<mi0::srv::InfoResponseParser>>(req.response_parser);
                    sendEmbed(details, res);
                }
                catch (std::invalid_argument& e) {
                    std::cout << e.what() << std::endl;
                }
            });
            _mngr->Tick();
        } while (!_mngr->IsDead());
    }

private:
    void sendEmbed(const ChannelsVector::channel_details details, std::shared_ptr<mi0::srv::InfoResponseParser> pr) {
        this->_bot->_bot.message_create(
            dpp::message(details.channel, "")
                .add_embed(dpp::embed()
                    .set_author(pr->name, "https://github.com/1mi0/discordserverquery/", "https://icons.iconarchive.com/icons/franksouza183/fs/256/apps-counter-strike-icon.png")
                    .set_image((std::ostringstream() << "https://image.gametracker.com/images/maps/160x120/cs/" << pr->map << ".jpg").str())
                    .add_field("players", (std::ostringstream() << (int32_t)pr->players << "(" << (int32_t)(pr->players - pr->bots) << ")/" << (int32_t)pr->max_players).str(), true)
                    .add_field("map", (std::ostringstream() << pr->map).str(), true)
                    .add_field("game", (std::ostringstream() << pr->game).str(), true)
                    .add_field("ip", (std::ostringstream() << details.ip).str() , true)
                    .add_field("port", (std::ostringstream() << details.port).str(), true)
                    .add_field("connect url", (std::ostringstream() << "steam://connect/" << details.ip << ":" << details.port).str(), true)));
    }

private:
    std::shared_ptr<ChannelsVector>     _channels;
    std::shared_ptr<ServerManagerTimer> _mngr;
    std::unique_ptr<mi0::bot::Bot>      _bot;
};

int main() {
    ServerManager sm(BOT_TOKEN);
}
