#include "log.hpp"

const __FlashStringHelper* getSeverityName(severity sev) {
  switch (sev) {
  case s_debug  : return F("debug"  );
  case s_info   : return F("info"   );
  case s_warning: return F("warning");
  case s_error  : return F("error"  );
  }
  return F("unknown");
}




