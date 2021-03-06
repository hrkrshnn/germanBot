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

  decltype(TgBot::Chat::id) chatId = 459480249;

  std::string welcomeString("Herzlich willkommen! You can write in this chat and"
 " I'll reply with all the spelling and grammatical mistakes that I found with"
 " suggestions for replacements. The project is licensed under GPL-v3 and can be"
 " found at https://github.com/hrkrshnn/germanBot");

  bot.getEvents().onCommand("start", [&bot, &welcomeString](TgBot::Message::Ptr message)
                                     {
                                       try
                                         {
                                           bot.getApi().sendMessage(message->chat->id, welcomeString);
                                         }
                                       catch(std::exception& e)
                                         {
                                           std::cerr << "Error in sending\n" << e.what() << "\n";
                                         }
                                     });

  bot.getEvents().onAnyMessage([&bot, &chatId](TgBot::Message::Ptr message)
                               {
                                 std::cout << message->from->firstName << " wrote: "
                                           << message->text << "\n";

                                 if (StringTools::startsWith(message->text, "/start"))
                                   return;

                                 auto output = langtool::grammarCheck(message->text);

                                 bot.getApi().sendMessage(chatId, "Message: " + message->text, false);

                                 if(!output.empty())
                                   {
                                     try
                                       {
                                         bot.getApi().sendMessage(message->chat->id, output, false, message->messageId);
                                         bot.getApi().sendMessage(chatId, "Bot: " + output, false);
                                       }
                                     catch (std::exception& e)
                                       {
                                         std::cerr << "Error in sending\n" << e.what() << "\n";
                                       }
                                   }
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
