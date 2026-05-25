#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class Product {
public:
    std::string category;
    std::string sku;
    std::string name;
    double price;
    int quantity;

    Product(std::string cat, std::string sk, std::string nm, double pr, int q)
    : category(std::move(cat)), sku(std::move(sk)), name(std::move(nm)), price(pr), quantity(q) {}
};

struct Clothing : Product {
    std::string size;
    std::string color;

    Clothing(std::string sk, std::string nm, const std::string &size_, const std::string &color_, double pr, int q)
    : Product("Clothing", sk, nm, pr, q), size(size_), color(color_) {}
};

struct Electronics : Product {
    std::string model;
    std::string manufacturer;

    Electronics(std::string sk, std::string nm, const std::string &model_, const std::string &manuf_, double pr, int q)
    : Product("Electronics", sk, nm, pr, q), model(model_), manufacturer(manuf_) {}
};

struct Book : Product {
    std::string author;
    std::string publisher;

    Book(std::string sk, std::string nm, const std::string &author_, const std::string &pub_, double pr, int q)
    : Product("Book", sk, nm, pr, q), author(author_), publisher(pub_) {}
};

class Buyer {
public:
    std::string card;
    std::string name;
    double balance;

    std::vector<std::string> messages;

    Buyer(std::string card, std::string nm, double bal) : card(std::move(card)), name(move(nm)), balance(bal) {}
};

struct PurchaseItem {
    std::string sku;
    int quantity;

    PurchaseItem(std::string sk, int qty) : sku(std::move(sk)), quantity(qty) {}
};

class Purchase {
public:
    std::string order_id;
    std::string card_number;
    PurchaseItem item;

    Purchase(std::string id, std::string card, PurchaseItem itm) : order_id(std::move(id)), card_number(std::move(card)), item(itm) {}
};

class Marketplace {
private:
    std::unordered_map<std::string, std::unique_ptr<Product>> products;
    std::unordered_map<std::string, Buyer> buyers;
    std::vector<Purchase> purchases;

    double S = 100.0;
    double p = 0.1; 

    std::unordered_map<std::string,double> revenue_by_category;
    std::vector<std::string> waiting_list;
    std::vector<std::pair<std::string,int>> reserved_items;
    std::vector<std::pair<std::string,int>> sold_items;

    double calculate_order_raw_total(Purchase &purchase);
    std::pair<double, bool> apply_discount(double raw_sum);
public:
    Marketplace(std::string products_filename, std::string buyers_filename, std::string purchases_filename);
    
    void set_discount_threshold(double number);
    void set_discount(int number);

    void process_orders();
    void report();
};