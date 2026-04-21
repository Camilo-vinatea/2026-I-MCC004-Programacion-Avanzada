#ifndef __SHAPE_H__
#define __SHAPE_H__

#include <string>
#include <iostream>
#include <sstream> // ostringstream
#include "../types.h"
using namespace std;

class Shape {
private:
    string m_name;
protected:
    ostream &m_os;
public:
    Shape(string name, ostream &os) : m_name(name), m_os(os) { m_os << "Construyendo Shape: " << m_name << endl; }
    virtual ~Shape()                  { m_os << "Destruyendo  Shape: " << m_name << endl; };
    virtual Area   GetArea() const = 0; // Metodo virtual puro
    virtual string GetName() const { return m_name; };
    virtual string ToString() const { return m_name; };
};

ostream& operator<<(ostream& os, const Shape& shape);

#endif // __SHAPE_H__