#include <iostream>
#include <csignal>
#include <cstdio>
#include <exception>

#include <tgbot/tgbot.h>

#include "auth.hpp"
#include "langtool.hpp"

int main()
{
  TgBot::Bot bot(auth::token);

  bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message)
                                     {
                                       bot.getApi().sendMessage(message->chat->id, "Wilkommen!");
                                     });

  bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message)
                               {
                                 std::cout<<"User wrote: "<<message->text<<"\n";
                                 if (StringTools::startsWith(message->text, "/start"))
                                   return;

                                 auto output = langtool::grammarCheck(message->text);
                                 if(!output.empty())
                                   bot.getApi().sendMessage(message->chat->id, output, false, message->messageId);
                               });

  signal(SIGINT, [](int signum)
                 {
                   std::cerr << "SIGINT caught\n";
                   std::exit(signum);
                 });

  try
    {
      std::cout << "Bot username: " << bot.getApi().getMe()->username << "\n";
      bot.getApi().deleteWebhook();

      TgBot::TgLongPoll longPoll(bot);
      while (true)
        {
          std::cout << "Long Polling.\n";
          longPoll.start();
        }
    }
  catch (std::exception& e)
    {
      std::cerr << "Error: " << e.what() << "\n";
    }

  return 0;
}
