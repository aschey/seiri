#include "track_file_type.h"

#include <tstring.h>
#include <fileref.h>
#include <array>
#include <optional>
#include <memory>

class TrackData
{
private:
	std::shared_ptr<TagLib::FileRef> f;

public:
	TrackData(const char *track_path);
	virtual ~TrackData(){};
	const void Save();
	const enum track_file_type GetFileType();
	const TagLib::String GetTitle();
	const void SetTitle(const TagLib::String);
	const TagLib::String GetArtist();
	const void SetArtist(const TagLib::String);
	const TagLib::String GetAlbumArtists();
	const void SetAlbumArtists(const TagLib::String);
	const TagLib::String GetAlbum();
	const void SetAlbum(const TagLib::String);
	const TagLib::String GetMusicBrainzTrackId();
	const unsigned int GetYear();
	const unsigned int GetTrackNumber();
	const bool HasAlbumArt();
	const int GetBitrate();
	const int GetSampleRate();
	const unsigned int GetDiscNumber();
	const long long GetDuration();
	std::unique_ptr<TagLib::ByteVector> GetAlbumArtBytes();
};