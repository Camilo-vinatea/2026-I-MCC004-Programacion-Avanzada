#ifndef __CIRCLE_H__
#define __CIRCLE_H__

#include "shape.h"

class Circle : public Shape {
private:
    Distance m_radius;
public:
    Circle(string name, ostream &os, Distance radius);
    virtual ~Circle();
    Area GetArea() const override;
    string ToString() const override { 
        ostringstream oss;
        oss << "Circle   : " << GetName() << " -> área: " << GetArea();
        return oss.str(); 
    };
};

#endif // __CIRCLE_H__