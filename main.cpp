#include <iostream>
#include <SDL.h>
#include <sstream>
#include <thread>

enum Color
{
  UNSET,
  RED,
  GREEN,
  BLUE,
  YELLOW // DO NOT CHANGE ORDER
};

#define ROWS 10
#define COLS 10

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

class PlayField
{
private:
  Color field[ROWS * COLS];
  std::mutex mutex;

public:
  constexpr PlayField() : field{UNSET} {}

  Color GetField(const int x, const int y)
  {
    std::lock_guard<std::mutex> lock{mutex};
    return field[x + y * COLS];
  }

  void SetField(const int x, const int y, const Color val)
  {
    std::lock_guard<std::mutex> lock{mutex};
    field[x + y * COLS] = val;
  }

  void PrintField()
  {
    for (int y = 0; y < ROWS; y++)
    {
      for (int x = 0; x < COLS; x++)
      {
        std::cout << x << "/" << y << "=" << GetField(x, y) << std::endl;
      }
    }
  }

  void InitializeWithRandomColors()
  {
    for (int cCounter = RED; cCounter <= YELLOW; cCounter++)
    {
      Color c = static_cast<Color>(cCounter);
      for (int j = 0; j < ROWS * COLS / 4; j++)
      {
        Color oldVal;
        int x, y;
        do
        {
          x = GetRandomCol();
          y = GetRandomRow();
          oldVal = GetField(x, y);
        } while (oldVal != UNSET);
        SetField(x, y, c);
      }
    }
  }

  int GetRandomRow() const { return rand() / ((RAND_MAX + 1u) / ROWS); }

  int GetRandomCol() const { return rand() / ((RAND_MAX + 1u) / COLS); }

  bool IsEnded()
  {
    Color first = GetField(0, 0);
    for (int y = 0; y < ROWS; y++)
    {
      for (int x = 0; x < COLS; x++)
      {
        if (GetField(x, y) != first)
        {
          return false;
        }
      }
    }
    return true;
  }

  void DoRound()
  {
    int x = GetRandomCol();
    int y = GetRandomRow();
    SetField(x, y, UNSET);

    Color val;
    do
    {
      val = GetField(GetRandomCol(), GetRandomRow());
    } while (val == UNSET);
    SetField(x, y, val);
  }

  void Render(SDL_Renderer *pRenderer)
  {
    for (int y = 0; y < ROWS; y++)
    {
      for (int x = 0; x < COLS; x++)
      {
        Color c = GetField(x, y);
        SDL_Rect fillRect = {x * SCREEN_WIDTH / ROWS, y * SCREEN_HEIGHT / COLS, SCREEN_WIDTH / ROWS,
                             SCREEN_HEIGHT / COLS};
        switch (c)
        {
        case RED:
          SDL_SetRenderDrawColor(pRenderer, 0xFF, 0x00, 0x00, 0xFF);
          break;
        case GREEN:
          SDL_SetRenderDrawColor(pRenderer, 0x00, 0xFF, 0x00, 0xFF);
          break;
        case BLUE:
          SDL_SetRenderDrawColor(pRenderer, 0x00, 0x00, 0xFF, 0xFF);
          break;
        case YELLOW:
          SDL_SetRenderDrawColor(pRenderer, 0xFF, 0xFF, 0x00, 0xFF);
          break;
        case UNSET:
          // no code here
          break;
        }
        SDL_RenderFillRect(pRenderer, &fillRect);
      }
    }
  }
};

class StatsGame
{
private:
  PlayField playField;
  int runde = 0;

public:
  void DoGame()
  {
    playField.InitializeWithRandomColors();
    while (!playField.IsEnded())
    {
      runde++;
      playField.DoRound();
    }
  }

  int GetRunden()
  {
    return runde;
  }
};

class UIGame
{
private:
  int round;
  bool quit;
  bool finished;
  PlayField playField;

public:
  UIGame() : round(0), quit(false), finished(false) {}

  void Calc()
  {
    while (!finished && !quit)
    {
      std::this_thread::sleep_for(std::chrono::nanoseconds(500000));
      if (!playField.IsEnded())
      {
        playField.DoRound();
        round++;
      }
      else
      {
        finished = true;
      }
    }
  }

  void DoGame()
  {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Darwin",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                          SCREEN_HEIGHT,
                                          0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_Event event;
    playField.InitializeWithRandomColors();

    std::thread t(&UIGame::Calc, this);

    while (!finished && !quit)
    {
      SDL_Delay(5);
      SDL_PollEvent(&event);
      switch (event.type)
      {
      case SDL_QUIT:
        quit = true;
        break;
      }
      SDL_SetRenderDrawColor(renderer, 242, 242, 242, 255);
      SDL_RenderClear(renderer);
      playField.Render(renderer);
      SDL_RenderPresent(renderer);
    }
    t.join();

    if (finished)
    {
      std::ostringstream sstr;
      sstr << "Total rounds: " << round;
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Result", sstr.str().c_str(), window);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
  }
};

int main()
{
  std::srand(std::time(nullptr));

  // THIS WILL PERFORM 100,000 ROUNDS AND PRINT STATISTICS
  //  long totalRounds = 0;
  //  int i = 0, minRounds = INT_MAX, maxRounds = 0;
  //  while (i++ < 100000) {
  //      StatsGame spiel;
  //      spiel.DoGame();
  //      totalRounds += spiel.GetRunden();
  //      minRounds = std::min(minRounds, spiel.GetRunden());
  //      maxRounds = std::max(maxRounds, spiel.GetRunden());
  //  }

  //  std::cout << "Min:" << minRounds << std::endl;
  //  std::cout << "Max:" << maxRounds << std::endl;
  //  std::cout << "Avg:" << (totalRounds / i) << std::endl;

  // THIS WILL DEMONSTRATE ONE ROUND VISUALLY
  UIGame uiGame;
  uiGame.DoGame();

  return 0;
}
