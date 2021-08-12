#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <tgbot/tgbot.h>

namespace fs = std::filesystem;
using namespace std::chrono;

void concat(std::stringstream &ss) {}

template <typename T1, typename... T2> void concat(std::stringstream &ss, T1 arg1, T2... args)
{
    ss << arg1;
    concat(ss, args...);
}
template <typename... T> void print(T... args)
{
    std::stringstream ss;
    concat(ss, args...);
    std::cout << ss.str();
}

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

enum event_type
{
    send_hello,
    check_user
};

struct event_loop_event
{
    event_type type;
    int64_t chat_id;
    int64_t member_id;
    int64_t message_to_remove;
    system_clock::time_point event_time;
};

void event_loop_func(TgBot::Bot &bot, std::queue<event_loop_event> &q, std::mutex &mtx)
{
    while (1)
    {
        mtx.lock();
        if (q.empty())
        {
            mtx.unlock();
            std::this_thread::sleep_for(1s);
        }
        else
        {
            auto now = q.front();
            q.pop();
            mtx.unlock();
            std::this_thread::sleep_until(now.event_time);
            if (now.type == send_hello)
            {
                print("user ", now.member_id, " was welcomed in chat ", now.chat_id, '\n');
                const auto msg = bot.getApi().sendMessage(now.chat_id, "Дороу");
                mtx.lock();
                q.push({check_user, now.chat_id, now.member_id, msg->messageId,
                        high_resolution_clock::now() + 62s});
                mtx.unlock();
            }
            if (now.type == check_user)
            {
                const std::string status =
                    bot.getApi().getChatMember(now.chat_id, now.member_id)->status;
                if (status == "left" || status == "kicked")
                {
                    try
                    {
                        bot.getApi().deleteMessage(now.chat_id, now.message_to_remove);
                        print("welcome in chat ", now.chat_id, " removed\n");
                    }
                    catch (const std::exception &e)
                    {
                        print("welcome in chat ", now.chat_id, " was removed by some admin\n");
                    }
                }
            }
        }
    }
}

const int64_t IP_01_chat_id = -1001189961610;
const int64_t test_chat_id = -1001250428136;
const int64_t FICT_talk_chat_id = -1001494680687;

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
    std::mutex mtx;

    // event loop
    std::queue<event_loop_event> events;
    std::thread event_loop(event_loop_func, std::ref(bot), std::ref(events), std::ref(mtx));

    bot.getEvents().onAnyMessage(
        [&bot, &pathes, &events, &mtx](TgBot::Message::Ptr message)
        {
            if (message->chat->id == FICT_talk_chat_id)
            {
                print("User ", message->from->username, " in chat type ", message->chat->title,
                      " wrote ", message->text, '\n');
                if (message->newChatMember)
                {
                    print("user ", message->from->id, " joined chat ", message->chat->title, '(',
                          message->chat->id, ")\n");
                    std::lock_guard lg{mtx};

                    events.push({send_hello, message->chat->id, message->from->id, 0,
                                 high_resolution_clock::now() + 2s});
                }
            }
        });

    // petrovich command
    bot.getEvents().onCommand(
        "petrovich",
        [&bot, &pathes /*, &opened, &ths*/](TgBot::Message::Ptr message)
        {
            static size_t idx = 0;
            // std::cout << "received photo requst\n";
            print("petrovich idx: ", idx, " time: ", message->date, " chat: ", message->chat->id,
                  " sender: ", message->from->username, '\n');
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