//! # Katatsuki
//!
//! `katatsuki` wraps [taglib2](https://taglib.org/) to allow safe access to
//! the metadata of various music files.

use libkatatsuki_sys as sys;

use std::ffi::{CStr, CString, NulError};
use std::io::{Error, ErrorKind, Result};
use std::os::raw::{c_char, c_void};
use std::path::Path;
use std::slice::from_raw_parts;

use chrono::Local;
use imagesize::blob_size;
pub use num_traits::{FromPrimitive, ToPrimitive};

pub use track::ReadOnlyTrack;
pub use track::ReadWriteTrack;
pub use track::TrackFileType;

mod track;

fn c_str_to_str(c_str: *const c_char) -> Option<String> {
    if c_str.is_null() {
        return None;
    }

    let bytes = unsafe { CStr::from_ptr(c_str).to_bytes() };
    let result = if bytes.is_empty() {
        None
    } else {
        Some(String::from_utf8_lossy(bytes).to_string())
    };

    unsafe {
        sys::free_allocated_data(c_str as *mut c_void);
    }

    result
}

#[derive(Debug)]
struct TrackData {
    raw: *mut sys::track_data,
}

/// Unsafe backing
impl TrackData {
    // Dangerous access here, path not existing is UB.
    /// Do not use `TrackData::new`, instead use `Track::from_path` to ensure
    /// safe access.
    pub fn new(path: &CString) -> Self {
        unsafe {
            let path_ptr = path.to_owned().into_raw();
            let track_data = TrackData {
                raw: sys::create_track_data(path_ptr),
            };
            let _ = CString::from_raw(path_ptr);

            track_data
        }
    }

    pub fn save(&self) {
        unsafe {
            sys::save(self.raw);
        }
    }

    pub fn title(&self) -> String {
        c_str_to_str(unsafe { sys::get_title(self.raw) }).unwrap_or_else(|| "".to_owned())
    }

    pub fn set_title(&self, title: &str) {
        let c_str = CString::new(title).unwrap();
        let ptr = c_str.as_ptr();

        unsafe {
            sys::set_title(self.raw, ptr);
        }
    }

    pub fn artist(&self) -> String {
        c_str_to_str(unsafe { sys::get_artist(self.raw) }).unwrap_or_else(|| "".to_owned())
    }

    pub fn set_artist(&self, artist: &str) {
        let c_str = CString::new(artist).unwrap();
        let ptr = c_str.as_ptr();

        unsafe {
            sys::set_artist(self.raw, ptr);
        }
    }

    pub fn album(&self) -> String {
        c_str_to_str(unsafe { sys::get_album(self.raw) }).unwrap_or_else(|| "".to_owned())
    }

    pub fn set_album(&self, album: &str) {
        let c_str = CString::new(album).unwrap();
        let ptr = c_str.as_ptr();

        unsafe {
            sys::set_album(self.raw, ptr);
        }
    }

    pub fn album_artists(&self) -> String {
        c_str_to_str(unsafe { sys::get_album_artists(self.raw) }).unwrap_or_else(|| "".to_owned())
    }

    pub fn set_album_artists(&self, album_artists: &str) {
        let c_str = CString::new(album_artists).unwrap();
        let ptr = c_str.as_ptr();

        unsafe {
            sys::set_album_artists(self.raw, ptr);
        }
    }

    pub fn musicbrainz_track_id(&self) -> Option<String> {
        c_str_to_str(unsafe { sys::get_musicbrainz_track_id(self.raw) })
    }

    pub fn year(&self) -> u32 {
        unsafe { sys::get_year(self.raw) }
    }

    pub fn track_number(&self) -> u32 {
        unsafe { sys::get_track_number(self.raw) }
    }

    pub fn bitrate(&self) -> i32 {
        unsafe { sys::get_bitrate(self.raw) }
    }

    pub fn disc_number(&self) -> u32 {
        unsafe { sys::get_disc_number(self.raw) }
    }

    pub fn duration(&self) -> i64 {
        unsafe { sys::get_duration(self.raw) }
    }

    pub fn sample_rate(&self) -> i32 {
        unsafe { sys::get_sample_rate(self.raw) }
    }

    pub fn file_type(&self) -> TrackFileType {
        let file_type = unsafe { sys::get_file_type(self.raw) };
        TrackFileType::from_u32(file_type as u32).unwrap()
    }

    pub fn has_front_cover(&self) -> bool {
        unsafe { sys::has_album_art(self.raw) }
    }

    pub unsafe fn cover_bytes(&self) -> CoverBytes {
        let val = sys::get_album_art_bytes(self.raw);
        CoverBytes {
            raw: val.data,
            size: val.size as i32,
        }
    }
}

struct CoverBytes {
    raw: *const u8,
    size: i32,
}

impl Drop for CoverBytes {
    fn drop(&mut self) {
        unsafe { sys::free_allocated_data(self.raw as *mut c_void) }
    }
}

impl Drop for TrackData {
    fn drop(&mut self) {
        unsafe {
            sys::delete_track_data(self.raw);
            self.raw.drop_in_place();
        }
    }
}

#[derive(Debug)]
pub enum FileError {
    OpenFailure,
    SaveFailure,
    PathAsString,
    NullPathString(NulError),
    InvalidTagFile,
}

fn get_path(path: &Path) -> Result<CString> {
    if !path.exists() {
        Err(Error::new(
            ErrorKind::NotFound,
            format!("File {:?} not found.", path),
        ))
    } else if let Ok(path_ptr) = path
        .to_owned()
        .to_str()
        .ok_or(FileError::PathAsString)
        .and_then(|path| CString::new(path).map_err(FileError::NullPathString))
    {
        Ok(path_ptr)
    } else {
        Err(Error::new(
            ErrorKind::UnexpectedEof,
            "Path was invalid.".to_string(),
        ))
    }
}

impl ReadWriteTrack {
    pub fn from_path(path: &Path, source: Option<&str>) -> Result<Self> {
        let path_ptr = get_path(path)?;
        let mut track_data: TrackData = TrackData::new(&path_ptr);
        let track = ReadOnlyTrack::from_track_data(path, &mut track_data, source)?;
        drop(path_ptr);
        Ok(Self {
            data: track,
            track_data,
        })
    }
}

impl ReadOnlyTrack {
    fn from_track_data(
        path: &Path,
        track_data: &mut TrackData,
        source: Option<&str>,
    ) -> Result<Self> {
        if let TrackFileType::Unknown = track_data.file_type() {
            Err(Error::new(
                ErrorKind::InvalidData,
                format!("File {:?} is unsupported", path),
            ))
        } else {
            let mut fcw = 0;
            let mut fch = 0;
            let mut art: Vec<u8> = Vec::new();
            if track_data.has_front_cover() {
                let bytes = unsafe { track_data.cover_bytes() };
                let slice = unsafe { from_raw_parts(bytes.raw, bytes.size as usize) };
                if let Ok(size) = blob_size(slice) {
                    fcw = size.width as i32;
                    fch = size.height as i32;
                    art = slice.to_owned();
                    // println!("Width: {}, Height: {}", fcw, fch);
                }
                // } else {
                //     println!("Cover but unreadable");
                // }
                // } else {
                //     println!("No front cover");
                // }
            }

            let track = Ok(ReadOnlyTrack {
                file_path: path.to_owned(),
                file_type: track_data.file_type(),
                title: track_data.title().trim().to_string(),
                artist: track_data.artist().trim().to_string(),
                album: track_data.album().trim().to_string(),
                album_artists: track_data.album_artists().trim().to_string(),
                year: track_data.year() as i32,
                track_number: track_data.track_number() as i32,
                musicbrainz_track_id: track_data.musicbrainz_track_id(),
                has_front_cover: track_data.has_front_cover(),
                front_cover_width: fcw,
                front_cover_height: fch,
                bitrate: track_data.bitrate(),
                sample_rate: track_data.sample_rate(),
                source: source.unwrap_or("None").to_owned(),
                disc_number: track_data.disc_number() as i32,
                duration: track_data.duration() as i32,
                updated: Local::now().format("%Y-%m-%d").to_string(),
                album_art: art,
            });

            track
        }
    }
    pub fn from_path(path: &Path, source: Option<&str>) -> Result<Self> {
        let path_ptr = get_path(path)?;
        let mut track_data: TrackData = TrackData::new(&path_ptr);
        let track = Self::from_track_data(path, &mut track_data, source);
        drop(path_ptr);
        track
    }
}
