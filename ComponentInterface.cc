#include "ComponentInterface.h"
#include <qobject.h>
#include <qthread.h>

ComponentInterface::ComponentInterface() {
    thread= new QThread;
}

void ComponentInterface::stop() {
    running.store(false);
}