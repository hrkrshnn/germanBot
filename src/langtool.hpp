#ifndef LANGTOOL_HPP
#define LANGTOOL_HPP

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>


// #include <curl/curl.h>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/locale.hpp>

#include "auth.hpp"


namespace langtool
{

  namespace pt = boost::property_tree;
  using namespace boost::asio;

  // From https://stackoverflow.com/questions/154536/encode-decode-urls-in-c
  // Encodes a string into a url format.
  std::string url_encode(const std::string &value)
  {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
      std::string::value_type c = (*i);

      // Keep alphanumeric and other accepted characters intact
      if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
        escaped << c;
        continue;
      }

      // Any other characters are percent-encoded
      escaped << std::uppercase;
      escaped << '%' << std::setw(2) << int((unsigned char) c);
      escaped << std::nouppercase;
    }

    return escaped.str();
  }

  // Return value: JSON string.
  std::string requestServer(const std::string& str)
  {
    try
      {
        // send request
        std::cout << str << "\n";
        ip::tcp::iostream request;
        request.connect(auth::host, auth::port);

        std::string encodedUrl = url_encode(str);
        std::cout << "Encoded: " << encodedUrl << "\n";

        request << "GET " << "/v2/check?language=de-DE&text=" << encodedUrl;
        request << " Content-Type: text/html; charset=utf-8\r\n";
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
        std::cerr << "Error: " << e.what() << "\n";
        return "";
      }

  }


  std::string grammarCheck(const std::string str = "Hallo")
  {
    std::stringstream ss;
    auto json = requestServer(str);
    ss << json;

    std::string output;

    try
      {
        pt::ptree root;
        pt::read_json(ss, root);

        // If langtool thinks the text isn't in german, then no corrections will
        // be provided.
        auto code = root.get<std::string>("language.detectedLanguage.code");
        std::cout << "Language code: " << code << "\n";
        if(code != "de-DE")
          return "";

        for(const auto& val: root.get_child("matches"))
          {
            auto text = val.second.get<std::string>("context.text");
            auto offset = val.second.get<std::size_t>("context.offset");
            auto length = val.second.get<std::size_t>("context.length");

            std::u32string part = boost::locale::conv::utf_to_utf<char32_t>(text);
            auto mistake = boost::locale::conv::utf_to_utf<char>(part.substr(offset, length));

            output += mistake + " -> ";

            for(const auto& replacements: val.second.get_child("replacements"))
              {
                auto val = replacements.second.get<std::string>("value");
                output += val + "/";
              }

            // The last / is replaced by a fullstop.
            output.back() = '.';

            output += "\n";
          }

        if(!output.empty())
          std::cout << "Bot reply: " << output << "\n";

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

