#include "log.hpp"

const __FlashStringHelper* getSeverityName(severity sev) {
  switch (sev) {
  case s_debug  : return F("D");
  case s_info   : return F("I");
  case s_warning: return F("W");
  case s_error  : return F("E");
  }
  return F("unknown");
}




