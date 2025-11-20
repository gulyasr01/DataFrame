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


using IntType = long long;
using StrType = std::string;

template<typename T>
concept ColumnTypes = std::same_as<T, IntType> || std::same_as<T, StrType>;

class Column {
public:
    using Storage = std::variant<std::vector<IntType>, std::vector<StrType>>;

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

    template<ColumnTypes T1, ColumnTypes T2>
    Column BinaryOp(std::function<T1(const T1&, const T2&)> func, const Column & col) const {
        const auto & a = std::get<std::vector<T1>>(data_);
        const auto & b = col.raw<T2>();

        if (a.size() != b.size()) throw std::invalid_argument("different column sizes");

        std::vector<T1> vec_res;
        
        vec_res.reserve(a.size());
        for (size_t i = 0; i < a.size(); i++) {
            vec_res.emplace_back(func(a[i], b[i]));
        }

        return Column{std::move(vec_res)};
    }

    template<ColumnTypes T>
    const std::vector<T> & raw() const { return std::get<std::vector<T>>(data_); }

private:
    Storage data_;
};


class DataFrame {
public:

    void AddColumn(const StrType name, Column && col) {
        if (rows == 0) {
            rows = col.size();
        } else if (col.size() != rows) {
            throw std::invalid_argument("wrong column size");
        }
        
        df_.insert({std::move(name), std::move(col)});
    } 

    template<ColumnTypes T>
    DataFrame Filter(std::function<bool(const T&)> func, const StrType& col_name) {
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

    const Column & getColumn (const StrType & col_name) const {
        return df_.at(col_name);
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
                if (it.second.isType<IntType>()) {
                    std::cout << it.second.raw<IntType>()[i]  << " ";
                } else {
                    std::cout << it.second.raw<StrType>()[i]  << " ";
                }
                
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

private:
    std::unordered_map<StrType, Column> df_;
    std::size_t rows = 0;
};

// created the dataframe from stdin, performs the filterings and binary operations on the column and calc the result
std::vector<IntType> getFeature(IntType price_min, IntType price_max) {
    
    // for filtering BTC/USDT in the dataframe
    std::function<bool(const StrType&)> is_btc = [](const StrType& arg) {
        if (arg == StrType("BTC/USDT")) {
            return true;
        } else {
            return false;
        }
    };

    // fol filtering between min and max price in the dataframe
    std::function<bool(const IntType&)> in_range = [&](const IntType& arg) {
        if (arg >= price_min && arg <= price_max) {
            return true;
        } else {
            return false;
        }
    };

    // for subsracting 2 columns in the dataframe
    std::function<IntType(const IntType&, const IntType&)> llsub = [](const IntType & a, const IntType & b){return a - b;};
    
    // for multipliening 2 coluns in the dataframe
    std::function<IntType(const IntType&, const IntType&)> llmul = [](const IntType & a, const IntType & b){return a * b;};

    /// filling up the dataframe from stdin
    // 5 colum names comes first
    std::vector<StrType> col_names;
    for (int i = 0; i < 5; i++) {
        StrType cname;
        std::cin >> cname;
        col_names.push_back(cname);
    }

    // data comes in 6 rows, 1st col is string, other 4 is int
    std::vector<StrType> str_vec;
    std::vector<std::vector<IntType>> int_vecs(4);
    for (int i = 0; i < 6; i++) {
        StrType str_val;
        std::cin >> str_val;
        str_vec.push_back(str_val);
        for (int j = 0; j < 4; j++) {
            IntType val;
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
    DataFrame df_btc = df.Filter<StrType>(is_btc, StrType("TICKER"));
    std::cout << "btc df:" << std::endl;
    df_btc.Dispaly();

    // filtering BID_PRICE between min and max price
    DataFrame df_btc_ranged = df_btc.Filter<IntType>(in_range, StrType("BID_PRICE"));
    std::cout << "btc ranged df:" << std::endl;
    df_btc_ranged.Dispaly();

    // create 2 columns first with BID_PRICE * BID_VOLUME and ASK_VOLUME * ASK_PRICE
    // then put them into a dataframe to perform the subraction between them
    Column col_bid = df_btc_ranged.getColumn(StrType("BID_PRICE")).BinaryOp<IntType, IntType>(llmul, df_btc_ranged.getColumn(StrType("BID_VOLUME")));
    Column col_ask = df_btc_ranged.getColumn(StrType("ASK_VOLUME")).BinaryOp<IntType, IntType>(llmul, df_btc_ranged.getColumn(StrType("ASK_PRICE")));

    Column feature = col_bid.BinaryOp<IntType, IntType>(llsub, col_ask);
    return feature.raw<IntType>();
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