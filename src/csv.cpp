#include "csv.h"
#include <fstream>
#include <format>
#include <iostream>

using namespace std;

CSV::CSV(string filename)
{
    ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Не мог открыть файл " + filename + ": " + strerror(errno));
    }

    string line;
    while (getline(file, line))
    {
        size_t start = 0;
        vector<string> cells;
        while (true)
        {
            size_t pos = line.find(",", start);
            if (pos == std::string::npos)
            {
                cells.emplace_back(line.substr(start));
                break;
            }
            cells.emplace_back(line.substr(start, pos - start));
            start = pos + 1;
        }

        if (csv.size() > 0 && cells.size() != csv[0].size())
        {
            throw std::runtime_error(format("Файл не является действительной CSV таблицей: столбцов должно быть {}, но на линии {} их {}", csv[0].size(), csv.size() + 1, cells.size()));
        }

        csv.push_back(cells);
    }
}

string CSV::read_string(size_t row, size_t column)
{
    string cell = csv[row][column];
    size_t a = 0;
    while (a < cell.size() && isspace((unsigned char)cell[a]))
        a++;
    size_t b = cell.size();
    while (b > a && isspace((unsigned char)cell[b - 1]))
        b--;
    return cell.substr(a, b - a);
}

double CSV::read_number(size_t row, size_t column)
{
    string cell = this->read_string(row, column);
    try
    {
        try
        {
            return stod(cell);
        }
        catch (exception &e)
        {
            return stoi(cell);
        }
    }
    catch (exception &e)
    {
        printf("%s\n", cell.c_str());
        throw runtime_error(format("Не смог получить значение строки {} ячейки {}: {}", row, column, e.what()));
    }
}

size_t CSV::row_count()
{
    return csv.size();
}

size_t CSV::column_count()
{
    return csv.size() > 0 ? csv[0].size() : 0;
}