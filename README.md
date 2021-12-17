# Http Client To Get Stickers From Giphy.com
Done as a testcase for the recruitment of a company. Used c++ with asio library. Built with gcc 6.3.0 using mingw on windows 10.

## Usage
.\sticker_client.exe <path-to-api-key-file>, api-key-file must be a text file containing only the api-key on the first line.

## Known Issues
Program is error prone and and single threaded. This forces it to open and close a new connection at each request. And for requests with the same search query, it would pull the same data most of the time as long as the limit is the same.