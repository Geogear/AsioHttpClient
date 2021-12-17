#include <iostream>
#include "../lib/functions.hpp"

using asio::ip::tcp;

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
        std::cout << "\nNo pages exist.\n";
        return;
    }

    size_t current_page = 1, selection = 1;
    while(1)
    {
        if(selection == 0)
        {
            break;
        }

        if((selection == 1 && current_page == 0) ||
        (selection == 2 && current_page == all_stickers->size()-1))
        {
            selection = -1;
        }

        switch(selection)
        {
            case 1:
                --current_page;
                display_sticker_page(&all_stickers->at(current_page));
                std::cout << "\nDISPLAYING PAGE NUMBER " + std::to_string(current_page+1) + "\n";               
                break;
            case 2:
                ++current_page;
                display_sticker_page(&all_stickers->at(current_page));
                std::cout << "\nDISPLAYING PAGE NUMBER " + std::to_string(current_page+1) + "\n";               
                break;
            default:
                std::cout << "\nEnter a valid option.\n";
                break;
        }

        std::cout << "\nEnter the specified number to select an option:\n";
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
    for(size_t i = 0; i < stickers->size(); ++i)
    {
        std::cout << stickers->at(i).to_string();
    }
}