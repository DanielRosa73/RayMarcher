#ifndef SPHERE_H
#define SPHERE_H

#include "object.h"

class Sphere : public Object {
public:
    Sphere();
    Sphere(const Vector3& center, float radius, const Color& color);

    bool intersect(const Ray& ray, float& t, Vector3& normal, Color& color) const override;
    const Vector3& getCenter() const { return m_center; }
    float getRadius() const { return m_radius; }
    Color getColor() const {return m_color; }

private:
    Vector3 m_center;
    float m_radius;
    Color m_color;
};

#endif // SPHERE_H
