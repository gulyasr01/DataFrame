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

private:
    std::unordered_map<std::string, Column> df_;
    std::size_t rows = 0;
};


// std::vector<long long> getFeature(long long price_min, long long price_max) {

// }

bool gt2f (const long long & in) {
    if (in > 2)
        return true; 
    else
        return false;
}

long long llsumf(const long long & a, const long long & b) {
    return a + b;
}

using namespace std; 
int main() {
    
    vector<long long> v{1, 2, 3};
    vector<long long> v2{4, 5, 6};

    Column c(std::move(v));
    Column c2(std::move(v2));
    //Column c(v);

    cout << get<0>(c.raw())[0] << endl;

    DataFrame df;
    df.AddColumn(string("sajt"), std::move(c));
    df.AddColumn(string("kenyer"), std::move(c2));

    function<bool(const long long&)> gt2 =gt2f;
    function<long long(const long long&, const long long&)> llsum = llsumf;

    DataFrame df2 = df.Filter(gt2, string("sajt"));

    Column cr = df.BinaryOp(llsum, string("sajt"), string("kenyer"));

    cout << get<0>(cr.raw())[0] << endl;

    // int price_min, price_max;
    // cin >> price_min >> price_max;

    // auto feature_vector = getFeature(price_min, price_max);

    // for (auto &val : feature_vector) {
    //     cout << val << endl;
    // }

    return 0;
} 