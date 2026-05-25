#pragma once

#include <string>
#include <vector>

class CSV {
private:
    std::vector<std::vector<std::string>> csv;
public:
    CSV(std::string filename);

    std::string read_string(size_t row, size_t column);
    double read_number(size_t row, size_t column);

    size_t row_count();
    size_t column_count();
};