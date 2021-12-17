#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <asio.hpp>

using asio::ip::tcp;

class sticker
{
    public:
        std::string id;
        std::string url;
        std::string title;
        std::string rating;

        sticker(std::string _id, std::string _url, std::string _title, std::string _rating)
        {
            id = _id;
            url = _url;
            title = _title;
            rating = _rating;
        }

        std::string to_string()
        {
            return "sticker with title: " + title + "\nurl: " +
            url + "\nrating: " + rating + "\nid: " + id +"\n"; 
        }
};

std::string get_request_string(std::string api_key, std::string search_phrase, int limit);
std::vector<sticker> pull_stickers_from_response(tcp::socket* socket, asio::streambuf* response);
void find_and_add_stickers(char *response_data, std::vector<sticker>* vec);
int count_till_char(const char* string, char until);

int main(int argc, char **argv)
{
    /* Api key file, is a text file where on the first line api key exists. */
    if (argc != 2)
    {
        std::cout << "Usage: sticker_client <path_to_api_key_file>\n";
        return 1;
    }

    /* Needed paths and names. */
    std::string server = "api.giphy.com";
    std::string path = "/v1/stickers/search";
    std::string api_key_path = argv[1];

    /* Open and file and check if it exists. */
    std::ifstream my_file;
    my_file.open(api_key_path);
    if (!my_file.is_open())
    {
        std::cout << "File doesn't exist.\n";
        return 1;
    }

    /* Read the api_key. */
    std::string api_key;
    my_file >> api_key;

    /* Close the file, don't need it anymore. */
    my_file.close();

    asio::io_context io_context;

    /* Get a list of endpoints corresponding to the server name. */
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(server, "http");

    /* Try each endpoint until we successfully establish a connection. */
    tcp::socket socket(io_context);
    asio::connect(socket, endpoints);

    /* Form the request. We specify the "Connection: close" header so that the
    server will close the socket after transmitting the response. This will
    allow us to treat all data up until the EOF as the content. */
    asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << path << get_request_string(api_key, "cheese", 1) << " HTTP/1.0\r\n";
    request_stream << "Host: " << server << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";  
    
    /* Send the request. */
    asio::write(socket, request);

    /* Read the response status line. The response streambuf will automatically
    grow to accommodate the entire line. The growth may be limited by passing
    a maximum size to the streambuf constructor. */
    asio::streambuf response;
    asio::read_until(socket, response, "\r\n");

    /* Check that response is OK. */
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
      std::cout << "Invalid response\n";
      return 1;
    }
    if (status_code != 200)
    {
      std::cout << "Response returned with status code " << status_code << status_message << "\n";     
      return 1;
    }

    /* Read the response headers, which are terminated by a blank line. */
    asio::read_until(socket, response, "\r\n\r\n");

    /* Process the response headers. */
    std::string header;
    while (std::getline(response_stream, header) && header != "\r")
      std::cout << header << "\n";
    std::cout << "\n";

    /* Write whatever content we already have to output. */
    if (response.size() > 0)
      std::cout << &response;

    /* Read until EOF, writing data to output as we go. */
    /*asio::error_code error;
    while (asio::read(socket, response, asio::transfer_at_least(1), error))
    {
        std::cout << &response;
    }*/
    std::vector<std::vector<sticker>> all_stickers;
    all_stickers.push_back(pull_stickers_from_response(&socket, &response));

    for(size_t i = 0; i < all_stickers[0].size(); ++i)
    {
        std::cout << all_stickers[0][i].to_string();
    }

    socket.close();
}

std::string get_request_string(std::string api_key, std::string search_phrase, int limit)
{
    if (limit > 50 || limit < 0)
    {
        limit = 1;
    }
    return "?api_key=" + api_key + "&q=" + search_phrase + "&limit=" + std::to_string(limit);
}

std::vector<sticker> pull_stickers_from_response(tcp::socket* socket, asio::streambuf* response)
{
    std::vector<sticker> vec;   
    
    asio::error_code error;
    while (asio::read(*socket, *response, asio::transfer_at_least(1), error))
    {        
        asio::streambuf::const_buffers_type data = response->data();
        std::string current(buffers_begin(data), buffers_begin(data) + data.size());
        response->consume(response->size());
        find_and_add_stickers(&current[0], &vec);
    }
    return vec;         
}

void find_and_add_stickers(char *response_data, std::vector<sticker>* vec)
{
    std::string frontier_search_term = "type\":\"sticker\"";
    std::string search_terms[4] = {"id\":", "url\":", "title\":", "rating\":"};
    char *found = NULL;
    const char count_till = '\"';
    
    std::vector<std::string> properties(4);

    const short term_lengths[4] = {5, 6, 8, 9};
    const short property_count = 4;   
    int len = 0, i = 0;

    found = strstr(response_data, &frontier_search_term[0]); 
    while(found != NULL)
    {
        found = strstr(found, &search_terms[i][0]);
        len = count_till_char(found+term_lengths[i], count_till);
        if (len == -1)
        {
            return;
        }
        properties[i].append(found+term_lengths[i], len);
        if(++i == 4)
        {
            i = 0;
            vec->push_back(sticker(properties[0], properties[1], properties[2], properties[3]));
            for(int j = 0; j < property_count; ++j)
            {
                properties[j].clear();
            }
            found = strstr(found, &frontier_search_term[0]);
        }
    } 
}

int count_till_char(const char* string, char until)
{
    int count = 0;
    for (; ; ++count)
    {
        /* Unexpected situation. */
        if (string[count] == '\0' && until != '\0')
        {
            return -1;
        }
        if (string[count] == until)
        {
            break;
        }
    }
    return count;
}