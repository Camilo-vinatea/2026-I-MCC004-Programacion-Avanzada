#include <cmath>
#include "circle.h"
#define M_PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679

Circle::Circle(string name, ostream &os, Distance radius)
    : Shape(name, os), m_radius(radius) {
        m_os << "Construyendo Circle: " << GetName() << endl;
    }

Circle::~Circle() {
    m_os << "Destruyendo  Circle: " << GetName() << endl;
}

Area Circle::GetArea() const {
    return M_PI * m_radius * m_radius;
}