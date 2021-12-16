#include <iostream>
#include <string>
#include <asio.hpp>

using asio::ip::tcp;

int main(int argc, char **argv)
{
    /* Api key file, is a text file where on the first line api key exists. */
    if (argc != 2)
    {
        std::cout << "Usage: sticker_client <path_to_api_key_file>\n";
        return 1;
    }

    std::string server = "api.giphy.com";
    std::string path = "/v1/stickers/search";
    std::string api_key_path = argv[1];
}