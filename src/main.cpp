#include <iostream>
#include <fstream>
#include <string>

#include <tgbot/tgbot.h>

int main()
{
    //get api key
    std::ifstream key_reader("petrovich_private_keys/key");
    std::string api_key;
    key_reader >> api_key;
    TgBot::Bot bot(api_key);
    
    // bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
    //     //printf("User wrote %s\n", message->text.c_str());
    //     //std::cout << "User " << message->from->username << " in chat type " << message->chat->title << " wrote " << message->text.c_str() << '\n';
    //     if (StringTools::startsWith(message->text, "/start")) {
    //         return;
    //     }
    //     std::cout << message->text << '\n';
    //     //bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
    // });

    //petrovuch command
    bot.getEvents().onCommand("petrovich", [&bot/*, &opened, &ths*/](TgBot::Message::Ptr message)
    {
        std::cout << "received photo requst\n";
        // if(!opened[message->chat->id])
        // {
        //     opened[message->chat->id] = true;
        //     ths.push_back(std::thread{send_petrovich,std::ref(bot), message->chat->id, message->chat->username, true});
        //     std::cout << "out";
        // }
    });
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        std::cout << "AAAAAA\n";
        printf("error: %s\n", e.what());
    }
}