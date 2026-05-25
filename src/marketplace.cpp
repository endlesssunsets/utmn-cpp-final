#include "marketplace.h"
#include "csv.h"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

Marketplace::Marketplace(string products_filename, string buyers_filename, string purchases_filename)
{
    CSV products_csv(products_filename);
    CSV buyers_csv(buyers_filename);
    CSV purchases_csv(purchases_filename);

    // Загрузка products.csv
    if (products_csv.column_count() != 7)
    {
        throw std::runtime_error("Неправильно построен products.csv: столбцов должно быть 7");
    }
    for (int i = 0; i < products_csv.row_count(); i++)
    {
        string category = products_csv.read_string(i, 0);
        string sku = products_csv.read_string(i, 1);
        if (category == "Clothing")
        {
            string name = products_csv.read_string(i, 2);
            string size = products_csv.read_string(i, 3);
            string color = products_csv.read_string(i, 4);
            double price = products_csv.read_number(i, 5);
            double quantity = products_csv.read_number(i, 6);
            products[sku] = make_unique<Clothing>(sku, name, size, color, price, (int)quantity);
        }
        else if (category == "Electronics")
        {
            string name = products_csv.read_string(i, 2);
            string model = products_csv.read_string(i, 3);
            string manufacturer = products_csv.read_string(i, 4);
            double price = products_csv.read_number(i, 5);
            double quantity = products_csv.read_number(i, 6);
            products[sku] = make_unique<Electronics>(sku, name, model, manufacturer, price, quantity);
        }
        else if (category == "Book")
        {
            string author = products_csv.read_string(i, 2);
            string name = products_csv.read_string(i, 3);
            double price = products_csv.read_number(i, 4);
            string publisher = products_csv.read_string(i, 5);
            double quantity = products_csv.read_number(i, 6);
            products[sku] = make_unique<Book>(sku, name, author, publisher, price, quantity);
        }
        else
        {
            throw runtime_error("Неверная категория товара");
        }
    }

    // Загрузка buyers.csv
    if (buyers_csv.column_count() != 3)
    {
        throw std::runtime_error("Неправильно построен buyers.csv: столбцов должно быть 3");
    }
    for (int i = 0; i < buyers_csv.row_count(); i++)
    {
        string name = buyers_csv.read_string(i, 0);
        string card = buyers_csv.read_string(i, 1);
        double balance = buyers_csv.read_number(i, 2);
        buyers.emplace(card, Buyer(card, name, balance));
    }

    if (purchases_csv.column_count() != 4)
    {
        throw runtime_error("Неправильно построен purchases.csv: столбцов должно быть 4");
    }
    for (int i = 0; i < purchases_csv.row_count(); i++)
    {
        string id = purchases_csv.read_string(i, 0);
        string card = purchases_csv.read_string(i, 1);
        string sku = purchases_csv.read_string(i, 2);
        int quantity = purchases_csv.read_number(i, 3);
        purchases.push_back(Purchase(id, card, PurchaseItem(sku, quantity)));
    }
}

void Marketplace::set_discount_threshold(double number)
{
    this->S = number;
}

void Marketplace::set_discount(int number)
{
    this->p = number / 100.0;
}

double Marketplace::calculate_order_raw_total(Purchase &purchase)
{
    double total = 0.0;
    auto pit = products.find(purchase.item.sku);
    if (pit != products.end())
    {
        total += pit->second->price * purchase.item.quantity;
    }
    return total;
}

pair<double, bool> Marketplace::apply_discount(double raw_sum)
{
    if (raw_sum > S)
    {
        return {raw_sum * (1.0 - p), true};
    }
    else
        return {raw_sum, false};
}

void Marketplace::process_orders()
{
    for (auto &purchase : purchases)
    {
        auto itb = buyers.find(purchase.card_number);
        if (itb == buyers.end())
        {
            continue;
        }
        Buyer &buyer = itb->second;

        PurchaseItem it = purchase.item;
        auto pit = products.find(it.sku);
        if (pit == products.end())
        {
            buyer.messages.push_back("Заказ " + purchase.order_id + ": продукт " + it.sku + " не был найден.");
            continue;
        }

        if (pit->second->quantity < it.quantity)
        {
            buyer.messages.push_back("Заказ " + purchase.order_id + ": нет в наличии требуемого продукта " + it.sku + ".");
            continue;
        }

        double raw_total = calculate_order_raw_total(purchase);
        auto [discounted_total, discount_applied] = apply_discount(raw_total);

        if (buyer.balance + 1e-9 >= discounted_total)
        {
            buyer.balance -= discounted_total;
            auto &p = pit->second;
            double line = p->price * it.quantity;
            revenue_by_category[p->category] += line;
            p->quantity -= it.quantity;
            sold_items.push_back({it.sku, it.quantity});
            string msg = "Заказ " + purchase.order_id + ": выполнен. За него снято " + to_string(discounted_total);
            if (discount_applied)
                msg += " (скидка применена)";
            buyer.messages.push_back(msg);
            continue;
        }

        double raw_sum = raw_total;
        auto [disc_total, _ap] = apply_discount(raw_sum);
        if (disc_total <= buyer.balance + 1e-9)
        {
            auto &p = pit->second;
            if (p->quantity >= it.quantity)
            {
                p->quantity -= it.quantity;
                reserved_items.push_back({it.sku, it.quantity});
            }
            else
            {
                buyer.messages.push_back("Заказ " + purchase.order_id + ": при резервации продукт " + it.sku + " стал недоступным и был пропущен.");
                continue;
            }
            {
                ostringstream oss;
                oss << "Заказ " << purchase.order_id << ": рекомендована оптимальная корзина стоимостью " << fixed << setprecision(2) << disc_total << (_ap ? " (применена скидка)" : "");
                oss << " Продукт: " << it.sku << " x" << it.quantity << "";
                buyer.messages.push_back(oss.str());
                waiting_list.push_back(buyer.card);
            }
            continue;
        }
        else
        {
            buyer.messages.push_back("Заказ " + purchase.order_id + ": нехватает средств. Заказ добавлен в список ожидания.");
            waiting_list.push_back(buyer.card);
            continue;
        }
    }

    for (auto &kv : buyers)
    {
        if (!kv.second.messages.empty())
        {
            kv.second.messages.push_back(string("Повторное: ") + kv.second.messages.back());
        }
    }
}

void Marketplace::report()
{
    cout << endl
         << endl
         << "ОТЧЁТ ТОРГОВОЙ ПЛОЩАДКИ" << endl
         << endl;
    cout << "Выручка по категориям:" << endl;
    double max_rev = -1.0;
    string best_cat = "";
    for (auto &kv : revenue_by_category)
    {
        cout << "  " << kv.first << ": " << fixed << setprecision(2) << kv.second << "\n";
        if (kv.second > max_rev)
        {
            max_rev = kv.second;
            best_cat = kv.first;
        }
    }
    if (max_rev < 0.0)
        cout << "(нет продаж)\n";
    cout << " Категория с наибольшей выручкой: " << best_cat << " (" << (max_rev < 0 ? 0.0 : max_rev) << ")" << endl;

    cout << "Покупатели с сообщениями:" << endl;
    for (auto &kv : buyers)
    {
        if (!kv.second.messages.empty())
        {
            cout << "  " << kv.first << " (" << kv.second.name << "):\n";
            for (auto &m : kv.second.messages)
                cout << "   - " << m << "\n";
        }
    }

    cout << "Лист ожидания:" << endl;
    for (auto &c : waiting_list)
    {
        auto it = buyers.find(c);
        if (it != buyers.end())
            cout << " " << c << " (" << it->second.name << ")" << endl;
        else
            cout << " " << c << "\n";
    }

    cout << "Зарезервированные товары (sku x qty):" << endl;
    for (auto &r : reserved_items)
        cout << " " << r.first << " x" << r.second << "\n";

    cout << "Проданные (выполненные) позиции:" << endl;
    for (auto &s : sold_items)
        cout << " " << s.first << " x" << s.second << "\n";

    cout << "Остатки на складе:" << endl;
    for (auto &p : products)
    {
        cout << "  " << p.first << " (" << p.second->name << ") x" << p.second->quantity << "\n";
    }
}