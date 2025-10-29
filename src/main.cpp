#include <iostream>
#include <vector>
#include <variant>
#include <string>
#include <expected>

enum class VariantError {
    WrongType
};

template<typename T>
class Column {
public:
    explicit Column (const std::vector<T>& data) : col_(data) {}
    
    explicit Column (std::vector<T>&& data) noexcept : col_(std::move(data)) {}

    constexpr bool isInt() noexcept {return std::holds_alternative<std::vector<long long>>(col_);}

    std::expected<std::reference_wrapper<std::vector<T>>, VariantError> as() {
        if (auto p = std::get_if<std::vector<T>>(&col_)) {
            return std::ref(*p);
        }
        else {
            return std::unexpected(VariantError::WrongType);
        }
    }

private:
    std::variant<std::vector<long long>, std::vector<std::string>> col_;
};

struct DataFrame
{
    // Filter()
    
    // BinOps()
};


std::vector<long long> getFeature(long long price_min, long long price_max) {

}

using namespace std; 
int main() {
    
    Column<long long> c(vector<long long>{1, 2, 3});

    auto r = c.as();
    if (r.has_value()) {
        cout << "siker" << endl;
    }

    // int price_min, price_max;
    // cin >> price_min >> price_max;

    // auto feature_vector = getFeature(price_min, price_max);

    // for (auto &val : feature_vector) {
    //     cout << val << endl;
    // }

    return 0;
}