#include <string>
#include <format>
#include <string_view>
#include <iostream>
#include "marketplace.h"

using namespace std;

std::string text_input(string text)
{
    while (true)
    {
        cout << text << endl;
        string text;
        getline(cin, text);
        if (text.empty())
            continue;
        try
        {
            return text;
        }
        catch (exception &e)
        {
        }
    }
}

double number_input(string text)
{
    while (true)
    {
        cout << text << endl;
        string text;
        getline(cin, text);
        if (text.empty())
            continue;
        try
        {
            return stod(text);
        }
        catch (exception &e)
        {
        }
    }
}

int main()
{
    setlocale(LC_CTYPE, "");

    Marketplace *marketplace = nullptr;

    std::string products = text_input("Пожалуйста, введите путь к products.csv");
    std::string buyers = text_input("Пожалуйста, введите путь к buyers.csv");
    std::string purchases = text_input("Пожалуйста, введите путь к purchases.csv");

    try
    {
        marketplace = new Marketplace(products, buyers, purchases);
    }
    catch (exception &e)
    {
        cout << "Ошибка: " << e.what() << endl;
        return 1;
    }

    marketplace->set_discount_threshold(number_input("Пожалуйста, введите сумму заказа с которой должна быть скидка"));
    marketplace->set_discount(number_input("Пожалуйста, введите размер скидки"));

    marketplace->process_orders();
    marketplace->report();

    return 0;
}