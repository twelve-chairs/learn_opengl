//
// Created by air on 12/21/21.
//

#ifndef LEARN_OPENGL_PRIMITIVES_H
#define LEARN_OPENGL_PRIMITIVES_H

//float planeVertices[] = {
//        -planeMax, floorMin , -planeMax, 0.0f, 0.0f,
//        planeMax, floorMin , -planeMax, planeMax, 0.0f,
//        -planeMax, floorMin , planeMax, planeMax/2, planeMax,
//
//        -planeMax, floorMin , planeMax, planeMax, 0.0f,
//        planeMax, floorMin , planeMax, 0.0f, 0.0f,
//        planeMax, floorMin , -planeMax, planeMax/2, planeMax
//};

//const float scale = 1.0f;
//
//float pyramidVertices[] = {
//        -2.0f, -2.0f, 2.0f, scale, 0.0f,
//        2.0f, -2.0f, 2.0f, 0.0f, 0.0f,
//        0.0f, 2.0f, 0.0f, scale/ 2, scale -0.1f, // front face
//        2.0f, -2.0f, 2.0f, scale, 0.0f,
//        2.0f, -2.0f, -2.0f, 0.0f, 0.0f,
//        0.0f, 2.0f, 0.0f, scale/ 2, scale-0.1f, // right face
//        2.0f,-2.0f, -2.0f, scale, 0.0f,
//        -2.0f, -2.0f, -2.0f, 0.0f, 0.0f,
//        0.0f, 2.0f, 0.0f, scale/ 2, scale-0.1f, // back face
//        -2.0f, -2.0f, -2.0f ,scale, 0.0f,
//        -2.0f, -2.0f, 2.0f, 0.0f, 0.0f,
//        0.0f, 2.0f, 0.0f, scale/ 2, scale-0.1f, // left face
//        -2.0f, -2.0f, -2.0f, scale, 0.0f,
//        2.0f, -2.0f, 2.0f, 0.0f, 0.0f,
//        -2.0f, -2.0f, 2.0f, scale/ 2, scale, // base – left front
//        2.0f, -2.0f, 2.0f, scale, 0.0f,
//        -2.0f, -2.0f, -2.0f, 0.0f, 0.0f,
//        2.0f, -2.0f, -2.0f, scale/ 2, scale // base – right back
//};


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
        0.1f, -0.1f, -20.0f,  1.0f, 0.0f,
        0.1f,  0.1f, -20.0f,  1.0f, 1.0f,
        0.1f,  0.1f, -20.0f,  1.0f, 1.0f,
        -0.1f,  0.1f, -20.0f,  0.0f, 1.0f,
        -0.1f, -0.1f, -20.0f,  0.0f, 0.0f,

        -0.1f, -0.1f,  20.0f,  0.0f, 0.0f,
        0.1f, -0.1f,  20.0f,  1.0f, 0.0f,
        0.1f,  0.1f,  20.0f,  1.0f, 1.0f,
        0.1f,  0.1f,  20.0f,  1.0f, 1.0f,
        -0.1f,  0.1f,  20.0f,  0.0f, 1.0f,
        -0.1f, -0.1f,  20.0f,  0.0f, 0.0f,

        -0.1f,  0.1f,  20.0f,  1.0f, 0.0f,
        -0.1f,  0.1f, -20.0f,  1.0f, 1.0f,
        -0.1f, -0.1f, -20.0f,  0.0f, 1.0f,
        -0.1f, -0.1f, -20.0f,  0.0f, 1.0f,
        -0.1f, -0.1f,  20.0f,  0.0f, 0.0f,
        -0.1f,  0.1f,  20.0f,  1.0f, 0.0f,

        0.1f,  0.1f,  20.0f,  1.0f, 0.0f,
        0.1f,  0.1f, -20.0f,  1.0f, 1.0f,
        0.1f, -0.1f, -20.0f,  0.0f, 1.0f,
        0.1f, -0.1f, -20.0f,  0.0f, 1.0f,
        0.1f, -0.1f,  20.0f,  0.0f, 0.0f,
        0.1f,  0.1f,  20.0f,  1.0f, 0.0f,

        //bottom
        -0.1f, -0.1f, -20.0f,  0.0f, 1.0f,
        0.1f, -0.1f, -20.0f,  1.0f, 1.0f,
        0.1f, -0.1f,  20.0f,  1.0f, 0.0f,
        0.1f, -0.1f,  20.0f,  1.0f, 0.0f,
        -0.1f, -0.1f,  20.0f,  0.0f, 0.0f,
        -0.1f, -0.1f, -20.0f,  0.0f, 1.0f,

        -0.1f,  0.1f, -20.0f,  0.0f, 1.0f,
        0.1f,  0.1f, -20.0f,  1.0f, 1.0f,
        0.1f,  0.1f,  20.0f,  1.0f, 0.0f,
        0.1f,  0.1f,  20.0f,  1.0f, 0.0f,
        -0.1f,  0.1f,  20.0f,  0.0f, 0.0f,
        -0.1f,  0.1f, -20.0f,  0.0f, 1.0f
};

//void dda( ){
//    float x;
//    float y;
//    float x1, y1;
//    float x2, y2, dx, dy, step;
//
//    cout << "Enter the value of x1 and y1: ";
//    cin >> x1 >> y1;
//    cout << "Enter the value of x2 and y2: ";
//    cin >> x2 >> y2;
//
//    dx = (x2 - x1);
//    dy = (y2 - y1);
//    if (abs(dx) >= abs(dy))
//        step = abs(dx);
//    else
//        step = abs(dy);
//    dx = dx / step;
//    dy = dy / step;
//    x = x1;
//    y = y1;
//    int i = 1;
//    while (i <= step) {
////        putpixel(x, y, 5);
//        x = x + dx;
//        y = y + dy;
//        i = i + 1;
//    }
//}

#endif //LEARN_OPENGL_PRIMITIVES_H
