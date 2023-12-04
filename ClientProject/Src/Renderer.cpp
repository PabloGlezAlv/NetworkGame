#include "Renderer.h"

#include <cstdio>
#include <SDL_video.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

Renderer* Renderer::renderer = nullptr;

Renderer::Renderer()
{

}

Renderer* Renderer::Instance()
{
	return renderer;
}

bool Renderer::initPrivate(bool vsync, int width, int height) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
		return false;
	}
	SDL_Window* window = SDL_CreateWindow("ClientProject", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, NULL);
	if (window == nullptr)
	{
		this->window = nullptr;
		return false;
	}

	SDL_Renderer* render = SDL_CreateRenderer(window, -1, vsync ? SDL_RENDERER_PRESENTVSYNC : SDL_RENDERER_ACCELERATED); //Set vsync or not
	if (render == nullptr)
	{
		SDL_DestroyWindow(window);
		this->window = nullptr;
		this->render = nullptr;
		return false;
	}

	//Initialize SDL_ttf
	if (TTF_Init() == -1)
	{
		SDL_DestroyWindow(window);
		this->window = nullptr;
		this->render = nullptr;
		return false;
	}

	font = TTF_OpenFont("../assets/Sans.ttf", 18);
	if (font == NULL)
	{
		fprintf(stderr, "could not initialize the font: %s\n", TTF_GetError());
		SDL_DestroyWindow(window);
		this->window = nullptr;
		this->render = nullptr;
		return false;
	}

	SDL_WarpMouseInWindow(window, width / 2, height / 2);
	this->screen_width = width;
	this->screen_height = height;


	this->window = window;
	this->render = render;

	return true;
}

bool Renderer::Init(bool vsync, int width, int height)
{
	renderer = new Renderer();
	bool ret = renderer->initPrivate(vsync, width, height);
	if (!ret) {
		printf("Error in privateInit of Renderer\n");
		delete renderer;
		renderer = nullptr;
	}
	return ret;
}

void Renderer::drawText(std::string message, SDL_Color color, int x, int y)
{
	renderer->surfaceMessage = TTF_RenderText_Solid(font, message.c_str(), color);
	renderer->Message = SDL_CreateTextureFromSurface(renderer->render, renderer->surfaceMessage);

	int textWidth, textHeight;
	TTF_SizeText(font, message.c_str(), &textWidth, &textHeight);

	SDL_Rect Message_rect; 
	Message_rect.x = x;  
	Message_rect.y = y; 
	Message_rect.w = textWidth; 
	Message_rect.h = textHeight;

	SDL_RenderCopy(renderer->render, renderer->Message, NULL, &Message_rect);

	SDL_FreeSurface(renderer->surfaceMessage);
	renderer->surfaceMessage = nullptr;
	SDL_DestroyTexture(renderer->Message);
	renderer->Message = nullptr;
}

void Renderer::clear(uint32_t color)
{
	rgba col = createRGBA(color);
	SDL_SetRenderDrawColor(renderer->render, col.r, col.g, col.b, col.a);
	SDL_RenderClear(renderer->render);
}

void Renderer::present()
{
	SDL_RenderPresent(renderer->render);
}

void Renderer::drawRect(int posX, int posY, int width, int height, uint32_t color)
{
	SDL_Rect rect;
	rect.x = posX;
	rect.y = posY;
	rect.w = width;
	rect.h = height;

	rgba color_rgba = createRGBA(color);

	SDL_SetRenderDrawColor(renderer->render, color_rgba.r, color_rgba.g, color_rgba.b, color_rgba.a);
	SDL_RenderFillRect(renderer->render, &rect);
}

void Renderer::drawRect(int posX, int posY, int width, int height, int r, int g, int b, int a)
{
	SDL_Rect rect;
	rect.x = posX;
	rect.y = posY;
	rect.w = width;
	rect.h = height;


	SDL_SetRenderDrawColor(renderer->render, r, g,b, a);
	SDL_RenderFillRect(renderer->render, &rect);
}

SDL_Renderer* Renderer::getRenderer()
{
	return renderer->render;
}

void Renderer::drawImage(SDL_Texture* img, int sx1, int sy1, int sx2, int sy2, int tx1, int ty1, int tx2, int ty2)
{
	if (sx2 == 0 && sy1 == 0) //Means the source is everything
	{
		SDL_Rect destRect{ tx1,ty1, (tx2 - tx1), (ty2 - ty1) };
		SDL_RenderCopy(renderer->render, img, NULL, &destRect);
	}
	else
	{
		SDL_Rect srcRect{ sx1,sy1, (sx2), (sy2) };
		SDL_Rect destRect{ tx1,ty1, (tx2 - tx1), (ty2 - ty1) };
		SDL_RenderCopy(renderer->render, img, &srcRect, &destRect);
	}
}

int Renderer::getHeight()
{
	return screen_height;
}

int Renderer::getWidth()
{
	return screen_width;
}

void Renderer::Release()
{
	SDL_DestroyRenderer(renderer->render);
	renderer->render = nullptr;

	SDL_DestroyWindow(renderer->window);
	renderer->window = nullptr;

	TTF_CloseFont(renderer->font);

	TTF_Quit();
	SDL_Quit();

	delete renderer;
	renderer = nullptr;
}

rgba Renderer::createRGBA(uint32_t baseColor) {
	uint8_t r = (baseColor >> 24) & 0xFF;
	uint8_t g = (baseColor >> 16) & 0xFF;
	uint8_t b = (baseColor >> 8) & 0xFF;
	uint8_t a = (baseColor >> 0) & 0xFF;
	return { r,g,b,a };

}

uint32_t Renderer::getTicks() //Get the number of milliseconds since SDL library initialization
{
	return SDL_GetTicks();
}

Renderer::~Renderer()
{

}