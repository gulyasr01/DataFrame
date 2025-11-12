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

// old Column with variant
// enum class VariantError {
//     WrongType
// };

// class Column {
// public:
//     // todo: just for example the by value and copy constructors
//     //template<typename T>
//     //explicit Column (std::vector<T> data) : col_(std::move(data)) {} // just for example the by-value and copy constructor too
//     //template<typename T>
//     //explicit Column (const std::vector<T>& data) : col_(data) {}

//     // todo: make it work to only accecpt rvalue, btw is it recommended?
//     // template<typename T>
//     // requires (std::same_as<std::remove_cvref_t<T>, std::vector<long long>> || 
//     //           std::same_as<std::remove_cvref_t<T>, std::vector<std::string>>) 
//     // explicit Column (T&& data) noexcept : col_(std::move(data)) {std::cout << "this called" << std::endl;}

//     // only move constructor for performance reasons
//     // or factory with expected return?
//     // maybe copy ctors too
//     explicit Column(std::vector<long long> && data) noexcept : col_(std::move(data)) {}
//     explicit Column(std::vector<std::string> && data) noexcept : col_(std::move(data)) {}

//     constexpr bool isInt() noexcept {return std::holds_alternative<std::vector<long long>>(col_);}
//     constexpr bool isString() noexcept {return std::holds_alternative<std::vector<std::string>>(col_);} // maybe this is not necessary

//     constexpr size_t size() const noexcept {
//         return std::visit([](const auto &col) {return col.size();}, col_); // todo use col_.visit instead but g++ complains about c++23
//     }

//     // throws, maybe use expected instead
//     constexpr const std::vector<long long> & ints() {return std::get<std::vector<long long>>(col_);}
//     constexpr const std::vector<std::string> & strings() {return std::get<std::vector<std::string>>(col_);}

// private:
//     std::variant<std::vector<long long>, std::vector<std::string>> col_;
// };

template <typename T>
concept AllowedColumnTypes = std::same_as<T, long long> || std::same_as<T, std::string>;

template <AllowedColumnTypes>
class Column {
public:
    explicit Column(std::vector<T> data) : col_(std::move(data)) {} // todo: move only ctor for perf reasons? - would skip 2 cheap copies


private:
    std::vector<T> col_;
}

struct DataFrame
{
public:
    void AddColumn(std::string name, Column && col) {df_.insert({name, std::move(col)});} // todo add size check

    template<typename T>
    DataFrame Filter(std::function<bool(T)> func, const std::string col_name) {
        DataFrame retval;

        if df_.contains(col_name) {
            std::vector<unsigned int> rows_needed;
            rows_needed.reserve(rows); // only one allocation for worst case - skip this is optimising for memory


            for (int i = 0; i < rows; i++) {

            }

            std::vector<std::vector<T>> new_cols(df_.size(), )
        }

        return DataFrame;
    }
    
    // BinOps()
private:
    std::unordered_map<std::string, Column> df_;
    unsigned int rows;
};


std::vector<long long> getFeature(long long price_min, long long price_max) {

}

using namespace std; 
int main() {
    
    vector<long long> v{1, 2, 3};

    Column c(std::move(v));
    //Column c(v);

    cout << c.size() << endl;

    DataFrame df;
    df.AddColumn(string("sajt"), std::move(c));

    // int price_min, price_max;
    // cin >> price_min >> price_max;

    // auto feature_vector = getFeature(price_min, price_max);

    // for (auto &val : feature_vector) {
    //     cout << val << endl;
    // }

    return 0;
} 