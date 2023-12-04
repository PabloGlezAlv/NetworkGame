#pragma once
#include <string>
#include <SDL_ttf.h>
#include "checkML.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
//struct SDL_Surface;
//struct SDL_Color;
//struct TTF_Font;


struct rgba {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

class Renderer
{
private:
	~Renderer();

	SDL_Window* window = nullptr;
	SDL_Renderer* render = nullptr;

	// Create text
	TTF_Font* font;
	SDL_Surface* surfaceMessage;
	SDL_Texture* Message;

	//Screen size
	int screen_width = 100, screen_height = 100;
protected:
	Renderer();

	bool initPrivate(bool vsync, int width, int height);

	static Renderer* renderer;
public:

	static bool Init(bool vsync, int width, int height);

	static Renderer* Instance();

	void clear(uint32_t color);

	void present();

	rgba createRGBA(uint32_t baseColor);

	void drawText(std::string message, SDL_Color color, int x, int y);

	void drawRect(int posX, int posY, int width, int height, uint32_t c);
	void drawRect(int posX, int posY, int width, int height, int r, int g, int b, int a);

	SDL_Renderer* getRenderer();

	int getHeight();

	int getWidth();

	uint32_t getTicks();

	void drawImage(SDL_Texture* img, int sx1, int sy1, int sx2, int sy2, int tx1, int ty1, int tx2, int ty2);

	static void Release();
};

