#ifndef SRC_LOG_HPP_
#define SRC_LOG_HPP_

#include <Arduino.h>

#include "type_traits.hpp"

#define DEFINE_LOGGER(Logger_, MinSeverity_, Forwarder_)                         \
  struct Logger_ : public logger_base<Logger_, MinSeverity_, Forwarder_>         \
  { static __FlashStringHelper const* name() {return F(#Logger_);} }

#define LOG(Logger_, Severity_, Expression_...)                                  \
  if constexpr ( Logger_::will_log(Severity_) )                                  \
    Logger_::template log< Severity_ >(Logger_::name(), Expression_)

#define LOG_CODE(Logger_, Severity_, Statement)                                  \
  if constexpr ( Logger_::will_log(Severity_) )                                  \
    Statement

enum severity: uint8_t {
  s_debug  ,
  s_info   ,
  s_warning,
  s_error  ,
};

extern const __FlashStringHelper* getSeverityName(severity sev);

class logger {
public:
  constexpr static bool will_log(severity) {
    return true;
  }

  static void log() {
    Serial.println();
  }
  template<typename T, typename ... Types>
  static void log(T t, Types ... types) {
    Serial.print(t);
    log(types...);
  }

  template<severity Severity, typename ... Types>
  static void log(const __FlashStringHelper* /*logname*/, Types ... types) {
//    Serial.print(millis());
//    Serial.print(F("-"));
//    Serial.print(getSeverityName(Severity));
//    Serial.print(F("-"));
//    Serial.print(logname);
//    Serial.print(F(": "));
    log(types...);
  }

};

template<typename Derived, severity MinSeverity, class FwdLogger>
class logger_base {
public:
  typedef typename if_<is_same_type<FwdLogger, void>::value, logger, FwdLogger>::result_type forward_logger_type;

  static constexpr bool will_log(severity s) {
    return (s >= MinSeverity) && forward_logger_type::will_log(s);
  }

  template<severity Severity, typename ... Types>
  static void log(const __FlashStringHelper* logname, Types ... types) {
    forward_logger_type::template log<Severity>(logname, types...);
  }
};

#endif /* SRC_LOG_HPP_ */
