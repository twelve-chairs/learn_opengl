//
// Created by air on 12/21/21.
//

#ifndef LEARN_OPENGL_PRIMITIVES_H
#define LEARN_OPENGL_PRIMITIVES_H

const float planeMax = 20.0f;
const float floorMin = 0.0f;

float planeVertices[] = {
        -planeMax, floorMin , -planeMax, 0.0f, 0.0f,
        planeMax, floorMin , -planeMax, planeMax, 0.0f,
        -planeMax, floorMin , planeMax, planeMax/2, planeMax,

        -planeMax, floorMin , planeMax, planeMax, 0.0f,
        planeMax, floorMin , planeMax, 0.0f, 0.0f,
        planeMax, floorMin , -planeMax, planeMax/2, planeMax
};

const float scale = 1.0f;

float pyramidVertices[] = {
        -2.0f, -2.0f, 2.0f, scale, 0.0f,
        2.0f, -2.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, scale/ 2, scale -0.1f, // front face
        2.0f, -2.0f, 2.0f, scale, 0.0f,
        2.0f, -2.0f, -2.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, scale/ 2, scale-0.1f, // right face
        2.0f,-2.0f, -2.0f, scale, 0.0f,
        -2.0f, -2.0f, -2.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, scale/ 2, scale-0.1f, // back face
        -2.0f, -2.0f, -2.0f ,scale, 0.0f,
        -2.0f, -2.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, scale/ 2, scale-0.1f, // left face
        -2.0f, -2.0f, -2.0f, scale, 0.0f,
        2.0f, -2.0f, 2.0f, 0.0f, 0.0f,
        -2.0f, -2.0f, 2.0f, scale/ 2, scale, // base – left front
        2.0f, -2.0f, 2.0f, scale, 0.0f,
        -2.0f, -2.0f, -2.0f, 0.0f, 0.0f,
        2.0f, -2.0f, -2.0f, scale/ 2, scale // base – right back
};


// set up vertex data (and buffer(s)) and configure vertex attributes
// 6 vectors (2 triangles) * 6 sides
// ------------------------------------------------------------------
float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

float woodVertices[] = {
        -0.1f, -0.1f, -20.0f,  0.0f, 0.0f,
        0.1f, -0.1f, -20.0f,  20.0f, 0.0f,
        0.1f,  0.1f, -20.0f,  20.0f, 20.0f,
        0.1f,  0.1f, -20.0f,  20.0f, 20.0f,
        -0.1f,  0.1f, -20.0f,  0.0f, 20.0f,
        -0.1f, -0.1f, -20.0f,  0.0f, 0.0f,

        -0.1f, -0.1f,  20.0f,  0.0f, 0.0f,
        0.1f, -0.1f,  20.0f,  20.0f, 0.0f,
        0.1f,  0.1f,  20.0f,  20.0f, 20.0f,
        0.1f,  0.1f,  20.0f,  20.0f, 20.0f,
        -0.1f,  0.1f,  20.0f,  0.0f, 20.0f,
        -0.1f, -0.1f,  20.0f,  0.0f, 0.0f,

        -0.1f,  0.1f,  20.0f,  20.0f, 0.0f,
        -0.1f,  0.1f, -20.0f,  20.0f, 20.0f,
        -0.1f, -0.1f, -20.0f,  0.0f, 20.0f,
        -0.1f, -0.1f, -20.0f,  0.0f, 20.0f,
        -0.1f, -0.1f,  20.0f,  0.0f, 0.0f,
        -0.1f,  0.1f,  20.0f,  20.0f, 0.0f,

        0.1f,  0.1f,  20.0f,  20.0f, 0.0f,
        0.1f,  0.1f, -20.0f,  20.0f, 20.0f,
        0.1f, -0.1f, -20.0f,  0.0f, 20.0f,
        0.1f, -0.1f, -20.0f,  0.0f, 20.0f,
        0.1f, -0.1f,  20.0f,  0.0f, 0.0f,
        0.1f,  0.1f,  20.0f,  20.0f, 0.0f,

        //bottom
        -0.1f, -0.1f, -20.0f,  0.0f, 20.0f,
        0.1f, -0.1f, -20.0f,  20.0f, 20.0f,
        0.1f, -0.1f,  20.0f,  20.0f, 0.0f,
        0.1f, -0.1f,  20.0f,  20.0f, 0.0f,
        -0.1f, -0.1f,  20.0f,  0.0f, 0.0f,
        -0.1f, -0.1f, -20.0f,  0.0f, 20.0f,

        -0.1f,  0.1f, -20.0f,  0.0f, 20.0f,
        0.1f,  0.1f, -20.0f,  20.0f, 20.0f,
        0.1f,  0.1f,  20.0f,  20.0f, 0.0f,
        0.1f,  0.1f,  20.0f,  20.0f, 0.0f,
        -0.1f,  0.1f,  20.0f,  0.0f, 0.0f,
        -0.1f,  0.1f, -20.0f,  0.0f, 20.0f
};

#endif //LEARN_OPENGL_PRIMITIVES_H
