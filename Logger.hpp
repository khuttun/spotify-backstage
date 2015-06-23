#ifndef SPOTIFY_BACKSTAGE_LOGGER_HPP
#define SPOTIFY_BACKSTAGE_LOGGER_HPP

#include <iostream>
#include <thread>

#define LOG(msg) std::cout << msg << "|" << __FILE__ << ":" << __LINE__ << "|" << std::this_thread::get_id() << std::endl

#endif
