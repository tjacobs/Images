#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <curl/curl.h>

#define TEMP_IMAGE_FILE "generated_image.jpg"
#define API_URL_POST "https://api.bfl.ml/v1/flux-pro-1.1"
#define API_URL_GET "https://api.bfl.ml/v1/get_result?id=%s"
#define API_KEY "d4f34e29-13c0-407d-bfea-86953020f5f3"
#define POST_DATA "{ \"prompt\": \"a wet green dog\", \"width\": 1024, \"height\": 768, \"prompt_upsampling\": true, \"seed\": 42, \"safety_tolerance\": 2, \"output_format\": \"jpeg\" }"

// Buffer to hold the response data from CURL
struct Memory {
    char *response;
    size_t size;
};

bool generate_image(char *image_id);
bool get_image_url(const char *image_id, char *image_url);
bool download_image(const char* url, const char* filename);
size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp);

int main() {
    char image_id[128];
    char image_url[512];

    // Generate the image and get the ID
    if (!generate_image(image_id)) {
        fprintf(stderr, "Failed to generate image\n");
        return 1;
    }
    printf("Generated image ID: %s\n", image_id);

    while (true) {
        sleep(1);

        // Get the URL of the generated image
        if (!get_image_url(image_id, image_url)) {
            continue;
        }
        break;
    }

    // Download the image
    if (!download_image(image_url, TEMP_IMAGE_FILE)) {
        fprintf(stderr, "Failed to download image\n");
        return 1;
    }
    printf("Image downloaded successfully as %s\n", TEMP_IMAGE_FILE);


    // Initialize SDL objects to NULL
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Surface* image = NULL;
    SDL_Texture* texture = NULL;

    // Initialize SDL video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize SDL_image for multiple image formats
    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF) == 0) {
        fprintf(stderr, "SDL_image could not initialize: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create SDL window
    window = SDL_CreateWindow("Image Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create renderer for the window
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Renderer could not be created: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Load downloaded image
    image = IMG_Load(TEMP_IMAGE_FILE);
    if (!image) {
        fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create texture from loaded image
    texture = SDL_CreateTextureFromSurface(renderer, image);
    if (!texture) {
        fprintf(stderr, "Unable to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(image);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Event handling variables
    SDL_Event event;
    bool quit = false;

    // Main event loop
    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
            }
        }

        // Clear renderer
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Render texture
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        // Present renderer
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // Limit frame rate
    }

    // Clean up
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(image);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}


// Callback function for libcurl to write data to a file
size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}

// Function to download image using libcurl
bool download_image(const char* url, const char* filename) {
    CURL* curl;
    FILE* file;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Could not initialize libcurl\n");
        return false;
    }

    file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Could not open file for writing: %s\n", filename);
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    res = curl_easy_perform(curl);

    fclose(file);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to download image: %s\n", curl_easy_strerror(res));
        return false;
    }

    return true;
}

// Callback to write response data into the buffer
size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    struct Memory *mem = (struct Memory *)userp;

    char *ptr = (char*)realloc(mem->response, mem->size + total_size + 1);
    if (!ptr) {
        fprintf(stderr, "Out of memory\n");
        return 0;
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), contents, total_size);
    mem->size += total_size;
    mem->response[mem->size] = '\0';

    return total_size;
}

// Function to perform a POST request and get the generated image ID
bool generate_image(char *image_id) {
    CURL *curl = curl_easy_init();
    struct Memory chunk = {0};

    if (!curl) {
        fprintf(stderr, "Could not initialize CURL\n");
        return false;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "X-Key: " API_KEY);

    curl_easy_setopt(curl, CURLOPT_URL, API_URL_POST);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, POST_DATA);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "POST request failed: %s\n", curl_easy_strerror(res));
        free(chunk.response);
        return false;
    }

    // Parse the response for the "id" field
    const char *id_key = "\"id\":\"";
    char *id_start = strstr(chunk.response, id_key);
    if (!id_start) {
        fprintf(stderr, "No ID found in response\n");
        free(chunk.response);
        return false;
    }

    id_start += strlen(id_key);
    char *id_end = strchr(id_start, '\"');
    if (!id_end) {
        fprintf(stderr, "Invalid ID format\n");
        free(chunk.response);
        return false;
    }

    strncpy(image_id, id_start, id_end - id_start);
    image_id[id_end - id_start] = '\0';

    free(chunk.response);
    return true;
}

// Function to perform a GET request to retrieve the image URL
bool get_image_url(const char *image_id, char *image_url) {
    CURL *curl = curl_easy_init();
    struct Memory chunk = {0};
    char url[256];

    if (!curl) {
        fprintf(stderr, "Could not initialize CURL\n");
        return false;
    }

    snprintf(url, sizeof(url), API_URL_GET, image_id);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "GET request failed: %s\n", curl_easy_strerror(res));
        free(chunk.response);
        return false;
    }

    // Parse the response for the "sample" field
    const char *url_key = "\"sample\":\"";
    char *url_start = strstr(chunk.response, url_key);
    if (!url_start) {
        fprintf(stderr, "Waiting for image... %s\n", chunk.response);
        free(chunk.response);
        return false;
    }

    url_start += strlen(url_key);
    char *url_end = strchr(url_start, '\"');
    if (!url_end) {
        fprintf(stderr, "Invalid URL format\n");
        free(chunk.response);
        return false;
    }

    strncpy(image_url, url_start, url_end - url_start);
    image_url[url_end - url_start] = '\0';

    free(chunk.response);
    return true;
}




