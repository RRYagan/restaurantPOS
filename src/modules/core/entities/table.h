#ifndef TABLE_H
#define TABLE_H

#pragma once
#include <QString>

struct Table {
    Q_GADGET
public:
    int id;
    QString label; //table no.
    int capacity;
    bool isOccupied;
    int currentOrderId;

    // UI coordinates for floor plan
    float x;
    float y;
};

#endif // TABLE_H
