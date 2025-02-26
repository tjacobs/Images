// Main program for robot control.
// Thomas Jacobs, 2025.

#include "image.h"
#include "box.h"
#include "face.h"
#include "servos.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <thread>

// Speech detection
int mainWhisper(int argc, char ** argv);

// Speech generation
int mainBark(int argc, char **argv);

// Window
extern int screen_width;
extern int screen_height;

// Face
Face face;

int main(int argc, char ** argv) {
    // Create window
    if (create_window() != 0) printf("Could not create window\n");

    // Connect to servos
    if (open_servos() != 0) printf("Could not open servos\n");

    // Create a thread for speech recognition
    std::thread speech_detection_thread([argc, argv]() {
        //if (mainWhisper(argc, argv) != 0) exit(-1);
    });
    speech_detection_thread.detach();

    // Create a thread for speech generation
    std::thread speech_generation_thread([argc, argv]() {
        //if (mainBark(argc, argv) != 0) exit(-1);
    });
    speech_generation_thread.detach();

    // Face rendering
    if (true) {
        // Create face
        face = create_face(screen_width, screen_height);

        // Process input
        bool quit = false;
        SDL_Event event;
        while (!quit) {
            // Get events
            while (SDL_PollEvent(&event) != 0) {
                // Quit on ESC
                if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) quit = true;

                // Adjust face with keys
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_UP)    { move_head(0, 40); update_face(&face, 0, 1); }
                    if (event.key.keysym.sym == SDLK_DOWN)  { move_head(0, -40); update_face(&face, 0, -1); }
                    if (event.key.keysym.sym == SDLK_RIGHT) { move_head(-40, 0); }
                    if (event.key.keysym.sym == SDLK_LEFT)  { move_head(40, 0); }
                    if (event.key.keysym.sym == SDLK_SPACE) { move_head(0, -500); update_face(&face, 5, 0); }
                    if (event.key.keysym.sym == SDLK_j)     { move_head(900, 0); update_face(&face, 5, 0); }
                    if (event.key.keysym.sym == SDLK_l)     { move_head(-900, 0); update_face(&face, -5, 0); }
                    if (event.key.keysym.sym == SDLK_i)     { move_head(0, 200); update_face(&face, 5, 0); }
                    if (event.key.keysym.sym == SDLK_k)     { move_head(0, -200); update_face(&face, -5, 0); }
                }
            }

            // Clear the screen
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderClear(renderer);

            // Render the face
            render_face(renderer, &face);

            // Present the updated screen
            SDL_RenderPresent(renderer);

            // Wait
            SDL_Delay(16);
        }

        // Done
        close_window();
        return 0;
    }

    return 0;
}
