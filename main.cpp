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

    Camera camera(Vector3(0.0f, 0.0f, 7.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f), 60.0f, float(width) / float(height));
    Light light(Vector3(0.0f, 4.0f, 1.0f), Color(1.0f, 1.0f, 1.0f), 0.1f);

    Material mat1(1, 1000,0.12f);
    Material mat2(1, 1500,0.0f);
    Material mat3(5, 120,0.0f);
    Material mat(1, 5000,0.0f);
    Sphere sphere1(Vector3(-1.0f, -0.28f, -2.0f), 1.2f, Color(0.8f, 0.2f, 0.2f), mat1);
    Sphere sphere2(Vector3(0.5f, -1.0f, 0.0f), 0.5f, Color(0.2f, 0.2f, 0.7f), mat2);
    //Sphere sphere3(Vector3(2.0f, -0.5f, 2.0f), 0.5f, Color(0.2f, 0.8f, 0.3f), mat3);
    
    //Cone cone1(Vector3(-2.0f, 0.0f, -2.0f), 1.0f, 1.0f, Color(0.2f, 0.8f, 0.2f));
    //Cone cone2(Vector3(2.0f, 0.0f, -2.0f), 1.5f, 0.5f, Color(0.8f, 0.2f, 0.2f));

    Scene scene;
    scene.setCamera(camera);
    scene.addLight(std::make_shared<Light>(light));
    scene.addObject(std::make_shared<Sphere>(sphere1));
    scene.addObject(std::make_shared<Sphere>(sphere2));
    //scene.addObject(std::make_shared<Sphere>(sphere3));
    //scene.addObject(std::make_shared<Cone>(cone1));
    //scene.addObject(std::make_shared<Cone>(cone2));
    
    
    Vector3 cubeCenterL(-5, 1 , -3);
    Vector3 cubeCenterR(5, 1 , -3);
    Vector3 cubeCenterBack(0, 1 , -5);
    Vector3 cubeCenterBot(0, -2 , 0);
    Vector3 cubeCenterTop(0, 5 , 0);
    Vector3 cubeDimensions(1, 10, 10);
    Vector3 cubeDimensions2(10, 10, 1);
    Vector3 cubeDimensions3(10, 1, 10);
    Color cubeColor1(0.5, 0.5, 0.5);
    Color cubeColor2(0.5, 0.5, 0.5);
    std::shared_ptr<Cube> cubeL = std::make_shared<Cube>(cubeCenterL, cubeDimensions, cubeColor1, mat);
    std::shared_ptr<Cube> cubeR = std::make_shared<Cube>(cubeCenterR, cubeDimensions, cubeColor1, mat);
    std::shared_ptr<Cube> cubeBack = std::make_shared<Cube>(cubeCenterBack, cubeDimensions2, cubeColor2, mat);
    std::shared_ptr<Cube> cubeBot = std::make_shared<Cube>(cubeCenterBot, cubeDimensions3, cubeColor2, mat);
    std::shared_ptr<Cube> cubeTop = std::make_shared<Cube>(cubeCenterTop, cubeDimensions3, cubeColor2, mat);
    scene.addObject(cubeL);
    scene.addObject(cubeR);
    scene.addObject(cubeBack);
    scene.addObject(cubeBot);
    scene.addObject(cubeTop);
    

    /*
    Vector3 torusCenter(3, -2.5, -10);
    float majorRadius = 1.2f;
    float minorRadius = 0.5f;
    Color torusColor(0.8, 0, 0.2);
    std::shared_ptr<Torus> torus = std::make_shared<Torus>(torusCenter, majorRadius, minorRadius, torusColor, mat);
    scene.addObject(torus);
    */
    
    
    Vector3 planeNormal(0.0f, 1.0f, 0.0f); // Normal pointing upwards
    float planeDistance = -20.0f; // Distance from the origin (adjust this value as needed)
    Color planeColor(0.5f, 0.5f, 0.5f); // Ground plane color
    std::shared_ptr<Object> groundPlane = std::make_shared<Plane>(planeNormal, planeDistance, planeColor);
    // scene.addObject(groundPlane);

    std::vector<std::vector<Color>> framebuffer(width, std::vector<Color>(height));
    Raymarcher raymarcher;
    raymarcher.render_antialiasing(scene, framebuffer);

    write_ppm("output.ppm", framebuffer);

    return 0;
}
