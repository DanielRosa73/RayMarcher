#include "camera.h"
#include "light.h"
#include "sphere.h"
#include "scene.h"
#include "raymarcher.h"
#include "color.h"
#include "vector3.h"
#include "plane.h"
#include "cube.h"
#include "torus.h"
#include "cone.h"

#include <vector>
#include <iostream>
#include <fstream>

void write_ppm(const std::string& filename, const std::vector<std::vector<Color>>& framebuffer) {
    int width = framebuffer.size();
    int height = framebuffer[0].size();

    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error: Unable to open the output file." << std::endl;
        return;
    }

    file << "P3\n" << width << " " << height << "\n255\n";

    for (int j = height - 1; j >= 0; --j) {
        for (int i = 0; i < width; ++i) {
            int r = int(255.99f * framebuffer[i][j].r());
            int g = int(255.99f * framebuffer[i][j].g());
            int b = int(255.99f * framebuffer[i][j].b());
            file << r << " " << g << " " << b << " ";
        }
        file << "\n";
    }

    file.close();
}



int main() {
    int width = 800;
    int height = 600;

    Camera camera(Vector3(0.0f, 0.0f, 5.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f), 60.0f, float(width) / float(height));
    Light light(Vector3(5.0f, 5.0f, 5.0f), Color(1.0f, 1.0f, 1.0f));

    Sphere sphere1(Vector3(-1.0f, 0.5f, -3.0f), 0.5f, Color(0.8f, 0.2f, 0.2f));
    Sphere sphere2(Vector3(1.0f, 1.5f, -4.0f), 0.5f, Color(0.2f, 0.2f, 0.7f));
    Sphere sphere3(Vector3(2.0f, -0.5f, 2.0f), 0.5f, Color(0.2f, 0.8f, 0.3f));
    
    //Cone cone1(Vector3(-2.0f, 0.0f, -2.0f), 1.0f, 1.0f, Color(0.2f, 0.8f, 0.2f));
    //Cone cone2(Vector3(2.0f, 0.0f, -2.0f), 1.5f, 0.5f, Color(0.8f, 0.2f, 0.2f));

    Scene scene;
    scene.setCamera(camera);
    scene.addLight(std::make_shared<Light>(light));
    scene.addObject(std::make_shared<Sphere>(sphere1));
    scene.addObject(std::make_shared<Sphere>(sphere2));
    scene.addObject(std::make_shared<Sphere>(sphere3));
    scene.addObject(std::make_shared<Cone>(cone1));
    //scene.addObject(std::make_shared<Cone>(cone2));
    
    Vector3 cubeCenter(2, 1 , -3);
    Vector3 cubeDimensions(1, 1, 1);
    Color cubeColor(0.5, 0.2, 0.8);
    std::shared_ptr<Cube> cube = std::make_shared<Cube>(cubeCenter, cubeDimensions, cubeColor);
    scene.addObject(cube);


     // Torus
    Vector3 torusCenter(3, -2.5, -10);
    float majorRadius = 1.2f;
    float minorRadius = 0.5f;
    Color torusColor(0.8, 0, 0.2);
    std::shared_ptr<Torus> torus = std::make_shared<Torus>(torusCenter, majorRadius, minorRadius, torusColor);
    scene.addObject(torus);

    
    Vector3 planeNormal(0.0f, 1.0f, 0.0f); // Normal pointing upwards
    float planeDistance = -20.0f; // Distance from the origin (adjust this value as needed)
    Color planeColor(0.5f, 0.5f, 0.5f); // Ground plane color
    std::shared_ptr<Object> groundPlane = std::make_shared<Plane>(planeNormal, planeDistance, planeColor);
    scene.addObject(groundPlane);

    std::vector<std::vector<Color>> framebuffer(width, std::vector<Color>(height));
    Raymarcher raymarcher;
    raymarcher.render(scene, framebuffer);

    write_ppm("output.ppm", framebuffer);

    return 0;
}


