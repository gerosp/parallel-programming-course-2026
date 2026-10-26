#pragma once
// Стратегии выбора следующего потока.
// FirstAlive — этап 1, Replay — этап 2, RandomPick — этап 3, Dfs — этап 4 (ограничение bound — этап 5).
// enabled — номера потоков, которые могут сделать шаг, по возрастанию; last — прошлый выбранный (-1 в начале).
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include "strategy.hpp"

namespace sched
{

    // Всегда первый поток из enabled.
    struct FirstAlive : Strategy
    {
        int pick(const std::vector<int> &enabled, int) override
        {
            if (enabled.empty())
                throw std::logic_error("FirstAlive requires an enabled worker");
            return enabled.front();
        }
    };

    // Проигрывает трассу: на i-м решении выбирает поток trace[i] ('A' -> 0, 'B' -> 1).
    struct Replay : Strategy
    {
        explicit Replay(std::string trace) : trace(trace) {}
        // Сбросить позицию в трассе перед прогоном.
        std::string trace;
        int pos = 0;
        void begin_run() override { pos = 0; }
        int pick(const std::vector<int> &enabled, int) override
        {
            if (pos >= trace.size())
                throw std::logic_error("Nothing left to replay");
            auto pick = std::find(enabled.begin(), enabled.end(), static_cast<int>(trace[pos] - 'A'));
            if (pick == enabled.end())
                throw std::logic_error("Picked thread is not enabled at this step");
            pos++;

            return *pick;
        }
    };

    // Случайный поток: enabled[rnd() % enabled.size()], rnd — std::mt19937(seed).
    struct RandomPick : Strategy
    {
        std::mt19937 rnd;
        explicit RandomPick(uint32_t seed) : rnd(std::mt19937(seed)) {}
        int pick(const std::vector<int> &enabled, int) override
        {
            if (enabled.empty())
                throw std::logic_error("FirstAlive requires an enabled worker");
            return enabled[rnd() % enabled.size()];
        }
    };

    // Stateless DFS по дереву выборов: каждый прогон с нуля, повторяет префикс прошлого и берёт следующий вариант.
    // bound — максимум вытеснений (этап 5); для dfs runner передаёт INT_MAX.
    struct Dfs : Strategy
    {
        explicit Dfs(int b) : bound(b) {}
        // Подготовиться к новому прогону (позиция в пути, счётчик вытеснений).
        std::vector<std::vector<int>> path;
        std::vector<int> chosen;
        std::size_t depth;
        int preemptions;
        int bound;
        void begin_run() override
        {
            depth = 0;
            preemptions = 0;
        }

        int pick(const std::vector<int> &enabled, int last) override
        {
            bool last_enabled =
                std::find(enabled.begin(), enabled.end(), last) != enabled.end();

            if (depth == path.size())
            {
                std::vector<int> options;

                for (int candidate : enabled)
                {
                    bool costs_preemption = last_enabled && candidate != last;

                    if (!costs_preemption || preemptions < bound)
                        options.push_back(candidate);
                }

                path.push_back(options);
                chosen.push_back(0);
            }

            int solution = path[depth][chosen[depth]];
            ++depth;

            if (last_enabled && solution != last)
                ++preemptions;

            return solution;
        }
        // Выбрать следующий путь; false — дерево обойдено.
        bool end_run() override
        {
            path.resize(depth);
            chosen.resize(depth);
            while (!path.empty())
            {
                const auto &options = path.back();
                if (chosen.back() + 1 < options.size())
                {
                    chosen.back()++;
                    return true;
                }
                path.pop_back();
                chosen.pop_back();
            }
            return false;
        }
    };

} // namespace sched
