#include "raymarcher.h"
#include "camera.h"
#include "light.h"
#include "sphere.h"
#include "cube.h"
#include "scene.h"
#include "color.h"
#include "vector3.h"
#include "plane.h"
#include "cube.h"
#include "torus.h"
#include "cone.h"


constexpr int MAX_MARCHING_STEPS = 400;
constexpr float MIN_HIT_DISTANCE = 1e-4f;
constexpr float MAX_DISTANCE = 1e14f;
constexpr float EPSILON = 1e-6f;
constexpr int SAMPLES_PER_PIXEL = 10;
constexpr float AMBIENT_LIGHT_INTENSITY = 0.2f;
constexpr int NUM_SHADOW_RAYS = 1;
constexpr float SHADOW_THRESHOLD = 0.1f;
constexpr float SHADOW_BIAS = 1e-3f;
constexpr int MAX_REFLECTION_BOUNCES = 5;

float sphereSDF(const Vector3& point, const Sphere& sphere) {
    return (point - sphere.getCenter()).length() - sphere.getRadius();
}

float planeSDF(const Vector3& point, const Plane& plane) {
    return point.dot(plane.getNormal()) - plane.getDistance();
}


float cubeSDF(const Vector3& point, const Cube& cube) {
    Vector3 p = point - cube.getCenter();
    Vector3 d = p.abs() - cube.getDimensions() * 0.5f;
    return d.max(0.0f).length() + std::min(std::max(d.x, std::max(d.y, d.z)), 0.0f);
}

float torusSDF(const Vector3& point, const Torus& torus) {
    Vector3 p = point - torus.getCenter();
    Vector3 q = Vector3((Vector3(p.x, 0.0f, p.z).length() - torus.getMajorRadius()), p.y, 0.0f);
    return q.length() - torus.getMinorRadius();
}

float coneSDF(const Vector3& point, const Cone& cone) {
    Vector3 p = point - cone.getCenter();
    Vector3 c = Vector3(0, cone.getHeight(), 0);  // cone tip at origin and in positive y direction
    float q = (p - c * std::max(0.0f, p.dot(c) / c.dot(c))).length(); // distance from p to line (c, c + cone direction)
    return std::max(-q, p.y); // q for region outside of cone, p.y for region inside cone
}




float sceneSDF(const Vector3& point, const std::vector<std::shared_ptr<Object>>& objects) {
    float min_distance = MAX_DISTANCE;
    for (const auto& object : objects) {
        float distance;
        if (auto sphere = std::dynamic_pointer_cast<Sphere>(object)) {
            distance = sphereSDF(point, *sphere);
        } else if (auto plane = std::dynamic_pointer_cast<Plane>(object)) {
            distance = planeSDF(point, *plane);
        } else if (auto cube = std::dynamic_pointer_cast<Cube>(object)) {
            distance = cubeSDF(point, *cube);
        } else if (auto torus = std::dynamic_pointer_cast<Torus>(object)) {
            distance = torusSDF(point, *torus);
        } else if (auto cone = std::dynamic_pointer_cast<Cone>(object)) {
            distance = coneSDF(point, *cone);
        }
        min_distance = std::min(min_distance, distance);
    }
    return min_distance;
}







bool raymarch(const Scene& scene, const Ray& ray, Vector3& hit_point, std::shared_ptr<Object>& hit_object) {
    float total_distance = 0.0f;

    for (int i = 0; i < MAX_MARCHING_STEPS; ++i) {
        hit_point = ray.pointAtParameter(total_distance);
        float distance = sceneSDF(hit_point, scene.getObjects());

        if (distance < MIN_HIT_DISTANCE) {
            hit_object = nullptr;
            for (const auto& object : scene.getObjects()) {
                if (auto sphere = std::dynamic_pointer_cast<Sphere>(object)) {
                    if (std::abs(sphereSDF(hit_point, *sphere)) < MIN_HIT_DISTANCE) {
                        hit_object = sphere;
                        break;
                    }
                } else if (auto plane = std::dynamic_pointer_cast<Plane>(object)) {
                    if (std::abs(planeSDF(hit_point, *plane)) < MIN_HIT_DISTANCE) {
                        hit_object = plane;
                        break;
                    }
                } else if (auto cube = std::dynamic_pointer_cast<Cube>(object)) {
                    if (std::abs(cubeSDF(hit_point, *cube)) < MIN_HIT_DISTANCE) {
                        hit_object = cube;
                        break;
                    }
                } else if (auto torus = std::dynamic_pointer_cast<Torus>(object)) {
                    if (std::abs(torusSDF(hit_point, *torus)) < MIN_HIT_DISTANCE) {
                        hit_object = torus;
                        break;
                    }
                } else if (auto cone = std::dynamic_pointer_cast<Cone>(object)) {
                    if (std::abs(coneSDF(hit_point, *cone)) < MIN_HIT_DISTANCE) {
                        hit_object = cone;
                        break;
                    }
                }
            }

            return true;
        }

        total_distance += distance;

        if (total_distance >= MAX_DISTANCE) {
            break;
        }
    }

    return false;
}


Vector3 estimateNormal(const Vector3& point, const std::vector<std::shared_ptr<Object>>& objects) {
    Vector3 normal(
        sceneSDF(Vector3(point.x + EPSILON, point.y, point.z), objects) - sceneSDF(Vector3(point.x - EPSILON, point.y, point.z), objects),
        sceneSDF(Vector3(point.x, point.y + EPSILON, point.z), objects) - sceneSDF(Vector3(point.x, point.y - EPSILON, point.z), objects),
        sceneSDF(Vector3(point.x, point.y, point.z + EPSILON), objects) - sceneSDF(Vector3(point.x, point.y, point.z - EPSILON), objects)
    );
    return normal.normalized();
}


Raymarcher::Raymarcher() {}



void Raymarcher::render(const Scene& scene, std::vector<std::vector<Color>>& framebuffer) {
    const Camera& camera = scene.getCamera();
    int width = framebuffer.size();
    int height = framebuffer[0].size();

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            float u = float(i) / float(width);
            float v = float(j) / float(height);
            Ray ray = camera.getRay(u, v);
            framebuffer[i][j] = trace(scene, ray);
        }
    }
}



void Raymarcher::render_antialiasing(const Scene& scene, std::vector<std::vector<Color>>& framebuffer) {
    const Camera& camera = scene.getCamera();
    int width = framebuffer.size();
    int height = framebuffer[0].size();

    
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            Color pixel_color(0.0f, 0.0f, 0.0f);
            for (int s = 0; s < SAMPLES_PER_PIXEL; ++s) {
                float u = float(i + (rand() / (RAND_MAX + 1.0))) / float(width);
                float v = float(j + (rand() / (RAND_MAX + 1.0))) / float(height);
                Ray ray = camera.getRay(u, v);
                pixel_color += trace(scene, ray);
            }
            framebuffer[i][j] = pixel_color / float(SAMPLES_PER_PIXEL);
        }
    }
}



Color Raymarcher::trace(const Scene& scene, const Ray& ray) {
    Vector3 hit_point;
    std::shared_ptr<Object> hit_object;
    bool hit = raymarch(scene, ray, hit_point, hit_object);

    if (hit && hit_object) {
        Vector3 normal = estimateNormal(hit_point, scene.getObjects());
        Color object_color;
        Material object_material;
        if (auto sphere = std::dynamic_pointer_cast<Sphere>(hit_object)) {
            object_color = sphere->getColor();
            object_material = sphere->getMaterial();
        } else if (auto plane = std::dynamic_pointer_cast<Plane>(hit_object)) {
            object_color = plane->getColor();
        } else if (auto cube = std::dynamic_pointer_cast<Cube>(hit_object)) {
            object_color = cube->getColor();
            object_material = cube->getMaterial();
        } else if (auto torus = std::dynamic_pointer_cast<Torus>(hit_object)) {
            object_color = torus->getColor();
            object_material = torus->getMaterial();
        }
        return shade(scene, hit_point, normal, object_material, object_color, ray);
    } 
    else {
        return getBackgroundColor(ray);
    }
    return getBackgroundColor(ray);
}



Color Raymarcher::shade(const Scene& scene, const Vector3& point, const Vector3& normal, const Material& object_material, const Color& object_color, const Ray& ray) {
    const auto& lights = scene.getLights();
    
    Color result = object_color * AMBIENT_LIGHT_INTENSITY; // Ambient light

    for (const auto& light : lights) {
        Vector3 light_direction = (light->getPosition() - point).normalized();
        Vector3 view_direction = (ray.getDirection() * -1.0f).normalized();
        Vector3 half_vector = (light_direction + view_direction).normalized();

        float shadow_ratio = shadow(scene, point, *light);  // Calculate shadow ratio

        // Only calculate lighting if the point is not fully in shadow
        if (shadow_ratio < 1.0f) {
            // Diffuse lighting
            float diffuse = std::max(normal.dot(light_direction), 0.0f);
            Color diffuse_color = object_color * diffuse * object_material.diffuse;

            // Specular lighting
            float shininess = object_material.specular;
            //float specular = std::pow(std::max(normal.dot(half_vector), 0.8f), shininess);
            float specular = std::pow(std::max(normal.dot(half_vector), 0.0f), shininess);

            Color specular_color = light->getColor() * specular * (1.0f - shadow_ratio);

            result = result + diffuse_color + specular_color;
        }
    }

    return result;
}




Color Raymarcher::getBackgroundColor(const Ray& ray) const {
    Vector3 unitDirection = ray.getDirection().normalized();
    float t = 0.5f * (unitDirection.y + 1.0f);
    return Color::lerp(Color(1.0f, 1.0f, 1.0f), Color(0.5f, 0.7f, 1.0f), t);
}

/* This is raytracing
bool Raymarcher::shadow(const Scene& scene, const Vector3& point, const Light& light) {
    Vector3 light_direction = (light.getPosition() - point).normalized();
    Ray shadow_ray(point + light_direction * EPSILON, light_direction);
    float t = (light.getPosition() - point).length();
    const auto& objects = scene.getObjects();

    for (const auto& object : objects) {
        float temp_t;
        Vector3 temp_normal;
        Color temp_color;

        if (object->intersect(shadow_ray, temp_t, temp_normal, temp_color) && temp_t < t) {
            return true;
        }
    }

    return false;
    }
*/

float Raymarcher::shadow(const Scene& scene, const Vector3& point, const Light& light) {
    int shadowCount = 0;

    for (int i = 0; i < NUM_SHADOW_RAYS; ++i) {
        Vector3 light_position = light.getRandomPoint();
        Vector3 light_direction = (light_position - point).normalized();
        Vector3 startPoint = point + light_direction * SHADOW_BIAS;
        float distanceToLight = (light_position - point).length();
        float totalDistance = 0.0f;

        for (int j = 0; j < MAX_MARCHING_STEPS; ++j) {
            Vector3 currentPoint = startPoint + light_direction * totalDistance;
            float distance = sceneSDF(currentPoint, scene.getObjects());

            if (distance < MIN_HIT_DISTANCE) {
                shadowCount += 1;
                break;
            }

            totalDistance += distance;

            if (totalDistance >= distanceToLight) {
                break;
            }
        }
    }

    return (float)shadowCount / NUM_SHADOW_RAYS;
}







       
