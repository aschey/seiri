/* automatically generated by rust-bindgen */

pub const __bool_true_false_are_defined: ::std::os::raw::c_uint = 1;
pub const false_: ::std::os::raw::c_uint = 0;
pub const true_: ::std::os::raw::c_uint = 1;
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct track_data {
    _unused: [u8; 0],
}
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct art_bytes {
    pub data: *const u8,
    pub size: u32,
}
extern "C" {
    pub fn create_track_data(track_path: *const ::std::os::raw::c_char) -> *mut track_data;
}
extern "C" {
    pub fn delete_track_data(track_path: *mut track_data);
}
extern "C" {
    pub fn free_allocated_data(data: *mut ::std::os::raw::c_void);
}
extern "C" {
    pub fn save(track_path: *mut track_data);
}
extern "C" {
    pub fn get_title(track_path: *mut track_data) -> *const ::std::os::raw::c_char;
}
extern "C" {
    pub fn set_title(track_path: *mut track_data, title: *const ::std::os::raw::c_char);
}
extern "C" {
    pub fn get_artist(track_data: *mut track_data) -> *const ::std::os::raw::c_char;
}
extern "C" {
    pub fn set_artist(track_path: *mut track_data, artist: *const ::std::os::raw::c_char);
}
extern "C" {
    pub fn get_album_artists(track_data: *mut track_data) -> *const ::std::os::raw::c_char;
}
extern "C" {
    pub fn set_album_artists(
        track_path: *mut track_data,
        album_artists: *const ::std::os::raw::c_char,
    );
}
extern "C" {
    pub fn get_album(track_data: *mut track_data) -> *const ::std::os::raw::c_char;
}
extern "C" {
    pub fn set_album(track_path: *mut track_data, album: *const ::std::os::raw::c_char);
}
extern "C" {
    pub fn get_musicbrainz_track_id(track_data: *mut track_data) -> *const ::std::os::raw::c_char;
}
extern "C" {
    pub fn get_year(track_data: *mut track_data) -> ::std::os::raw::c_uint;
}
extern "C" {
    pub fn get_track_number(track_data: *mut track_data) -> ::std::os::raw::c_uint;
}
extern "C" {
    pub fn get_disc_number(track_data: *mut track_data) -> ::std::os::raw::c_uint;
}
extern "C" {
    pub fn get_duration(track_data: *mut track_data) -> ::std::os::raw::c_longlong;
}
extern "C" {
    pub fn get_bitrate(track_data: *mut track_data) -> ::std::os::raw::c_int;
}
extern "C" {
    pub fn get_sample_rate(track_data: *mut track_data) -> ::std::os::raw::c_int;
}
extern "C" {
    pub fn get_album_art_bytes(track_data: *mut track_data) -> art_bytes;
}
extern "C" {
    pub fn get_file_type(track_data: *mut track_data) -> ::std::os::raw::c_int;
}
extern "C" {
    pub fn has_album_art(track_data: *mut track_data) -> bool;
}
