#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#include <emscripten.h>
EMSCRIPTEN_KEEPALIVE

#define SCREEN_H 32
#define SCREEN_W 64
#define STACK_LEVELS 16
#define MAXSIZE 4096-512 



const SDL_Color COLOR_A = {.r = 0, .g = 0, .b = 0, .a = 255};

const SDL_Color COLOR_B = {.r = 255, .g = 255, .b = 255, .a = 255};

unsigned char font[80] =
{ 
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

struct CHIP8{

    uint8_t V[16];
    uint8_t memory[4096];
    uint8_t gfx[SCREEN_H*SCREEN_W];
    uint16_t stack[STACK_LEVELS];
    uint16_t sp;
    uint16_t pc;
    uint16_t I;
    uint8_t dt;
    uint8_t st;
    uint8_t kb[16];
    uint8_t draw_flag;
};

void c8_init(struct CHIP8 *c8){

    memset(c8->memory, 0, sizeof(c8->memory) );
    memset(c8->gfx, 0, sizeof(c8->gfx) );
    memset(c8->V, 0, sizeof(c8->V) );
    memset(c8->stack, 0, sizeof(c8->stack) );
    memset(c8->kb, 0, sizeof(c8->kb) );
    
    memcpy(c8->memory, font, sizeof(font) );

    c8->sp = 0;
    c8->pc = 0x200;
    c8->I = 0;
    c8->dt = 0;
    c8->st = 0;
    c8->draw_flag = 0;
}

uint16_t get_opcode(struct CHIP8 *c8){
    
    return (c8->memory[c8->pc] << 8) + c8->memory[c8->pc+1];
}

void c8_next_op(struct CHIP8 *c8){

    uint16_t opcode = get_opcode(c8);
    
    /*
    printf("current opcode: 0x%X\n", opcode);
    printf("current pc: %d\n", c8->pc);
    printf("current sp: %d\n", c8->sp);
    */
    //usleep(500000);

    switch(opcode & 0xF000){

        case 0x0000:
            switch(opcode & 0x000F){
                case 0x0000:
                    memset(c8->gfx, 0, sizeof(c8->gfx));
                    c8->pc += 2;
                    break;
                case 0x000E:
                    c8->pc = c8->stack[--c8->sp];
                    c8->pc += 2;
                    break;
                default:
                    c8->pc += 2;
                    break;
            }
            break;
        case 0x1000:
            c8->pc = (opcode & 0x0FFF);
            break;
        case 0x2000:
            c8->stack[c8->sp++] = c8->pc;
            c8->pc = opcode & 0x0FFF;
            break;
        case 0x3000:
            c8->V[(opcode&0x0F00)>>8]==(opcode&0x0FF) ? 
            c8->pc += 4 : 
            (c8->pc += 2);
            break;
        case 0x4000:
            c8->V[(opcode&0x0F00)>>8]==(opcode&0x0FF) ? 
            c8->pc += 2 : 
            (c8->pc += 4);
            break;
        case 0x5000:
            c8->V[(opcode&0x0F00)>>8]==c8->V[(opcode&0x00F0)>>4] ? 
            c8->pc += 4 : 
            (c8->pc += 2);
            break;
        case 0x6000:
            c8->V[(opcode&0x0F00)>>8] = (opcode&0x0FF);
            c8->pc += 2;
            break;
        case 0x7000:
            c8->V[(opcode & 0x0F00)>>8] += (opcode & 0xFF);
            c8->pc += 2;
            // printf("V[0x%X]: %X\n", (opcode&0x0F00)>>8, c8->V[(opcode&0x0F00)>>8]);
            break;
        case 0x8000:
            switch(opcode&0x000F){
            
                case 0x0000:
                    c8->V[(opcode&0x0F00)>>8] = c8->V[(opcode&0x00F0)>>4];
                    c8->pc += 2;
                    break;
                case 0x0001:
                    c8->V[(opcode&0x0F00)>>8] |= c8->V[(opcode&0x00F0)>>4];
                    c8->pc += 2;
                    break;
                case 0x0002:
                    c8->V[(opcode&0x0F00)>>8] &= c8->V[(opcode&0x00F0)>>4];
                    c8->pc += 2;
                    break;
                case 0x0003:
                    c8->V[(opcode&0x0F00)>>8] ^= c8->V[(opcode&0x00F0)>>4];
                    c8->pc += 2;
                    break;
                case 0x0004:
                    1==1;
                    uint16_t a = c8->V[(opcode&0x0F00)>>8] + 
                        c8->V[(opcode&0x00F0)>>4];
                    c8->V[15] = a>255;
                    c8->V[(opcode&0x0F00)>>8] += c8->V[(opcode&0x00F0)>>4];
                    c8->pc += 2;
                    break;
                case 0x0005:
                    c8->V[15] = c8->V[(opcode&0x0F00)>>8] > 
                        c8->V[(opcode&0x00F0)>>4];
                    c8->V[(opcode&0x0F00)>>8] -= c8->V[(opcode&0x00F0)>>4];
                    c8->pc += 2;
                    break;
                case 0x0006:
                    1==1;
                    uint16_t x = (opcode&0x0F00)>>8;
                    c8->V[15] = c8->V[x]>>7;
                    c8->V[x] /= 2;
                    c8->pc += 2;
                    break;
                case 0x0007:
                    c8->V[15] = c8->V[(opcode&0x00F0)>>4] > 
                        c8->V[(opcode&0x0F00)>>8];
                    c8->V[(opcode&0x0F00)>>8] = c8->V[(opcode&0x00F0)>>4] -
                        c8->V[(opcode&0x0F00)>>8];
                    c8->pc += 2;
                    break;
                case 0x000E:
                    1==1;
                    uint16_t temp = (opcode&0x0F00)>>8;
                    c8->V[15] = c8->V[temp]>>7; 
                    c8->V[temp] *= 2;
                    c8->pc += 2;
                    break;
            }
            break;
        case 0x9000:
            c8->V[(opcode&0x0F00)>>8]!=c8->V[(opcode&0x00F0)>>4] ? c8->pc += 4 : (c8->pc += 2);
            break;
        case 0xA000:
            c8->I = (opcode&0x0FFF);
            c8->pc += 2;
            break;
        case 0xB000:
            c8->pc = c8->V[0] + (opcode&0x0FFF);
            break;
        case 0xC000:
            c8->V[(opcode&0x0F00)>>8] = (rand()%256)&(opcode&0x00FF);
            c8->pc += 2;
            break;
        case 0xD000:
            c8->draw_flag = 1;
            
            uint8_t x = c8->V[(opcode&0x0F00)>>8];
            uint8_t y = c8->V[(opcode&0x00F0)>>4];
            
            c8->V[15] = 0;
            for(int i = 0; i < (opcode&0x000F); i++){

                for(int j = 0; j < 8; j++){

                    if((c8->memory[c8->I+i] & (0x80 >> j)) != 0){  

                        c8->V[15] |= c8->gfx[(x+j)+((y+i)*64)];
                        c8->gfx[(x+j)+((y+i)*64)] ^= 1;
                    }
                }
            }
            c8->pc+=2;
            break;
        case 0xE000:
            c8->pc += 2 + 2*(c8->kb[c8->V[(opcode&0x0F00)>>8]] == 
                ((opcode&0x000F) == 0xE) );
            break;
        case 0xF000:
            switch(opcode&0x00FF){
                case 0x0007:
                    c8->V[(opcode&0x0F00)>>8] = c8->dt;
                    c8->pc += 2; 
                    break;
                case 0x000A:
                    1==1;
                    uint8_t pressed = 0;
                    for(int i = 0; i < 16; i++){
                        
                        c8->V[(opcode&0x0F00)>>8] = i*c8->kb[i];
                        pressed |= c8->kb[i];
                    }

                    c8->pc += 2*(pressed);
                    break;
                case 0x0015:
                    c8->dt = c8->V[(opcode&0x0F00)>>8];
                    c8->pc += 2; 
                    break;
                case 0x0018:
                    c8->st = c8->V[(opcode&0x0F00)>>8];
                    c8->pc += 2; 
                    break;
                case 0x001E:
                    c8->V[15] = (c8->I+c8->V[(opcode&0x0F00)>>8])>0xFFF;
                    c8->I += c8->V[(opcode&0x0F00)>>8];
                    c8->pc += 2; 
                    break;
                case 0x0029:
                    c8->I = c8->V[(opcode&0x0F00)>>8] * 5;
                    c8->pc += 2;
                    break;
                case 0x0033:
                    c8->memory[c8->I] = c8->V[(opcode&0x0F00)>>8]/100;
                    c8->memory[c8->I+1] = (c8->V[(opcode&0x0F00)>>8]/10)%10;
                    c8->memory[c8->I+2] = (c8->V[(opcode&0x0F00)>>8]%100)%10;
                    c8->pc+=2;
                    break;
                case 0x0055:
                    for(int i = 0; i <=((opcode & 0x0F00)>>8); i++){
                        c8->memory[c8->I+i] = c8->V[i];
                    }
                    c8->pc+=2;
                    break;
                case 0x0065:
                    for(int i = 0; i <=((opcode & 0x0F00)>>8); i++){
                        c8->V[i] = c8->memory[c8->I+i];
                    }
                    c8->pc+=2;
                    break;
            }
            break;
    }
    if(c8->dt){
        c8->dt--;
    }
}

int c8_load_game(struct CHIP8 *c8, char *filename){

    c8_init(c8);

    FILE *game = fopen(filename, "rb");

    if(!game){

        fputs("FILE ERROR", stderr);
        return 1;
    }

    fseek(game, 0, SEEK_END);
    long size = ftell(game);
    rewind(game);

    char *buffer = (char*)malloc(size);

    if(!buffer){

        fputs("BUFFER ERROR", stderr);
        return 1;
    }

    if(fread(buffer, 1, size, game) != size){

        fputs("READ ERROR", stderr);
        return 1;
    }

    if(MAXSIZE > size){

        memcpy(c8->memory+512, buffer, size);
    }else{

        fputs("ROM too big", stderr);
        return 1;
    }

    fclose(game);
    free(buffer);

    return 0;
}

void c8_render(struct CHIP8 *c8, SDL_Renderer *renderer){

    SDL_SetRenderDrawColor(renderer, COLOR_A.r, COLOR_A.g, COLOR_A.b, COLOR_A.a);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, COLOR_B.r, COLOR_B.g, COLOR_B.b, COLOR_B.a);

    for(int i = 0; i < (SCREEN_H*SCREEN_W); i++){

        if(c8->gfx[i] == 1){
            int x = (i%64)*8;
            int y = (i/64)*8;
            
            SDL_Rect rect = {x, y, 8, 8};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void c8_press(struct CHIP8 *c8, uint8_t value, int key){

    switch(key){
        case SDLK_1:
            c8->kb[1] = value;
            break;
        case SDLK_2:
            c8->kb[2] = value;
            break;
        case SDLK_3:
            c8->kb[3] = value;
            break; 
        case SDLK_4:
            c8->kb[0xC] = value;
            break; 
        case SDLK_q:
            c8->kb[4] = value;
            break; 
        case SDLK_w:
            c8->kb[5] = value;
            break; 
        case SDLK_e:
            c8->kb[6] = value;
            break; 
        case SDLK_r:
            c8->kb[0xD] = value;
            break; 
        case SDLK_a:
            c8->kb[7] = value;
            break; 
        case SDLK_s:
            c8->kb[8] = value;
            break; 
        case SDLK_d:
            c8->kb[9] = value;
            break; 
        case SDLK_f:
            c8->kb[0xE] = value;
            break; 
        case SDLK_z:
            c8->kb[0xA] = value;
            break; 
        case SDLK_x:
            c8->kb[0] = value; 
            break;
        case SDLK_c:
            c8->kb[0xB] = value;
            break;
        case SDLK_v:
            c8->kb[0xF] = value; 
            break;
    }
}

int main(int argc, char *argv[]){
    
    printf("fun\n");
    
    if(SDL_Init(SDL_INIT_VIDEO) != 0){

        fprintf(stderr, "SDL INIT ERROR: %s", SDL_GetError());
        return 1;
    }
    
    SDL_Window *window = SDL_CreateWindow("Procedural", 
                                            100, 100, 
                                            SCREEN_W*8, SCREEN_H*8, 
                                            SDL_WINDOW_SHOWN);
    
    if(!window){
        
        fprintf(stderr, "SDL CREATE WINDOW ERROR: %s", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                               SDL_RENDERER_ACCELERATED |
                                               SDL_RENDERER_PRESENTVSYNC 
                                            );

    if(renderer == NULL){

        SDL_DestroyWindow(window);
        fprintf(stderr, "SDL CREATE RENDERER ERROR", SDL_GetError());
        return 1;
    }
    
    SDL_Event e;

    srand(time(0));
    struct CHIP8 c8;
    
    c8_load_game(&c8, "snake.ch8");
    int quit = 0;
    int count = 0;
    while(!quit){
        c8_next_op(&c8);
        if(c8.draw_flag == 1){

            c8.draw_flag = 0;
            c8_render(&c8, renderer);
            SDL_RenderPresent(renderer);
        }
    
        while(SDL_PollEvent(&e)){
            
            switch (e.type){
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    c8_press(&c8, 1, e.key.keysym.sym);
                    break;
                case SDL_KEYUP:
                    c8_press(&c8, 0, e.key.keysym.sym);
                    break;
            }
        }

        if (count == 3){

            emscripten_sleep(0);
            count = 0;
        }
        count++;
    }
}
