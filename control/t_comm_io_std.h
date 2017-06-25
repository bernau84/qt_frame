#ifndef T_VI_COMM_STD_TERMINAL
#define T_VI_COMM_STD_TERMINAL

#include "t_comm_parser_string.h"
#include "i_comm_io_generic.h"
#include <iostream>
#include <thread>
#include <string>
#include <condition_variable>
#include <chrono>
#include <mutex>
#include <queue>

class t_comm_stdte : public i_comm_generic {

    Q_OBJECT

private:
    std::mutex mu;
    std::atomic_bool running;
    std::thread reader;
    std::string readed;

public:
    virtual void on_read(QByteArray &dt){

        std::lock_guard<std::mutex> tl(mu);  //odemkne po ukonceni platnosti promenne, RAII principy
        if(!readed.empty())
        {
            dt.append(QString::fromStdString(readed));
            readed.clear();
        }
    }

    virtual void on_write(const QByteArray &dt){

        std::cout << dt.toStdString();
    }

    t_comm_stdte (i_comm_parser *parser):
        i_comm_generic(parser),
        running(true),
        reader([&]()
            {
                while(running/* && std::cin*/)
                {
                    std::string s;
                    std::cin >> s;

                    std::unique_lock<std::mutex> tl(mu);
                    readed.append(s);
                    readed.append("\n");
                    mu.unlock();
                }

                std::cout << "reader finished!";
            })
    {

    }

    ~t_comm_stdte (){

        running = false;
        reader.join();
    }
};

#endif // T_VI_COMM_STD_TERMINAL

