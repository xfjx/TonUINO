#ifndef SRC_MP3_HPP_
#define SRC_MP3_HPP_

#include <Arduino.h>
#include <DFMiniMp3.h>
#include <SoftwareSerial.h>

#include "settings.hpp"

enum class mp3Tracks: uint16_t {
  t_0                          =   0,
  t_262_pling                  = 262,
  t_300_new_tag                = 300,
  t_301_select_folder          = 301,
  t_310_select_mode            = 310,
  t_311_mode_random_episode    = 311,
  t_312_mode_album             = 312,
  t_313_mode_party             = 313,
  t_314_mode_single_track      = 314,
  t_315_mode_audio_book        = 315,
  t_316_admin                  = 316,
  t_317_special_random         = 317,
  t_318_special_album          = 318,
  t_319_special_party          = 319,
  t_320_mode_audio_book_single = 320,
  t_327_select_file            = 327,
  t_328_select_first_file      = 328,
  t_329_select_last_file       = 329,
  t_330_select_say_number      = 330,
  t_331_do_not_say_number      = 331,
  t_332_say_number             = 332,
  t_400_ok                     = 400,
  t_401_error                  = 401,
  t_800_waiting_for_card       = 800,
  t_802_reset_aborted          = 802,
  t_900_admin                  = 900,
  t_901_card_reset             = 901,
  t_902_max_volume             = 902,
  t_903_min_volume             = 903,
  t_904_init_volume            = 904,
  t_905_eq                     = 905,
  t_906_modifiers              = 906,
  t_907_shortcut               = 907,
  t_908_standbytimer           = 908,
  t_909_batch_cards            = 909,
  t_910_switch_volume          = 910,
  t_911_reset                  = 911,
  t_912_admin_lock             = 912,
  t_920_eq_intro               = 920,
  t_921_normal                 = 921,
  t_922_pop                    = 922,
  t_923_rock                   = 923,
  t_924_jazz                   = 924,
  t_925_classic                = 925,
  t_926_bass                   = 926,
  t_930_max_volume_intro       = 930,
  t_931_min_volume_into        = 931,
  t_932_init_volume_into       = 932,
  t_933_switch_volume_intro    = 933,
  t_934_no                     = 934,
  t_935_yes                    = 935,
  t_936_batch_cards_intro      = 936,
  t_940_shortcut_into          = 940,
  t_941_pause                  = 941,
  t_942_up                     = 942,
  t_943_down                   = 943,
  t_944_startup                = 944,
  t_960_timer_intro            = 960,
  t_961_timer_5                = 961,
  t_962_timer_15               = 962,
  t_963_timer_30               = 963,
  t_964_timer_60               = 964,
  t_965_timer_disabled         = 965,
  t_970_modifier_Intro         = 970,
  t_971_modifier_SleepTimer    = 971,
  t_972_modifier_FreezeDance   = 972,
  t_973_modifier_Locked        = 973,
  t_974_modifier_Toddler       = 974,
  t_975_modifier_KinderGarden  = 975,
  t_976_modifier_repeat1       = 976,
  t_980_admin_lock_intro       = 980,
  t_981_admin_lock_disabled    = 981,
  t_982_admin_lock_card        = 982,
  t_983_admin_lock_pin         = 983,
  t_984_admin_lock_calc        = 984,
  t_991_admin_pin              = 991,
  t_992_admin_calc             = 992,
  t_993_admin_calc             = 993,
  t_994_admin_calc             = 994,
  t_999_reset_ok               = 999,
};

inline mp3Tracks operator+(mp3Tracks lhs, uint16_t  rhs) { return static_cast<mp3Tracks>(static_cast<uint16_t>(lhs)+rhs); }

enum class advertTracks: uint16_t {
  t_260_activate_mod_card      = 260,
  t_261_deactivate_mod_card    = 261,
  t_262_pling                  = 262,
  t_300_freeze_into            = 300,
  t_301_freeze_freeze          = 301,
  t_302_sleep                  = 302,
  t_303_locked                 = 303,
  t_304_buttonslocked          = 304,
  t_305_kindergarden           = 305,
};

// implement a notification class,
// its member methods will get called
//
class Mp3Notify {
public:
  static void OnError             (uint16_t errorCode);
  static void OnPlayFinished      (DfMp3_PlaySources source, uint16_t track);
  static void OnPlaySourceOnline  (DfMp3_PlaySources source);
  static void OnPlaySourceInserted(DfMp3_PlaySources source);
  static void OnPlaySourceRemoved (DfMp3_PlaySources source);

  static void ResetLastTrackFinished() { lastTrackFinished = 0; }
private:
  static void PrintlnSourceAction (DfMp3_PlaySources source, const __FlashStringHelper* action);
  static uint16_t lastTrackFinished;
};

class Mp3: public DFMiniMp3<SoftwareSerial, Mp3Notify> {
public:
  Mp3(const Settings& settings);

  bool isPlaying() const;
  void waitForTrackToFinish();
  void playMp3FolderTrack(uint16_t track);
  void playMp3FolderTrack(mp3Tracks track);
  void playAdvertisement(uint16_t     track, bool olnyIfIsPlaying = true);
  void playAdvertisement(advertTracks track, bool olnyIfIsPlaying = true);

  void increaseVolume();
  void decreaseVolume();
  void setVolume     ();

private:
  SoftwareSerial       softwareSerial;
  const Settings&      settings;

  uint8_t              volume{};
};

#endif /* SRC_MP3_HPP_ */
