#ifndef STICKER_HPP
#define STICKER_HPP

#include <string>

class sticker
{
    public:
        std::string id;
        std::string url;
        std::string title;
        std::string rating;

        sticker()
        {
            id = url = title = rating = "";
        }

        sticker(std::string _id, std::string _url, std::string _title, std::string _rating)
        {
            id = _id;
            url = _url;
            title = _title;
            rating = _rating;
        }

        std::string to_string()
        {
            return "\nsticker with title: " + title + "\nurl: " +
            url + "\nrating: " + rating + "\nid: " + id +"\n"; 
        }
};

struct get_request_params
{
    public:
        std::string api_key;
        std::string search_phrase;
        int limit;

        std::string get_request_string()
        {
            if (limit > 50 || limit <= 0)
            {
                limit = 1;
            }
            return "?api_key=" + api_key + "&q=" + search_phrase + "&limit=" + std::to_string(limit);
        }
};

#endif