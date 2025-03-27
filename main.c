#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <strings.h>
#include <time.h>

#define FACTOR 120
#define FPS 60
#define START_SPEED 4
#define START_SPEED_B 15
#define WINDOW_WIDTH  16*FACTOR
#define WINDOW_HEIGHT  9*FACTOR
#define SOIL_HEIGHT FACTOR*60/100
#define SOIL_Y WINDOW_HEIGHT/10
#define DINO_W FACTOR*346/100
#define DINO_H FACTOR*376/100
#define BIRD_W FACTOR*98/100
#define BIRD_H FACTOR*55/100
#define CACTUS_H FACTOR
#define CACTUS_1W FACTOR*51/100
#define CACTUS_2W FACTOR*98/100
#define CACTUS_3W FACTOR*103/100
#define CLOUD_W FACTOR*166/100
#define CLOUD_H FACTOR*74/100
#define GUN_H FACTOR*60/100
#define GUN_W FACTOR*111/100
#define BULLET_H FACTOR*10/100
#define BULLET_W FACTOR*24/100

#define PI 3.14159265358979323846

float SPEED = START_SPEED/(FPS/60.0f);
float BULLET_SPEED = START_SPEED_B/(FPS/60.0f);

#define LOG(...) (SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, __VA_ARGS__))

#define CHECK_ERROR_int(code, state) do {                   \
    typeof(code) _code = (code);                            \
    if (_code != 0) {                                       \
        const char *error = SDL_GetError();                 \
        printf("Line: %d, Error: %s\n", __LINE__, error);   \
        state->CLOSE = true;                                \
    }} while (0)


#define CHECK_ERROR_ptr(code, state) do {                   \
    if (code == NULL) {                \
        const char *error = SDL_GetError();                 \
        printf("Line: %d, Error: %s\n", __LINE__, error);   \
        state->CLOSE = true;                                \
    }} while (0)


typedef struct {
    SDL_Rect src;  
    SDL_Rect dst;
    SDL_Surface *srf;
    SDL_Texture *txt;
} Asset;

typedef struct {
    SDL_Rect src;  
    SDL_Rect dst;
    SDL_Point rot_c;
    SDL_Surface *srf;
    SDL_Texture *txt;
    float angle;
} AssetRot;

typedef struct {
    Asset *Dino;
    SDL_Texture *Dinos_txt;
    SDL_Surface *Dinos_srf;

    Asset *Gun;

    Asset *Back_1;
    Asset *Back_2;
    Asset *Back_3;
    SDL_Texture *Backs[3];

    Asset *Bird_Up;
    Asset *Bird_Down;
    
    Asset *Cactus_1;
    Asset *Cactus_2;
    Asset *Cactus_3;

    Asset *Cloud;
    AssetRot *Bullet;
} Assets;

typedef struct {
    size_t Dino_start;
    size_t Bird_spawn;
    size_t Bird_flap;
    size_t Cactus_spawn;
    size_t Cloud_spawn;
    size_t Last_added_bullet;
} Animations_start;

typedef struct {
    Asset **data;
    size_t size;
    size_t count;
} DArrayOfEntities;

typedef struct {
    AssetRot **data;
    size_t size;
    size_t count;
} DArrayOfBullets;

typedef struct{
    int x;
    int y;
    int w;
    int h;
} Particle;

typedef struct {
    Particle **particles;
    int cx;
    int cy;
    size_t count;
    size_t size;
} DArrayOfParticles;

typedef struct {
    DArrayOfParticles **clusters;
    size_t count;
    size_t size;
} DArrayOfParticlesCLusters;


typedef enum {
    DA_TYPE_ENTITIES,
    DA_TYPE_BULLETS,
    DA_TYPE_PARTICLES,
    DA_TYPE_CLUSTERS
} DAtype;

typedef struct {
    union {
        DArrayOfEntities *DAEe;
        DArrayOfBullets *DAEb;
        DArrayOfParticles *DAEp;
        DArrayOfParticlesCLusters *DAEpc;
    } ptr;
    DAtype type;
} DA;

typedef struct {
    bool START;
    bool CLOSE;
    bool PAUSE;
    bool RESTART;
    bool GAMEOVER;
    size_t POINTS;
    size_t AMMO;
} State;

typedef struct{
    Mix_Chunk *shot_sound;
    Mix_Chunk *stepl_sound;
    Mix_Chunk *stepr_sound;
    Mix_Chunk *death_sound;
} Sounds;


void cap_fps(size_t t1, size_t t2) {
    size_t frametime = 1000/(FPS);
    if (t2 - t1 < frametime) {
        SDL_Delay(frametime - (t2 - t1));
    }
}

void init_assets(SDL_Renderer *renderer, Assets* A) {
    A->Back_1 = (Asset*)malloc(sizeof(Asset));
    A->Back_1->src = (SDL_Rect){.x=0, .y=0, .h=60, .w=WINDOW_WIDTH};
    A->Back_1->dst = (SDL_Rect){.x=0, .y=WINDOW_HEIGHT - SOIL_HEIGHT - SOIL_Y, .h=SOIL_HEIGHT, .w=WINDOW_WIDTH};
    A->Back_1->srf = IMG_Load("./assets/img/back_1.png");
    A->Back_1->txt = SDL_CreateTextureFromSurface(renderer, A->Back_1->srf);

    A->Back_2 = (Asset*)malloc(sizeof(Asset));
    A->Back_2->src = (SDL_Rect){.x=0, .y=0, .h=60, .w=WINDOW_WIDTH};
    A->Back_2->dst = (SDL_Rect){.x=WINDOW_WIDTH, .y=WINDOW_HEIGHT - SOIL_HEIGHT - SOIL_Y, .h=SOIL_HEIGHT, .w=WINDOW_WIDTH};
    A->Back_2->srf = IMG_Load("./assets/img/back_2.png");
    A->Back_2->txt = SDL_CreateTextureFromSurface(renderer, A->Back_2->srf);


    A->Back_3 = (Asset*)malloc(sizeof(Asset));
    A->Back_3->src = (SDL_Rect){.x=0, .y=0, .h=60, .w=WINDOW_WIDTH};
    // A->Back_3->dst = (SDL_Rect){.x=WINDOW_WIDTH, .y=WINDOW_HEIGHT - SOIL_HEIGHT - SOIL_Y, .h=SOIL_HEIGHT, .w=WINDOW_WIDTH};
    // No need for dst, since it will be used for the texture, not to display it
    A->Back_3->srf = IMG_Load("./assets/img/back_3.png");
    A->Back_3->txt = SDL_CreateTextureFromSurface(renderer, A->Back_3->srf);
    A->Backs[0] = A->Back_1->txt;
    A->Backs[1] = A->Back_2->txt;
    A->Backs[2] = A->Back_3->txt;

    A->Dino = (Asset*)malloc(sizeof(Asset));
    A->Dino->src = (SDL_Rect){.x=0, .y=0, .h=286, .w=232};
    A->Dino->dst = (SDL_Rect){.x=WINDOW_WIDTH/10, .y=WINDOW_HEIGHT - SOIL_HEIGHT - SOIL_Y - DINO_H*5/6, .h=DINO_H, .w=DINO_W};
    A->Dino->srf = IMG_Load("./assets/img/dino_l.png");
    A->Dinos_srf = IMG_Load("./assets/img/dino_r.png");
    A->Dino->txt = SDL_CreateTextureFromSurface(renderer, A->Dino->srf);
    A->Dinos_txt = SDL_CreateTextureFromSurface(renderer, A->Dinos_srf);

    A->Gun = (Asset*)malloc(sizeof(Asset));
    A->Gun->src = (SDL_Rect){.x=0, .y=0, .h=388, .w=750};
    A->Gun->dst = (SDL_Rect){.x=WINDOW_WIDTH/10 + DINO_W*35/48, .y=WINDOW_HEIGHT - SOIL_HEIGHT - SOIL_Y - DINO_H*0.4, .h=GUN_H, .w=GUN_W};
    A->Gun->srf = IMG_Load("./assets/img/gun.png");
    A->Gun->txt = SDL_CreateTextureFromSurface(renderer, A->Gun->srf);

    A->Bird_Down = (Asset*)malloc(sizeof(Asset));
    A->Bird_Down->src = (SDL_Rect){.x=0, .y=0, .h=55, .w=98};
    A->Bird_Down->dst = (SDL_Rect){.x=WINDOW_WIDTH + 100, .y=WINDOW_HEIGHT/2, .h=BIRD_H, .w=BIRD_W};
    A->Bird_Down->srf = IMG_Load("./assets/img/bird_down.png");
    A->Bird_Down->txt = SDL_CreateTextureFromSurface(renderer, A->Bird_Down->srf);

    A->Bird_Up = (Asset*)malloc(sizeof(Asset));
    A->Bird_Up->src = (SDL_Rect){.x=0, .y=0, .h=55, .w=98};
    A->Bird_Up->dst = (SDL_Rect){.x=WINDOW_WIDTH + 100, .y=WINDOW_HEIGHT/2, .h=BIRD_H, .w=BIRD_W};
    A->Bird_Up->srf = IMG_Load("./assets/img/bird_up.png");
    A->Bird_Up->txt = SDL_CreateTextureFromSurface(renderer, A->Bird_Up->srf);

    A->Cactus_1 = (Asset*)malloc(sizeof(Asset));
    A->Cactus_1->src = (SDL_Rect){.x=0, .y=0, .h=100, .w=51};
    A->Cactus_1->dst = (SDL_Rect){.x=WINDOW_WIDTH + 150, .y=WINDOW_HEIGHT - SOIL_HEIGHT - SOIL_Y - CACTUS_H*0.5, .h=CACTUS_H, .w=CACTUS_1W};
    A->Cactus_1->srf = IMG_Load("./assets/img/cactus_1.png");
    A->Cactus_1->txt = SDL_CreateTextureFromSurface(renderer, A->Cactus_1->srf);

    A->Cactus_2 = (Asset*)malloc(sizeof(Asset));
    A->Cactus_2->src = (SDL_Rect){.x=0, .y=0, .h=100, .w=98};
    A->Cactus_2->dst = (SDL_Rect){.x=WINDOW_WIDTH + 150, .y=WINDOW_HEIGHT - SOIL_HEIGHT - SOIL_Y - CACTUS_H*0.5, .h=CACTUS_H, .w=CACTUS_2W};
    A->Cactus_2->srf = IMG_Load("./assets/img/cactus_2.png");
    A->Cactus_2->txt = SDL_CreateTextureFromSurface(renderer, A->Cactus_2->srf);

    A->Cactus_3 = (Asset*)malloc(sizeof(Asset));
    A->Cactus_3->src = (SDL_Rect){.x=0, .y=0, .h=100, .w=103};
    A->Cactus_3->dst = (SDL_Rect){.x=WINDOW_WIDTH + 150, .y=WINDOW_HEIGHT - SOIL_HEIGHT - SOIL_Y - CACTUS_H*0.5, .h=CACTUS_H, .w=CACTUS_3W};
    A->Cactus_3->srf = IMG_Load("./assets/img/cactus_3.png");
    A->Cactus_3->txt = SDL_CreateTextureFromSurface(renderer, A->Cactus_3->srf);

    A->Cloud = (Asset*)malloc(sizeof(Asset));
    A->Cloud->src = (SDL_Rect){.x=0, .y=0, .h=37, .w=83};
    A->Cloud->dst = (SDL_Rect){.x=WINDOW_WIDTH + 150, .y=WINDOW_HEIGHT/2, .h=CLOUD_H, .w=CLOUD_W};
    A->Cloud->srf = IMG_Load("./assets/img/cloud.png");
    A->Cloud->txt = SDL_CreateTextureFromSurface(renderer, A->Cloud->srf);

    A->Bullet = (AssetRot*)malloc(sizeof(AssetRot));
    A->Bullet->src = (SDL_Rect){.x=0, .y=0, .h=20, .w=48};
    A->Bullet->dst = (SDL_Rect){.x=0, .y=0, .h=BULLET_H, .w=BULLET_W};
    A->Bullet->angle = 0.0f;
    A->Bullet->rot_c = (SDL_Point) {.x = 0, .y = 0};
    A->Bullet->srf = IMG_Load("./assets/img/bullet.png");
    A->Bullet->txt = SDL_CreateTextureFromSurface(renderer, A->Bullet->srf);

}

void destroy_assets(Assets *A) {
    Asset *arrayOfAssets[9] = {A->Dino, A->Back_1, A->Back_2, A->Back_3, A->Bird_Down, A->Bird_Up, A->Cactus_1, A->Cactus_2, A->Cactus_3};
    for (int x=0; x<9; x++) {
        Asset *ptr = arrayOfAssets[x];
        if (ptr && ptr->srf) SDL_FreeSurface(ptr->srf);
        if (ptr && ptr->txt) SDL_DestroyTexture(ptr->txt);
        if (ptr) free(ptr);
    }
    if (A->Backs[0]) SDL_DestroyTexture(A->Backs[0]);
    if (A->Backs[1]) SDL_DestroyTexture(A->Backs[1]);
    if (A->Backs[2]) SDL_DestroyTexture(A->Backs[2]);
}

void init_DA(DA *DA){
    if (DA->type == DA_TYPE_ENTITIES) {
            DArrayOfEntities *d = (DArrayOfEntities*)malloc(sizeof(DArrayOfEntities));
            d->count = 0;
            d->size = 20;
            d->data = (Asset**)malloc(sizeof(Asset*)*20);
            memset(d->data, 0, sizeof(d->data)*20);
            DA->ptr.DAEe = d;
    } else if (DA->type == DA_TYPE_BULLETS) {
            DArrayOfBullets *d = (DArrayOfBullets*)malloc(sizeof(DArrayOfBullets));
            d->count = 0;
            d->size = 20;
            d->data = (AssetRot**)malloc(sizeof(AssetRot*)*20);
            memset(d->data, 0, sizeof(d->data)*20);
            DA->ptr.DAEb = d;
    } else if (DA->type == DA_TYPE_PARTICLES) {
            DArrayOfParticles *d = (DArrayOfParticles*)malloc(sizeof(DArrayOfParticles));
            d->count = 0;
            d->size = 9;
            d->particles = (Particle**)malloc(sizeof(Particle*)*9);
            memset(d->particles, 0, sizeof(d->particles)*9);
            DA->ptr.DAEp = d;
    } else if (DA->type == DA_TYPE_CLUSTERS) {
            DArrayOfParticlesCLusters *d = (DArrayOfParticlesCLusters*)malloc(sizeof(DArrayOfParticlesCLusters));
            d->count = 0;
            d->size = 20;
            d->clusters = (DArrayOfParticles**)malloc(sizeof(DArrayOfParticles*)*20);
            memset(d->clusters, 0, sizeof(d->clusters)*20);
            DA->ptr.DAEpc = d;

    }
}

void uninit_DA(DA *DA) {
    switch(DA->type) {
        case DA_TYPE_ENTITIES:
            for (size_t x = 0; x < DA->ptr.DAEe->size; x++) {
                if (DA->ptr.DAEe->data[x]) free(DA->ptr.DAEe->data[x]);
            }
            free(DA->ptr.DAEe);
            break;
        case DA_TYPE_BULLETS: 
            for (size_t x = 0; x < DA->ptr.DAEb->size; x++) {
                if (DA->ptr.DAEb->data[x]) free(DA->ptr.DAEb->data[x]);
            }
            free(DA->ptr.DAEb);
            break;
        case DA_TYPE_PARTICLES:
            for (size_t x = 0; x < DA->ptr.DAEp->size; x++) {
                if (DA->ptr.DAEp->particles[x]) free(DA->ptr.DAEp->particles[x]);
            }
            free(DA->ptr.DAEp);
            break;
        case DA_TYPE_CLUSTERS:
            for (size_t x = 0; x < DA->ptr.DAEpc->size; x++) {
                if (DA->ptr.DAEpc->clusters[x]) free(DA->ptr.DAEpc->clusters[x]);
            }
            free(DA->ptr.DAEpc);
            break;
        
        default:
            break;
    }
}

float get_angle2(int ax, int ay, int bx, int by) {
    float angle = atan2(ay - by, ax - bx) * 180/PI + 180;
    return angle;
    printf("%f", angle);
}

void display_dino_back_gun_cloud(State *state, SDL_Renderer *renderer, DArrayOfEntities *DAE, Assets *A) {
    for (size_t x = 0; x < DAE->size; x++) {
        if (DAE->data[x] && DAE->data[x]->txt == A->Cloud->txt) {
            Asset *current = DAE->data[x];
            CHECK_ERROR_int(SDL_RenderCopy(renderer, current->txt, &current->src, &current->dst), state);
        }
    }

    
    Asset *arrayOfAssets[4] = {A->Back_1, A->Back_2, A->Dino, A->Gun};
    for (int x=0; x<4; x++) {
        Asset *ptr = arrayOfAssets[x];
        if (ptr && ptr->txt == A->Gun->txt){
            SDL_Point c = {
                .x = GUN_W/8,
                .y = GUN_H*2/3
            };

            int mouse_x;
            int mouse_y;
            SDL_GetMouseState(&mouse_x, &mouse_y);

            int gun_rot_cx = A->Gun->dst.x + c.x;
            int gun_rot_cy = A->Gun->dst.y + c.y;
            
            
            CHECK_ERROR_int(SDL_RenderCopyEx(renderer, ptr->txt, &ptr->src, &ptr->dst, get_angle2(gun_rot_cx, gun_rot_cy, mouse_x, mouse_y), &c, SDL_FLIP_NONE), state);
            continue;
        }
        if (ptr && ptr->txt) {
            CHECK_ERROR_int(SDL_RenderCopy(renderer, ptr->txt, &ptr->src, &ptr->dst), state);
        };
    }
}

void display_entities(State *state, Assets *A, SDL_Renderer *renderer, DArrayOfEntities *DAE) {
    for (size_t x = 0; x < DAE->size; x++) {
        if (DAE->data[x] && DAE->data[x]->txt != A->Cloud->txt) {
            Asset *current = DAE->data[x];
            CHECK_ERROR_int(SDL_RenderCopy(renderer, current->txt, &current->src, &current->dst), state);
        }
    }
}

void display_bullets(State *state, SDL_Renderer *renderer, DArrayOfBullets *Bullets) {
     for (size_t x = 0; x < Bullets->size; x++) {
        if (Bullets->data[x]) {
            AssetRot *current = Bullets->data[x];
            CHECK_ERROR_int(SDL_RenderCopyEx(renderer, current->txt, &current->src, &current->dst, current->angle, &current->rot_c, SDL_FLIP_NONE), state);
        }
    }
}

void display_particles(State *state, SDL_Renderer *renderer, DArrayOfParticlesCLusters *Clusters) {
    for (size_t x = 0; x < Clusters->size; x++) {
        DArrayOfParticles *c = Clusters->clusters[x];
        if (c != NULL) {
            for (int y = 0; y < c->size; y++) {
                Particle *p = c->particles[y];
                if (p != NULL) {
                    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
                    SDL_Rect r = {
                        .x = p->x,
                        .y = p->y,
                        .w = p->w,
                        .h = p->h
                    };
                    SDL_RenderDrawRect(renderer, &r);
                    SDL_RenderFillRect(renderer, &r);
                    SDL_SetRenderDrawColor(renderer, 255,255,255,255);
                    LOG("particle %d x: %d, y:%d", y, p->x, p->y);
                }
            }
        }
    }
}

void display_points(SDL_Renderer *renderer, State *state, TTF_Font *font) {
    size_t n_numbers;
    char *points;
    if (state->POINTS == 0) {
        n_numbers = 8;
        points = "SCORE: 0";
    } else {
        n_numbers = floorf(log10(state->POINTS)) + 9;
        points = malloc(sizeof(char)*(n_numbers + 7));
        sprintf(points, "SCORE: ");
        SDL_itoa(state->POINTS, points + 7, 10);
    }
    
    SDL_Surface *srf = TTF_RenderText_Solid(font, points, (SDL_Color) {0, 0, 0, 255});
    CHECK_ERROR_ptr(srf, state);

    SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, srf);
    SDL_Rect dst = {
        .w = n_numbers * FACTOR*30/100,
        .h =  FACTOR*50/100,
        .x = WINDOW_WIDTH - n_numbers * FACTOR*30/100 - FACTOR*30/100,
        .y = FACTOR*10/100
    };

    CHECK_ERROR_int(SDL_RenderCopy(renderer, txt, NULL, &dst), state);
    if (state->POINTS != 0) free(points);
    SDL_FreeSurface(srf);
    SDL_DestroyTexture(txt);
}

void display_ammo(SDL_Renderer *renderer, State *state, TTF_Font *font) {
    size_t n_numbers;
    char *ammo;
    if (state->AMMO == 0) {
        n_numbers = 8;
        ammo = "AMMO: 0";
    } else {
        n_numbers = floorf(log10(state->AMMO)) + 8;
        ammo = malloc(sizeof(char)*(n_numbers + 6));
        sprintf(ammo, "AMMO: ");
        SDL_itoa(state->AMMO, ammo + 6, 10);
    }
    
    SDL_Surface *srf = TTF_RenderText_Solid(font, ammo, (SDL_Color) {0, 0, 0, 255});
    CHECK_ERROR_ptr(srf, state);

    SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, srf);
    SDL_Rect dst = {
        .w = n_numbers * FACTOR*30/100,
        .h =  FACTOR*50/100,
        .x = WINDOW_WIDTH/20,
        .y = FACTOR*10/100
    };

    CHECK_ERROR_int(SDL_RenderCopy(renderer, txt, NULL, &dst), state);
    if (state->AMMO != 0) free(ammo);
    SDL_FreeSurface(srf);
    SDL_DestroyTexture(txt);
}

void display(State *state, SDL_Renderer *renderer, DArrayOfEntities *DAE, DArrayOfBullets *Bullets, DArrayOfParticlesCLusters *Clusters, Assets *A, TTF_Font *font) {
        display_dino_back_gun_cloud(state, renderer, DAE, A);
        display_entities(state, A, renderer, DAE);
        display_bullets(state, renderer, Bullets);
        display_particles(state, renderer, Clusters);
        display_points(renderer, state, font);
        display_ammo(renderer, state, font);

}

void display_menu(SDL_Renderer *renderer, State *state, TTF_Font *font) {
    char *text = "                  Press [ESC] to pause or exit (if already paused)\n\
                  Press [SPACE] to shoot\n\
                  Press [P] to pause or resume\n\
                  Press [R] to restart";
    SDL_Surface *srf = TTF_RenderText_Solid_Wrapped(font, text, (SDL_Color) {0, 0, 0, 255}, 0);
    CHECK_ERROR_ptr(srf, state);

    SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, srf);
    SDL_Rect dst = {
        .w = 48 * FACTOR*20/100,
        .h =  FACTOR*40*4/100,
        .x = WINDOW_WIDTH/2 - 25 * FACTOR*20/100,
        .y = WINDOW_HEIGHT*3/7
    };

    CHECK_ERROR_int(SDL_RenderCopy(renderer, txt, NULL, &dst), state);
    SDL_FreeSurface(srf);
    SDL_DestroyTexture(txt);
}

void display_start(SDL_Renderer *renderer, State *state, TTF_Font *font) {
    char *text = "PRESS SPACE TO START";
    SDL_Surface *srf = TTF_RenderText_Solid(font, text, (SDL_Color) {0, 0, 0, 255});
    CHECK_ERROR_ptr(srf, state);

    SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, srf);
    SDL_Rect dst = {
        .w = 21 * FACTOR*50/100,
        .h =  FACTOR,
        .x = WINDOW_WIDTH/2 - 10 * FACTOR*50/100,
        .y = WINDOW_HEIGHT/4
    };

    CHECK_ERROR_int(SDL_RenderCopy(renderer, txt, NULL, &dst), state);
    SDL_FreeSurface(srf);
    SDL_DestroyTexture(txt);
}

void display_gameover(SDL_Renderer *renderer, State *state, TTF_Font *font) {
    char *text = "GAMEOVER!";
    char *subtext = "press [R] to restart or [ESC] to exit";

    SDL_Surface *srf = TTF_RenderText_Solid_Wrapped(font, text, (SDL_Color) {0, 0, 0, 255}, 0);
    CHECK_ERROR_ptr(srf, state);

    SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, srf);
    SDL_Rect dst = {
        .w = 10 * FACTOR*50/100,
        .h =  FACTOR,
        .x = WINDOW_WIDTH/2 - 5 * FACTOR*50/100,
        .y = WINDOW_HEIGHT/4
    };

    CHECK_ERROR_int(SDL_RenderCopy(renderer, txt, NULL, &dst), state);
    SDL_FreeSurface(srf);
    SDL_DestroyTexture(txt);

    srf = TTF_RenderText_Solid_Wrapped(font, subtext, (SDL_Color) {0, 0, 0, 255}, 0);
    CHECK_ERROR_ptr(srf, state);

    txt = SDL_CreateTextureFromSurface(renderer, srf);
    SDL_Rect dst_s = {
        .w = 38 * FACTOR*20/100,
        .h =  FACTOR*50/100,
        .x = WINDOW_WIDTH/2 - 19 * FACTOR*20/100,
        .y = WINDOW_HEIGHT*3/7
    };

    CHECK_ERROR_int(SDL_RenderCopy(renderer, txt, NULL, &dst_s), state);
    SDL_FreeSurface(srf);
    SDL_DestroyTexture(txt);

}

void display_pause(SDL_Renderer *renderer, State *state, TTF_Font *font) {
    char *text = "PAUSE";
    SDL_Surface *srf = TTF_RenderText_Solid(font, text, (SDL_Color) {0, 0, 0, 255});
    CHECK_ERROR_ptr(srf, state);

    SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, srf);
    SDL_Rect dst = {
        .w = 5 * FACTOR*50/100,
        .h = FACTOR,
        .x = WINDOW_WIDTH/2 - 3 * FACTOR*50/100,
        .y = WINDOW_HEIGHT/4
    };

    CHECK_ERROR_int(SDL_RenderCopy(renderer, txt, NULL, &dst), state);
    SDL_FreeSurface(srf);
    SDL_DestroyTexture(txt);
}

void animate_soil(Assets *A)  {
    int *x1 = &A->Back_1->dst.x;
    int *x2 = &A->Back_2->dst.x;
    *x1 -= (int)(ceil(SPEED));
    *x2 -=(int)(ceil(SPEED));

    if (*x1 <= -WINDOW_WIDTH) {
        *x1 = WINDOW_WIDTH;
        A->Back_1->txt = A->Backs[rand()%3];
    } 

    if (*x2 <= -WINDOW_WIDTH) {
        *x2 = WINDOW_WIDTH;
        A->Back_2->txt = A->Backs[rand()%3];
    }
}

void animate_dino(Assets* A, Animations_start *starts, size_t now, Sounds *sounds) {
    if (now - starts->Dino_start >= (size_t)1000/(int)(ceil(SPEED))) {
        starts->Dino_start = SDL_GetTicks();
        SDL_Texture *tmp = A->Dinos_txt;
        A->Dinos_txt = A->Dino->txt;
        A->Dino->txt = tmp;
        Mix_PlayChannel(-1, sounds->stepl_sound, 0);
        Mix_Chunk *tmpc = sounds->stepl_sound;
        sounds->stepl_sound = sounds->stepr_sound;
        sounds->stepr_sound = tmpc;
    } else if (starts->Dino_start > now){
        starts->Dino_start = SDL_GetTicks();
    }
    
}

void animate_entities(Assets *A, DArrayOfEntities *DAE, Animations_start *starts, size_t now, State *state, Sounds *sounds) {
    bool reset_t = false;
    int dino_x = WINDOW_WIDTH/10 + DINO_W;
    int dino_y = WINDOW_HEIGHT - SOIL_HEIGHT - SOIL_Y - DINO_H + (DINO_H - DINO_H*200/286);
    for (size_t x=0; x < DAE->size; x++) {
        if (DAE->data[x] && (DAE->data[x]->txt == A->Bird_Up->txt || DAE->data[x]->txt == A->Bird_Down->txt)) {
            if (DAE->data[x]->dst.x <= dino_x){
                state->GAMEOVER = true;
                Mix_PlayChannel(-1, sounds->death_sound, 0);
                continue;
            }

            int dino_x_dist = DAE->data[x]->dst.x - dino_x;
            int dino_y_dist = DAE->data[x]->dst.y - dino_y;
            
            if (dino_x_dist > 0 && DAE->data[x]->dst.x <= rand()%(WINDOW_WIDTH/2) + WINDOW_WIDTH/2) {
                float dino_x_norm = (float)dino_x_dist/(dino_x_dist + abs(dino_y_dist));
                float dino_y_norm = (float)dino_y_dist/(dino_x_dist + abs(dino_y_dist));
                DAE->data[x]->dst.y -= (int)floor(SPEED*dino_y_norm);
                DAE->data[x]->dst.x -= (int)floor(SPEED*dino_x_norm);
            } else {
                DAE->data[x]->dst.x -=(int)(ceil(SPEED));
            }

            if (now - starts->Bird_flap >= 300) {
                if(DAE->data[x]->txt == A->Bird_Down->txt) {
                    DAE->data[x]->txt = A->Bird_Up->txt;
                } else if (DAE->data[x]->txt == A->Bird_Up->txt){
                    DAE->data[x]->txt = A->Bird_Down->txt;
                }
                reset_t = true;
            }
        } else if (DAE->data[x] && (DAE->data[x]->txt == A->Cactus_1->txt || DAE->data[x]->txt == A->Cactus_2->txt || DAE->data[x]->txt == A->Cactus_3->txt) ) {
            if (DAE->data[x]->dst.x <= dino_x - dino_x/4){
                state->GAMEOVER = true;
                Mix_PlayChannel(-1, sounds->death_sound, 0);
                continue;
            }
            DAE->data[x]->dst.x -=(int)(ceil(SPEED));
        } else if (DAE->data[x] && DAE->data[x]->txt == A->Cloud->txt){
            if (DAE->data[x]->dst.x <= -CLOUD_W){
                free(DAE->data[x]);
                DAE->data[x] = NULL;
                DAE->count--;
                continue;
            }
            DAE->data[x]->dst.x -=(int)(ceil(SPEED));
        }
    }
    if (reset_t) starts->Bird_flap = SDL_GetTicks();
}

void animate_bullets(DArrayOfBullets *DAE) {
    for (size_t x = 0; x < DAE->size; x++) {
        if (DAE->data[x]) {
            if (DAE->data[x]->dst.x >= WINDOW_WIDTH || DAE->data[x]->dst.x <= -BULLET_W || DAE->data[x]->dst.y >= WINDOW_HEIGHT || DAE->data[x]->dst.y <= -BULLET_H) {
                free(DAE->data[x]);
                DAE->data[x] = NULL;
                DAE->count--;
                continue;
            }
            DAE->data[x]->dst.x += 2*(int)ceil(BULLET_SPEED)*cosf(DAE->data[x]->angle/180*PI);
            DAE->data[x]->dst.y += 2*(int)ceil(BULLET_SPEED)*sinf(DAE->data[x]->angle/180*PI);
        }
    }
}

void animate(Assets *A, DArrayOfEntities *DAE, DArrayOfBullets *Bullets, State *state, Animations_start *starts, size_t now, Sounds *sounds) {      
    animate_soil(A);
    animate_dino(A, starts, now, sounds);
    animate_entities(A, DAE, starts, now, state, sounds);
    animate_bullets(Bullets);
}

void DA_append(DA *DA, void *ent) {
    switch (DA->type) {
        case  DA_TYPE_ENTITIES:
            if (DA->ptr.DAEe->count == DA->ptr.DAEe->size - 1) {
                DA->ptr.DAEe->data = (Asset**)realloc(DA->ptr.DAEe->data, sizeof(Asset*)*DA->ptr.DAEe->size*2);
                memset(DA->ptr.DAEe->data + DA->ptr.DAEe->size, 0, sizeof(Asset*)*DA->ptr.DAEe->size);
                DA->ptr.DAEe->size *= 2;
                DA->ptr.DAEe->data[DA->ptr.DAEe->count] = (Asset*)ent;
                DA->ptr.DAEe->count++;
                return;
            }
            for (size_t x=0; x < DA->ptr.DAEe->size; x++) {
                if (DA->ptr.DAEe->data[x] == NULL) {
                    DA->ptr.DAEe->data[x] = (Asset*)ent;
                    DA->ptr.DAEe->count++;
                    return;
                }
            }
            break;
        case DA_TYPE_BULLETS:
            if (DA->ptr.DAEb->count == DA->ptr.DAEb->size - 1) {
                DA->ptr.DAEb->data = (AssetRot**)realloc(DA->ptr.DAEb->data, sizeof(AssetRot*)*DA->ptr.DAEb->size*2);
                memset(DA->ptr.DAEb->data + DA->ptr.DAEb->size, 0, sizeof(AssetRot*)*DA->ptr.DAEb->size);
                DA->ptr.DAEb->size *= 2;
                DA->ptr.DAEb->data[DA->ptr.DAEb->count] = (AssetRot*)ent;
                DA->ptr.DAEb->count++;
                return;
            }
            for (size_t x=0; x < DA->ptr.DAEb->size; x++) {
                if (DA->ptr.DAEb->data[x] == NULL) {
                    DA->ptr.DAEb->data[x] = (AssetRot*)ent;
                    DA->ptr.DAEb->count++;
                    return;
                }
            }
            break;
        case DA_TYPE_PARTICLES:
            if (DA->ptr.DAEp->count == DA->ptr.DAEp->size - 1) {
                DA->ptr.DAEp->particles = (Particle**)realloc(DA->ptr.DAEp->particles, sizeof(Particle*)*DA->ptr.DAEp->size*2);
                memset(DA->ptr.DAEp->particles + DA->ptr.DAEp->size, 0, sizeof(Particle*)*DA->ptr.DAEp->size);
                DA->ptr.DAEp->size *= 2;
                DA->ptr.DAEp->particles[DA->ptr.DAEp->count] = (Particle*)ent;
                DA->ptr.DAEp->count++;
                return;
            }
            for (size_t x=0; x < DA->ptr.DAEp->size; x++) {
                if (DA->ptr.DAEp->particles[x] == NULL) {
                    DA->ptr.DAEp->particles[x] = (Particle*)ent;
                    DA->ptr.DAEp->count++;
                    return;
                }
            }
            break;
        case DA_TYPE_CLUSTERS:
            if (DA->ptr.DAEpc->count == DA->ptr.DAEpc->size - 1) {
                DA->ptr.DAEpc->clusters = (DArrayOfParticles**)realloc(DA->ptr.DAEpc->clusters, sizeof(DArrayOfParticles*)*DA->ptr.DAEpc->size*2);
                memset(DA->ptr.DAEpc->clusters + DA->ptr.DAEpc->size, 0, sizeof(DArrayOfParticles*)*DA->ptr.DAEpc->size);
                DA->ptr.DAEpc->size *= 2;
                DA->ptr.DAEpc->clusters[DA->ptr.DAEpc->count] = (DArrayOfParticles*)ent;
                DA->ptr.DAEpc->count++;
                return;
            }
            for (size_t x=0; x < DA->ptr.DAEpc->size; x++) {
                if (DA->ptr.DAEpc->clusters[x] == NULL) {
                    DA->ptr.DAEpc->clusters[x] = (DArrayOfParticles*)ent;
                    DA->ptr.DAEpc->count++;
                    return;
                }
            }
    }
    
}

void spawn_bird(Assets *A, DA *DAE) {
    Asset *bird = (Asset*)malloc(sizeof(Asset));
    if (rand()%2) {
        bird->txt = A->Bird_Down->txt;
    } else {
        bird->txt = A->Bird_Up->txt;
    }
    bird->dst = A->Bird_Down->dst;
    bird->src = A->Bird_Down->src;
    bird->dst.y = rand() % (WINDOW_HEIGHT - SOIL_HEIGHT - SOIL_Y - BIRD_H * 3);
    DA_append(DAE, (void*)bird);

}

void spawn_cacti(Assets *A, DA* DAE) {
    Asset *cactus = (Asset*)malloc(sizeof(Asset));
    int chose = rand()%3;
    if (chose == 0) {
        cactus->src = A->Cactus_1->src;
        cactus->dst = A->Cactus_1->dst;
        cactus->txt = A->Cactus_1->txt;
    } else if (chose == 1){
        cactus->src = A->Cactus_2->src;
        cactus->dst = A->Cactus_2->dst;
        cactus->txt = A->Cactus_2->txt;
    } else {
        cactus->src = A->Cactus_3->src;
        cactus->dst = A->Cactus_3->dst;
        cactus->txt = A->Cactus_3->txt;
    }
    DA_append(DAE, (void*)cactus);
}

void spawn_cloud(Assets *A, DA* DAE) {
    Asset *cloud = (Asset*)malloc(sizeof(Asset));
    cloud->txt = A->Cloud->txt;
    cloud->src = A->Cloud->src;
    cloud->dst = A->Cloud->dst;
    cloud->dst.y = rand()% (WINDOW_HEIGHT/2);
    DA_append(DAE, (void*)cloud);

}

void spawn_bullet(Assets *A, DA* DAE) {
    SDL_Point rot = {
        .x = -GUN_W*7/8,
        .y = GUN_H*2/3
    };

    int mouse_x;
    int mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);


    AssetRot *a = (AssetRot*)malloc(sizeof(AssetRot));

    a->src = A->Bullet->src;
    a->dst = A->Bullet->dst;
    a->srf = A->Bullet->srf;
    a->txt = A->Bullet->txt;
    a->rot_c = rot;
    a->angle = get_angle2(A->Gun->dst.x + GUN_W/8, A->Gun->dst.y + GUN_H*2/3, mouse_x, mouse_y) + 3;
    a->dst.x = A->Gun->dst.x + A->Gun->dst.w;
    a->dst.y = A->Gun->dst.y;
    DA_append(DAE, (void*)a);
}

void spawn_entities(Assets *A, DA *DAE, Animations_start *starts, size_t now) {
    if (now - starts->Bird_spawn >= (size_t)(rand()%15000 + 7500)/(int)(ceil(SPEED))) {
        spawn_bird(A, DAE);
        starts->Bird_spawn = SDL_GetTicks();        
    } else if (starts->Bird_spawn > now) {
        starts->Bird_spawn = SDL_GetTicks();         
    }
    
    if (now - starts->Cactus_spawn >= (size_t)(rand()%15000 + 7500)/(int)(ceil(SPEED))) {
        spawn_cacti(A, DAE);
        starts->Cactus_spawn = SDL_GetTicks();
    } else if (starts->Cactus_spawn > now){
        starts->Cactus_spawn = SDL_GetTicks();
    }
    
    if (now - starts->Cloud_spawn >= (size_t)(rand()%25000 + 5000)/(int)(ceil(SPEED))) {
        spawn_cloud(A, DAE);
        starts->Cloud_spawn = SDL_GetTicks();
    } else if (starts->Cactus_spawn > now){
        starts->Cloud_spawn = SDL_GetTicks();
    }
}

void spawn_particles(DA *Clusters, int cx, int cy) {
    DA particles = {
        .type=DA_TYPE_PARTICLES
    };
    init_DA(&particles);

    for (int x = 0; x < 3; x++) {
        for (int y=0; y < 3; y++) {
            Particle *p = (Particle*)malloc(sizeof(Particle));
            p->x = cx + x*10;
            p->y = cy + y*10;
            p->w = 5;
            p->h = 5;
            DA_append(&particles, (void*)p);
        }
    }
    particles.ptr.DAEp->cx = cx;
    particles.ptr.DAEp->cy = cy;
    DA_append(Clusters, (void*)particles.ptr.DAEp);
}

void check_bcollisions(Assets *A, DArrayOfEntities *DAE, DArrayOfBullets *Bullets, DA *Clusters, State *state) {
    for (size_t x = 0; x < DAE->size; x++) {
        for (size_t y = 0; y < Bullets->size; y++) {
            if (DAE->data[x] && Bullets->data[y] && DAE->data[x]->txt != A->Cloud->txt) {
                Asset *ent = DAE->data[x];
                AssetRot *bull = Bullets->data[y]; 
                float b_angle_rad = bull->angle*PI/180;
                float bx = bull->dst.x;
                float by = bull->dst.y;

                float cx = A->Gun->dst.x + GUN_W/8;
                float cy = A->Gun->dst.y + GUN_H/3;

                float bx_delta = cx + (A->Gun->dst.x + A->Gun->dst.w - cx) * cosf(b_angle_rad) - (A->Gun->dst.y - cy) * sinf(b_angle_rad) + BULLET_H*sinf(b_angle_rad);
                float by_delta = cy + (A->Gun->dst.x + A->Gun->dst.w - cx) * sinf(b_angle_rad) + (A->Gun->dst.y - cy) * cosf(b_angle_rad);
                float bx_d = (A->Gun->dst.x + A->Gun->dst.w) - bx_delta ;
                float by_d = A->Gun->dst.y - by_delta ;

                if (bx - bx_d >= ent->dst.x &&
                    bx - bx_d <= ent->dst.x + ent->dst.w &&
                    by - by_d <= ent->dst.y + ent->dst.h &&
                    by - by_d >= ent->dst.y) {

                    if (ent->txt == A->Bird_Down->txt || ent->txt == A->Bird_Up->txt) {
                        state->POINTS += 20;
                    } else {
                         state->POINTS += 10;
                    }
                    
                    spawn_particles(Clusters, ent->dst.x, ent->dst.y);
                    free(ent);
                    DAE->data[x] = NULL;
                    DAE->count--;
                    
                    free(bull);
                    Bullets->data[y] = NULL;
                    Bullets->count--;
                }
            }
        }
    }
}

void free_sounds(Sounds *sounds) {
    if (sounds->death_sound) free(sounds->death_sound);
    if (sounds->shot_sound) free(sounds->shot_sound);
    if (sounds->stepl_sound) free(sounds->stepl_sound);
    if (sounds->stepr_sound) free(sounds->stepr_sound);
}

void manage_events(State* state, Assets* A, DA *DAE, Sounds *sounds) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                state->CLOSE = true;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_SPACE:
                        if (state->START) state->START = false;
                        if (state->PAUSE) {
                            state->PAUSE = false;
                            break;
                        }
                        if (state->AMMO > 0 && !state->GAMEOVER && !event.key.repeat) {
                            state->AMMO--;
                            spawn_bullet(A, DAE);
                            Mix_PlayChannel(-1, sounds->shot_sound, 0);
                        }
                        break;
                    
                    case SDL_SCANCODE_P:
                        state->PAUSE = !state->PAUSE;
                        break;
                    case SDL_SCANCODE_R:
                        if (state->PAUSE) {
                            state->RESTART = true;
                        }
                        if (state->GAMEOVER) state->GAMEOVER = false;
                        break;
                    case SDL_SCANCODE_ESCAPE:
                        if (state->GAMEOVER) {
                            state->CLOSE = true;
                        }
                        if (state->PAUSE) {
                            state->CLOSE = true;
                        } else {
                            state->PAUSE = true;
                        }
                        break;
                    default:
                        break;
                    }
                break;
            default:
                break;
        }
    }
}

void increment_speed(size_t lrt) {
    size_t now = SDL_GetTicks() - lrt;
    if (now < 90000) {
        SPEED = (START_SPEED + (8.0f*now/90000.0f))/(FPS/60.0f);
    }
}

void handle(State *state, SDL_Renderer *renderer, DA *DA_e, DA *DA_b, DA *DA_pc, Animations_start *starts, Assets *A, TTF_Font *font, Sounds *sounds, size_t *lrt) {
    if (state->RESTART) {
        state->RESTART = false;
        state->PAUSE = false;
        state->POINTS = 0;
        state->AMMO = 0;
        SPEED = START_SPEED/(FPS/60.f);
        *lrt = SDL_GetTicks();
        destroy_assets(A);
        init_assets(renderer, A);
        uninit_DA(DA_e);
        uninit_DA(DA_b);
        init_DA(DA_e);
        init_DA(DA_b);
        return;
    }

    display(state, renderer, DA_e->ptr.DAEe, DA_b->ptr.DAEb, DA_pc->ptr.DAEpc, A, font);
    if (state->START) {
        state->PAUSE = true;
        display_start(renderer, state, font);
    }

    if (!state->PAUSE && !state->GAMEOVER) {
        spawn_entities(A, DA_e, starts, SDL_GetTicks());
        animate(A, DA_e->ptr.DAEe, DA_b->ptr.DAEb, state, starts, SDL_GetTicks(), sounds);
        check_bcollisions(A, DA_e->ptr.DAEe, DA_b->ptr.DAEb, DA_pc, state);
        size_t now = SDL_GetTicks();
        if (now - starts->Last_added_bullet >= 3500/SPEED && state->AMMO < 10) {
            state->AMMO++;
            starts->Last_added_bullet = now;
        }
        increment_speed(*lrt);
    } else if (!state->START && !state->GAMEOVER){
        display_menu(renderer, state, font);
        display_pause(renderer, state, font);
    } else if (state->GAMEOVER){
        display_gameover(renderer, state, font);
        state->PAUSE = true;
    } else {
        display_menu(renderer, state, font);
    }
}

int main(int argc, char *argv[]) {
    size_t last_round_ticks = SDL_GetTicks();
    State GameState = {
        .CLOSE = false,
        .PAUSE = false,
        .POINTS = 0,
        .AMMO = 0,
        .RESTART = false,
        .START = true,
    };
    State *GSptr = &GameState;

    CHECK_ERROR_int(SDL_Init(SDL_INIT_EVERYTHING), GSptr);
    CHECK_ERROR_int(TTF_Init(), GSptr);
    CHECK_ERROR_int(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 128), GSptr);

    SDL_Window* window = SDL_CreateWindow("Texas T-REX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (window == NULL) {
        printf("No window pointer\n");
        return 1;
    }
    srand(time(NULL));

    Assets GameAssets = {0};
    DA DAE = {
        .type=DA_TYPE_ENTITIES
    };
    DA Bullets = {
        .type=DA_TYPE_BULLETS
    };

    DA Clusters = {
        .type=DA_TYPE_CLUSTERS
    };

    Animations_start Starts = {
        .Dino_start = SDL_GetTicks(),
        .Bird_spawn = SDL_GetTicks(),
        .Bird_flap = SDL_GetTicks(),
        .Cactus_spawn = SDL_GetTicks(),
        .Cloud_spawn = SDL_GetTicks(),
        .Last_added_bullet = 0.0f
    };
    
    Sounds GameSounds = {
        .shot_sound = Mix_LoadWAV("./assets/sound/shot.wav"),
        .death_sound = Mix_LoadWAV("./assets/sound/death.wav"),
        .stepl_sound = Mix_LoadWAV("./assets/sound/stepl.wav"),
        .stepr_sound = Mix_LoadWAV("./assets/sound/stepr.wav"),
    };

    TTF_Font *font = TTF_OpenFont("./assets/font/Muli-Bold.ttf", 120);
    CHECK_ERROR_ptr(font, GSptr);

    init_assets(renderer, &GameAssets);
    init_DA(&DAE);
    init_DA(&Bullets);
    init_DA(&Clusters);

    while (!GameState.CLOSE) {
        size_t t1 = SDL_GetTicks();
       
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        manage_events(&GameState, &GameAssets, &Bullets, &GameSounds);
        handle(&GameState, renderer,
            &DAE, &Bullets, &Clusters,
            &Starts, &GameAssets, font, &GameSounds, &last_round_ticks);
        
        SDL_RenderPresent(renderer);
        size_t t2 = SDL_GetTicks();
        cap_fps(t1, t2);
    }
    free_sounds(&GameSounds);    
    TTF_CloseFont(font);
    destroy_assets(&GameAssets);
    uninit_DA(&DAE);
    uninit_DA(&Bullets);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}

// TODO:
// - add sound:
//      entitieas death sound
// - add mute option and volume regulation
// - add enemies death particles:
//      decide dimensions, randomize quantity    
//      make animate them with gravity and rolling background velocity
//      make them bouncy
//      add inertia