#ifndef SRC_STATE_MACHINE_HPP_
#define SRC_STATE_MACHINE_HPP_

#include "tinyfsm.hpp"
#include "buttons.hpp"
#include "chip_card.hpp"
#include "mp3.hpp"

struct ChMode;
class Tonuino;
class Mp3;

// ----------------------------------------------------------------------------
// Event Declarations
//
struct button_e: tinyfsm::Event {
  button_e(buttonRaw b):b{b} {}
  buttonRaw b;
};
struct card_e  : tinyfsm::Event {
  card_e(cardEvent c): c{c} {}
  cardEvent c;
};

// ----------------------------------------------------------------------------
// State Machine Base Class Declaration
//
enum class SM_type {
  tonuino,
  setupCard,
  writeCard,
};
template<SM_type SMT>
class SM: public tinyfsm::Fsm<SM<SMT>>
{
public:
  virtual void react(button_e const &) { };
  virtual void react(card_e   const &) { };

  virtual void entry(void) { };
  void         exit(void)  { };

  static folderSettings  folder; // TODO: put this into base of setupCard states
protected:
  static Tonuino        &tonuino;
  static Mp3            &mp3;
  static Buttons        &buttons;
  static Settings       &settings;
  static Chip_card      &chip_card;
};

typedef SM<SM_type::tonuino  > SM_tonuino;
typedef SM<SM_type::setupCard> SM_setupCard;
typedef SM<SM_type::writeCard> SM_writeCard;

class Base: public SM_tonuino
{
protected:
  bool readCard();
  nfcTagObject tempCard;
};

class Idle: public Base
{
public:
  void entry() override;
  void react(button_e const &) override;
  void react(card_e   const &) override;
};

class StartPlay: public Base
{
public:
  void entry() override;
  void react(button_e const &) override;
};

class Play: public Base
{
public:
  void entry() override;
  void react(button_e const &) override;
  void react(card_e   const &) override;
};

class Pause: public Base
{
public:
  void entry() override;
  void react(button_e const &) override;
  void react(card_e   const &) override;
};

// ----------------------------------------------------------------------------
// State Machine end states
//
class finished_setupCard      : public SM_setupCard{};
class finished_abort_setupCard: public SM_setupCard{};
class finished_writeCard      : public SM_writeCard{};
class finished_abort_writeCard: public SM_writeCard{};



#endif /* SRC_STATE_MACHINE_HPP_ */
