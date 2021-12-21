#ifndef SRC_LOG_HPP_
#define SRC_LOG_HPP_

#include <Arduino.h>

#define DEFINE_LOGGER(Logger_, MinSeverity_, Forwarder_)                         \
  struct Logger_ : public logger_base<Logger_, MinSeverity_, Forwarder_>         \
  { static __FlashStringHelper const* name() {return F(#Logger_);} }

#define LOG(Logger_, Severity_, Expression_...)                                  \
  if constexpr ( Logger_::will_log(Severity_) )                                  \
    Logger_::template log< Severity_ >(Logger_::name(), Expression_)

enum severity {
  s_debug  ,
  s_info   ,
  s_warning,
  s_error  ,
};

extern const __FlashStringHelper* getSeverityName(severity sev);

//! compile-time if
template<bool cond, typename T1, typename T2> struct if_              { typedef T1 result_type; };
template<           typename T1, typename T2> struct if_<false,T1,T2> { typedef T2 result_type; };
//! bool-to-type
template<bool b>         struct bool_ { enum { result = b }; typedef bool_ result_type; };
//! determines whether two types have the same type
template<typename T1, typename T2> struct is_same_type      : bool_<false> {};
template<typename T>               struct is_same_type<T,T> : bool_<true > {};

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
  typedef typename if_<is_same_type<FwdLogger, void>::result, logger, FwdLogger>::result_type forward_logger_type;

  static constexpr bool will_log(severity s) {
    return (s >= MinSeverity) && forward_logger_type::will_log(s);
  }

  template<severity Severity, typename ... Types>
  static void log(const __FlashStringHelper* logname, Types ... types) {
    forward_logger_type::template log<Severity>(logname, types...);
  }
};

#endif /* SRC_LOG_HPP_ */
