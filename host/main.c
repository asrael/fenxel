#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "host.h"
#include "script.c"

typedef struct FXL_Host {
    SDL_Window     *window;
    SDL_Renderer   *renderer;
    FXL_Script      script;
} FXL_Host;

internal const char *app_result_name(SDL_AppResult result) {
    switch(result) {
        case SDL_APP_CONTINUE:  return "SDL_APP_CONTINUE";
        case SDL_APP_SUCCESS:   return "SDL_APP_SUCCESS";
        case SDL_APP_FAILURE:   return "SDL_APP_FAILURE";
    }

    return "unknown";
}

internal void clear(FXL_Host *host, uint32_t rgb) {
    SDL_SetRenderDrawColor(host->renderer, (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF, 0xFF);
    SDL_RenderClear(host->renderer);
}

internal void present(FXL_Host *host) {
    SDL_RenderPresent(host->renderer);
}

internal SDL_AppResult fail(const char *desc) {
    const char *err = SDL_GetError();

    if (!err) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", desc);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", desc, err);
    }

    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    (void)argc;
    (void)argv;

    static FXL_Host state;
    FXL_Host *host = &state;
    *appstate = host;

    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_GAMEPAD | SDL_INIT_VIDEO)) return fail("SDL_Init");
    if (!SDL_CreateWindowAndRenderer("fenxel", 1920, 1080, 0, &host->window, &host->renderer)) {
        return fail("SDL_CreateWindowAndRenderer");
    }

    SDL_SetRenderVSync(host->renderer, 1);
    SDL_SetWindowPosition(host->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    if (!script_init(&host->script)) return fail("script_init failed");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    (void)appstate;

    switch(event->type) {
        case SDL_EVENT_KEY_DOWN: if (event->key.key == SDLK_ESCAPE) return SDL_APP_SUCCESS; break;
        case SDL_EVENT_QUIT: return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    FXL_Host* host = appstate;

    clear(host, 0x1D2021);
    present(host);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    FXL_Host *host = appstate;

    SDL_Log("quit result: %s", app_result_name(result));

    if (host) script_quit(&host->script);
}
