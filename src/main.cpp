#include <iostream>
#include <vector>
#include <memory>
#include <cstdio>
#include <chrono>
#include <thread>
#include <array>

#include <curses.h>

static constexpr int BOARD_WIDTH = 10;
static constexpr int BOARD_HEIGHT = 30;

struct invader
{
    size_t x, y, hp;
};

struct defender
{
    size_t x, y, hp;
};

struct shot
{
    size_t x, y;
};

static std::vector<std::unique_ptr<invader>> invaders;
static std::unique_ptr<defender> def;
static std::vector<std::unique_ptr<shot>> shots;


auto build_board() -> void
{
    def = std::unique_ptr<defender>(new defender);
    def->x = BOARD_WIDTH / 2;
    def->y = 0;
    def->hp = 1;

    for (int i = 0; i < BOARD_WIDTH; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            auto invader = std::unique_ptr<struct invader>(new struct invader);

            (*invader).x = i;
            (*invader).y = BOARD_HEIGHT - j;
            (*invader).hp = 3;

            invaders.push_back(std::move(invader));
        }
    }
}

enum class entities : uint32_t
{
    NONE = 0,
    INVADER = 1,
    SHOT = 2,
    DEFENDER = 3
};

auto draw_board() -> void
{
    std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";

    std::array<std::array<entities, BOARD_WIDTH>, BOARD_HEIGHT> bs = { entities::NONE };

    for (auto const &inv : invaders)
    {
        bs[inv->x][inv->y] = entities::INVADER;
    }

    for (auto const &shot : shots)
    {
        bs[shot->x][shot->y] = entities::SHOT;
    }

    bs[def->x][def->y] = entities::DEFENDER;

    for (int j = BOARD_HEIGHT; j > 0; j--)
    {
        for (int i = 0; i < BOARD_WIDTH; i++)
        {
            switch (bs[i][j])
            {
            case entities::NONE:
                std::cout << " ";
                break;
            case entities::INVADER:
                std::cout << "U";
                break;
            case entities::SHOT:
                std::cout << "i";
                break;
            case entities::DEFENDER:
                std::cout << "m";
                break;
            default:
                break;
            }
        }

        std::cout << "\n";
    }
}

auto update_board() -> void
{ 
    bool board_changed = false;

    // Need to iterate by index rather than iterator since we intend to mutate the container.
    for (int i = 0; i < invaders.size(); i++)
    {
        auto& inv = invaders[i];

        if (inv->x + 1 == BOARD_WIDTH)
        {
            inv->x = 0;
            inv->y--;
        }
        else
            inv->x++;

        if (inv->x == def->x && inv->y == def->y)
        {
            invaders.erase(invaders.begin() + i);
            // Move back in iteration due to removal of an element.
            i--;

            def->hp--;

            board_changed = true;

            if (!def->hp)
            {
                std::cout << "Game over!";
                exit(0);
            }
        }
    }

    for (int i = 0; i < shots.size(); i++)
    {
        auto& s = shots[i];

        s->y++;

        board_changed = true;

        for (int j = 0; j < invaders.size(); j++)
        {
            auto& inv = invaders[j];

            if (s->x == inv->x && s->y == inv->y)
            {
                inv->hp--;

                if (inv->hp == 0)
                {
                    invaders.erase(invaders.begin() + j);
                }

                shots.erase(shots.begin() + i);
                i--;
                break;
            }
        }
    }

    if (board_changed)
        draw_board();
}

auto gs() -> void
{
    while (true)
    {
        update_board();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

auto fire() -> void
{
    auto s = std::unique_ptr<shot>(new shot);

    s->x = def->x;
    s->y = def->y + 1;

    shots.push_back(std::move(s));
}

auto await_input() -> void
{
    while (true)
    {
        char input = std::getchar();

        switch (input)
        {
        case 'a':
            def->x = std::max((size_t)0, def->x - 1);
            draw_board();
            break;
        case 'w':
            fire();
            draw_board();
            break;
        case 'd':
            def->x = std::min((size_t)BOARD_WIDTH - 1, def->x + 1);
            draw_board();
            break;
        default:
            break;
        }

        // Limit the number of instructions per second
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main()
{
    std::cout << "Hello world\n";

    build_board();

    auto gamestate_thread = std::thread(gs);
    auto input_thread = std::thread(await_input);

    gamestate_thread.join();
    input_thread.join();

    return 0;
}