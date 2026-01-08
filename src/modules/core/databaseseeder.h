#ifndef DATABASESEEDER_H
#define DATABASESEEDER_H

#include <QObject>

class DatabaseSeeder : public QObject {
    Q_OBJECT
public:
    static void seed();
};
#endif
