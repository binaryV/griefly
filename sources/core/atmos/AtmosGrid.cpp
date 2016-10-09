#include "AtmosGrid.h"

void AtmosGrid::Process()
{
    Prepare(0);
    //Prepare(1);
    //Prepare(2);
    //Prepare(3);
    //Prepare(4);

    // TODO: thin coridors
    // TODO: weird behaviour on left depressurization

    Finalize();
}

const int DIRS_SIZE = 4;

inline void ProcessFiveCells(AtmosGrid::Cell* near_cells[])
{
    int near_size = 0;
    int gases_sums[GASES_NUM];
    for (int i = 0; i < GASES_NUM; ++i)
    {
        gases_sums[i] = 0;
    }
    int energy_sum = 0;

    for (int dir = 0; dir < DIRS_SIZE + 1; ++dir)
    {
        if (AtmosGrid::Cell* nearby = near_cells[dir])
        {
            for (int i = 0; i < GASES_NUM; ++i)
            {
                gases_sums[i] += nearby->data.gases[i];
            }
            energy_sum += nearby->data.energy;
            ++near_size;
        }
    }

    int gases_average[GASES_NUM];
    int gases_remains[GASES_NUM];
    for (int i = 0; i < GASES_NUM; ++i)
    {
        gases_average[i] = gases_sums[i] / near_size;
        gases_remains[i] = gases_sums[i] % near_size;
    }
    int energy_average = energy_sum / near_size;
    int energy_remains = energy_sum % near_size;

    for (int dir = DIRS_SIZE; dir >= 0; --dir)
    {
        if (near_cells[dir])
        {
            AtmosGrid::Cell& nearby = *near_cells[dir];
            for (int i = 0; i < GASES_NUM; ++i)
            {
                int diff = gases_average[i] - nearby.data.gases[i];
               /* if (gases_remains[i])
                {
                    diff += 1;
                    gases_remains[i] -= 1;
                }*/

                nearby.data.gases[i] += diff;
            }
            int diff = energy_average - nearby.data.energy;
           /* if (energy_remains)
            {
                diff += 1;
                energy_remains -= 1;
            }*/
            nearby.data.energy += diff;
        }
    }
    AtmosGrid::Cell& nearby = *near_cells[DIRS_SIZE];
    for (int i = 0; i < GASES_NUM; ++i)
    {
        nearby.data.gases[i] += gases_remains[i];
    }
    nearby.data.energy += energy_remains;
}

void AtmosGrid::Prepare(int stage)
{
    int pos = 0;

    // Left border line
    for (; pos < height_; ++pos)
    {
        Cell& current = cells_[pos];
        current.Truncate();
    }

    for (int line = 1; line < width_ - 1; ++line)
    {
        // Upper border line
        {
            Cell& current = cells_[pos];
            current.Truncate();
        }
        ++pos;
        for (int column = 1; column < height_ - 1; ++column, ++pos)
        {

            Cell& current = cells_[pos];

            if (!current.IsPassable(Cell::CENTER))
            {
                // TODO: if something still here
                continue;
            }

            if (((column + (line * 2)) % 5) == ((MAIN_TICK + stage) % 5))
            {
                continue;
            }

            const IAtmosphere::Flags DIRS[DIRS_SIZE]
                = { Cell::LEFT, Cell::UP, Cell::DOWN, Cell::RIGHT };
            const IAtmosphere::Flags REVERT_DIRS[DIRS_SIZE]
                = { Cell::RIGHT, Cell::DOWN, Cell::UP, Cell::LEFT };

            Cell* near_cells[DIRS_SIZE + 1];

            for (int dir = 0; dir < DIRS_SIZE; ++dir)
            {
                if (current.IsPassable(DIRS[dir]))
                {
                    Cell& nearby = Get(pos, DIRS[dir]);
                    if (   nearby.IsPassable(REVERT_DIRS[dir])
                        && nearby.IsPassable(Cell::CENTER))
                    {
                        near_cells[dir] = &nearby;
                        continue;
                    }
                }
                near_cells[dir] = nullptr;
            }
            near_cells[DIRS_SIZE] = &current;

            ProcessFiveCells(near_cells);
        }
        // Lower border line
        {
            Cell& current = cells_[pos];
            current.Truncate();
        }
        ++pos;
    }

    // Right border line
    for (int i = 0; i < height_; ++i, ++pos)
    {
        Cell& current = cells_[pos];
        current.Truncate();
    }
}

void AtmosGrid::Finalize()
{
    for (int pos = 0; pos < length_; ++pos)
    {
        Cell& cell = cells_[pos];

        if (cell.flags & AtmosGrid::Cell::SPACE)
        {
            for (int i = 0; i < GASES_NUM; ++i)
            {
                cell.data.gases[i] *= 4;
                cell.data.gases[i] /= 5;
            }
            cell.data.energy *= 4;
            cell.data.energy /= 5;
        }

        UpdateMacroParams(&cell.data);
    }
}