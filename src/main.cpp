#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <tgbot/tgbot.h>

namespace fs = std::filesystem;

// random generator function:
int myrandom(int i)
{
    return std::rand() % i;
}

struct path_leaf_string
{
    std::string operator()(const fs::directory_entry &entry) const { return entry.path().string(); }
};

void read_directory(const std::string &name, std::vector<std::string> &v)
{
    fs::path p(name);

    fs::directory_iterator start(p);
    fs::directory_iterator end;
    std::transform(start, end, std::back_inserter(v), path_leaf_string());
}

int main()
{
    std::srand(unsigned(std::time(0)));
    // get api key
    std::ifstream key_reader("petrovich_private_keys/key");
    std::string api_key;
    key_reader >> api_key;
    TgBot::Bot bot(api_key);

    // bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
    //     //printf("User wrote %s\n", message->text.c_str());
    //     //std::cout << "User " << message->from->username << " in chat type " <<
    //     message->chat->title << " wrote " << message->text.c_str() << '\n'; if
    //     (StringTools::startsWith(message->text, "/start")) {
    //         return;
    //     }
    //     std::cout << message->text << '\n';
    //     //bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
    // });

    // get files for petrovich command
    std::vector<std::string> pathes;

    read_directory("photos", pathes);
    std::random_shuffle(pathes.begin(), pathes.end());
    bot.getEvents().onCommand(
        "petrovich",
        [&bot, &pathes /*, &opened, &ths*/](TgBot::Message::Ptr message)
        {
            static size_t idx = 0;
            // std::cout << "received photo requst\n";
            std::cout << "petrovich idx: " << idx << " time: " << message->date
                      << " chat: " << message->chat->id << " sender: " << message->from->username
                      << '\n';
            if (message->chat->id == -1001189961610)
            {
                std::cout << "Doesn't work in IP-01\n";
                return;
            }
            auto msg = bot.getApi().sendPhoto(
                message->chat->id,
                TgBot::InputFile::fromFile(pathes[(idx++) % pathes.size()], "image/jpeg"));
            // if(!opened[message->chat->id])
            // {
            //     opened[message->chat->id] = true;
            //     ths.push_back(std::thread{send_petrovich,std::ref(bot), message->chat->id,
            //     message->chat->username, true}); std::cout << "out";
            // }
        });
    while (1)
    {

        try
        {
            printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
            TgBot::TgLongPoll longPoll(bot);
            while (true)
            {
                // printf("Long poll started\n");
                longPoll.start();
            }
        }
        catch (TgBot::TgException &e)
        {
            std::cout << "AAAAAA\n";
            printf("error: %s\n", e.what());
        }
        catch (...)
        {
        }
    }
}