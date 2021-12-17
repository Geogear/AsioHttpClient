#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <vector>
#include <asio.hpp>
#include <string>
#include "sticker.hpp"

using asio::ip::tcp;

std::vector<sticker> pull_stickers_from_response(tcp::socket* socket, asio::streambuf* response);
void find_and_add_stickers(char *response_data, std::vector<sticker>* vec);
int count_till_char(const char* string, char until);
void set_request_stream(std::ostream* request_stream, struct get_request_params grp, std::string path, std::string server);
void display_pages(std::vector<std::vector<sticker>>* all_stickers);
void display_sticker_page(std::vector<sticker> *stickers);

#endif