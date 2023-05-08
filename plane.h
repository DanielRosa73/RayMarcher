#pragma once

#include "object.h"
#include "vector3.h"
#include "color.h"
#include "ray.h"

class Plane : public Object {
public:
    
    Plane();
    Plane(const Vector3& normal, float distance, const Color& color, float opacity);

    virtual bool intersect(const Ray& ray, float& t, Vector3& normal, Color& color) const override;
    
    Vector3 getNormal() const { return m_normal; }
    float getDistance() const { return m_distance; }
    Color getColor() const { return m_color; }
    float getOpacity() const {return m_opacity; }

private:
    Vector3 m_normal;
    float m_distance;
    Color m_color;
    float m_opacity;
};
