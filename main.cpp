#include <iostream>
#include "./Socket.h"
#include <semaphore>
// goal here that every class you want to impliment this has 
// to inherite from the class then you can either override or 
// impliement what you need your server to do
int main() {
    constexpr int MAX_CLIENTS = 10;
    mysocket::socket server;
    server.listen("127.0.0.1",8080);  // make sure your server is listening on some port

    // A counting semaphore initialized to MAX_CLIENTS.
    std::counting_semaphore<MAX_CLIENTS> sem(MAX_CLIENTS);

    while (server.isActive()) {
        // Wait until there's "room" for one more client
        sem.acquire();

        // Block until a new client connects
        mysocket::socket client = server.accept();
        
        // Launch a thread to handle this client
        std::thread([&server, client = std::move(client), &sem]() mutable {
            try {
                std::string text = client.receive();
                std::cout << "we have this from the server: " << text << "\n";
                server.handle(text);
                client.send("HTTP/1.1 200 OK\r\n");
            }
            catch (const std::exception& e) {
                std::cerr << "Handler error: " << e.what() << "\n";
            }
            catch (...) {
                std::cerr << "Unknown handler error\n";
            }
            client.close();

            // Signal that this slot is now free
            sem.release();
        })
        .detach();
    }

    return 0;
}
