#include "face.h"
#include <math.h>
#include <stdio.h>
#include "servos.h"
#include <iostream>
using namespace std;

// Face
Face face;

Face create_face(int screen_width, int screen_height) {
    Face face;

    // Eyes
    face.eye_left_x = screen_width / 4;
    face.eye_left_y = screen_height / 3;
    face.eye_right_x = (screen_width * 3) / 4;
    face.eye_right_y = screen_height / 3;
    face.eye_width = screen_width / 20;
    face.eye_height = 10;

    // Mouth
    face.mouth_x = screen_width / 4;
    face.mouth_y = (screen_height * 2) / 3;
    face.mouth_width = screen_width / 2;
    face.mouth_height = screen_height / 15;
    face.mouth_smile = screen_height / 30;
    face.mouth_shape = '_';

    return face;
}

void render_face(SDL_Renderer* renderer, Face* face) {
    // Render eyes
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black for eyes
    SDL_Rect left_eye = {face->eye_left_x - face->eye_width / 2, face->eye_left_y - face->eye_height / 2, face->eye_width, face->eye_height};
    SDL_Rect right_eye = {face->eye_right_x - face->eye_width / 2, face->eye_right_y - face->eye_height / 2, face->eye_width, face->eye_height};
    SDL_RenderFillRect(renderer, &left_eye);
    SDL_RenderFillRect(renderer, &right_eye);

    // Render mouth
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black for mouth line
    int thickness = 5; // Thickness of the mouth line
    
    // Different mouth shapes based on phonemes
    switch (face->mouth_shape) {
        case 'M': // Closed mouth
            for (int t = 0; t < thickness; t++) {
                for (int i = 0; i < face->mouth_width; i++) {
                    int y_offset = (int)(face->mouth_smile * sin(M_PI * i / face->mouth_width));
                    SDL_RenderDrawPoint(renderer, face->mouth_x + i, face->mouth_y + y_offset + t);
                }
            }
            break;
            
        case 'F': // Slight opening
            for (int t = 0; t < thickness; t++) {
                // Top lip
                for (int i = 0; i < face->mouth_width; i++) {
                    SDL_RenderDrawPoint(renderer, face->mouth_x + i, face->mouth_y - 5 + t);
                }
                // Bottom lip
                for (int i = 0; i < face->mouth_width; i++) {
                    SDL_RenderDrawPoint(renderer, face->mouth_x + i, face->mouth_y + 15 + t);
                }
            }
            break;
            
        case 'T': // Wide open
            for (int t = 0; t < thickness; t++) {
                // Top lip
                for (int i = 0; i < face->mouth_width; i++) {
                    SDL_RenderDrawPoint(renderer, face->mouth_x + i, face->mouth_y - 25 + t);
                }
                // Bottom lip
                for (int i = 0; i < face->mouth_width; i++) {
                    SDL_RenderDrawPoint(renderer, face->mouth_x + i, face->mouth_y + 25 + t);
                }
            }
            break;
            
        case 'L': // Narrow opening
            for (int t = 0; t < thickness; t++) {
                // Top lip
                for (int i = 0; i < face->mouth_width; i++) {
                    SDL_RenderDrawPoint(renderer, face->mouth_x + i, face->mouth_y - 3 + t);
                }
                // Bottom lip  
                for (int i = 0; i < face->mouth_width; i++) {
                    SDL_RenderDrawPoint(renderer, face->mouth_x + i, face->mouth_y + 5 + t);
                }
            }
            break;
            
        default: // Default closed mouth
            for (int t = 0; t < thickness; t++) {
                for (int i = 0; i < face->mouth_width; i++) {
                    int y_offset = (int)(face->mouth_smile * sin(M_PI * i / face->mouth_width));
                    SDL_RenderDrawPoint(renderer, face->mouth_x + i, face->mouth_y + y_offset + t);
                }
            }
    }
}

void update_face(Face* face, int eye_height, int smile_curve) {
    face->eye_height += eye_height;
    face->mouth_smile += smile_curve;
}

void move_face(int smile) {
    // Move the face
    update_face(&face, 0, smile);
}

