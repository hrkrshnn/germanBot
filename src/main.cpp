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
                                       bot.getApi().sendMessage(message->chat->id, "Hi!");
                                     });

  bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message)
                               {
                                 std::cout<<"User wrote: "<<message<<"\n";
                                 if (StringTools::startsWith(message->text, "/start"))
                                   return;
                                 // bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text, false, message->messageId);
                                 auto output = langtool::grammarCheck(message->text);
                                 if(!output.empty())
                                   bot.getApi().sendMessage(message->chat->id, output, false, message->messageId);
                               });

  // TODO: Figure out how to deal with these signals
  signal(SIGINT, [](int s)
                 {
                   std::cerr<<"SIGINT caught\n";
                   std::exit(0);
                 });

  try
    {
      std::cout<<"Bot username: "<<bot.getApi().getMe()->username<<"\n";
      bot.getApi().deleteWebhook();

      TgBot::TgLongPoll longPoll(bot);
      while (true)
        {
          printf("Long poll started\n");
          longPoll.start();
        }
    }
  catch (std::exception& e)
    {
      std::cerr<<"Error: "<<e.what()<<"\n";
    }

  return 0;
}
