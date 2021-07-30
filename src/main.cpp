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

const int64_t IP_01_chat_id = -1001189961610;
const int64_t test_chat_id = -1001250428136;

int main()
{
    std::srand(unsigned(std::time(0)));
    // get api key
    std::ifstream key_reader("petrovich_private_keys/key");
    std::string api_key;
    key_reader >> api_key;
    TgBot::Bot bot(api_key);

    // get files for petrovich command
    std::vector<std::string> pathes;
    read_directory("photos", pathes);
    std::random_shuffle(pathes.begin(), pathes.end());

    // for counting chat members
    std::map<int64_t, int32_t> member_count;

    bot.getEvents().onAnyMessage(
        [&bot, &pathes, &member_count](TgBot::Message::Ptr message)
        {
            if (message->chat->id == test_chat_id)
            {
                std::cout << "User " << message->from->username << " in chat type "
                          << message->chat->title << " wrote " << message->text << '\n';
                // std::cout
                //     << bot.getApi().getChatMember(message->chat->id, message->from->id)->status
                //     << '\n';
                int32_t real_chat_member_count =
                    bot.getApi().getChatMembersCount(message->chat->id);
                if (auto it = member_count.find(message->chat->id);
                    it != member_count.end() && it->second < real_chat_member_count)
                {
                    bot.getApi().sendMessage(message->chat->id, "Дороу");
                }

                member_count[message->chat->id] = real_chat_member_count;
            }
        });

    // petrovich command
    bot.getEvents().onCommand(
        "petrovich",
        [&bot, &pathes /*, &opened, &ths*/](TgBot::Message::Ptr message)
        {
            static size_t idx = 0;
            // std::cout << "received photo requst\n";
            std::cout << "petrovich idx: " << idx << " time: " << message->date
                      << " chat: " << message->chat->id << " sender: " << message->from->username
                      << '\n';
            if (message->chat->id == IP_01_chat_id)
            {
                std::cout << "Doesn't work in IP-01\n";
                return;
            }
            auto msg = bot.getApi().sendPhoto(
                message->chat->id,
                TgBot::InputFile::fromFile(pathes[(idx++) % pathes.size()], "image/jpeg"));
        });

    // infinite main loop
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