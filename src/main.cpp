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
    request_stream << "GET " << path << "?api_key=" << api_key << "&q=cheese&limit=1" << " HTTP/1.0\r\n";
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

    // Read until EOF, writing data to output as we go.
    asio::error_code error;
    while (asio::read(socket, response, asio::transfer_at_least(1), error))
    {
        std::cout << &response;
    }
      
    socket.close();
}