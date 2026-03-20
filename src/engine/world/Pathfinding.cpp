#include "engine/world/Pathfinding.h"

#include <algorithm>
#include <array>
#include <limits>
#include <queue>
#include <vector>

namespace war
{
    namespace
    {
        struct Node
        {
            int index = -1;
            int priority = 0;

            bool operator<(const Node& rhs) const
            {
                return priority > rhs.priority;
            }
        };

        int heuristic(TileCoord a, TileCoord b)
        {
            const int dx = a.x - b.x;
            const int dy = a.y - b.y;
            return (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
        }
    }

    std::vector<TileCoord> Pathfinding::findPath(const WorldGrid& grid, TileCoord start, TileCoord goal)
    {
        if (!grid.isWalkable(start) || !grid.isWalkable(goal))
        {
            return {};
        }

        const int width = grid.getWidth();
        const int height = grid.getHeight();
        const int cellCount = width * height;

        const auto toIndex = [width](TileCoord tile)
        {
            return tile.y * width + tile.x;
        };

        const auto toTile = [width](int index)
        {
            return TileCoord{ index % width, index / width };
        };

        std::priority_queue<Node> open;
        std::vector<int> cameFrom(static_cast<size_t>(cellCount), -1);
        std::vector<int> costSoFar(static_cast<size_t>(cellCount), std::numeric_limits<int>::max());

        const int startIndex = toIndex(start);
        const int goalIndex = toIndex(goal);

        open.push({ startIndex, 0 });
        costSoFar[static_cast<size_t>(startIndex)] = 0;

        constexpr std::array<TileCoord, 4> directions = {
            TileCoord{ 1, 0 },
            TileCoord{ -1, 0 },
            TileCoord{ 0, 1 },
            TileCoord{ 0, -1 }
        };

        while (!open.empty())
        {
            const Node current = open.top();
            open.pop();

            if (current.index == goalIndex)
            {
                break;
            }

            const TileCoord currentTile = toTile(current.index);

            for (const TileCoord dir : directions)
            {
                TileCoord next{
                    currentTile.x + dir.x,
                    currentTile.y + dir.y
                };

                if (!grid.isWalkable(next))
                {
                    continue;
                }

                const int nextIndex = toIndex(next);
                const int newCost = costSoFar[static_cast<size_t>(current.index)] + 1;

                if (newCost < costSoFar[static_cast<size_t>(nextIndex)])
                {
                    costSoFar[static_cast<size_t>(nextIndex)] = newCost;
                    cameFrom[static_cast<size_t>(nextIndex)] = current.index;
                    const int priority = newCost + heuristic(next, goal);
                    open.push({ nextIndex, priority });
                }
            }
        }

        if (cameFrom[static_cast<size_t>(goalIndex)] == -1 && startIndex != goalIndex)
        {
            return {};
        }

        std::vector<TileCoord> path;
        int current = goalIndex;
        path.push_back(toTile(current));

        while (current != startIndex)
        {
            current = cameFrom[static_cast<size_t>(current)];
            if (current < 0)
            {
                return {};
            }
            path.push_back(toTile(current));
        }

        std::reverse(path.begin(), path.end());
        return path;
    }
}
