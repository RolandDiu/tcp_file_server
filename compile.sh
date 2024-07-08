gcc -o server main.c server.c file_handler.c client_handler.c -lpthread

if [ $? -eq 0 ]; then
    echo "Compilation successful. Run './server <port>' to start the server."
else
    echo "Compilation failed."
fi
