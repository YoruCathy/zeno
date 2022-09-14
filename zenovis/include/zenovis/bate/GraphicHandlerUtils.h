#pragma once

#include <memory>

#include <zeno/utils/vec.h>
#include <zenovis/opengl/buffer.h>

#include <glm/glm.hpp>

namespace zenovis {
namespace {

using zeno::vec3f;
using opengl::Buffer;

void drawAxis(vec3f pos, vec3f axis, vec3f color, float size, std::unique_ptr<Buffer> &vbo) {
    std::vector<vec3f> mem;

    auto end = pos + axis * size;

    mem.push_back(pos);
    mem.push_back(color);
    mem.push_back(end);
    mem.push_back(color);

    auto vertex_count = mem.size() / 2;
    vbo->bind_data(mem.data(), mem.size() * sizeof(mem[0]));

    vbo->attribute(0, sizeof(float) * 0, sizeof(float) * 6, GL_FLOAT, 3);
    vbo->attribute(1, sizeof(float) * 3, sizeof(float) * 6, GL_FLOAT, 3);

    CHECK_GL(glDrawArrays(GL_LINES, 0, vertex_count));

    vbo->disable_attribute(0);
    vbo->disable_attribute(1);
    vbo->unbind();
}

void drawCone(vec3f pos, vec3f a, vec3f b, vec3f color, float size, std::unique_ptr<Buffer> &vbo) {
    std::vector<vec3f> mem;

    auto normal = normalize(cross(a, b));

    auto top = pos + size * normal;
    auto ctr = pos - size * normal;

    float r = size / 2.0f;
    auto p = ctr + r * cos(0) * a + r * sin(0) * b;
    mem.push_back(p);
    mem.push_back(color);
    for (double t = 0.01; t < 1.0; t += 0.01) {
        double theta = 2.0 * 3.14159 * t;
        auto next_p = ctr + r * cos(theta) * a + r * sin(theta) * b;
        mem.push_back(next_p);
        mem.push_back(color);
        mem.push_back(ctr);
        mem.push_back(color);
        mem.push_back(p);
        mem.push_back(color);
        mem.push_back(top);
        mem.push_back(color);
        mem.push_back(next_p);
        mem.push_back(color);
        p = next_p;
    }

    auto vertex_count = mem.size() / 2;
    vbo->bind_data(mem.data(), mem.size() * sizeof(mem[0]));

    vbo->attribute(0, sizeof(float) * 0, sizeof(float) * 6, GL_FLOAT, 3);
    vbo->attribute(1, sizeof(float) * 3, sizeof(float) * 6, GL_FLOAT, 3);

    CHECK_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count));

    vbo->disable_attribute(0);
    vbo->disable_attribute(1);
    vbo->unbind();
}

void drawSquare(vec3f pos, vec3f a, vec3f b, vec3f color, float size, std::unique_ptr<Buffer> &vbo) {
    std::vector<vec3f> mem;

    auto dir1 = normalize(a + b);
    auto normal = normalize(cross(a, b));
    auto dir2 = normalize(cross(dir1, normal));

    auto ctr = pos + size * 10.0f * dir1;
    auto p1 = ctr + size * dir1;
    auto p3 = ctr - size * dir1;
    auto p2 = ctr + size * dir2;
    auto p4 = ctr - size * dir2;

    mem.push_back(p1);
    mem.push_back(color);
    mem.push_back(p2);
    mem.push_back(color);
    mem.push_back(p4);
    mem.push_back(color);
    mem.push_back(p3);
    mem.push_back(color);

    auto vertex_count = mem.size() / 2;
    vbo->bind_data(mem.data(), mem.size() * sizeof(mem[0]));

    vbo->attribute(0, sizeof(float) * 0, sizeof(float) * 6, GL_FLOAT, 3);
    vbo->attribute(1, sizeof(float) * 3, sizeof(float) * 6, GL_FLOAT, 3);

    CHECK_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count));

    vbo->disable_attribute(0);
    vbo->disable_attribute(1);
    vbo->unbind();
}

void drawCube(vec3f pos, vec3f a, vec3f b, vec3f color, float size, std::unique_ptr<Buffer> &vbo) {
    std::vector<vec3f> mem;

    auto n = normalize(cross(a, b));
    auto p1 = pos + size * a + size * b + size * n; // 1 1 1
    auto p2 = pos - size * a + size * b + size * n; // 0 1 1
    auto p3 = pos + size * a + size * b - size * n; // 1 1 0
    auto p4 = pos - size * a + size * b - size * n; // 0 1 0
    auto p5 = pos + size * a - size * b + size * n; // 1 0 1
    auto p6 = pos - size * a - size * b + size * n; // 0 0 1
    auto p7 = pos + size * a - size * b - size * n; // 1 0 0
    auto p8 = pos - size * a - size * b - size * n; // 0 0 0

    // top
    mem.push_back(p1);
    mem.push_back(color);
    mem.push_back(p2);
    mem.push_back(color);
    mem.push_back(p5);
    mem.push_back(color);
    mem.push_back(p6);
    mem.push_back(color);

    // back
    mem.push_back(p2);
    mem.push_back(color);
    mem.push_back(p8);
    mem.push_back(color);
    mem.push_back(p4);
    mem.push_back(color);

    // right
    mem.push_back(p2);
    mem.push_back(color);
    mem.push_back(p3);
    mem.push_back(color);
    mem.push_back(p1);
    mem.push_back(color);

    // front
    mem.push_back(p3);
    mem.push_back(color);
    mem.push_back(p5);
    mem.push_back(color);
    mem.push_back(p7);
    mem.push_back(color);

    // left
    mem.push_back(p5);
    mem.push_back(color);
    mem.push_back(p8);
    mem.push_back(color);
    mem.push_back(p6);
    mem.push_back(color);

    // bottom
    mem.push_back(p8);
    mem.push_back(color);
    mem.push_back(p4);
    mem.push_back(color);
    mem.push_back(p7);
    mem.push_back(color);
    mem.push_back(p3);
    mem.push_back(color);

    auto vertex_count = mem.size() / 2;
    vbo->bind_data(mem.data(), mem.size() * sizeof(mem[0]));

    vbo->attribute(0, sizeof(float) * 0, sizeof(float) * 6, GL_FLOAT, 3);
    vbo->attribute(1, sizeof(float) * 3, sizeof(float) * 6, GL_FLOAT, 3);

    CHECK_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count));

    vbo->disable_attribute(0);
    vbo->disable_attribute(1);
    vbo->unbind();
}

bool rightOn(glm::vec3 v1, glm::vec3 v2, glm::vec3 n) {
    return glm::dot(glm::cross(v2, v1), n) > 0;
}

bool rayIntersectSquare(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 square_min, glm::vec3 square_max,
                        glm::vec3 normal, glm::mat4 ModelMatrix) {
    auto v1 = glm::normalize(square_max - square_min);
    auto v2 = glm::normalize(glm::cross(v1, normal));
    auto len = glm::length(v1) / 2.0f;
    auto ctr = (square_min + square_max) / 2.0f;
    auto tp1 = ModelMatrix * glm::vec4(ctr + v2 * len, 1);
    auto tp3 = ModelMatrix * glm::vec4(ctr - v2 * len, 1);
    auto tp2 = ModelMatrix * glm::vec4(square_max, 1);
    auto tp4 = ModelMatrix * glm::vec4(square_min, 1);
    glm::vec3 p1 = tp1 / tp1[3];
    glm::vec3 p2 = tp2 / tp2[3];
    glm::vec3 p3 = tp3 / tp3[3];
    glm::vec3 p4 = tp4 / tp4[3];

    auto e1 = p1 - p2;
    auto e2 = p2 - p3;
    auto e3 = p3 - p4;
    auto e4 = p4 - p1;
    normal = ModelMatrix * glm::vec4(normal, 0);

    // calc intersection
    auto t = glm::dot((p1 - ray_origin), normal) / glm::dot(ray_direction, normal);
    if (t <= 0) return false;
    auto intersect_p = ray_origin + t * ray_direction;

    // test if intersection in square
    bool right_on_e1 = rightOn(e1, intersect_p - p1, normal);
    bool right_on_e2 = rightOn(e2, intersect_p - p2, normal);
    bool right_on_e3 = rightOn(e3, intersect_p - p3, normal);
    bool right_on_e4 = rightOn(e4, intersect_p - p4, normal);
    if ((right_on_e1 && right_on_e2 && right_on_e3 && right_on_e4) ||
        (!right_on_e1 && !right_on_e2 && !right_on_e3 && !right_on_e4))
        return true;
    return false;
}

/**
 * test if ray intersect an OBB
 * https://github.com/opengl-tutorials/ogl/blob/master/misc05_picking/misc05_picking_custom.cpp#L83
 */
bool rayIntersectOBB(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 aabb_min, glm::vec3 aabb_max,
                     glm::mat4 ModelMatrix, float &intersection_distance) {
    // Intersection method from Real-Time Rendering and Essential Mathematics for Games

    float tMin = 0.0f;
    float tMax = 100000.0f;

    glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);

    glm::vec3 delta = OBBposition_worldspace - ray_origin;

    // Test intersection with the 2 planes perpendicular to the OBB's X axis
    {
        glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
        float e = glm::dot(xaxis, delta);
        float f = glm::dot(ray_direction, xaxis);

        if (fabs(f) > 0.001f) { // Standard case

            float t1 = (e + aabb_min.x) / f; // Intersection with the "left" plane
            float t2 = (e + aabb_max.x) / f; // Intersection with the "right" plane
            // t1 and t2 now contain distances between ray origin and ray-plane intersections

            // We want t1 to represent the nearest intersection,
            // so if it's not the case, invert t1 and t2
            if (t1 > t2) {
                float w = t1;
                t1 = t2;
                t2 = w; // swap t1 and t2
            }

            // tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
            if (t2 < tMax)
                tMax = t2;
            // tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
            if (t1 > tMin)
                tMin = t1;

            // And here's the trick :
            // If "far" is closer than "near", then there is NO intersection.
            // See the images in the tutorials for the visual explanation.
            if (tMax < tMin)
                return false;

        } else { // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
            if (-e + aabb_min.x > 0.0f || -e + aabb_max.x < 0.0f)
                return false;
        }
    }

    // Test intersection with the 2 planes perpendicular to the OBB's Y axis
    // Exactly the same thing than above.
    {
        glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
        float e = glm::dot(yaxis, delta);
        float f = glm::dot(ray_direction, yaxis);

        if (fabs(f) > 0.001f) {

            float t1 = (e + aabb_min.y) / f;
            float t2 = (e + aabb_max.y) / f;

            if (t1 > t2) {
                float w = t1;
                t1 = t2;
                t2 = w;
            }

            if (t2 < tMax)
                tMax = t2;
            if (t1 > tMin)
                tMin = t1;
            if (tMin > tMax)
                return false;

        } else {
            if (-e + aabb_min.y > 0.0f || -e + aabb_max.y < 0.0f)
                return false;
        }
    }

    // Test intersection with the 2 planes perpendicular to the OBB's Z axis
    // Exactly the same thing than above.
    {
        glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
        float e = glm::dot(zaxis, delta);
        float f = glm::dot(ray_direction, zaxis);

        if (fabs(f) > 0.001f) {

            float t1 = (e + aabb_min.z) / f;
            float t2 = (e + aabb_max.z) / f;

            if (t1 > t2) {
                float w = t1;
                t1 = t2;
                t2 = w;
            }

            if (t2 < tMax)
                tMax = t2;
            if (t1 > tMin)
                tMin = t1;
            if (tMin > tMax)
                return false;

        } else {
            if (-e + aabb_min.z > 0.0f || -e + aabb_max.z < 0.0f)
                return false;
        }
    }

    intersection_distance = tMin;
    return true;
}

}
}