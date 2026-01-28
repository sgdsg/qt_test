#include "WebSocketServer.h"
#include "lib_c.h"
#include <QObject>
#include <QThread>
#include <dlfcn.h>
#include <qcoreapplication.h>
#include <qdebug.h>
#include <qlogging.h>
#include <qobject.h>
#include <qthread.h>
#include <qtimer.h>
struct api {
  calc_h *(*create)();
  void (*destroy)(calc_h *);
  void (*input)(calc_h *, int, int);
  int (*plus)(calc_h *);
  int (*minus)(calc_h *);
};

int main(int argc, char *argv[]) {
  void *h = dlopen("./liblib_test.so", RTLD_NOW);
  api api;
  api.create = reinterpret_cast<calc_h *(*)()>(dlsym(h, "calc_create"));
  api.destroy = reinterpret_cast<void (*)(calc_h *)>(dlsym(h, "calc_destroy"));
  api.input =
      reinterpret_cast<void (*)(calc_h *, int, int)>(dlsym(h, "calc_input"));
  api.plus = reinterpret_cast<int (*)(calc_h *)>(dlsym(h, "calc_plus"));
  api.minus = reinterpret_cast<int (*)(calc_h *)>(dlsym(h, "calc_minus"));
  calc_h *c = api.create();
  api.input(c, 10, 3);
  printf("plus  = %d\n", api.plus(c));
  printf("minus = %d\n", api.minus(c));
  api.destroy(c);
  dlclose(h);

  // QCoreApplication a(argc, argv);
  // WebSocketServer s(1037);
  // return a.exec();
}
