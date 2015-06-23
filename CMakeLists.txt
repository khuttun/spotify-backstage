cmake_minimum_required(VERSION 3.1)

project(spotify-backstage)

set(CMAKE_CXX_FLAGS "-std=c++11 -pedantic -Wall -Wextra -Weffc++ -g")

set(src
	AudioDevice.cpp
	Equalizer.cpp
	FilterBank.cpp
	IirFilter.cpp
	SoundSystem.cpp
	SpotifyBackstage.cpp
	SpotifySession.cpp
)

add_library(spotify-backstage ${src})
