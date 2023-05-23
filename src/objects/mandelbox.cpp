#include "mandelbox.h"
#include <algorithm>

Mandelbox::Mandelbox() {}

Mandelbox::Mandelbox(const Vector3& center, float scale, const Color& color, const Material& mat)
    : m_center(center), m_scale(scale) {
        this->m_color = color;
        this->m_material = mat;
    }

Vector3 Mandelbox::boxFold(const Vector3& v) const {
    Vector3 result = v;
    float FOLDING_LIMIT = 1.0;
    float FOLDING_VALUE = 2.0;
    
    if (v.x > FOLDING_LIMIT) result.x = FOLDING_VALUE - v.x;
    else if (v.x < -FOLDING_LIMIT) result.x = -FOLDING_VALUE - v.x;

    if (v.y > FOLDING_LIMIT) result.y = FOLDING_VALUE - v.y;
    else if (v.y < -FOLDING_LIMIT) result.y = -FOLDING_VALUE - v.y;

    if (v.z > FOLDING_LIMIT) result.z = FOLDING_VALUE - v.z;
    else if (v.z < -FOLDING_LIMIT) result.z = -FOLDING_VALUE - v.z;

    return result;
}

Vector3 Mandelbox::sphereFold(const Vector3& v) const {
    Vector3 result = v;
    float r2 = v.x * v.x + v.y * v.y + v.z * v.z;
    float MIN_RADIUS = 0.5;
    float FIXED_RADIUS = 1.0;

    if (r2 < MIN_RADIUS) result *= FIXED_RADIUS / MIN_RADIUS;
    else if (r2 < FIXED_RADIUS) result *= FIXED_RADIUS / r2;

    return result;
}

float Mandelbox::SDF(const Vector3& point) const {
        Vector3 z = (point - m_center) / m_scale;
        Vector3 c = z;
        float dr = 1.0;

        for (int i = 0; i < 100; i++) {
            z = boxFold(z);
            z = sphereFold(z);

            z = z * m_scale + c;
            dr = dr * abs(m_scale) + 1.0;

            if (z.length() > 2.0) break;
        }

        return 0.5 * log(z.length()) * z.length() / dr;
}
