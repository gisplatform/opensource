#include <QApplication>
#include <QtGui>
#include <stdio.h>
#include <unistd.h>

#include "gp-qtrunner.h"


static QApplication *qt_runner_app;


void gp_qt_runner_run(int *argc, char ***argv)
{
  // Костыли, чтобы Qt с помощью dlopen не грузил libgtk-2 для QGtkStyle -->
    if(qgetenv("DESKTOP_SESSION") == "gnome")
      qputenv("DESKTOP_SESSION", NULL);

    if(!qgetenv("GNOME_DESKTOP_SESSION_ID").isEmpty())
      qputenv("GNOME_DESKTOP_SESSION_ID", NULL);
  // Костыли, чтобы Qt с помощью dlopen не грузил libgtk-2 для QGtkStyle <--

  QApplication a(*argc, *argv);

  qt_runner_app = &a;

  a.exec();
}


void gp_qt_runner_stop()
{
  qt_runner_app->quit();
}

