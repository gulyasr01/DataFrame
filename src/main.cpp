#include <iostream>
#include <vector>
#include <variant>
#include <string>
#include <expected>
#include <concepts>
#include <type_traits>
#include <unordered_map>
#include <functional>
#include <concepts>


class Column {
public:
    using Storage = std::variant<std::vector<long long>, std::vector<std::string>>;

    explicit Column(Storage data) : data_(std::move(data)) {} // todo: explicit keyword needed?

    std::size_t size() const {
        return std::visit([](auto & arg){
            return arg.size();
        }, data_);
    }

    bool isInt() const {
        if (std::holds_alternative<std::vector<long long>>(data_)) {
            return true;
        } else {
            return false;
        }
    }

    const Storage & raw() const { return data_; }

private:
    Storage data_;
};


template<typename T>
concept ColumnTypes = std::same_as<T, long long> || std::same_as<T, std::string>;

class DataFrame {
public:

    void AddColumn(const std::string & name, Column && col) {
        if (rows == 0) {
            rows = col.size();
        } else if (col.size() != rows) {
            throw std::invalid_argument("wrong column size");
        }
        
        df_.insert({name, std::move(col)});
    } 

    template<ColumnTypes T>
    DataFrame Filter(std::function<bool(const T&)> func, const std::string& col_name) {
        DataFrame retval;

        if (!df_.contains(col_name)) {
            return retval; // empty
        }

        const auto & col_vec = std::get<std::vector<T>>(df_.at(col_name).raw()); // thros if wrong type

        std::vector<unsigned int> rows_needed; // only one allocation for worst case - skip this is optimising for memory
        rows_needed.reserve(rows);

        for (unsigned int i = 0; i < rows; i++) {
            if (func(col_vec[i])) {
                rows_needed.push_back(i);
            }
        }

        for (const auto &it : df_) {
            std::visit([&](auto & src_vec){
                using ColT = std::decay_t<decltype(src_vec)>;

                ColT new_vec;
                new_vec.reserve(rows_needed.size());

                for (const auto idx : rows_needed) {
                    new_vec.emplace_back(src_vec[idx]);
                }

                retval.AddColumn(it.first, Column{std::move(new_vec)});

            }, it.second.raw());
        }

        retval.rows = rows_needed.size();

        return retval;
    }
    
    template <ColumnTypes T>
    Column BinaryOp(std::function<T(const T&, const T&)> func, const std::string & colf_first_str, const std::string & col_second_str) {
        auto it_first = df_.find(colf_first_str);
        auto it_second = df_.find(col_second_str);

        if (it_first == df_.end() || it_second == df_.end()) {
            throw std::invalid_argument("invalid column name");
        }

        auto col_vec_first = std::get<std::vector<T>>(it_first->second.raw()); // throws on bad type
        auto col_vec_second = std::get<std::vector<T>>(it_second->second.raw()); // throws on bad type

        std::vector<T> vec_res;
        
        vec_res.reserve(col_vec_first.size());
        for (size_t i = 0; i < col_vec_first.size(); i++) {
            vec_res.emplace_back(func(col_vec_first[i], col_vec_second[i]));
        }

        return Column{std::move(vec_res)};
    }

    void Dispaly() {
        // print the col names
        for (const auto & it : df_) {
            std::cout << it.first << " ";
        }
        std::cout << std::endl;

        // print the values
        for (std::size_t i = 0; i < rows; i++) {
            for (const auto & it : df_) {
                if (it.second.isInt()) {
                    std::cout << std::get<std::vector<long long>>(it.second.raw())[i]  << " ";
                } else {
                    std::cout << std::get<std::vector<std::string>>(it.second.raw())[i]  << " ";
                }
                
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

private:
    std::unordered_map<std::string, Column> df_;
    std::size_t rows = 0;
};


std::vector<long long> getFeature(long long price_min, long long price_max) {
    
    std::function<bool(const std::string&)> is_btc = [](const std::string& arg) {
        if (arg == std::string("BTC/USDT")) {
            return true;
        } else {
            return false;
        }
    };

    std::function<bool(const long long&)> in_range = [&](const long long& arg) {
        if (arg >= price_min && arg <= price_max) {
            return true;
        } else {
            return false;
        }
    };

    std::function<long long(const long long&, const long long&)> llsum = [](const long long & a, const long long & b){return a + b;};
    std::function<long long(const long long&, const long long&)> llsub = [](const long long & a, const long long & b){return a - b;};
    std::function<long long(const long long&, const long long&)> llmul = [](const long long & a, const long long & b){return a * b;};

    // todo: refactor this df creation
    std::string col0, col1, col2, col3, col4;
    std::cin >> col0 >> col1 >> col2 >> col3 >> col4;

    std::vector<std::string> col0_vec;
    std::vector<long long> col1_vec, col2_vec, col3_vec, col4_vec;
    std::string col0_val;
    long long col1_val, col2_val, col3_val, col4_val;
    for (int i = 0; i < 6; i++) {
        std::cin >> col0_val >> col1_val >> col2_val >> col3_val >> col4_val;
        col0_vec.push_back(col0_val);
        col1_vec.push_back(col1_val);
        col2_vec.push_back(col2_val);
        col3_vec.push_back(col3_val);
        col4_vec.push_back(col4_val);
    }

    std::cout << std::endl;

    DataFrame df;
    df.AddColumn(col0, Column{move(col0_vec)});
    df.AddColumn(col1, Column{move(col1_vec)});
    df.AddColumn(col2, Column{move(col2_vec)});
    df.AddColumn(col3, Column{move(col3_vec)});
    df.AddColumn(col4, Column{move(col4_vec)});

    std::cout << "original df:" << std::endl;
    df.Dispaly();

    // todo: refactor the column name creations
    DataFrame df_btc = df.Filter(is_btc, std::string("TICKER"));
    std::cout << "btc df:" << std::endl;
    df_btc.Dispaly();

    DataFrame df_btc_ranged = df_btc.Filter(in_range, std::string("BID_PRICE"));
    std::cout << "btc ranged df:" << std::endl;
    df_btc_ranged.Dispaly();


    Column col_bid = df_btc_ranged.BinaryOp(llmul, std::string("BID_PRICE"), std::string("BID_VOLUME"));
    Column col_ask = df_btc_ranged.BinaryOp(llmul, std::string("ASK_VOLUME"), std::string("ASK_PRICE"));

    DataFrame df_bid_ask;
    df_bid_ask.AddColumn(std::string("BID"), std::move(col_bid));
    df_bid_ask.AddColumn(std::string("ASK"), std::move(col_ask));
    std::cout << "bid ask df:" << std::endl;
    df_bid_ask.Dispaly();

    Column feature = df_bid_ask.BinaryOp(llsub, std::string("BID"), std::string("ASK"));

    return std::get<std::vector<long long>>(feature.raw());
}


using namespace std; 
int main() {

    cout << "enter the inputs" << endl;

    int price_min, price_max;
    cin >> price_min >> price_max;

    auto feature_vector = getFeature(price_min, price_max);

    for (auto &val : feature_vector) {
        cout << val << endl;
    }

    return 0;
} 