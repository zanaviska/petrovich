#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <set>
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
    const bool operator<(const event_loop_event &rhs) const { return event_time < rhs.event_time; }
};

void event_loop_func(TgBot::Bot &bot, std::set<event_loop_event> &q, std::mutex &mtx)
{
#ifdef NDEBUG
    auto remove_message_seconds = 62s;
#else
    auto remove_message_seconds = 17s;
#endif
    while (1)
    {
        std::this_thread::sleep_for(1s);
        while (1)
        {
            event_loop_event top;
            {
                std::lock_guard lg{mtx};
                if (q.empty()) break;
                top = *q.begin();
                if (top.event_time > high_resolution_clock::now()) break;
                q.erase(q.begin());
            }
            if (top.type == send_hello)
            {
                print("user ", top.member_id, " was welcomed in chat ", top.chat_id, '\n');
                const auto msg = bot.getApi().sendMessage(top.chat_id, "Дороу");

                // lock mutex another time to add new element to queue
                std::lock_guard lg{mtx};
                q.insert({check_user, top.chat_id, top.member_id, msg->messageId,
                          high_resolution_clock::now() + remove_message_seconds});
            }
            if (top.type == check_user)
            {
                const std::string status =
                    bot.getApi().getChatMember(top.chat_id, top.member_id)->status;
                if (status == "left" || status == "kicked")
                {
                    try
                    {
                        bot.getApi().deleteMessage(top.chat_id, top.message_to_remove);
                        print("welcome in chat ", top.chat_id, " removed\n");
                    }
                    catch (const std::exception &e)
                    {
                        print("welcome in chat ", top.chat_id, " was removed by some admin\n");
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
#ifdef NDEBUG
    std::ifstream key_reader("petrovich_private_keys/pocket_thanos_bot");
#else
    std::ifstream key_reader("petrovich_private_keys/nona_test_bot");
#endif
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
    std::set<event_loop_event> events;
    // std::queue<event_loop_event> events;
    std::thread event_loop(event_loop_func, std::ref(bot), std::ref(events), std::ref(mtx));

    bot.getEvents().onAnyMessage(
        [&bot, &pathes, &events, &mtx](TgBot::Message::Ptr message)
        {
#ifdef NDEBUG
            if (message->chat->id == FICT_talk_chat_id)
#endif
            {
                auto end = std::chrono::system_clock::now();
                std::time_t end_time = std::chrono::system_clock::to_time_t(end);
                print("User ", message->from->username, " in chat type ", message->chat->title,
                      " wrote ", message->text, ' ', std::ctime(&end_time));
                if (message->newChatMember)
                {
                    print("user ", message->from->id, " joined chat ", message->chat->title, '(',
                          message->chat->id, ")\n");
                    std::lock_guard lg{mtx};

                    events.insert({send_hello, message->chat->id, message->from->id, 0,
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
            std::cout << "main error\n";
        }
    }
}