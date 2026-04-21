#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__

#include "shape.h"

class Rectangle : public Shape {
private:
    Distance m_width;
    Distance m_height;
public:
    Rectangle(string name, ostream &os, Distance width, Distance height);
    virtual ~Rectangle();
    Area GetArea() const override;
    string ToString() const override { 
        ostringstream oss;
        oss << "Rectangle: " << GetName() << " -> área: " << GetArea();
        return oss.str(); 
    };
};

#endif // __RECTANGLE_H__