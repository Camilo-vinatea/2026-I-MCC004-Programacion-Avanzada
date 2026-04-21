#include "shape.h"

ostream& operator<<(ostream& os, const Shape& shape) {
    return os << shape.ToString();
}

