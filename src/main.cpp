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


template<typename T>
concept ColumnTypes = std::same_as<T, long long> || std::same_as<T, std::string>;

class Column {
public:
    using Storage = std::variant<std::vector<long long>, std::vector<std::string>>;

    explicit Column(Storage data) : data_(std::move(data)) {}

    std::size_t size() const {
        return std::visit([](auto & arg){
            return arg.size();
        }, data_);
    }

    template<ColumnTypes T>
    bool isType() const {
        if (std::holds_alternative<std::vector<T>>(data_)) {
            return true;
        } else {
            return false;
        }
    }

    Column fromIndexes(const std::vector<std::size_t> & indexes) const {
        Column new_col = std::visit([&](auto & arg){
            using ColT = std::decay_t<decltype(arg)>;
            ColT new_vec;
            new_vec.reserve(indexes.size());

            for (const auto i : indexes) {
                if (i > arg.size()) {
                    throw std::out_of_range("from index is out of range");
                }
                new_vec.emplace_back(arg[i]);
            }

            return Column{std::move(new_vec)};
        }, data_);
        
        return new_col;
    }

    template<ColumnTypes T>
    const std::vector<T> & raw() const { return std::get<std::vector<T>>(data_); }

private:
    Storage data_;
};


class DataFrame {
public:

    void AddColumn(const std::string name, Column && col) {
        if (rows == 0) {
            rows = col.size();
        } else if (col.size() != rows) {
            throw std::invalid_argument("wrong column size");
        }
        
        df_.insert({std::move(name), std::move(col)});
    } 

    template<ColumnTypes T>
    DataFrame Filter(std::function<bool(const T&)> func, const std::string& col_name) {
        DataFrame retval; // todo: create move ctor with print and test with it (move semantic)

        if (!df_.contains(col_name)) {
            return retval; // empty
        }

        const auto & col_vec = df_.at(col_name).raw<T>(); // thros if wrong type

        std::vector<std::size_t> rows_needed; // only one allocation for worst case - skip this is optimising for memory
        rows_needed.reserve(rows);

        for (std::size_t i = 0; i < rows; i++) {
            if (func(col_vec[i])) {
                rows_needed.push_back(i);
            }
        }

        // todo note: in prod, benchmark the versions
        for (const auto & it : df_) {
            Column new_col = it.second.fromIndexes(rows_needed);
            retval.AddColumn(it.first, std::move(new_col));
        }

        retval.rows = rows_needed.size();

        return retval;
    }
    
    // todo: remove this from dataframe, with 2 generics (T1, T2)
    template <ColumnTypes T>
    Column BinaryOp(std::function<T(const T&, const T&)> func, const std::string & colf_first_str, const std::string & col_second_str) {
        auto it_first = df_.find(colf_first_str);
        auto it_second = df_.find(col_second_str);

        if (it_first == df_.end() || it_second == df_.end()) {
            throw std::invalid_argument("invalid column name");
        }

        auto col_vec_first = it_first->second.raw<T>(); // throws on bad type
        auto col_vec_second = it_second->second.raw<T>(); // throws on bad type

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
                if (it.second.isType<long long>()) {
                    std::cout << it.second.raw<long long>()[i]  << " ";
                } else {
                    std::cout << it.second.raw<std::string>()[i]  << " ";
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

// created the dataframe from stdin, performs the filterings and binary operations on the column and calc the result
std::vector<long long> getFeature(long long price_min, long long price_max) {
    
    // for filtering BTC/USDT in the dataframe
    std::function<bool(const std::string&)> is_btc = [](const std::string& arg) {
        if (arg == std::string("BTC/USDT")) {
            return true;
        } else {
            return false;
        }
    };

    // fol filtering between min and max price in the dataframe
    std::function<bool(const long long&)> in_range = [&](const long long& arg) {
        if (arg >= price_min && arg <= price_max) {
            return true;
        } else {
            return false;
        }
    };

    // for subsracting 2 columns in the dataframe
    std::function<long long(const long long&, const long long&)> llsub = [](const long long & a, const long long & b){return a - b;};
    
    // for multipliening 2 coluns in the dataframe
    std::function<long long(const long long&, const long long&)> llmul = [](const long long & a, const long long & b){return a * b;};

    /// filling up the dataframe from stdin
    // 5 colum names comes first
    std::vector<std::string> col_names;
    for (int i = 0; i < 5; i++) {
        std::string cname;
        std::cin >> cname;
        col_names.push_back(cname);
    }

    // data comes in 6 rows, 1st col is string, other 4 is int
    std::vector<std::string> str_vec;
    std::vector<std::vector<long long>> int_vecs(4);
    for (int i = 0; i < 6; i++) {
        std::string str_val;
        std::cin >> str_val;
        str_vec.push_back(str_val);
        for (int j = 0; j < 4; j++) {
            long long val;
            std::cin >> val;
            int_vecs[j].push_back(val);
        }
    }

    std::cout << std::endl;

    DataFrame df;
    // 1st col is string
    df.AddColumn(col_names[0], Column{move(str_vec)});
    // 4 next cols is int
    for (int i = 0; i < 4; i++) {
        df.AddColumn(col_names[i+1], Column{move(int_vecs[i])});    
    }

    std::cout << "original df:" << std::endl;
    df.Dispaly();

    // filtering TICKER for BTC/USDT
    DataFrame df_btc = df.Filter<std::string>(is_btc, std::string("TICKER"));
    std::cout << "btc df:" << std::endl;
    df_btc.Dispaly();

    // filtering BID_PRICE between min and max price
    DataFrame df_btc_ranged = df_btc.Filter<long long>(in_range, std::string("BID_PRICE"));
    std::cout << "btc ranged df:" << std::endl;
    df_btc_ranged.Dispaly();

    // create 2 columns first with BID_PRICE * BID_VOLUME and ASK_VOLUME * ASK_PRICE
    // then put them into a dataframe to perform the subraction between them
    Column col_bid = df_btc_ranged.BinaryOp<long long>(llmul, std::string("BID_PRICE"), std::string("BID_VOLUME"));
    Column col_ask = df_btc_ranged.BinaryOp<long long>(llmul, std::string("ASK_VOLUME"), std::string("ASK_PRICE"));

    DataFrame df_bid_ask;
    df_bid_ask.AddColumn(std::string("BID"), std::move(col_bid));
    df_bid_ask.AddColumn(std::string("ASK"), std::move(col_ask));
    std::cout << "bid ask df:" << std::endl;
    df_bid_ask.Dispaly();

    Column feature = df_bid_ask.BinaryOp<long long>(llsub, std::string("BID"), std::string("ASK"));

    return feature.raw<long long>();
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