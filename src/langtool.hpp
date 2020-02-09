#ifndef LANGTOOL_HPP
#define LANGTOOL_HPP

#include <iostream>
#include <sstream>
#include <algorithm>

// #include <curl/curl.h>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "auth.hpp"


namespace langtool
{

  namespace pt = boost::property_tree;
  using namespace boost::asio;

  // request the langtool server, str should be + separated, e.g.,
  // Ich+habe+ein+Frage.

  // Return value: JSON string.
  std::string requestServer(const std::string& str)
  {
    try
      {
        // send request
        std::cout<<str<<"\n";
        ip::tcp::iostream request;
        request.connect(auth::host, auth::port);

        request << "GET " << "/v2/check?language=de-DE&text=" << str;
        request << " HTTP/1.1\r\n";
        request << "Connection: close\r\n\r\n";
        request.flush();

        std::string line;
        std::getline(request, line);

        std::stringstream response_stream(line);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);

        std::string header;
        std::vector<std::string> headers;
        while (std::getline(request, header) && header != "\r")
          headers.push_back(header);

        std::string jsonString;
        std::getline(request, jsonString);
        std::cout << "Output JSON: " << jsonString << "\n";

        return jsonString;
      }
    catch(std::exception& e)
      {
        std::cerr<<"Error: "<<e.what()<<"\n";
        return "";
      }

  }

  // TODO, perhaps this should be a reference to the message?
  std::string grammarCheck(std::string str = "Hallo")
  {
    // change spaces into plus for a proper HTTP request
    std::replace(str.begin(), str.end(), ' ', '+');

    std::stringstream ss;
    ss<<requestServer(str);

    std::string output;

    try
      {
        pt::ptree root;
        pt::read_json(ss, root);

        auto code = root.get<std::string>("language.detectedLanguage.code");
        std::cout<<"Code: "<<code<<"\n";
        if(code != "de-DE")
          return "";

        for(const auto& val: root.get_child("matches"))
          {
            auto message = val.second.get<std::string>("message");
            std::cout<<message<<"\n";

            auto text = val.second.get<std::string>("context.text");
            auto offset = val.second.get<std::size_t>("context.offset");
            auto length = val.second.get<std::size_t>("context.length");

            auto mistake = text.substr(offset, length);

            output += mistake + " -> ";


            for(const auto& replacements: val.second.get_child("replacements"))
              {
                auto val = replacements.second.get<std::string>("value");
                output += val;
                break;
              }

            output += "\n";
          }

        if(output.empty())
            std::cout<<output<<"\n";

        return output;
      }
    catch(std::exception& e)
      {
        std::cerr<<"Error "<<e.what()<<"\n";
        return "";
      }
  }


} // namespace langtool

#endif  // LANGTOOL_HPP

