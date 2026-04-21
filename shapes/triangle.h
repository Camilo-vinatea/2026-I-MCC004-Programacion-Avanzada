#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include "shape.h"

class Triangle : public Shape {
private:
    Distance m_base;
    Distance m_height;
public:
    Triangle(string name, ostream &os, Distance base, Distance height);
    virtual ~Triangle();
    Area GetArea() const override;
    string ToString() const override { 
        ostringstream oss;
        oss << "Triangle : " << GetName() << " -> área: " << GetArea();
        return oss.str(); 
    };
};

#endif // __TRIANGLE_H__