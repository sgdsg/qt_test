#include "SignalConfig.h"
ConfigManager& ConfigManager::instance() {
static ConfigManager instance;
return instance;
}
