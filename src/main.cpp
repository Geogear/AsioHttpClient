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

std::vector<sticker> pull_stickers_from_response(tcp::socket* socket, asio::streambuf* response);
void find_and_add_stickers(char *response_data, std::vector<sticker>* vec);
int count_till_char(const char* string, char until);
void set_request_stream(std::ostream* request_stream, struct get_request_params grp, std::string path, std::string server);
void display_pages(std::vector<std::vector<sticker>>* all_stickers);
void display_sticker_page(std::vector<sticker> *stickers);

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

    struct get_request_params grp;
    grp.limit = 20;
    grp.search_phrase = "sword";
    /* Read the api_key. */
    my_file >> grp.api_key;

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
    set_request_stream(&request_stream, grp, path, server);
    
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

    std::vector<std::vector<sticker>> all_stickers;
    all_stickers.push_back(pull_stickers_from_response(&socket, &response));

    for(size_t i = 0; i < all_stickers[0].size(); ++i)
    {
        std::cout << all_stickers[0][i].to_string();
    }

    std::cout << "Welcome to Giphy Sticker Getter Client\n";
    int selection = -1;
    while(1){
        std::cout << "Enter the specified number to select an option:\n" << "1.Enter a search term to get stickers.\n"
        << "2.See existing pages\n" << "0.Exit client\n";
        std::cin >> selection;

        if(selection == 0)
        {
            std::cout << "Bye bye.\n";
            break;
        }

        switch(selection)
        {
            case 1:
                break;
            case 2:
                display_pages(&all_stickers);
                break;
            default:
                std::cout << "Enter a valid option.\n";
                continue;
                break;
        }
    }

    socket.close();
}

std::vector<sticker> pull_stickers_from_response(tcp::socket* socket, asio::streambuf* response)
{
    std::string frontier_search_term = "type\":\"sticker\"";
    std::vector<sticker> vec;
    /* It is known that only one sticker data can exist, in one streambuf read. Sometimes not even that is the case,
    when that happens we have to split the nex streambuf read and append*/
    const size_t minimum_whole_data = 1000;
    char* place_holder = NULL;
    
    asio::error_code error;
    while (asio::read(*socket, *response, asio::transfer_at_least(1), error))
    {               
        /* Pull data from streambuf. */
        asio::streambuf::const_buffers_type data = response->data();
        /* Convert it to string. */ 
        std::string current(buffers_begin(data), buffers_begin(data) + data.size());
        /* Empty the streambuf. */    
        response->consume(response->size());

        /* Find the starting of sticker data, if it exists in the current context. */   
        place_holder = strstr(&current[0], &frontier_search_term[0]);

        if(place_holder != NULL)
        {
            /* If data is large enough, it is safe to objectify it. */   
            if(strlen(place_holder) >= minimum_whole_data){
                find_and_add_stickers(place_holder, &vec);
                continue;
            }

            /* If not large enough, read again. */   
            if(0 == asio::read(*socket, *response, asio::transfer_at_least(1), error))
            {
                break;
            }
            data = response->data();
            size_t size_to_read = (data.size() > minimum_whole_data) ? minimum_whole_data : data.size();
            std::string tmp(buffers_begin(data), buffers_begin(data) + size_to_read);
            response->consume(response->size());
            current.append(tmp);    
            place_holder = strstr(&current[0], &frontier_search_term[0]);        
            find_and_add_stickers(place_holder, &vec);
        }    
    }
    return vec;         
}

void find_and_add_stickers(char *response_data, std::vector<sticker>* vec)
{
    std::string search_terms[4] = {"id\":", "url\":", "title\":", "rating\":"};
    std::vector<std::string> properties(4);
    char *found = response_data;
    const char count_till = '\"';

    const short term_lengths[4] = {5, 6, 8, 9};
    int len = 0;

    for(int i = 0; i < 4; ++i)
    {
        found = strstr(found, &search_terms[i][0]);
        len = count_till_char(found+term_lengths[i], count_till);
        if (len == -1)
        {
            return;
        }
        properties[i].append(found+term_lengths[i], len);
    }

    vec->push_back(sticker(properties[0], properties[1], properties[2], properties[3]));
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

void set_request_stream(std::ostream* request_stream, struct get_request_params grp, std::string path, std::string server)
{
    request_stream->clear();
    (*request_stream) << "GET " << path << grp.get_request_string() << " HTTP/1.0\r\n";
    (*request_stream) << "Host: " << server << "\r\n";
    (*request_stream) << "Accept: */*\r\n";
    (*request_stream) << "Connection: close\r\n\r\n";  
}


void display_pages(std::vector<std::vector<sticker>>* all_stickers)
{
    if (all_stickers->size() == 0)
    {
        std::cout << "No pages exist.\n";
        return;
    }

    int current_page = 1, selection = 1;
    while(1)
    {
        if(selection == 0)
        {
            break;
        }

        switch(selection)
        {
            case 1:
                --current_page;
                std::cout << "DISPLAYING PAGE NUMBER " + std::to_string(current_page+1) + "\n";
                display_sticker_page(&((*all_stickers)[current_page]));
                break;
            case 2:
                ++current_page;
                std::cout << "DISPLAYING PAGE NUMBER " + std::to_string(current_page+1) + "\n";
                display_sticker_page(&((*all_stickers)[current_page]));
                break;
            default:
                std::cout << "Enter a valid option.\n";
                break;
        }

        std::cout << "Enter the specified number to select an option:\n";
        if(current_page > 0)
        {
            std::cout << "1.Show the previous page.\n";
        }
        if(current_page < all_stickers->size()-1)
        {
            std::cout << "2.Show the next page.\n";
        }
        std::cout << "0.Exit page viewing\n";
        std::cin >> selection;
    }
}

void display_sticker_page(std::vector<sticker> *stickers)
{
    for(int i = 0; i < stickers->size(); ++i)
    {
        (*stickers)[i].to_string();
    }
}