# spotify-backstage

spotify-backstage is a C++ library for playing music from [Spotify](https://www.spotify.com/). It's based on [libspotify](https://developer.spotify.com/technologies/libspotify/).

spotify-backstage implements two new features that the official Spotify client doesn't have:

- Equalizer. spotify-backstage includes a simple three channel equalizer (in class `Equalizer`).

- Selecting Output Device. spotify-backstage supports changing the audio output device.

## API

The users of spotify-backstage should include the header SpotifyBackstage.hpp and instantiate the `SpotifyBackstage` class. This opens up the Spotify connection and initializes the audio device for playback.

## Dependencies

spotify-backstage has some dependencies to external libraries:

- [libspotify](https://developer.spotify.com/technologies/libspotify/). The integration to Spotify is implemented with libspotify. To use libspotify (and therefore, to use spotify-backstage), you need [Spotify Premium account](https://www.spotify.com/premium/). You also need a [Spotify application key](https://devaccount.spotify.com/my-account/keys/). To build spotify-backstage, input your application key to `Appkey.hpp`.

- [PortAudio](http://www.portaudio.com/). spotify-backstage audio playback is implemented with PortAudio.

- [PolyM](https://github.com/khuttun/PolyM). spotify-backstage thread communication is implemented with PolyM.

- [boost](https://github.com/boostorg). spotify-backstage passes audio data to sound driver through `boost::lockfree::spsc_queue`. boost/lockfree/spsc_queue.hpp is the only header spotify-backstage includes from boost.