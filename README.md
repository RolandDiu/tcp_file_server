# File Transfer Server
Multi-threaded server, which allows clients to connect, exchange messages and transfer files. Works on Linux only.

### Compilation
`bash compile.sh`
or
`gcc -o server main.c server.c file_handler.c client_handler.c -lpthread`

### Running
To start the server: `./server <port>`
To connect as client: `telnet [localhost/127.0.0.1/::1] <port>`

### File Commands
`@show`: Display available files on the server

`@get <filename>`: Download a file from the other user folder, who can be on the other server ('S'+port number+':'+name+directory) (for example: @get S1500:john/example.txt)

`@put <filename>`: Upload a file to the user folder
