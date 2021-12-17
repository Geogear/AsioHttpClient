#include <iostream>
#include <fstream>
#include "../lib/functions.hpp"

using asio::ip::tcp;

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
    /* grp.limit = 20;
    grp.search_phrase = "sword"; */
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

    /* Form the request. We specify the "Connection: close" header so that the
    server will close the socket after transmitting the response. This will
    allow us to treat all data up until the EOF as the content. */
    asio::streambuf request;
    std::ostream request_stream(&request);

    /* Read the response status line. The response streambuf will automatically
    grow to accommodate the entire line. The growth may be limited by passing
    a maximum size to the streambuf constructor. */
    asio::streambuf response;
        
    std::istream response_stream(&response);
    std::string http_version;   
    unsigned int status_code;  
    std::string status_message;
  
    std::string header;  

    std::vector<std::vector<sticker>> all_stickers;
    
    std::cout << "\n-------Welcome to Giphy Sticker Getter Client-------\n";
    int selection = -1;
    while(1){
        std::cout << "\nEnter the specified number to select an option:\n" << "1.Enter a search term to get stickers.\n"
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
                std::cout << "Enter search term: ";
                std::cin >> grp.search_phrase;
                std::cout << "Enter search limit (between 0 and 50, if invalid client assumes 1 for you.): ";
                std::cin >> grp.limit;

                asio::connect(socket, endpoints);

                set_request_stream(&request_stream, grp, path, server);
    
                /* Send the request. */
                asio::write(socket, request);

                asio::read_until(socket, response, "\r\n");

                /* Check that response is OK. */
                response_stream >> http_version;
                response_stream >> status_code;
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
                while (std::getline(response_stream, header) && header != "\r");                   

                /* Consume whatever content we already have to output. */
                response.consume(response.size());

                all_stickers.push_back(pull_stickers_from_response(&socket, &response));

                socket.close();
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
}