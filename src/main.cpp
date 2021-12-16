#include <iostream>
#include <fstream>
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

    
}