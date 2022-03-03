#ifndef SRC_STATE_MACHINE_HPP_
#define SRC_STATE_MACHINE_HPP_

#include "tinyfsm.hpp"
#include "buttons.hpp"
#include "chip_card.hpp"
#include "mp3.hpp"
#include "timer.hpp"

class Tonuino;

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
class finished_setupCard      ;
class finished_abort_setupCard;
class finished_writeCard      ;
class finished_abort_writeCard;
class Idle                    ;

enum class SM_type: uint8_t {
  tonuino,
  setupCard,
  writeCard,
};
template<SM_type SMT>
class SM: public tinyfsm::Fsm<SM<SMT>>
{
public:
  typedef typename conditional<
             (SMT == SM_type::setupCard), finished_setupCard,
          typename conditional<
             (SMT == SM_type::writeCard), finished_writeCard,
          Idle>::type
  >::type finished;
  typedef typename conditional<
             (SMT == SM_type::setupCard), finished_abort_setupCard,
          typename conditional<
             (SMT == SM_type::writeCard), finished_abort_writeCard,
          Idle>::type
  >::type finished_abort;

  virtual void react(button_e const &) { };
  virtual void react(card_e   const &) { };

  virtual void entry(void) { };
  void         exit(void)  { waitForPlayFinish = false; };

  bool isAbort(button_e const &b);

  static folderSettings folder;
protected:
  static Tonuino        &tonuino;
  static Mp3            &mp3;
  static Buttons        &buttons;
  static Settings       &settings;
  static Chip_card      &chip_card;

  static Timer          timer;
  static bool           waitForPlayFinish; // with this it needs 66 Byte lesser program code ;-)
};

using SM_tonuino   = SM<SM_type::tonuino  >;
using SM_setupCard = SM<SM_type::setupCard>;
using SM_writeCard = SM<SM_type::writeCard>;

class Base: public SM_tonuino
{
protected:
  bool readCard();
  static nfcTagObject lastCardRead;
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
