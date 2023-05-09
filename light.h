#ifndef LIGHT_H
#define LIGHT_H

#include "vector3.h"
#include "color.h"

class Light {
public:
    Light();
    
    Light(const Vector3& position, const Color& color, float radius);

    const Vector3& getPosition() const;
    const Color& getColor() const;
    Vector3 getRandomPoint() const;

private:
    Vector3 m_position;
    Color m_color;
    float m_radius;
};

#endif // LIGHT_H
