#include "TrackData.h"
#include "StringUtils.h"
#include "track_file_type.h"

#include <tpicture.h>
#include <tpicturemap.h>
#include <flacfile.h>
#include <aifffile.h>
#include <apefile.h>
#include <mpegfile.h>
#include <mp4file.h>
#include <oggfile.h>
#include <vorbisfile.h>
#include <opusfile.h>
#include <oggflacfile.h>
#include <tstring.h>
#include <xiphcomment.h>
#include <flacpicture.h>
#include <tlist.h>

#include <array>
#include <optional>
#include <iostream>
#include <utility>
#include <memory>

using namespace std;

TrackData::TrackData(const char *track_path)
{
  TagLib::String path(track_path, TagLib::String::UTF8);

#ifdef _WIN32
  f = make_shared<TagLib::FileRef>(TagLib::FileName(path.toWString().data()), true, TagLib::AudioProperties::Accurate);
#else
  f = make_shared<TagLib::FileRef>(TagLib::FileName(path.to8Bit(true).data()), true, TagLib::AudioProperties::Accurate);
#endif
}

const void TrackData::Save()
{
  f->file()->save();
}

const TagLib::String TrackData::GetTitle()
{
  return f->tag()->title();
}

const void TrackData::SetTitle(const TagLib::String title)
{
  f->tag()->setTitle(title);
}

const TagLib::String TrackData::GetArtist()
{
  return f->tag()->artist();
}

const void TrackData::SetArtist(const TagLib::String artist)
{
  f->tag()->setArtist(artist);
}

const unsigned int TrackData::GetYear()
{
  return f->tag()->year();
}

unique_ptr<TagLib::ByteVector> TrackData::GetAlbumArtBytes()
{
  if (auto flacFile = dynamic_cast<TagLib::FLAC::File *>(f->file()))
  {
    const TagLib::List<TagLib::FLAC::Picture *> pictureList = flacFile->pictureList();
    // Prefer FrontCover.
    for (const TagLib::FLAC::Picture *picture : pictureList)
    {
      if (picture->type() == TagLib::FLAC::Picture::FrontCover)
      {
        return make_unique<TagLib::ByteVector>(std::move(picture->data()));
      }
    }

    // Settle for Other.
    for (const TagLib::FLAC::Picture *picture : pictureList)
    {
      if (picture->type() == TagLib::FLAC::Picture::Other)
      {
        return make_unique<TagLib::ByteVector>(std::move(picture->data()));
      }
    }
  }

  // OGG Files tag()->pictures() is unimplemented in taglib2.
  // Use the legacy method of xiphComment picture list then.

  if (auto xiphComment = dynamic_cast<TagLib::Ogg::XiphComment *>(f->file()->tag()))
  {
    const TagLib::List<TagLib::FLAC::Picture *> pictureList = xiphComment->pictureList();
    for (const TagLib::FLAC::Picture *picture : pictureList)
    {
      if (picture->type() == TagLib::FLAC::Picture::FrontCover)
      {
        return make_unique<TagLib::ByteVector>(std::move(picture->data()));
      }
    }

    for (const TagLib::FLAC::Picture *picture : pictureList)
    {
      if (picture->type() == TagLib::FLAC::Picture::Other)
      {
        return make_unique<TagLib::ByteVector>(std::move(picture->data()));
      }
    }
  }

  auto pictureMap = f->tag()->pictures();
  // Prefer FrontCover, but settle for other.
  if (pictureMap.contains(TagLib::Picture::Type::FrontCover))
  {
    auto picture = pictureMap[TagLib::Picture::Type::FrontCover].front();
    return make_unique<TagLib::ByteVector>(std::move(picture.data()));
  }
  else if (pictureMap.contains(TagLib::Picture::Type::Other))
  {
    auto picture = pictureMap[TagLib::Picture::Type::Other].front();
    return make_unique<TagLib::ByteVector>(std::move(picture.data()));
  }
  return nullptr;
}

const unsigned int TrackData::GetTrackNumber()
{
  return f->tag()->track();
}

const bool TrackData::HasAlbumArt()
{
  return GetAlbumArtBytes() != nullptr;
}

const int TrackData::GetBitrate()
{
  return f->audioProperties()->bitrate();
}

const int TrackData::GetSampleRate()
{
  return f->audioProperties()->sampleRate();
}

const unsigned int TrackData::GetDiscNumber()
{
  try
  {
    if (!f->tag()->properties()["DISCNUMBER"].isEmpty())
    {
      TagLib::String discNumber = f->tag()->properties()["DISCNUMBER"].front();
      return static_cast<unsigned int>(stoul(discNumber.to8Bit()));
    }
  }
  catch (std::exception)
  {
    return 1;
  }
  return 1;
}

const long long TrackData::GetDuration()
{
  return f->audioProperties()->lengthInMilliseconds();
}

const enum track_file_type TrackData::GetFileType()
{

  if (auto mp3File = dynamic_cast<TagLib::MPEG::File *>(f->file()))
  {
    // https://github.com/mono/taglib-sharp/blob/b1155885656c9625c2cc6d928b9329e2a5206048/src/TagLib/Mpeg/AudioHeader.cs#L519
    // Mimics taglib-sharp behaviour, even though VBR files may not necessarily have a xing header.
    if (mp3File->audioProperties()->xingHeader())
    {
      return track_file_type::MP3VBR;
    }
    return track_file_type::MP3CBR;
  }
  if (auto mp4File = dynamic_cast<TagLib::MP4::File *>(f->file()))
  {
    switch (mp4File->audioProperties()->codec())
    {
    case TagLib::MP4::AudioProperties::Codec::AAC:
      return track_file_type::AAC;
    case TagLib::MP4::AudioProperties::Unknown:
      return track_file_type::Unknown;
    case TagLib::MP4::AudioProperties::Codec::ALAC:
      int bitDepth = mp4File->audioProperties()->bitsPerSample();
      return get_alac_type(bitDepth);
    }
  }
  if (auto flacFile = dynamic_cast<TagLib::FLAC::File *>(f->file()))
  {
    int bitDepth = flacFile->audioProperties()->bitsPerSample();
    return get_flac_type(bitDepth);
  }
  if (auto oggFlacFile = dynamic_cast<TagLib::Ogg::FLAC::File *>(f->file()))
  {
    int bitDepth = oggFlacFile->audioProperties()->bitsPerSample();
    return get_flac_type(bitDepth);
  }
  if (auto aiffFile = dynamic_cast<TagLib::RIFF::AIFF::File *>(this->f->file()))
  {
    int bitDepth = aiffFile->audioProperties()->bitsPerSample();
    return get_aiff_type(bitDepth);
  }
  if (auto apeFile = dynamic_cast<TagLib::APE::File *>(f->file()))
  {
    int bitDepth = apeFile->audioProperties()->bitsPerSample();
    return get_monkeys_audio_type(bitDepth);
  }
  if (dynamic_cast<TagLib::Ogg::Vorbis::File *>(f->file()))
  {
    return track_file_type::Vorbis;
  }
  if (dynamic_cast<TagLib::Ogg::Opus::File *>(f->file()))
  {
    return track_file_type::Opus;
  }
  return track_file_type::Unknown;
}

const TagLib::String TrackData::GetAlbum()
{
  return f->tag()->album();
}

const void TrackData::SetAlbum(const TagLib::String album)
{
  f->tag()->setAlbum(album);
}

const TagLib::String TrackData::GetAlbumArtists()
{
  if (!f->tag()->properties()["ALBUMARTIST"].isEmpty())
  {
    return join(f->tag()->properties()["ALBUMARTIST"], ";");
  }
  return TagLib::String();
}

const void TrackData::SetAlbumArtists(const TagLib::String albumArtists)
{
  auto albumArtistList = albumArtists.split(";");
  auto props = f->tag()->properties();
  props.replace("ALBUMARTIST", albumArtistList);
  f->tag()->setProperties(props);
}

const TagLib::String TrackData::GetMusicBrainzTrackId()
{
  if (!f->tag()->properties()["MUSICBRAINZ_TRACKID"].isEmpty())
  {
    return f->tag()->properties()["MUSICBRAINZ_TRACKID"].front();
  }
  return TagLib::String();
}
