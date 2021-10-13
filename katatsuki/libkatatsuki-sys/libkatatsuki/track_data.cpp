#include "track_data.h"
#include <tstring.h>
#include <fileref.h>
#include <tfile.h>
#include <tbytevector.h>

#include "TrackData.h"
#include <utility>
#include <optional>
#include <iostream>
#include <memory>
#include <cstring>

extern "C" const int get_file_type(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    return trackData->GetFileType();
}

extern "C" track_data *create_track_data(const char *track_path)
{
    auto trackData = new TrackData(track_path);
    return reinterpret_cast<track_data *>(trackData);
}

extern "C" void delete_track_data(track_data *track_data)
{
    delete reinterpret_cast<TrackData *>(track_data);
}

extern "C" void save(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    trackData->Save();
}

extern "C" const char *get_title(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    return strdup(trackData->GetTitle().to8Bit(true).c_str());
}

extern "C" void set_title(track_data *track_data, const char *title)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    trackData->SetTitle(TagLib::String(title, TagLib::String::UTF8));
}

extern "C" const char *get_artist(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    return strdup(trackData->GetArtist().to8Bit(true).c_str());
}

extern "C" void set_artist(track_data *track_data, const char *artist)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    trackData->SetArtist(TagLib::String(artist, TagLib::String::UTF8));
}

extern "C" const char *get_album_artists(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    return strdup(trackData->GetAlbumArtists().to8Bit(true).c_str());
}

extern "C" void set_album_artists(track_data *track_data, const char *album_artists)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    trackData->SetAlbumArtists(TagLib::String(album_artists, TagLib::String::UTF8));
}

extern "C" const char *get_album(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    return strdup(trackData->GetAlbum().to8Bit(true).c_str());
}

extern "C" void set_album(track_data *track_data, const char *album)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    trackData->SetAlbum(TagLib::String(album, TagLib::String::UTF8));
}

extern "C" const char *get_musicbrainz_track_id(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    return strdup(trackData->GetMusicBrainzTrackId().to8Bit(true).c_str());
}

extern "C" const unsigned int get_year(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    return trackData->GetYear();
}

extern "C" const unsigned int get_track_number(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    return trackData->GetTrackNumber();
}

extern "C" const unsigned int get_disc_number(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    return trackData->GetDiscNumber();
}

extern "C" const long long get_duration(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    return trackData->GetDuration();
}

extern "C" const int get_bitrate(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    return trackData->GetBitrate();
}

extern "C" const int get_sample_rate(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    return trackData->GetSampleRate();
}

extern "C" const art_bytes get_album_art_bytes(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    auto bytes = trackData->GetAlbumArtBytes();
    art_bytes ab;
    if (bytes)
    {
        unsigned char *copy = new unsigned char[bytes->size()];
        memcpy(copy, bytes->data(), bytes->size());
        ab.data = copy;
        ab.size = bytes->size();
        return ab;
    }
    return ab;
}

extern "C" const bool has_album_art(track_data *track_data)
{
    auto *trackData = reinterpret_cast<TrackData *>(track_data);
    return trackData->HasAlbumArt();
}

extern "C" void free_allocated_data(void *data)
{
    std::free(data);
}